/* engine.c -- exact conflict-set repair engine for Ramsey colourings (+1 vertex).
 *
 * Matrix convention: '1' = RED unless s1b (stored-1-is-blue) = 1, which is the
 * convention of the six .scratch/exoo/* files.  Files written by this program
 * and by twin.c always have '1' = RED.
 *
 * State: the colouring, the exact set of conflicts (red K_R, blue K_S) and,
 * per edge, the list of conflicts through it.  Flipping uv from colour c to c':
 *   removes every live c-conflict through uv,
 *   creates one c'-conflict per (K'-2)-clique inside the common c'-neighbourhood
 *   of u and v (K' = clique size of colour c').
 * Both sides are exact; `testflip` checks the predicted change against the
 * post-flip enumeration, and .scratch/rs/count checks a written matrix
 * independently (cross-checked: agreed on every trial).
 *
 * Objective O = wR * (#red conflicts) + #blue conflicts, default Cremer weight
 * wR = C(S,2)/C(R,2).
 *
 * modes:
 *  engine F R S s1b delta                       one-move landscape of all conflict edges
 *  engine F R S s1b testflip u v                exactness self-test
 *  engine F R S s1b desc   maxsteps zero pref   deterministic steepest descent
 *  engine F R S s1b corrupt permille seed out   flip permille/1000 of all edges at random
 *  engine F R S s1b run steps cand seed maxtime stallkick pref [wR]
 *                                               tabu + Metropolis + kicks
 * s1b = 1 when the file's stored 1s are BLUE (.scratch/exoo/*), else 0.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define MAXV 420
#define MW 7
#define MAXK 16

static int N, W, R, S;
static uint64_t ADJ[2][MAXV][MW];
static inline int has(int c, int u, int v) { return (ADJ[c][u][v >> 6] >> (v & 63)) & 1; }
static inline int ecol(int u, int v) { return has(0, u, v) ? 0 : 1; }   /* 0 red, 1 blue */
static inline int eidx(int u, int v) { if (u > v) { int t = u; u = v; v = t; } return u * N + v; }

typedef struct { int k; int v[MAXK]; int dead; } Conf;
static Conf *cf; static int ncf, capcf;
static int *liveids, *posof, nlids, caplids;   /* live conflict ids, for uniform sampling */
static int **el; static int *eln, *elcap;
static long long nred, nblue;

static void push_conf(const int *v, int k) {
    if (ncf == capcf) {
        capcf = capcf ? capcf * 2 : 4096;
        cf = realloc(cf, sizeof(Conf) * capcf);
        posof = realloc(posof, sizeof(int) * capcf);
    }
    cf[ncf].k = k; cf[ncf].dead = 0; memcpy(cf[ncf].v, v, sizeof(int) * k);
    if (nlids == caplids) { caplids = caplids ? caplids * 2 : 4096; liveids = realloc(liveids, sizeof(int) * caplids); }
    posof[ncf] = nlids; liveids[nlids++] = ncf;
    ncf++;
}
static inline void kill_conf(int id) {
    cf[id].dead = 1;
    int p = posof[id], last = liveids[--nlids];
    liveids[p] = last; posof[last] = p;
}
static void el_add(int e, int id) {
    if (eln[e] == elcap[e]) { elcap[e] = elcap[e] ? elcap[e] * 2 : 8; el[e] = realloc(el[e], sizeof(int) * elcap[e]); }
    el[e][eln[e]++] = id;
}
static void rebuild_el(void) {
    for (int e = 0; e < N * N; e++) eln[e] = 0;
    nred = nblue = 0;
    for (int i = 0; i < ncf; i++) {
        if (cf[i].dead) continue;
        if (ecol(cf[i].v[0], cf[i].v[1]) == 0) nred++; else nblue++;
        for (int a = 0; a < cf[i].k; a++)
            for (int b = a + 1; b < cf[i].k; b++) el_add(eidx(cf[i].v[a], cf[i].v[b]), i);
    }
}
static long long live(void) { return nred + nblue; }
static void gc(void) {          /* drop dead conflicts, rebuild edge lists + liveids */
    int j = 0;
    for (int i = 0; i < ncf; i++) if (!cf[i].dead) { if (j != i) cf[j] = cf[i]; j++; }
    ncf = j; nlids = j;
    for (int i = 0; i < ncf; i++) { liveids[i] = i; posof[i] = i; cf[i].dead = 0; }
    rebuild_el();
}

