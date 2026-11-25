/* keen1_selftest KEEN1.EXE vectors.txt
   Loads the original game executable, unpacks it (LZEXE), places the image in the 1 MB address space and replays recorded runs of the ORIGINAL machine code
   (random data-segment bytes, registers, arguments -> result register and every changed memory byte): each lifted C function must reproduce them exactly. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lifted.h"
#include "unlzexe.h"
#include "lifted_table.h"
static int rdi(FILE *f, long *v){ char t[64]; if(fscanf(f,"%63s",t)!=1) return 0; if(t[0]=='-'&&!t[1]){ *v=-1; return 2; } *v=atol(t); return 1; }
int main(int argc, char **argv){
    if(argc<3){ fprintf(stderr,"usage: %s KEEN1.EXE vectors.txt\n",argv[0]); return 2; }
    char err[128]; size_t isz=0; uint8_t *img=unlzexe_file(argv[1],&isz,err);
    if(!img){ fprintf(stderr,"%s\n",err); return 2; }
    if(isz+0x10000>sizeof(M)){ fprintf(stderr,"image too large\n"); return 2; }
    FILE *f=fopen(argv[2],"r"); if(!f){ perror(argv[2]); return 2; }
    uint8_t *exp=(uint8_t*)malloc(sizeof(M)); if(!exp) return 2;
    int pass[NFUNC]={0}, fail[NFUNC]={0}; char tag[8]; long n;
    while(fscanf(f,"%7s",tag)==1){
        long idx; rdi(f,&idx); int k=FUNCS[idx].arity; int16_t args[16]={0}, regs[7];
        if(k==0){ rdi(f,&n); } else for(int i=0;i<k;i++){ rdi(f,&n); args[i]=(int16_t)n; }
        for(int i=0;i<7;i++){ rdi(f,&n); regs[i]=(int16_t)n; }
        memset(M,0,sizeof(M)); memcpy(M+((size_t)KEEN1_LOAD_SEG<<4),img,isz);
        long np; rdi(f,&np); size_t dsb=(size_t)KEEN1_DSEG<<4;
        if(np==0) rdi(f,&n); else for(long i=0;i<np;i++){ long o,v; rdi(f,&o); rdi(f,&v); M[dsb+o]=(uint8_t)v; }
        memcpy(exp,M,sizeof(M));
        long ea; rdi(f,&ea); long nd; rdi(f,&nd);
        if(nd==0) rdi(f,&n); else for(long i=0;i<nd;i++){ long o,v; rdi(f,&o); rdi(f,&v); exp[o]=(uint8_t)v; }
        if(getenv("KEEN1_TRACE")){ fprintf(stderr,"run %s args %d %d %d %d %d\n",FUNCS[idx].name,args[0],args[1],args[2],args[3],args[4]); fflush(stderr); }
        set_regs(regs); int32_t r=call_by_index((int)idx,args);
        int ok=1;
        if(FUNCS[idx].ret && (uint16_t)(r&0xFFFF)!=(uint16_t)ea) ok=0;
        size_t a=dsb+0xFE00, b=dsb+0x10000;                    /* the stack area is not compared */
        if(memcmp(M,exp,a)!=0 || memcmp(M+b,exp+b,sizeof(M)-b)!=0) ok=0;
        if(ok) pass[idx]++; else fail[idx]++;
    }
    fclose(f); int tp=0,tf=0,nfun=0,nbad=0;
    for(int i=0;i<NFUNC;i++){ if(pass[i]+fail[i]==0) continue; nfun++; tp+=pass[i]; tf+=fail[i]; if(fail[i]){ nbad++; printf("FAIL %-28s %d/%d\n",FUNCS[i].name,pass[i],pass[i]+fail[i]); } }
    printf("%d functions, %d vectors: %d passed, %d failed\n",nfun,tp+tf,tp,tf);
    free(exp); free(img); return tf?1:0;
}
