/* Exhaustive check: is there a 1- or 2-edge-flip repair of a (R,S) colouring?
   Repairs must destroy every conflict; a second flip must lie in the intersection
   of the edges of all remaining conflicts. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define NMAX 400
#define W 7
static int n, R, S;
static uint64_t A[2][NMAX][W];
static int cur[16];
static long nfound;
static int cl[NMAX*400][16], clk[NMAX*400], ncl, clcol[NMAX*400];
static void rec(int col,int k,uint64_t*cand,int d){
    if(d==k){ if(ncl<NMAX*400){ for(int i=0;i<k;i++) cl[ncl][i]=cur[i]; clk[ncl]=k; clcol[ncl]=col; ncl++; }
              nfound++; return; }
    int need=k-d;
    /* count bits */
    int tot=0; for(int w=0;w<W;w++) tot+=__builtin_popcountll(cand[w]);
    if(tot<need) return;
    uint64_t c[W]; memcpy(c,cand,sizeof c);
    for(int w=0;w<W;w++){
        while(c[w]){
            int b=__builtin_ctzll(c[w]); int v=w*64+b; c[w]&=c[w]-1;
            /* prune: remaining candidates after v (same word rest + later words) */
            int rem=0; { uint64_t t=c[w]; rem+=__builtin_popcountll(t); for(int x=w+1;x<W;x++) rem+=__builtin_popcountll(c[x]); }
            if(rem+1<need) return;
            cur[d]=v;
            uint64_t n2[W]; for(int x=0;x<W;x++) n2[x]=c[x]&A[col][v][x];
            rec(col,k,n2,d+1);
        }
    }
}
static void findconf(void){
    ncl=0;
    uint64_t all[W]; memset(all,0,sizeof all);
    for(int v=0;v<n;v++) all[v>>6]|=1ULL<<(v&63);
    rec(1,R,all,0);
    rec(0,S,all,0);
}
static void flip(int u,int v){
    A[0][u][v>>6]^=1ULL<<(v&63); A[0][v][u>>6]^=1ULL<<(u&63);
    A[1][u][v>>6]^=1ULL<<(v&63); A[1][v][u>>6]^=1ULL<<(u&63);
}
static void save(const char*p){
    FILE*f=fopen(p,"w"); if(!f){perror("out");return;}
    for(int i=0;i<n;i++){ for(int j=0;j<n;j++){ int b=(A[1][i][j>>6]>>(j&63))&1; fputc(b?'1':'0',f);} fputc('\n',f);} fclose(f);
}
int main(int argc,char**argv){
    FILE*f=fopen(argv[1],"r"); if(!f){perror("in");return 1;}
    char line[8192]; n=0;
    while(fgets(line,sizeof line,f)){ int L=strlen(line); while(L&&(line[L-1]=='\n'||line[L-1]=='\r')) line[--L]=0; if(!L) continue;
        for(int j=0;j<L;j++){ if(line[j]=='1'){ A[1][n][j>>6]|=1ULL<<(j&63); } else { A[0][n][j>>6]|=1ULL<<(j&63);} } n++; }
    fclose(f);
    R=atoi(argv[2]); S=atoi(argv[3]); const char*out=argv[4];
    for(int i=0;i<n;i++){ uint64_t m=~(1ULL<<(i&63)); A[0][i][i>>6]&=m; A[1][i][i>>6]&=m; }
    findconf();
    printf("n=%d R=%d S=%d  conflicts=%d\n",n,R,S,ncl); fflush(stdout);
    if(ncl==0){ printf("ALREADY VALID\n"); save(out); return 0; }
    /* candidate first flips: all edges of all conflicts */
    static int eu[200000],ev[200000]; int ne=0;
    for(int i=0;i<ncl;i++) for(int a=0;a<clk[i];a++) for(int b=a+1;b<clk[i];b++){
        int u=cl[i][a],v=cl[i][b]; if(u>v){int t=u;u=v;v=t;}
        int dup=0; for(int q=0;q<ne;q++) if(eu[q]==u&&ev[q]==v){dup=1;break;}
        if(!dup && ne<200000){ eu[ne]=u; ev[ne]=v; ne++; }
    }
    printf("candidate first flips: %d\n",ne); fflush(stdout);
    for(int q=0;q<ne;q++){
        int u=eu[q],v=ev[q];
        flip(u,v); findconf();
        if(ncl==0){ printf("1-FLIP REPAIR: %d-%d\n",u,v); save(out); return 0; }
        /* intersection of edges across all current conflicts */
        static int iu[200000],iv[200000]; int ni=0;
        for(int i=0;i<ncl;i++) for(int a=0;a<clk[i];a++) for(int b=a+1;b<clk[i];b++){
            int x=cl[i][a],y=cl[i][b]; if(x>y){int t=x;x=y;y=t;}
            int ok=1;
            for(int j=0;j<ncl && ok;j++){ int has=0;
                for(int p=0;p<clk[j]&&!has;p++) for(int r2=p+1;r2<clk[j]&&!has;r2++){
                    int X=cl[j][p],Y=cl[j][r2]; if(X>Y){int t=X;X=Y;Y=t;}
                    if(X==x&&Y==y) has=1; }
                if(!has) ok=0; }
            if(ok){ int dup=0; for(int t2=0;t2<ni;t2++) if(iu[t2]==x&&iv[t2]==y){dup=1;break;}
                    if(!dup && ni<200000){ iu[ni]=x; iv[ni]=y; ni++; } }
        }
        for(int t2=0;t2<ni;t2++){
            flip(iu[t2],iv[t2]); findconf();
            if(ncl==0){ printf("2-FLIP REPAIR: %d-%d then %d-%d\n",u,v,iu[t2],iv[t2]); save(out); return 0; }
            flip(iu[t2],iv[t2]);
        }
        flip(u,v);
        if((q%10)==0){ printf("  ...%d/%d done\n",q,ne); fflush(stdout); }
    }
    printf("NO 1- OR 2-FLIP REPAIR\n");
    return 2;
}
