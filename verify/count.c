#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MAXV 400
#define MW 7
static int N,W,R,S;
static uint64_t A[2][MAXV][MW];
static inline int firstv(const uint64_t*s){ for(int w=0;w<W;w++) if(s[w]) return w*64+__builtin_ctzll(s[w]); return -1; }
static long cnt;
static void rec(int c,int k,uint64_t*P,int depth){
    if(depth==k){cnt++;return;}
    uint64_t Q[MW]; memcpy(Q,P,sizeof(uint64_t)*MW);
    for(;;){
        int avail=0; for(int w=0;w<W;w++) avail+=__builtin_popcountll(Q[w]);
        if(avail<k-depth) return;
        int v=firstv(Q); if(v<0) return;
        uint64_t P2[MW]; for(int w=0;w<W;w++) P2[w]=Q[w]&A[c][v][w];
        rec(c,k,P2,depth+1);
        Q[v>>6]&=~(1ULL<<(v&63));
    }
}
int main(int argc,char**argv){
    FILE*f=fopen(argv[1],"r"); static char line[8192];
    fgets(line,sizeof line,f); N=strcspn(line,"\r\n"); rewind(f);
    W=(N+63)/64; int i=0;
    while(fgets(line,sizeof line,f)){ if((int)strcspn(line,"\r\n")<N) continue;
        for(int j=0;j<N;j++){ if(line[j]=='1') A[1][i][j>>6]|=1ULL<<(j&63); } i++; }
    fclose(f);
    for(int a=0;a<N;a++) for(int b=0;b<N;b++) if(a!=b && !((A[1][a][b>>6]>>(b&63))&1)) A[0][a][b>>6]|=1ULL<<(b&63);
    R=atoi(argv[2]); S=atoi(argv[3]);
    uint64_t P[MW]; for(int w=0;w<MW;w++) P[w]=~0ULL;
    for(int j=N;j<W*64;j++) P[j>>6]&=~(1ULL<<(j&63));
    cnt=0; rec(1,R,P,0); long cr=cnt;
    cnt=0; rec(0,S,P,0); long cb=cnt;
    printf("%s: n=%d  red K%d = %ld, blue K%d = %ld, total %ld\n",argv[1],N,R,cr,S,cb,cr+cb);
    return 0;
}