/* ---------- clique counting ---------- */
static long long count_rec(int c, int k, int depth, uint64_t *P) {
    if (depth == k) return 1;
    long long tot = 0;
    uint64_t Q[MW]; memcpy(Q, P, sizeof(uint64_t) * W);
    for (;;) {
        int avail = 0;
        for (int w = 0; w < W; w++) avail += __builtin_popcountll(Q[w]);
        if (avail < k - depth) return tot;
        int v = -1;
        for (int w = 0; w < W; w++) if (Q[w]) { v = w * 64 + __builtin_ctzll(Q[w]); break; }
        uint64_t P2[MW];
        for (int w = 0; w < W; w++) P2[w] = Q[w] & ADJ[c][v][w];
        tot += count_rec(c, k, depth + 1, P2);
        Q[v >> 6] &= ~(1ULL << (v & 63));
    }
}
static int nfix, fixv[4], cur[16];
static void push_rec(int c, int k, int depth, uint64_t *P) {
    if (depth == k) {
        int tmp[16];
        for (int i = 0; i < nfix; i++) tmp[i] = fixv[i];
        for (int i = 0; i < k; i++) tmp[nfix + i] = cur[i];
        push_conf(tmp, nfix + k);
        return;
    }
    uint64_t Q[MW]; memcpy(Q, P, sizeof(uint64_t) * W);
    for (;;) {
        int avail = 0;
        for (int w = 0; w < W; w++) avail += __builtin_popcountll(Q[w]);
        if (avail < k - depth) return;
        int v = -1;
        for (int w = 0; w < W; w++) if (Q[w]) { v = w * 64 + __builtin_ctzll(Q[w]); break; }
        uint64_t P2[MW];
        for (int w = 0; w < W; w++) P2[w] = Q[w] & ADJ[c][v][w];
        cur[depth] = v;
        push_rec(c, k, depth + 1, P2);
        Q[v >> 6] &= ~(1ULL << (v & 63));
    }
}
static long long enum_all(int c, int k) {
    uint64_t P[MW];
    for (int w = 0; w < MW; w++) P[w] = ~0ULL;
    for (int j = N; j < W * 64; j++) P[j >> 6] &= ~(1ULL << (j & 63));
    long long before = ncf; nfix = 0;
    push_rec(c, k, 0, P);
    return ncf - before;
}
static long long capval = 128;   /* creation counts are capped at this for RANKING only */
static long long count_rec_cap(int c, int k, int depth, uint64_t *P, long long lim) {
    if (depth == k) return 1;
    long long tot = 0;
    uint64_t Q[MW]; memcpy(Q, P, sizeof(uint64_t) * W);
    for (;;) {
        int avail = 0;
        for (int w = 0; w < W; w++) avail += __builtin_popcountll(Q[w]);
        if (avail < k - depth) return tot;
        int v = -1;
        for (int w = 0; w < W; w++) if (Q[w]) { v = w * 64 + __builtin_ctzll(Q[w]); break; }
        uint64_t P2[MW];
        for (int w = 0; w < W; w++) P2[w] = Q[w] & ADJ[c][v][w];
        tot += count_rec_cap(c, k, depth + 1, P2, lim);
        if (tot > lim) return tot;
        Q[v >> 6] &= ~(1ULL << (v & 63));
    }
}
static long long creation_of_capped(int u, int v, int *pcolour) {
    int oc = ecol(u, v), nc = 1 - oc;
    *pcolour = nc;
    int K = (nc == 0) ? R : S, k = K - 2;
    uint64_t M[MW];
    for (int w = 0; w < W; w++) M[w] = ADJ[nc][u][w] & ADJ[nc][v][w];
    if (k <= 0) { int any = 0; for (int w = 0; w < W; w++) any |= (M[w] != 0); return any ? 1 : 0; }
    return count_rec_cap(nc, k, 0, M, capval);
}
static long long creation_of(int u, int v, int *pcolour) {
    int oc = ecol(u, v), nc = 1 - oc;
    *pcolour = nc;
    int K = (nc == 0) ? R : S, k = K - 2;
    uint64_t M[MW];
    for (int w = 0; w < W; w++) M[w] = ADJ[nc][u][w] & ADJ[nc][v][w];
    if (k <= 0) { int any = 0; for (int w = 0; w < W; w++) any |= (M[w] != 0); return any ? 1 : 0; }
    return count_rec(nc, k, 0, M);
}
static long removal_of(int u, int v) {
    int e = eidx(u, v), oc = ecol(u, v);
    long r = 0;
    for (int i = 0; i < eln[e]; i++) { int id = el[e][i]; if (!cf[id].dead && ecol(cf[id].v[0], cf[id].v[1]) == oc) r++; }
    return r;
}
static void do_flip(int u, int v) {
    int oc = ecol(u, v), nc = 1 - oc;
    int e = eidx(u, v);
    for (int i = 0; i < eln[e]; i++) {
        int id = el[e][i];
        if (!cf[id].dead && ecol(cf[id].v[0], cf[id].v[1]) == oc) {
            kill_conf(id);
            if (oc == 0) nred--; else nblue--;
        }
    }
    ADJ[oc][u][v >> 6] &= ~(1ULL << (v & 63));
    ADJ[oc][v][u >> 6] &= ~(1ULL << (u & 63));
    ADJ[nc][u][v >> 6] |= 1ULL << (v & 63);
    ADJ[nc][v][u >> 6] |= 1ULL << (u & 63);
    int K = (nc == 0) ? R : S, k = K - 2;
    uint64_t M[MW];
    for (int w = 0; w < W; w++) M[w] = ADJ[nc][u][w] & ADJ[nc][v][w];
    long long before = ncf;
    fixv[0] = u; fixv[1] = v; nfix = 2;
    if (k <= 0) {
        int any = 0;
        for (int w = 0; w < W; w++) any |= (M[w] != 0);
        if (any) push_rec(nc, 0, 0, M);
    } else push_rec(nc, k, 0, M);
    for (long long i = before; i < ncf; i++) {
        if (nc == 0) nred++; else nblue++;
        for (int a = 0; a < cf[i].k; a++)
            for (int b = a + 1; b < cf[i].k; b++) el_add(eidx(cf[i].v[a], cf[i].v[b]), (int)i);
    }
}

