#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXV 400
#define MAXW 7
static int N, W;
static uint64_t A[2][MAXV][MAXW];
static int best;
static int ord[MAXV], colr[MAXV];

static inline int firstv(const uint64_t*s){ for(int w=0;w<W;w++) if(s[w]) return w*64+__builtin_ctzll(s[w]); return -1; }

static void expand(uint64_t *P, int size, int k){
    int m = 0, cls = 0;
    uint64_t R[MAXW]; memcpy(R,P,(size_t)W*8);
    for(;;){
        uint64_t U[MAXW]; memcpy(U,R,(size_t)W*8);
        int placed = 0, v;
        while ((v = firstv(U)) >= 0){
            ord[m] = v; colr[m] = cls; m++; placed++;
            for (int w=0;w<W;w++) U[w] &= ~A[k][v][w];
            U[v>>6] &= ~(1ULL<<(v&63));
        }
        if (!placed) break;
        for (int i=m-placed;i<m;i++) R[ord[i]>>6] &= ~(1ULL<<(ord[i]&63));
        cls++;
    }
    for (int i = m-1; i >= 0; i--){
        if (size + colr[i] + 1 <= best) return;
        int v = ord[i];
        uint64_t P2[MAXW]; int any = 0;
        for (int w=0;w<W;w++){ P2[w] = P[w] & A[k][v][w]; if (P2[w]) any = 1; }
        if (!any) { if (size+1 > best) best = size+1; }
        else expand(P2, size+1, k);
        P[v>>6] &= ~(1ULL<<(v&63));
    }
}
static int maxclique(int k){
    best = 0;
    uint64_t P[MAXW];
    for (int w=0;w<W;w++) P[w] = ~0ULL;
    for (int i=N;i<W*64;i++) P[i>>6] &= ~(1ULL<<(i&63));
    expand(P,0,k);
    return best;
}
int main(int argc,char**argv){
    FILE *f = fopen(argv[1],"r"); if(!f){perror("open");return 1;}
    static char line[8192];
    fgets(line,sizeof line,f); N = strcspn(line,"\r\n"); rewind(f);
    W = (N+63)/64;
    int i=0; long red=0;
    while (fgets(line,sizeof line,f)){
        if ((int)strcspn(line,"\r\n") < N) continue;
        for (int j=0;j<N;j++) if (line[j]=='1'){ A[1][i][j>>6] |= 1ULL<<(j&63); if (i<j) red++; }
        i++;
    }
    fclose(f);
    for (int a=0;a<N;a++) for (int b=0;b<N;b++) if (a!=b && !((A[1][a][b>>6]>>(b&63))&1)) A[0][a][b>>6] |= 1ULL<<(b&63);
    printf("n=%d red_edges=%ld\n", N, red); fflush(stdout);
    printf("omega_red=%d\n", maxclique(1)); fflush(stdout);
    printf("omega_blue=%d\n", maxclique(0)); fflush(stdout);
    return 0;
}
