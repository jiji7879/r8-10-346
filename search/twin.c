/* twin.c -- exact post-twin conflict scan for Ramsey colourings.
 *
 * Fact used: if v' is a twin of v with twin edge of colour c, then the new
 * colouring has, for the other colour d != c, zero conflicts (v' copies v
 * exactly, and no d-clique can hold both v and v').  The conflicts of colour c
 * are exactly those c-cliques that contain both v and v', i.e. in bijection
 * with the c-cliques of size (K-1) through v (K = clique size of colour c).
 * So:  cost(v, red twin edge) = #red (R-1)-cliques through v
 *      cost(v, blue twin edge) = #blue (S-1)-cliques through v
 *
 * usage: twin <matrix> <R> <S> <swap> [w <v> <red|blue> <outfile>]
 *   swap=0 : stored '1' is RED (R6_8, R8_10, rs/* files)
 *   swap=1 : stored '1' is BLUE (the six .scratch/exoo/ files)
 *   w mode : write the (n+1)-vertex matrix with twin of v, twin edge colour given
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAXV 420
#define MW 7

static int N, W, R, S;
static uint64_t ADJ[2][MAXV][MW];
static long long tally[2][MAXV];
static int verts[32];
static int stop_now = 0;

static double now(void) { return (double)clock() / CLOCKS_PER_SEC; }

static void rec(int c, int k, int depth, uint64_t *P) {
    if (depth == k) {
        for (int i = 0; i < k; i++) tally[c][verts[i]]++;
        return;
    }
    uint64_t Q[MW];
    memcpy(Q, P, sizeof(uint64_t) * W);
    for (;;) {
        int avail = 0;
        for (int w = 0; w < W; w++) avail += __builtin_popcountll(Q[w]);
        if (avail < k - depth) return;
        int v = -1;
        for (int w = 0; w < W; w++) if (Q[w]) { v = w * 64 + __builtin_ctzll(Q[w]); break; }
        uint64_t P2[MW];
        for (int w = 0; w < W; w++) P2[w] = Q[w] & ADJ[c][v][w];
        verts[depth] = v;
        rec(c, k, depth + 1, P2);
        Q[v >> 6] &= ~(1ULL << (v & 63));
    }
}

static void rec_top(int c, int k, uint64_t *P, long long cap) {
    /* like rec but checks the cap between top-level branches */
    uint64_t Q[MW];
    memcpy(Q, P, sizeof(uint64_t) * W);
    for (;;) {
        int avail = 0;
        for (int w = 0; w < W; w++) avail += __builtin_popcountll(Q[w]);
        if (avail < k) return;
        if (cap > 0) {
            long long tot = 0;
            for (int i = 0; i < N; i++) tot += tally[c][i];
            if (tot > cap) { stop_now = 1; return; }
        }
        int v = -1;
        for (int w = 0; w < W; w++) if (Q[w]) { v = w * 64 + __builtin_ctzll(Q[w]); break; }
        uint64_t P2[MW];
        for (int w = 0; w < W; w++) P2[w] = Q[w] & ADJ[c][v][w];
        verts[0] = v;
        rec(c, k, 1, P2);
        Q[v >> 6] &= ~(1ULL << (v & 63));
        if (stop_now) return;
    }
}