static double tnow(void) { return (double)clock() / CLOCKS_PER_SEC; }
static double t0;
static int load(const char *path, int s1b) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return 0; }
    static char line[8192];
    if (!fgets(line, sizeof line, f)) return 0;
    N = strcspn(line, "\r\n"); W = (N + 63) / 64;
    if (N > MAXV) { fprintf(stderr, "n=%d > MAXV\n", N); return 0; }
    rewind(f);
    int i = 0;
    while (fgets(line, sizeof line, f)) {
        if ((int)strcspn(line, "\r\n") < N) continue;
        for (int j = 0; j < N; j++) if (line[j] == '1') ADJ[1][i][j >> 6] |= 1ULL << (j & 63);
        i++;
    }
    fclose(f);
    if (i != N) { fprintf(stderr, "rows %d != n %d\n", i, N); return 0; }
    for (int a = 0; a < N; a++)
        for (int b = 0; b < N; b++)
            if (a != b && !((ADJ[1][a][b >> 6] >> (b & 63)) & 1)) ADJ[0][a][b >> 6] |= 1ULL << (b & 63);
    if (!s1b) { uint64_t tmp[MW]; for (int a = 0; a < N; a++) { memcpy(tmp, ADJ[0][a], sizeof tmp); memcpy(ADJ[0][a], ADJ[1][a], sizeof tmp); memcpy(ADJ[1][a], tmp, sizeof tmp); } }
    el = calloc((size_t)N * N, sizeof(int *)); eln = calloc((size_t)N * N, sizeof(int)); elcap = calloc((size_t)N * N, sizeof(int));
    return 1;
}
static void write_matrix(const char *path) {
    FILE *o = fopen(path, "w");
    for (int a = 0; a < N; a++) { for (int b = 0; b < N; b++) fputc(has(0, a, b) ? '1' : '0', o); fputc('\n', o); }
    fclose(o);
}
static unsigned long long rs = 88172645463325252ULL;
static unsigned long long rnd(void);
static uint64_t bestADJ[2][MAXV][MW];
static void save_best(void) { memcpy(bestADJ, ADJ, sizeof bestADJ); }
static void restore_best(void) {
    memcpy(ADJ, bestADJ, sizeof ADJ);
    ncf = 0; nlids = 0;
    enum_all(0, R); enum_all(1, S);
    rebuild_el();
}
/* patch: randomise the colours of all pairs inside a small vertex set around a
   conflicted vertex (Cremer's mutation step, simplified) */
