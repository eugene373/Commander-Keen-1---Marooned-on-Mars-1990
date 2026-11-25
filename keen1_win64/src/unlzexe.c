#include "unlzexe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Port of tools/unlzexe.py. The bit buffer is refilled immediately after the 16th bit is consumed. */
typedef struct { const uint8_t *d; size_t n, pos; unsigned bits; int cnt; int bad; } Rd;
static unsigned rb(Rd *r){ if (r->pos>=r->n){ r->bad=1; return 0; } return r->d[r->pos++]; }
static unsigned gb(Rd *r){
    unsigned b=r->bits&1u; r->cnt--;
    if (r->cnt>0) r->bits>>=1;
    else { unsigned lo=rb(r); unsigned hi=rb(r); r->bits=lo|(hi<<8); r->cnt=16; }
    return b;
}
uint8_t *unlzexe_file(const char *path, size_t *size, char *err){
    FILE *f=fopen(path,"rb"); if(!f){ snprintf(err,128,"cannot open %s",path); return NULL; }
    fseek(f,0,SEEK_END); long len=ftell(f); fseek(f,0,SEEK_SET);
    uint8_t *d=(uint8_t*)malloc((size_t)len+4); if(!d||fread(d,1,(size_t)len,f)!=(size_t)len){ fclose(f); free(d); snprintf(err,128,"read error"); return NULL; }
    fclose(f); memset(d+len,0,4);
    if(len<32||d[0]!='M'||d[1]!='Z'){ free(d); snprintf(err,128,"not a DOS executable"); return NULL; }
    Rd r={d,(size_t)len,(size_t)(d[8]|(d[9]<<8))*16,0,0,0};
    { unsigned lo=rb(&r), hi=rb(&r); r.bits=lo|(hi<<8); r.cnt=16; }
    size_t cap=1<<18, o=0; uint8_t *out=(uint8_t*)malloc(cap);
    for(;;){
        if(r.bad||!out){ free(out); free(d); snprintf(err,128,"corrupt LZEXE data"); return NULL; }
        if(o+300>cap){ cap*=2; out=(uint8_t*)realloc(out,cap); if(!out) continue; }
        if(gb(&r)){ out[o++]=(uint8_t)rb(&r); continue; }
        int disp; unsigned n;
        if(!gb(&r)){ n=gb(&r)<<1; n|=gb(&r); n+=2; disp=(int)rb(&r)-256; }
        else {
            unsigned lo=rb(&r), hi=rb(&r); disp=(int)(((hi>>3)<<8)|lo)-8192; n=hi&7;
            if(n==0){ n=rb(&r); if(n==0) break; if(n==1) continue; n+=1; } else n+=2;
        }
        if((long)o+disp<0){ free(out); free(d); snprintf(err,128,"corrupt LZEXE data"); return NULL; }
        for(unsigned i=0;i<n;i++){ out[o]=out[(size_t)((long)o+disp)]; o++; }
    }
    free(d); *size=o; return out;
}