int main(int argc, char **argv) {
    if (argc < 5) { fprintf(stderr, "usage: %s matrix R S swap [w v red|blue out]\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }
    static char line[8192];
    if (!fgets(line, sizeof line, f)) return 1;
    N = strcspn(line, "\r\n");
    W = (N + 63) / 64;
    rewind(f);
    int i = 0;
    while (fgets(line, sizeof line, f)) {
        if ((int)strcspn(line, "\r\n") < N) continue;
        if (i >= MAXV) { fprintf(stderr, "too big\n"); return 1; }
        for (int j = 0; j < N; j++) if (line[j] == '1') ADJ[1][i][j >> 6] |= 1ULL << (j & 63);
        i++;
    }
    fclose(f);
    if (i != N) { fprintf(stderr, "expected %d rows, read %d\n", N, i); return 1; }
    /* ADJ[1] = stored 1s.  swap=0: 1 is red.  swap=1: 1 is blue. */
    for (int a = 0; a < N; a++)
        for (int b = 0; b < N; b++)
            if (a != b && !((ADJ[1][a][b >> 6] >> (b & 63)) & 1))
                ADJ[0][a][b >> 6] |= 1ULL << (b & 63);
    /* argv[4] = 1 means "stored 1s are BLUE" (the six .scratch/exoo files).
     * Then stored 0s are RED.  Internally we want ADJ[0]=red, ADJ[1]=blue.
     * Loading left ADJ[1]=stored 1s, ADJ[0]=complement(stored 1s)=stored 0s,
     * which is already (red,blue) when stored 1s are blue; otherwise swap. */
    int stored1_is_blue = atoi(argv[4]);
    if (!stored1_is_blue) { uint64_t tmp[MW]; for (int a = 0; a < N; a++) { memcpy(tmp, ADJ[0][a], sizeof tmp); memcpy(ADJ[0][a], ADJ[1][a], sizeof tmp); memcpy(ADJ[1][a], tmp, sizeof tmp); } }
    R = atoi(argv[2]); S = atoi(argv[3]);

    if (argc >= 9 && strcmp(argv[5], "w") == 0) {
        int v = atoi(argv[6]);
        int c = (strcmp(argv[7], "red") == 0) ? 0 : 1;
        FILE *o = fopen(argv[8], "w");
        int n = N + 1;
        /* output convention: '1' = RED always */
        for (int a = 0; a < n; a++) {
            for (int b = 0; b < n; b++) {
                int val;
                if (a == N && b == N) val = 0;
                else if (a == N && b == v) val = (c == 0);            /* twin edge */
                else if (b == N && a == v) val = (c == 0);            /* twin edge */
                else if (a == N) val = (int)((ADJ[0][b][v >> 6] >> (v & 63)) & 1);  /* copy v's RED nbrs */
                else if (b == N) val = (int)((ADJ[0][a][v >> 6] >> (v & 63)) & 1);
                else val = (int)((ADJ[0][a][b >> 6] >> (b & 63)) & 1);
                fputc(val ? '1' : '0', o);
            }
            fputc('\n', o);
        }
        fclose(o);
        fprintf(stderr, "wrote %d vertices to %s (twin of %d, twin edge %s; output '1' = RED)\n",
                n, argv[8], v, argv[7]);
        return 0;
    }

    uint64_t P[MW];
    for (int w = 0; w < MW; w++) P[w] = ~0ULL;
    for (int j = N; j < W * 64; j++) P[j >> 6] &= ~(1ULL << (j & 63));

    double t0 = now();
    long long cap = (argc >= 6) ? atoll(argv[5]) : 0;   /* optional cap on found cliques */
    for (int c = 0; c < 2; c++) {
        for (int v = 0; v < N; v++) tally[c][v] = 0;
        int k = (c == 0) ? R - 1 : S - 1;
        if (k <= 0) { if (k == 0) for (int v = 0; v < N; v++) tally[c][v] = 1; }
        else if (k == 1) { for (int v = 0; v < N; v++) { int d = 0; for (int w = 0; w < W; w++) d += __builtin_popcountll(ADJ[c][v][w]); tally[c][v] = d; } }
        else rec_top(c, k, P, cap);
    }
    double t1 = now();
    long long totR = 0, totB = 0;
    for (int v = 0; v < N; v++) { totR += tally[0][v]; totB += tally[1][v]; }
    printf("# %s  n=%d  R=%d S=%d stored1isblue=%d\n", argv[1], N, R, S, stored1_is_blue);
    printf("# totals: red %d-cliques=%lld  blue %d-cliques=%lld   (raw incidence sums R=%lld B=%lld)  time %.1fs\n",
           R - 1, (R > 2 ? totR / (R - 1) : (R == 3 ? totR / 2 : totR)), S - 1,
           (S > 2 ? totB / (S - 1) : (S == 3 ? totB / 2 : totB)), totR, totB, t1 - t0);
    /* cost_c(v) = # c-(K-2)-cliques inside N_c(v) = # c-(K-1)-cliques through v.
       For printing the validity check we need # red K_R = # red K_R and # blue K_S. */
    printf("# (for validity see the K_R / K_S counts printed by a run with R+1 / S+1)\n");
    long long bestR = -1, bestB = -1; int bvR = -1, bvB = -1;
    for (int v = 0; v < N; v++) {
        if (bestR < 0 || tally[0][v] < bestR) { bestR = tally[0][v]; bvR = v; }
        if (bestB < 0 || tally[1][v] < bestB) { bestB = tally[1][v]; bvB = v; }
    }
    printf("# best red twin : v=%d cost=%lld\n", bvR, bestR);
    printf("# best blue twin: v=%d cost=%lld\n", bvB, bestB);
    printf("# v redcost bluecost\n");
    for (int v = 0; v < N; v++) printf("%d %lld %lld\n", v, tally[0][v], tally[1][v]);
    /* ranked best twins overall */
    for (int c = 0; c < 2; c++) {
        int idx[MAXV];
        for (int v = 0; v < N; v++) idx[v] = v;
        for (int a = 0; a < N; a++) for (int b = a + 1; b < N; b++) if (tally[c][idx[b]] < tally[c][idx[a]]) { int t = idx[a]; idx[a] = idx[b]; idx[b] = t; }
        printf("# ranked %s twins (cost, v):", c == 0 ? "RED" : "BLUE");
        for (int a = 0; a < N && a < 6; a++) printf(" (%lld,v%d)", tally[c][idx[a]], idx[a]);
        printf("\n");
    }
    return 0;
}
