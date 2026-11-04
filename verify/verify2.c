/* Independent second checker: plain target-clique search (no colouring bound).
   usage: verify2 file r s   -> reports whether a red K_r or blue K_s exists. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define MAXV 420
#define MW 7
static int N,W;
static uint64_t A[2][MAXV][MW];
static long nodes;
static int found;
static int cur[MAXV];

static inline int firstv(const uint64_t*s){ for(int w=0;w<W;w++) if(s[w]) return w*64+__builtin_ctzll(s[w]); return -1; }

/* look for a k-clique of colour c inside P; vertices are taken in increasing order */
static void rec(int c,int k,uint64_t*P,int depth){
    if (depth==k){ found=1; return; }
    nodes++;
    uint64_t Q[MW]; memcpy(Q,P,sizeof(uint64_t)*MW);
    for(;;){
        if (found) return;
        int avail=0; int words[MW];
        for(int w=0;w<W;w++){ words[w]=__builtin_popcountll(Q[w]); avail+=words[w]; }
        if (avail < k-depth) return;
        int v=firstv(Q);
        uint64_t P2[MW];
        for(int w=0;w<MW;w++) P2[w]=Q[w]&A[c][v][w];
        cur[depth]=v;
        rec(c,k,P2,depth+1);
        Q[v>>6]&=~(1ULL<<(v&63));
    }
}
int main(int argc,char**argv){
    FILE*f=fopen(argv[1],"r"); if(!f){perror("open");return 2;}
    static char line[8192];
    fgets(line,sizeof line,f); N=strcspn(line,"\r\n"); rewind(f);
    W=(N+63)/64; int i=0; long red=0;
    while(fgets(line,sizeof line,f)){
        if((int)strcspn(line,"\r\n")<N) continue;
        for(int j=0;j<N;j++) if(line[j]=='1'){ A[1][i][j>>6]|=1ULL<<(j&63); if(i<j) red++; }
        i++;
    }
    fclose(f);
    for(int a=0;a<N;a++) for(int b=0;b<N;b++) if(a!=b && !((A[1][a][b>>6]>>(b&63))&1)) A[0][a][b>>6]|=1ULL<<(b&63);
    int r=atoi(argv[2]), s=atoi(argv[3]);
    uint64_t P[MW];
    for(int w=0;w<MW;w++) P[w]=~0ULL;
    for(int j=N;j<W*64;j++) P[j>>6]&=~(1ULL<<(j&63));
    clock_t t0=clock();
    nodes=0; found=0; rec(1,r,P,0); int hasR=found; long nodesR=nodes;
    double tR=(double)(clock()-t0)/CLOCKS_PER_SEC;
    t0=clock(); nodes=0; found=0; rec(0,s,P,0); int hasB=found; long nodesB=nodes;
    double tB=(double)(clock()-t0)/CLOCKS_PER_SEC;
    printf("%-34s n=%3d red=%6ld | red K%-2d: %-9s nodes=%-10ld %5.2fs | blue K%-2d: %-9s nodes=%-10ld %5.2fs | %s\n",
        argv[1], N, red, r, hasR?"FOUND":"none", nodesR, tR, s, hasB?"FOUND":"none", nodesB, tB,
        (!hasR && !hasB) ? "VALID" : "INVALID");
    return (!hasR && !hasB) ? 0 : 1;
}