static void patch(int strength) {
    if (nlids == 0) return;
    int id = liveids[(int)(rnd() % (unsigned long long)nlids)];
    int x = cf[id].v[(int)(rnd() % (unsigned long long)cf[id].k)];
    int Sset[16], ns = 0;
    Sset[ns++] = x;
    for (int tries = 0; tries < 200 && ns < strength; tries++) {
        int c = (int)(rnd() & 1);
        int v = -1;
        for (int w = 0; w < W && v < 0; w++) {
            uint64_t bits = ADJ[c][x][w];
            if (bits) { int pick = (int)(rnd() % (unsigned long long)__builtin_popcountll(bits)); while (pick--) bits &= bits - 1; v = w * 64 + __builtin_ctzll(bits); }
        }
        if (v < 0) continue;
        int dup = 0;
        for (int t = 0; t < ns; t++) if (Sset[t] == v) { dup = 1; break; }
        if (!dup) Sset[ns++] = v;
    }
    for (int a = 0; a < ns; a++) for (int b = a + 1; b < ns; b++)
        if (rnd() & 1) do_flip(Sset[a], Sset[b]);
    gc();
}
static unsigned long long rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
static double urand(void) { return (double)(rnd() >> 11) / 9007199254740992.0; }

int main(int argc, char **argv) {
    if (argc < 6) { fprintf(stderr, "usage: %s file R S s1b mode ...\n", argv[0]); return 1; }
    R = atoi(argv[2]); S = atoi(argv[3]);
    if (!load(argv[1], atoi(argv[4]))) return 1;
    t0 = tnow();
    const char *mode = argv[5];

    if (strcmp(mode, "corrupt") == 0) {
        int permille = atoi(argv[6]);
        rs = strtoull(argv[7], 0, 10) * 2654435761ULL + 12345; if (!rs) rs = 7;
        int tot = N * (N - 1) / 2, want = (int)((long long)tot * permille / 1000), did = 0;
        while (did < want) {
            int u = (int)(rnd() % (unsigned long long)N), v = (int)(rnd() % (unsigned long long)N);
            if (u == v) continue;
            int c = ecol(u, v), nc = 1 - c;
            ADJ[c][u][v >> 6] &= ~(1ULL << (v & 63)); ADJ[c][v][u >> 6] &= ~(1ULL << (u & 63));
            ADJ[nc][u][v >> 6] |= 1ULL << (v & 63); ADJ[nc][v][u >> 6] |= 1ULL << (u & 63);
            did++;
        }
        write_matrix(argv[8]);
        printf("# corrupted %d/%d edges (%.1f%%) -> %s\n", did, tot, 100.0 * did / tot, argv[8]);
        return 0;
    }

    long long nR = enum_all(0, R), nB = enum_all(1, S);
    rebuild_el();
    printf("# %s n=%d R=%d S=%d  red K%d=%lld blue K%d=%lld total=%lld [%.2fs]\n",
           argv[1], N, R, S, R, nR, S, nB, nred + nblue, tnow() - t0);

    if (strcmp(mode, "testflip") == 0) {
        int u = atoi(argv[6]), v = atoi(argv[7]);
        long long before = live(); int newc;
        long long cr = creation_of(u, v, &newc), rm = removal_of(u, v);
        printf("# testflip %d-%d (%s -> %s): removal=%lld creation=%lld expected=%lld\n",
               u, v, ecol(u, v) ? "blue" : "red", newc ? "blue" : "red", rm, cr, before - rm + cr);
        do_flip(u, v);
        printf("#   actual after flip = %lld [%s]\n", live(), (before - rm + cr == live()) ? "OK" : "MISMATCH");
        write_matrix("testflip.out.txt");
        return 0;
    }
    if (strcmp(mode, "delta") == 0) {
        char *done = calloc((size_t)N * N, 1);
        long long best = 0; int bu = -1, bv = -1, freek = 0, impr = 0, tested = 0;
        for (int i = 0; i < ncf; i++) {
            if (cf[i].dead) continue;
            for (int a = 0; a < cf[i].k; a++) for (int b = a + 1; b < cf[i].k; b++) {
                int u = cf[i].v[a], w = cf[i].v[b], e = eidx(u, w);
                if (done[e]) continue;
                done[e] = 1; int newc;
                long long cr = creation_of(u, w, &newc), rm = removal_of(u, w);
                long long d = cr - rm;
                tested++;
                if (cr == 0 && rm > 0) freek++;
                if (d < 0) impr++;
                if (bu < 0 || d < best) { best = d; bu = u; bv = w; }
            }
        }
        printf("# delta: conflict edges=%lld free-kills=%lld improving=%lld best=flip %d-%d d=%lld [%.2fs]\n",
               tested, freek, impr, bu, bv, best, tnow() - t0);
        return 0;
    }
    if (strcmp(mode, "desc") == 0) {
        long long maxsteps = argc > 6 ? atoll(argv[6]) : 1000;
        int allowzero = argc > 7 ? atoi(argv[7]) : 0;
        const char *pref = argc > 8 ? argv[8] : "desc";
        long long start = live(), cur = start, steps = 0;
        int *us = malloc(sizeof(int) * (size_t)N * N), *vs = malloc(sizeof(int) * (size_t)N * N);
        char *seen = calloc((size_t)N * N, 1);
        printf("# desc start=%lld allowzero=%d\n", cur, allowzero);
        for (steps = 1; steps <= maxsteps; steps++) {
            if (cur == 0) break;
            long long bestd = 0; int bu = -1, bv = -1, ncand = 0;
            for (int i = 0; i < ncf; i++) {
                if (cf[i].dead) continue;
                for (int a = 0; a < cf[i].k; a++) for (int b = a + 1; b < cf[i].k; b++) {
                    int e = eidx(cf[i].v[a], cf[i].v[b]);
                    if (seen[e]) continue;
                    seen[e] = 1; us[ncand] = cf[i].v[a]; vs[ncand] = cf[i].v[b]; ncand++;
                }
            }
            for (int t = 0; t < ncand; t++) {
                seen[eidx(us[t], vs[t])] = 0;
                int newc; long long d = creation_of(us[t], vs[t], &newc) - removal_of(us[t], vs[t]);
                if (bu < 0 || d < bestd) { bestd = d; bu = us[t]; bv = vs[t]; }
            }
            if (bu < 0) break;
            if (bestd >= 0 && !(allowzero && bestd == 0)) { printf("# local minimum at %lld after %lld steps (best delta %lld)\n", cur, steps - 1, bestd); break; }
            do_flip(bu, bv);
            printf("# step %lld: flip %d-%d d=%lld  %lld -> %lld (%.2fs)\n", steps, bu, bv, bestd, cur, live(), tnow() - t0);
            cur = live();
            if (cur < start) { char buf[512]; snprintf(buf, sizeof buf, "%s.best.txt", pref); write_matrix(buf); }
        }
        printf("# DESC %lld -> %lld  steps=%lld %.2fs\n", start, live(), steps - 1, tnow() - t0);
        return 0;
    }

    /* ---- run: weighted tabu + Metropolis + kicks ---- */
    long long steps = atoll(argv[6]);
    int candC = atoi(argv[7]);
    rs = strtoull(argv[8], 0, 10) * 2654435761ULL + 88172645463325252ULL; if (!rs) rs = 1;
    double maxtime = atof(argv[9]);
    long long stallkick = atoll(argv[10]);
    const char *pref = argv[11];
    double wR = (double)(S * (S - 1)) / (double)(R * (R - 1));
    if (argc > 12) wR = atof(argv[12]);
    if (argc > 13) capval = atoll(argv[13]);
    long long bestobj = (long long)(wR * (double)nred + (double)nblue);
    char bestfile[512]; snprintf(bestfile, sizeof bestfile, "%s.best.txt", pref);
    write_matrix(bestfile);
    save_best();
    int *tu = malloc(sizeof(int) * (size_t)(candC + 8)), *tv = malloc(sizeof(int) * (size_t)(candC + 8));
    long long *td = malloc(sizeof(long long) * (size_t)(candC + 8));
    int *tabuuntil = calloc((size_t)N * N, sizeof(int));
    long long lastimp = 0, iters = 0;
    printf("# run steps=%lld cand=%d stallkick=%lld wR=%.3f maxtime=%.0fs cap=%lld start red=%lld blue=%lld\n",
           steps, candC, stallkick, wR, maxtime, capval, nred, nblue);
    fflush(stdout);
    long long nempty = 0, neval = 0;
    for (long long it = 1; it <= steps; it++) {
        iters = it;
        if (tnow() - t0 > maxtime) { printf("# time budget reached at iter %lld\n", it); break; }
        if (live() == 0) { printf("# SOLVED at iter %lld\n", it); write_matrix(bestfile); break; }
        int nc = 0;
        for (int tries = 0; tries < candC * 4 && nc < candC; tries++) {
            int id = liveids[(int)(rnd() % (unsigned long long)nlids)];
            int a = (int)(rnd() % (unsigned long long)cf[id].k), b = (int)(rnd() % (unsigned long long)cf[id].k);
            if (a == b) continue;
            int u = cf[id].v[a], v = cf[id].v[b];
            if (tabuuntil[eidx(u, v)] > it) continue;
            int dup = 0;
            for (int t = 0; t < nc; t++) if (tu[t] == u && tv[t] == v) { dup = 1; break; }
            if (!dup) { tu[nc] = u; tv[nc] = v; nc++; }
        }
        if (nc == 0) { nempty++; for (int e = 0; e < N * N; e++) tabuuntil[e] = 0; continue; }
        long long bd = 0; int bi = -1;
        for (int t = 0; t < nc; t++) {
            int newc; long long cr = creation_of_capped(tu[t], tv[t], &newc), rm = removal_of(tu[t], tv[t]);
            neval++;
            long long d;
            if (ecol(tu[t], tv[t]) == 0) d = cr - (long long)(wR * (double)rm);   /* red->blue: +cr blue, -rm red */
            else                        d = (long long)(wR * (double)cr) - rm;   /* blue->red: +cr red, -rm blue */
            td[t] = d;
            if (bi < 0 || d < bd) { bd = d; bi = t; }
        }
        double T = 0.25;
        { double tt = (double)(it - lastimp) / 100.0; if (tt > 2.5) tt = 2.5; T = 0.25 + tt; }
        int chosen = -1;
        if (bd <= 0) chosen = bi;
        else if (urand() < exp(-(double)bd / T)) chosen = bi;
        if (chosen < 0) continue;
        int u = tu[chosen], v = tv[chosen];
        do_flip(u, v);
        tabuuntil[eidx(u, v)] = (int)(it + 5 + (long long)(rnd() % 15));
        long long obj = live();   /* checkpoint on TOTAL conflicts; the walk uses the weighted delta */
        if (obj < bestobj) {
            bestobj = obj; lastimp = it;
            write_matrix(bestfile); save_best();
            printf("# it=%lld best total=%lld red=%lld blue=%lld %.1fs\n", it, bestobj, nred, nblue, tnow() - t0);
            fflush(stdout);
        }
        if (it % 4000 == 0 || ncf > 8 * (int)live() + 200000) gc();
        if (stallkick > 0 && it - lastimp == stallkick) {
            restore_best();
            patch(4 + (int)(rnd() % 5));
            for (int e = 0; e < N * N; e++) tabuuntil[e] = 0;
            lastimp = it - stallkick / 2;
            printf("# patch+restore at iter %lld -> red=%lld blue=%lld (best obj %lld)\n", it, nred, nblue, bestobj);
            fflush(stdout);
        }
    }
    printf("# RUN end: best total=%lld (current red=%lld blue=%lld) iters=%lld evals=%lld empty=%lld %.1fs file=%s\n",
           bestobj, nred, nblue, iters, neval, nempty, tnow() - t0, bestfile);
    return 0;
}
