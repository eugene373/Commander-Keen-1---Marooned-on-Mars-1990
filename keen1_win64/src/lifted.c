#include <stdint.h>
typedef int16_t i16; typedef uint16_t u16; typedef uint8_t u8; typedef uint32_t u32;
u8 M[1<<20]; enum { DSEG=0x2305, DB=0x23050 };
#define MA(a) (M+((((u32)(u16)DS)<<4)+(u16)(a)&0xFFFFF))
#define FA(s,o) (M+((((u32)(u16)(s))<<4)+(u16)(o)&0xFFFFF))
#define MW(a) (*(i16*)MA(a))
#define MB(a) (*(u8*)MA(a))
#define MD(a) (*(int32_t*)MA(a))
#define W(x) (*(i16*)MA(A_##x))
#define B(x) (*(u8*)MA(A_##x))
#define D(x) (*(int32_t*)MA(A_##x))
#define WF(s,o) (*(i16*)FA(s,o))
#define BF(s,o) (*(u8*)FA(s,o))
#define SEG(x) ((u16)((u32)(x)>>16))
#define OFF(x) ((u16)(x))
#define MK_LONG(h,l) ((int32_t)((((u32)(u16)(h))<<16)|(u16)(l)))
#define LOWORD(x) ((u16)(x))
#define HIWORD(x) ((u16)((u32)(x)>>16))
#define LOBYTE(x) ((u8)(x))
#define HIBYTE(x) ((u8)((x)>>8))
#define SIGN(x) ((x)<0?-1:0)
i16 ax,bx,cx,dx,si,di,bp,sp,carry;
#define al (((u8*)&ax)[0])
#define ah (((u8*)&ax)[1])
#define bl (((u8*)&bx)[0])
#define bh (((u8*)&bx)[1])
#define cl (((u8*)&cx)[0])
#define ch (((u8*)&cx)[1])
#define dl (((u8*)&dx)[0])
#define dh (((u8*)&dx)[1])
 u16 ES=DSEG,DS=DSEG,SS=DSEG,CS=0x1000;
#define es ES
#define ds DS
#define ss SS
#define cs CS

#define memcpy_repb(a,b,n) do{ u16 _n=(u16)cx; while(_n--){ *FA(ES,di)=*FA(DS,si); di++; si++; } cx=0; }while(0)
#define memcpy_repw(a,b,n) do{ u16 _n=(u16)cx; while(_n--){ *(i16*)FA(ES,di)=*(i16*)FA(DS,si); di+=2; si+=2; } cx=0; }while(0)
#define memset_repb(a,b,n) do{ u16 _n=(u16)cx; while(_n--){ *FA(ES,di)=(u8)ax; di++; } cx=0; }while(0)
#define memset_repw(a,b,n) do{ u16 _n=(u16)cx; while(_n--){ *(i16*)FA(ES,di)=ax; di+=2; } cx=0; }while(0)
#define stosb(a,b) do{ *FA(ES,di)=(u8)al; di++; }while(0)
#define stosw(a,b) do{ *(i16*)FA(ES,di)=ax; di+=2; }while(0)
#define movsb(a,b) do{ *FA(ES,di)=*FA(DS,si); di++; si++; }while(0)
#define outportb(p,v) ((void)(p),(void)(v),0)
#define outport(p,v) ((void)(p),(void)(v),0)
#define inportb(p) ((void)(p),(al=0))
#define inport(p) ((void)(p),(ax=0))
#define CLI() 0
#define STI() 0
#define int short
int L_ResetDrawLists();
int L_Joystick_ReadButtons();
int L_Tank_Pause();
int L_Tank_Contact();
int L_IceChunk_Think();
int L_Obj_Remove();
int L_Obj_AccelX();
int L_Obj_AccelY();
int L_Obj_ApplyGravity();
int L_Nop();
int L_ShotSplash_Think();
int L_Demo_StartRecording();
int L_EGA_SetLineCompare();
int L_EGA_SetScreenStart();
int L_Text_DrawBox();
int L_Text_DrawBoxCentered();
int L_Text_FillRect();
int L_Text_OpenBoxAnimated();
int L_Text_GrowWidth();
int L_Text_GrowHeight();
int L_Text_DrawStyledFrame();
int L_Text_WaitKey();
int L_Text_InputLine();
int L_RLEW_Compress_UNUSED();
int L_RlewDecode();
int L_WorldMap_ScanMarkers();
int L_Text_ClearLastBox();
int L_TextPager_ScrollEGA();
int L_TileCache_Invalidate();
int L_EGA_ComputePageOffsets();
int L_EGA_DrawTile8_Latch();
int L_Keyboard_GetKey();
int L_Sound_Play();
int L_Sound_Stop();
int L_Sound_Push();
int L_Sound_Pop();
int L_Rnd_Next();
int L_WaitVBL();
int L_EGA_SelectPlane();
int L_Video_ProbeCRTC();
int L_Rle_Init();
int L_atexit();
int L_printf();
int L_toupper();
int L_brk();
int L___brk();
int L_CritErr_Return();
int L___IOerror();
int L___vprinter();
int L__vprinter_putc();
int L__far_split_block();
int L__F_SCOPY_prolog();
int L_F_SCOPY();
int L__near_unlink_free();
int L__near_insert_free();
int L__fpnormalize();
// ===== L_ResetDrawLists @0x0239 =====
int L_ResetDrawLists() {
    MW(0x6ed8) = 0;
    MW(0x6ecc) = 0;
    MW(0x6ebe) = 0;
    MW(0x6ec6) = 0x604e;
    MW(0x6ece) = 0x6c0a;
    MW(0x7fba) = 0x6c64;
    MW(0x5db6) = 0x5660;
    do {
        ax = MW(0x5135) - MW(0x563e);
        MW(0x5b14) = ax;
    } while ((u16)ax < 6);
    if ((u16)ax > 0xf) {
        MW(0x5b14) = 0xf;
    }
    ax = MW(0x5137);
    MW(0x563e) = MW(0x5135);
    MW(0x5640) = ax;
    return ax;
}


// ===== L_Joystick_ReadButtons @0x08b2 =====
int L_Joystick_ReadButtons() {
    int sv_si;
    sv_si = si;
    inportb(0x201);
    si = al;
    if ((al & 0x10) == 0) {
        return 1;
    }
    if ((si & 0x20) == 0) {
        return 2;
    }
    ax = 0;
    si = sv_si;
    return ax;
}


// ===== L_Tank_Pause @0x1f8f =====
int L_Tank_Pause() {
    MW(0x824a) = MW(0x824a) + MW(0x5b14);
    ax = ((((u16)MW(0x5135)) >> 4) & 1) + 0x6a;
    MW(0x8248) = ax;
    if (MW(0x824a) <= 0x32) return ax;
    MW(0x8252) = 0x1ea9;
    ax = MW(0x824c);
    MW(0x8240) = ax;
L1fbf:;
    return ax;
}


// ===== L_Tank_Contact @0x2045 =====
int L_Tank_Contact() {
    return ax;
}


// ===== L_IceChunk_Think @0x2151 =====
int L_IceChunk_Think() {
    ax = MW(0x824a) + MW(0x5b14);
    MW(0x824a) = ax;
    if (ax > 0x3c) {
        MW(0x8220) = 0;
        return ax;
    }
    L_Obj_ApplyGravity();
    MW(0x823c) = MW(0x823c) + MW(0x8240) * MW(0x5b14);
    ax = MW(0x8242) * MW(0x5b14);
    MW(0x823e) = MW(0x823e) + MW(0x8242) * MW(0x5b14);
    return ax;
}


// ===== L_Obj_Remove @0x290e =====
int L_Obj_Remove() {
    MW(0x8220) = 0;
    return ax;
}


// ===== L_Obj_AccelX @0x2b9a =====
int L_Obj_AccelX(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int sv_di;
    int sv_si;
    sv_di = di;
    sv_si = si;
    di = P.arg0;
    si = 1;
    while ((u16)si <= (u16)MW(0x5b14)) {
        MW(0x8240) += di;
        if (MW(0x8240) > 0x78) {
            MW(0x8240) = 0x78;
        } else if (MW(0x8240) < -120) {
            MW(0x8240) = 0xff88;
        }
        if (si != MW(0x5b14)) {
            ax = MW(0x8240);
            MW(0x823c) += MW(0x8240);
        }
        si++;
    }
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Obj_AccelY @0x2bdf =====
int L_Obj_AccelY(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    int sv_di;
    int sv_si;
    sv_di = di;
    sv_si = si;
    di = P.arg0;
    si = 1;
    dx = P.arg1;
    while ((u16)si <= (u16)MW(0x5b14)) {
        MW(0x8242) += dx;
        ax = dx;
        if (MW(0x8242) > di) {
            MW(0x8242) = di;
        } else {
            ax = -di;
            if ((-di) > MW(0x8242)) {
                MW(0x8242) = -di;
                ax = -di;
            }
        }
        if (si != MW(0x5b14)) {
            ax = MW(0x8242);
            MW(0x823e) += MW(0x8242);
        }
        si++;
    }
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Obj_ApplyGravity @0x2c2a =====
int L_Obj_ApplyGravity() {
    int sv_si;
    sv_si = si;
    si = 1;
    while ((u16)si <= (u16)MW(0x5b14)) {
        MW(0x8242) += 3;
        if (MW(0x8242) > 0xc8) {
            MW(0x8242) = 0xc8;
        } else if (MW(0x8242) < -400) {
            MW(0x8242) = 0xfe70;
        }
        if (si != MW(0x5b14)) {
            ax = MW(0x8242);
            MW(0x823e) += MW(0x8242);
        }
        si++;
    }
    si = sv_si;
    return ax;
}


// ===== L_Nop @0x3360 =====
int L_Nop() {
    return ax;
}


// ===== L_ShotSplash_Think @0x47e8 =====
int L_ShotSplash_Think() {
    MW(0x8220) = 0xe;
    ax = MW(0x824a) + MW(0x5b14);
    MW(0x824a) = ax;
    if (ax <= 0x14) return ax;
    MW(0x8220) = 0;
L4806:;
    return ax;
}


// ===== L_Demo_StartRecording @0x5b3d =====
int L_Demo_StartRecording() {
    MB(0x8370) = MB(0x8304);
    MW(0x8362) = 0x8371;
    MW(0x9732) = 2;
    return ah << 8 | MB(0x8304);
}


// ===== L_EGA_SetLineCompare @0x5e41 =====
int L_EGA_SetLineCompare(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int sv_si;
    int ts3;
    int ts4;
    sv_si = si;
    si = P.arg0;
    L_WaitVBL(1);
    if (MW(0x9730) == 5) {
        si = si * 2;
    }
    ts3 = ah << 8 | 0x18;
    ts4 = 0x3d4;
    outportb(0x3d4, ah << 8 | 0x18);
    ts3 = si % 0x100;
    ts4 = 0x3d5;
    outportb(0x3d5, si % 0x100);
    ts3 = ah << 8 | 7;
    ts4 = 0x3d4;
    outportb(0x3d4, ah << 8 | 7);
    ts3 = HIBYTE(si / 0x100) << 8 | (LOBYTE(si / 0x100) << 4) + 1;
    ts4 = 0x3d5;
    ax = outportb(0x3d5, HIBYTE(si / 0x100) << 8 | (LOBYTE(si / 0x100) << 4) + 1);
    if (MW(0x9730) != 5) return ax;
    ts3 = ah << 8 | 9;
    ts4 = 0x3d4;
    outportb(0x3d4, ah << 8 | 9);
    ts3 = 0x3d5;
    ts4 = HIBYTE(inportb(0x3d5)) << 8 | LOBYTE(inportb(0x3d5)) & 0xbf;
    ax = outportb(0x3d5, HIBYTE(inportb(0x3d5)) << 8 | LOBYTE(inportb(0x3d5)) & 0xbf);
L5ed1:;
    si = sv_si;
    return ax;
}


// ===== L_EGA_SetScreenStart @0x5ed4 =====
int L_EGA_SetScreenStart(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int ts2;
    int ts3;
    L_WaitVBL(1);
    ts2 = ah << 8 | 0xd;
    ts3 = 0x3d4;
    outportb(0x3d4, ah << 8 | 0xd);
    ts2 = ah << 8 | (*(u8*)&P.arg0) & 0xff;
    ts3 = 0x3d5;
    outportb(0x3d5, ah << 8 | (*(u8*)&P.arg0) & 0xff);
    ts2 = ah << 8 | 0xc;
    ts3 = 0x3d4;
    outportb(0x3d4, ah << 8 | 0xc);
    ts2 = ((u16)P.arg0) >> 8;
    ts3 = 0x3d5;
    return outportb(0x3d5, ((u16)P.arg0) >> 8);
}


// ===== L_Text_DrawBox @0x5f1e =====
int L_Text_DrawBox(p0,p1,p2,p3)
int p0; int p1; int p2; int p3; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3;

    int sv_di;
    int sv_si;
    int ts3;
    int ts4;
    int ts5;
    sv_di = di;
    sv_si = si;
    MW(0x8358) = P.arg0;
    MW(0x835e) = P.arg1;
    MW(0x835a) = P.arg2;
    MW(0x8360) = P.arg3;
    ts3 = 1;
    ts4 = ((P.arg1 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg0, ((P.arg1 << 1) << 1) << 1, 1);
    si = P.arg0 + 1;
    while (si < P.arg2) {
        ts3 = 2;
        ts4 = ((P.arg1 << 1) << 1) << 1;
        ts5 = si;
        L_EGA_DrawTile8_Latch(si, ((P.arg1 << 1) << 1) << 1, 2);
        si++;
    }
    ts3 = 3;
    ts4 = ((P.arg1 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg2, ((P.arg1 << 1) << 1) << 1, 3);
    di = P.arg1 + 1;
    while (di < P.arg3) {
        ts3 = 4;
        ts4 = ((di << 1) << 1) << 1;
        L_EGA_DrawTile8_Latch(P.arg0, ((di << 1) << 1) << 1, 4);
        si = P.arg0 + 1;
        while (si < P.arg2) {
            ts3 = 0x20;
            ts4 = ((di << 1) << 1) << 1;
            ts5 = si;
            L_EGA_DrawTile8_Latch(si, ((di << 1) << 1) << 1, 0x20);
            si++;
        }
        ts3 = 5;
        ts4 = ((di << 1) << 1) << 1;
        L_EGA_DrawTile8_Latch(P.arg2, ((di << 1) << 1) << 1, 5);
        di++;
    }
    ts3 = 6;
    ts4 = ((P.arg3 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg0, ((P.arg3 << 1) << 1) << 1, 6);
    si = P.arg0 + 1;
    while (si < P.arg2) {
        ts3 = 7;
        ts4 = ((P.arg3 << 1) << 1) << 1;
        ts5 = si;
        L_EGA_DrawTile8_Latch(si, ((P.arg3 << 1) << 1) << 1, 7);
        si++;
    }
    ts3 = 8;
    ts4 = ((P.arg3 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg2, ((P.arg3 << 1) << 1) << 1, 8);
    MW(0x8354) = P.arg0 + 1;
    MW(0x8282) = P.arg0 + 1;
    MW(0x82f8) = P.arg1 + 1;
    di = sv_di;
    si = sv_si;
    return P.arg1 + 1;
}


// ===== L_Text_DrawBoxCentered @0x604e =====
int L_Text_DrawBoxCentered(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    int sv_di;
    int sv_si;
    sv_di = di;
    sv_si = si;
    di = MW(0x24ce) - (P.arg0 / 2);
    dx = MW(0x24d0) - (P.arg1 / 2);
    si = MW(0x24d0) - (P.arg1 / 2);
    ax = L_Text_DrawBox(di, dx, (di + P.arg0) + 1, (si + P.arg1) + 1);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_FillRect @0x608c =====
int L_Text_FillRect(p0,p1,p2,p3,p4)
int p0; int p1; int p2; int p3; int p4; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; i16 arg4; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3; P.arg4=p4;

    int sv_di;
    int sv_si;
    sv_di = di;
    sv_si = si;
    di = P.arg1;
    while (di <= P.arg3) {
        si = P.arg0;
        while (si <= P.arg2) {
            ax = L_EGA_DrawTile8_Latch(si, ((di << 1) << 1) << 1, P.arg4);
            si++;
        }
        di++;
    }
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_OpenBoxAnimated @0x60be =====
int L_Text_OpenBoxAnimated(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    int sv_di;
    int sv_si;
    int ts3;
    int ts4;
    sv_di = di;
    sv_si = si;
    di = P.arg0;
    si = P.arg1;
    if (di > 2) {
        if (si > 2) {
            ts3 = si - 2;
            ts4 = di - 2;
            L_Text_OpenBoxAnimated();
            /* leftover push si - 2 */
            /* leftover push di - 2 */
            goto L6102;
        }
        ts3 = si;
        ts4 = di - 2;
        L_Text_GrowWidth(di - 2, si);
        goto L6105;
    }
    if (si <= 2) goto L6105;
    ts3 = si - 2;
    ts4 = di;
    L_Text_GrowHeight();
    /* leftover push si - 2 */
    /* leftover push di */
L6102:;
L6105:;
    ts3 = 1;
    L_WaitVBL(1);
    ts4 = si;
    ax = L_Text_DrawBoxCentered(di, si);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_GrowWidth @0x611a =====
int L_Text_GrowWidth(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    int sv_si;
    sv_si = si;
    si = P.arg0;
    if (si > 2) {
        L_Text_GrowWidth(si - 2, P.arg1);
    }
    L_WaitVBL(1);
    ax = L_Text_DrawBoxCentered(si, P.arg1);
    si = sv_si;
    return ax;
}


// ===== L_Text_GrowHeight @0x614b =====
int L_Text_GrowHeight(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    int sv_si;
    int ts2;
    sv_si = si;
    si = P.arg1;
    if (si > 2) {
        ts2 = si - 2;
        L_Text_GrowHeight(P.arg0, si - 2);
    }
    ts2 = 1;
    L_WaitVBL(1);
    ax = L_Text_DrawBoxCentered(P.arg0, si);
    si = sv_si;
    return ax;
}


// ===== L_Text_DrawStyledFrame @0x617c =====
int L_Text_DrawStyledFrame(p0,p1,p2,p3,p4)
int p0; int p1; int p2; int p3; int p4; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; i16 arg4; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3; P.arg4=p4;

    int sv_di;
    int sv_si;
    int ts3;
    int ts4;
    int ts5;
    sv_di = di;
    sv_si = si;
    di = P.arg4;
    di = (di * 0x16) + 1;
    si = P.arg0 + 1;
    while (si < P.arg2) {
        ts3 = di + 1;
        ts4 = ((P.arg1 << 1) << 1) << 1;
        ts5 = si;
        L_EGA_DrawTile8_Latch(si, ((P.arg1 << 1) << 1) << 1, di + 1);
        ts3 = di + 6;
        ts4 = ((P.arg3 << 1) << 1) << 1;
        ts5 = si;
        L_EGA_DrawTile8_Latch(si, ((P.arg3 << 1) << 1) << 1, di + 6);
        si++;
    }
    si = P.arg1 + 1;
    while (si < P.arg3) {
        ts3 = di + 3;
        ts4 = ((si << 1) << 1) << 1;
        L_EGA_DrawTile8_Latch(P.arg0, ((si << 1) << 1) << 1, di + 3);
        ts3 = di + 4;
        ts4 = ((si << 1) << 1) << 1;
        L_EGA_DrawTile8_Latch(P.arg2, ((si << 1) << 1) << 1, di + 4);
        si++;
    }
    ts3 = di;
    ts4 = ((P.arg1 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg0, ((P.arg1 << 1) << 1) << 1, di);
    ts3 = (di + 1) + 1;
    ts4 = ((P.arg1 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg2, ((P.arg1 << 1) << 1) << 1, (di + 1) + 1);
    ts3 = di + 7;
    ts4 = ((P.arg3 << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(P.arg2, ((P.arg3 << 1) << 1) << 1, di + 7);
    ts3 = di + 5;
    ts4 = ((P.arg3 << 1) << 1) << 1;
    ax = L_EGA_DrawTile8_Latch(P.arg0, ((P.arg3 << 1) << 1) << 1, di + 5);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_WaitKey @0x6268 =====
int L_Text_WaitKey() {
    int sv_di;
    int sv_si;
    int ts4;
    int ts5;
    sv_di = di;
    sv_si = si;
    do {
        si = 9;
        goto L6293;
    L6272:;
        ax = si;
        si++;
        ts4 = ax;
        ts5 = ((MW(0x82f8) << 1) << 1) << 1;
        L_EGA_DrawTile8_Latch(MW(0x8282), ((MW(0x82f8) << 1) << 1) << 1, ax);
        ts4 = 5;
        L_WaitVBL(5);
    L6293:;
        ax = L_Keyboard_GetKey(1) & 0xff;
        di = L_Keyboard_GetKey(1) & 0xff;
        if (!ax && si < 0xd) goto L6272;
    } while (!di);
    ts4 = 0x20;
    ts5 = ((MW(0x82f8) << 1) << 1) << 1;
    L_EGA_DrawTile8_Latch(MW(0x8282), ((MW(0x82f8) << 1) << 1) << 1, 0x20);
    ts4 = 0;
    ax = L_Keyboard_GetKey(0);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_InputLine @0x65b7 =====
int L_Text_InputLine(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;
    struct { i16 v1; } V = {0};

    int sv_di;
    int sv_si;
    int ts4;
    int ts5;

    sv_di = di;
    sv_si = si;
    si = 0;
    /* leftover push si */
    /* leftover push di */
    do {
        ax = L_Text_WaitKey() & 0xff;
        ax = L_toupper(L_Text_WaitKey() & 0xff);
        (*(u8*)&V.v1) = al;
        if ((al == 0x7f || al == 8) && si > 0) {
            si--;
            ts4 = 0x20;
            ts5 = ((MW(0x82f8) << 1) << 1) << 1;
            L_EGA_DrawTile8_Latch(MW(0x8282), ((MW(0x82f8) << 1) << 1) << 1, 0x20);
            MW(0x8282)--;
        }
        if ((*(u8*)&V.v1) >= 0x20 && (*(u8*)&V.v1) <= 0x7a && si < P.arg1) {
            MB(P.arg0 + si) = (*(u8*)&V.v1);
            si++;
            ts4 = (signed char)(*(u8*)&V.v1);
            ts5 = ((MW(0x82f8) << 1) << 1) << 1;
            ax = MW(0x8282);
            MW(0x8282)++;
            L_EGA_DrawTile8_Latch(ax, ((MW(0x82f8) << 1) << 1) << 1, (signed char)(*(u8*)&V.v1));
        }
    } while ((*(u8*)&V.v1) != 0x1b && (*(u8*)&V.v1) != 0xd);
    di = si;
    while (di < P.arg1) {
        MB(P.arg0 + di) = 0;
        di++;
    }
    if ((*(u8*)&V.v1) == 0xd) {
        return 1;
    }
    ax = 0;
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_RLEW_Compress_UNUSED @0x778e =====
int L_RLEW_Compress_UNUSED(p0,p1,p2,p3,p4,p5)
int p0; int p1; int p2; int p3; int p4; int p5; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; i16 arg4; i16 arg5; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3; P.arg4=p4; P.arg5=p5;
    struct { i16 ve; i16 vc; i16 va; i16 v8; i16 v6; i16 v4; i16 v2; } V = {0};



    int sv_di;
    int sv_si;
    int32_t t1;
    int32_t t2;
    int32_t t3;
    int32_t t4;
    int32_t t5;
    int32_t t6;
    int32_t t7;









    sv_di = di;
    sv_si = si;
    V.va = P.arg4;
    V.v8 = P.arg5;
    P.arg4 += 4;
    V.ve = P.arg0 + (LOWORD(((*(int32_t*)&P.arg2) + 1L) / 2L) << 1);
    V.vc = P.arg1;
    do {
        si = 1;
        t1 = (*(int32_t*)&P.arg0); /* far ptr */
        es = SEG(t1);
        V.v6 = WF(SEG(t1), OFF(t1));
        goto L77dd;
    L77dc:;
        si++;
    L77dd:;
        P.arg0 += 2;
        t2 = (*(int32_t*)&P.arg0); /* far ptr */
        es = SEG(t2);
        if (WF(SEG(t2), OFF(t2)) == V.v6 && (u16)P.arg0 < (u16)V.ve) goto L77dc;
        if ((u16)si > 3 || V.v6 == -258) {
            t3 = (*(int32_t*)&P.arg4); /* far ptr */
            es = SEG(t3);
            WF(SEG(t3), OFF(t3)) = 0xfefe;
            P.arg4 += 2;
            t4 = (*(int32_t*)&P.arg4); /* far ptr */
            es = SEG(t4);
            WF(SEG(t4), OFF(t4)) = si;
            P.arg4 += 2;
            t5 = (*(int32_t*)&P.arg4); /* far ptr */
            es = SEG(t5);
            WF(SEG(t5), OFF(t5)) = V.v6;
            P.arg4 += 2;
        } else {
            di = 1;
            while ((u16)di <= (u16)si) {
                t6 = (*(int32_t*)&P.arg4); /* far ptr */
                es = SEG(t6);
                WF(SEG(t6), OFF(t6)) = V.v6;
                P.arg4 += 2;
                di++;
            }
        }
    } while ((u16)P.arg0 < (u16)V.ve);
    V.v4 = LOWORD(((((uint32_t)P.arg4) - ((uint32_t)V.va)) / 2L) << 1);
    V.v2 = HIWORD(((((uint32_t)P.arg4) - ((uint32_t)V.va)) / 2L) << 1);
    t7 = (*(int32_t*)&V.va); /* far ptr */
    es = SEG(t7);
    WF(SEG(t7), OFF(t7)) = P.arg2;
    WF(SEG(t7), OFF(t7) + 2) = P.arg3;
    di = sv_di;
    si = sv_si;
    return (*(int32_t*)&V.v4);
}


// ===== L_RlewDecode @0x7881 =====
int L_RlewDecode(p0,p1,p2,p3)
int p0; int p1; int p2; int p3; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3;
    struct { i16 va; i16 v8; i16 v6; i16 v4; i16 v2; } V = {0};


    int sv_di;
    int sv_si;
    int32_t t1;
    int32_t t2;
    int32_t t3;
    int32_t t4;
    int32_t t5;
    int32_t t6;






    sv_di = di;
    sv_si = si;
    t1 = (*(int32_t*)&P.arg0); /* far ptr */
    es = SEG(t1);
    ax = WF(SEG(t1), OFF(t1) + 2);
    V.v4 = WF(SEG(t1), OFF(t1));
    V.v2 = ax;
    V.va = P.arg2 + (LOWORD((*(int32_t*)&V.v4) / 2L) << 1);
    V.v8 = P.arg3;
    P.arg0 += 4;
    do {
        t2 = (*(int32_t*)&P.arg0); /* far ptr */
        es = SEG(t2);
        di = WF(SEG(t2), OFF(t2));
        P.arg0 += 2;
        if (di != -258) {
            t3 = (*(int32_t*)&P.arg2); /* far ptr */
            es = SEG(t3);
            WF(SEG(t3), OFF(t3)) = di;
            P.arg2 += 2;
        } else {
            t4 = (*(int32_t*)&P.arg0); /* far ptr */
            es = SEG(t4);
            V.v6 = WF(SEG(t4), OFF(t4));
            P.arg0 += 2;
            t5 = (*(int32_t*)&P.arg0); /* far ptr */
            es = SEG(t5);
            di = WF(SEG(t5), OFF(t5));
            P.arg0 += 2;
            si = 1;
            while ((u16)si <= (u16)V.v6) {
                t6 = (*(int32_t*)&P.arg2); /* far ptr */
                es = SEG(t6);
                WF(SEG(t6), OFF(t6)) = di;
                P.arg2 += 2;
                si++;
            }
        }
        ax = P.arg2;
    } while ((u16)P.arg2 < (u16)V.va);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_WorldMap_ScanMarkers @0x805e =====
int L_WorldMap_ScanMarkers() {
    struct { i16 v2; } V = {0};
    int sv_di;
    int sv_si;
    int32_t t2;

    sv_di = di;
    sv_si = si;
    V.v2 = 0;
    /* leftover push si */
    /* leftover push di */
    while (V.v2 < 0x10) {
        MW((V.v2 * 6) - 0x58f2) = 0;
        V.v2++;
    }
    cx = 0;
    goto L8121;
L808a:;
    di = 0;
    while (di < MW(0x6c08)) {
        ax = ((cx * MW(0x6c08)) + di) << 1;
        t2 = MD(0x6c4c); /* far ptr */
        es = SEG(t2);
        ax = WF(SEG(t2), OFF(t2) + ax) & 0x7fff;
        si = WF(SEG(t2), OFF(t2) + ax) & 0x7fff;
        if (ax > 0 && ax < 0x100 && MW((ax * 6) - 0x58f8) == 0) {
            MW((si * 6) - 0x58f8) = di;
            MW((si * 6) - 0x58f6) = cx;
            MW((si * 6) - 0x58f4) = 0;
            if ((WF(es, MW(0x6c4c) + (((di + (cx * MW(0x6c08))) + 1) << 1)) & 0x7fff) == si) {
                di++;
                MW((si * 6) - 0x58f4) = 1;
            }
        }
        di++;
    }
    cx = cx + 1;
L8121:;
    ax = cx;
    if (cx >= MW(0x6c58)) return ax;
    goto L808a;
L812c:;
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_Text_ClearLastBox @0x8132 =====
int L_Text_ClearLastBox() {
    return L_Text_FillRect(MW(0x8358), MW(0x835e), MW(0x835a), MW(0x8360), 0);
}


// ===== L_TextPager_ScrollEGA @0xb640 =====
int L_TextPager_ScrollEGA(p0,p1,p2)
int p0; int p1; int p2; {
    struct { i16 arg0; i16 arg1; i16 arg2; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2;
    struct { i16 v2; } V = {0};

    int sv_di;
    int sv_si;
    int32_t t1;
    int32_t t2;
    int ts3;
    int ts4;

    sv_di = di;
    sv_si = si;
    V.v2 = ((((P.arg1 - P.arg0) << 1) << 1) << 1) * 0x30;
    ax = P.arg2;
    /* leftover push si */
    /* leftover push di */
    if (P.arg2) {
        if (ax == 1) goto Lb6cd;
        return ax;
    }
    ts3 = ah << 8 | 5;
    ts4 = 0x3ce;
    outportb(0x3ce, ah << 8 | 5);
    ts3 = ah << 8 | 1;
    ts4 = 0x3cf;
    outportb(0x3cf, ah << 8 | 1);
    ts3 = ah << 8 | 2;
    ts4 = 0x3c4;
    outportb(0x3c4, ah << 8 | 2);
    ts3 = ah << 8 | 0xf;
    ts4 = 0x3c5;
    outportb(0x3c5, ah << 8 | 0xf);
    P.arg0 = P.arg0 * 0x180;
    /* pushf */
    ts3 = si;
    ts4 = di;
    di = P.arg0;
    si = di;
    si = si + 0x180;
    es = MW(0x35b4);
    ax = MW(0x35b4);
    t1 = DS;
    DS = ax;
    cx = V.v2;
    memcpy_repb(di, si, cx);
    DS = t1;
    di = ts4;
    si = ts3;
    /* popf */
    return ax;
Lb6cd:;
    ts3 = ah << 8 | 5;
    ts4 = 0x3ce;
    outportb(0x3ce, ah << 8 | 5);
    ts3 = ah << 8 | 1;
    ts4 = 0x3cf;
    outportb(0x3cf, ah << 8 | 1);
    ts3 = ah << 8 | 2;
    ts4 = 0x3c4;
    outportb(0x3c4, ah << 8 | 2);
    ts3 = ah << 8 | 0xf;
    ts4 = 0x3c5;
    outportb(0x3c5, ah << 8 | 0xf);
    P.arg1 = P.arg1 * 0x180;
    P.arg1 += 0x17f;
    /* pushf */
    ts3 = si;
    ts4 = di;
    di = P.arg1;
    si = di;
    si = si - 0x180;
    es = MW(0x35b4);
    ax = MW(0x35b4);
    t2 = DS;
    DS = ax;
    cx = V.v2;
    memcpy_repb(di, si, cx);
    DS = t2;
    di = ts4;
    si = ts3;
    /* popf */
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L_TileCache_Invalidate @0xb736 =====
int L_TileCache_Invalidate() {
    int sv_di;
    sv_di = di;
    es = DS;
    di = 0x5b16;
    cx = 0x126;
    ax = 0xffff;
    memset_repw(di, ax, cx);
    di = 0x5db8;
    cx = 0x126;
    memset_repw(di, ax, cx);
    di = sv_di;
    return ax;
}


// ===== L_EGA_ComputePageOffsets @0xb87c =====
int L_EGA_ComputePageOffsets() {
    MW(0x35ba) = 4;
    if (!(!(MW(0x6ed0) & 0x800))) {
        MW(0x35ba)++;
    }
    MW(0x35b8) = MW(0x6ed1) & 7;
    MW(0x5652) ^= 1;
    if ((MW(0x5652) & 1) == 0) {
        cx = 0xa000;
    } else {
        MW(0x35ba) += 0x3000;
        cx = 0xa300;
    }
    MW(0x35b2) = cx;
    MW(0x35ba) += ((MW(0x6ed5) & 0xf) * 0x30) + 0x600;
    ax = (((u16)(((u16)(((u16)(((u16)MW(0x35ba)) >> 1)) >> 1)) >> 1)) >> 1) + 0xa000;
    MW(0x35b4) = ax;
    return ax;
}


// ===== L_EGA_DrawTile8_Latch @0xb8e0 =====
int L_EGA_DrawTile8_Latch(p0,p1,p2)
int p0; int p1; int p2; {
    struct { i16 arg0; i16 arg1; i16 arg2; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2;

    int sv_di;
    int sv_si;
    sv_di = di;
    sv_si = si;
    es = MW(0x35b4);
    di = P.arg0;
    di = di + MW((P.arg1 << 1) + 0x380c);
    CLI();
    outport(0x3ce, 0x105);
    si = P.arg2;
    si <<= 1;
    si <<= 1;
    si <<= 1;
    DS = MW(0x8300);
    cx = di;
    ax = MW(0x8300);
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    movsb(di, si);
    di = di + 0x2f;
    STI();
    DS = SS;
    di = sv_di;
    si = sv_si;
    return MK_LONG(0x3ce, SS);
}


// ===== L_Keyboard_GetKey @0xbdce =====
int L_Keyboard_GetKey(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    if (!P.arg0) {
        do {
            bl = MB(0x50ac);
            if (MB(0x50ac) >= 0) goto Lbdda;
            MB(0x50ac) = bl & 0x7f;
            al = MB((bh << 8 | bl & 0x7f) + 0x50b0);
            ah = bl & 0x7f;
        } while (!MB((bh << 8 | bl & 0x7f) + 0x50b0));
        MB(0x50ae) = al;
        return ax;
    }
    bl = MB(0x50ac);
    if ((MB(0x50ac) & 0x80) == 0) {
        return 0;
    }
    MB(0x50ae) = MB((bh << 8 | bl & 0x7f) + 0x50b0);
    al = MB((bh << 8 | bl & 0x7f) + 0x50b0);
    ah = bl & 0x7f;
    if (MB((bh << 8 | bl & 0x7f) + 0x50b0) != 0) return ax;
    ah = 0;
Lbe18:;
    return ax;
    Lbdda:;
}

// ===== L_Sound_Play @0xbef3 =====
int L_Sound_Play(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int sv_si;
    sv_si = si;
    if (!(!(MW(0x5151) & 0xffff))) {
        si = sv_si;
        return ax;
    }
    si = P.arg0;
    si <<= 1;
    si <<= 1;
    si <<= 1;
    si <<= 1;
    es = MW(0x5141);
    al = BF(MW(0x5141), si + 2);
    ah = HIBYTE(MW(0x5141));
    if ((u16)BF(MW(0x5141), si + 2) < (u16)MB(0x514c)) return ax;
    MB(0x514c) = al;
    ax = WF(es, si);
    MW(0x514a) = ax;
Lbf27:;
    si = sv_si;
    return ax;
}


// ===== L_Sound_Stop @0xbf2a =====
int L_Sound_Stop() {
    if (!(!(MW(0x5151) & 0xffff))) {
        return ax;
    }
    MW(0x514a) = 0;
    MB(0x514c) = LOBYTE(0);
    CLI();
    al = inportb(0x61);
    outportb(0x61, al & 0xfd);
    STI();
    return HIBYTE(0) << 8 | al & 0xfd;
}


// ===== L_Sound_Push @0xbf44 =====
int L_Sound_Push() {
    if (!(!(MW(0x5151) & 0xffff))) {
        return ax;
    }
    MW(0x514d) = MW(0x514a);
    MB(0x514f) = MB(0x514c);
    MB(0x5150) = MB(0x5149);
    return L_Sound_Stop();
}


// ===== L_Sound_Pop @0xbf63 =====
int L_Sound_Pop() {
    if (!(!(MW(0x5151) & 0xffff))) {
        return ax;
    }
    MW(0x514a) = MW(0x514d);
    MB(0x514c) = MB(0x514f);
    MB(0x5149) = MB(0x5150);
    return HIBYTE(MW(0x514d)) << 8 | MB(0x5150);
}


// ===== L_Rnd_Next @0xc0ce =====
int L_Rnd_Next() {
    bx = (MW(0x5157) + 1) & 0xff;
    MW(0x5157) = bx;
    return MB(bx + 0x5159);
}


// ===== L_WaitVBL @0xc0e2 =====
int L_WaitVBL(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int sv_si;
    sv_si = si;
    cx = P.arg0;
    do {
        al = inportb(0x3da);
        if ((al & 8) != 0) goto Lc0e9;
        do {
            al = inportb(0x3da);
        } while (!(al & 8));
        bx = 0x64;
        do {
            bx = bx - 1;
        } while ((bx - 1) != 0);
        cx--;
    } while (cx != 0);
    si = sv_si;
    return ax;
    Lc0e9:;
}

// ===== L_EGA_SelectPlane @0xc104 =====
int L_EGA_SelectPlane(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    CLI();
    outport(0x3ce, 5);
    outport(0x3ce, ((*(u8*)&P.arg0) << 8 | 4));
    outport(0x3c4, (1 << (*(u8*)&P.arg0) << 8 | 2));
    STI();
    return MK_LONG(0x3c4, 1 << (*(u8*)&P.arg0) << 8 | 2);
}


// ===== L_Video_ProbeCRTC @0xc27d =====
int L_Video_ProbeCRTC() {
    outportb(dx, 0xf);
    al = inportb(dx + 1);
    outportb(dx + 1, 0x66);
    cx = 0x100;
    dx = dx + 1;
    do {
        cx--;
    } while (cx != 0);
    al = inportb(dx);
    /* xchg al,ah */
    outportb(dx, al);
    if (ah == 0x66) return ax;
    /* stc */
Lc296:;
    return ax;
}


// ===== L_Rle_Init @0xc2d5 =====
int L_Rle_Init() {
    MW(0x52c5) = 0;
    MW(0x52c8) = 1;
    MW(0x52c1) = 0;
    MW(0x52c3) = 0;
    si = MW(0x52bb);
    di = MW(0x52b9);
    return ax;
}


// ===== L_atexit @0xc587 =====
int L_atexit(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    if (MW(0x52cc) == 0x20) {
        return 1;
    }
    MW((MW(0x52cc) << 1) - 0x5400) = P.arg0;
    MW(0x52cc)++;
    ax = 0;
    return ax;
}


// ===== L_printf @0xc935 =====
int L_printf(p0,p1)
int p0; int p1; {
    struct { i16 arg0; i16 arg1; } P; P.arg0=p0; P.arg1=p1;

    return L___vprinter();
}


// ===== L_toupper @0xcd36 =====
int L_toupper(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    dx = P.arg0;
    if (P.arg0 == -1) {
        return 0xffff;
    }
    if (!(!(MB(dl + 0x52cf) & 8))) {
        return dl - 32;
    }
    al = dl;
    ah = 0;
    return ax;
}


// ===== L_brk @0xcefe =====
int L_brk(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    ax = P.arg0;
    if ((u16)P.arg0 < (u16)(sp - 0x200)) {
        MW(0x9a) = ax;
        return 0;
    }
    MW(0x92) = 8;
    ax = 0xffff;
    return ax;
}


// ===== L___brk @0xcf51 =====
int L___brk(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    return L_brk(P.arg0);
}


// ===== L_CritErr_Return @0xd20f =====
int L_CritErr_Return(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int ts0;
    int ts1;
    di = ts1;
    si = ts0;
    es = DS;
    return ax; /* iret */
}


// ===== L___IOerror @0xd296 =====
int L___IOerror(p0)
int p0; {
    struct { i16 arg0; } P; P.arg0=p0;

    int sv_si;
    sv_si = si;
    si = P.arg0;
    if (si < 0) goto Ld2b6;
    if (si <= 0x58) goto Ld2a9;
Ld2a6:;
    si = 0x57;
Ld2a9:;
    MW(0x5566) = si;
    si = (signed char)MB(si + 0x5568);
    goto Ld2c7;
Ld2b6:;
    ax = -si;
    si = -si;
    if (ax > 0x23) goto Ld2a6;
    MW(0x5566) = 0xffff;
Ld2c7:;
    MW(0x92) = si;
    si = sv_si;
    return 0xffff;
}


// ===== L___vprinter @0xd7a9 =====
int L___vprinter() {
    struct { i16 v16; i16 v14; i16 v12; } V = {0};



    V.v12 = 0;
    V.v14 = 0x50;
    V.v16 = 0;
    goto Ld809;
    Ld809:;
}

// ===== L__vprinter_putc @0xd7d0 =====
int L__vprinter_putc() {
    struct { i16 v14; } V = {0};

    BF(ss, di) = al;
    di++;
    (*(u8*)&V.v14)--;
    if ((*(u8*)&V.v14) <= 0) goto Ld808;
    Ld808:;
}

// ===== L__far_split_block @0xdec1 =====
int L__far_split_block() {
    MW(0) -= ax;
    bx = dx;
    dx = dx + MW(0);
    DS = dx;
    MW(0) = ax;
    MW(2) = bx;
    DS = dx + MW(0);
    MW(2) = dx;
    return 4;
}


// ===== L__F_SCOPY_prolog @0xe29b =====
int L__F_SCOPY_prolog() {
    es = DS;
    /* leftover push CS */
    /* leftover push ES */
}


// ===== L_F_SCOPY @0xe29e =====
int L_F_SCOPY(p0,p1,p2,p3,p4)
int p0; int p1; int p2; int p3; int p4; {
    struct { i16 arg0; i16 arg1; i16 arg2; i16 arg3; i16 arg4; } P; P.arg0=p0; P.arg1=p1; P.arg2=p2; P.arg3=p3; P.arg4=p4;


    int sv_di;
    int sv_si;
    int32_t t1;
    int32_t t2;
    sv_di = di;
    sv_si = si;
    t1 = (*(int32_t*)&P.arg1); /* far ptr */
    si = OFF(t1);
    t2 = (*(int32_t*)&P.arg3); /* far ptr */
    di = OFF(t2);
    es = SEG(t2);
    cx = ((u16)cx) >> 1;
    memcpy_repw(di, si, cx);
    cx = cx + cx /*carry*/;
    memcpy_repb(di, si, cx);
    di = sv_di;
    si = sv_si;
    return ax;
}


// ===== L__near_unlink_free @0xe380 =====
int L__near_unlink_free() {
    di = MW(bx + 6);
    if (bx != di) {
        MW(0x562e) = di;
        si = MW(bx + 4);
        MW(di + 4) = si;
        MW(si + 6) = di;
        return ax;
    }
    MW(0x562e) = 0;
    return ax;
}


// ===== L__near_insert_free @0xe39c =====
int L__near_insert_free() {
    si = MW(0x562e);
    if (si) {
        di = MW(si + 6);
        MW(si + 6) = bx;
        MW(di + 4) = bx;
        MW(bx + 6) = di;
        MW(bx + 4) = si;
        return ax;
    }
    MW(0x562e) = bx;
    MW(bx + 4) = bx;
    MW(bx + 6) = bx;
    return ax;
}


// ===== L__fpnormalize @0xe57a =====
int L__fpnormalize() {
    int ts0;
    ts0 = cx;
    ax = (bl << 8 | al) & 0xf;
    bx = (HIBYTE(((u16)bx) >> 4) << 8 | bl) & 0xf;
    if ((dx + (((u16)ax) >> 4)) != (ts0 + (((u16)bx) >> 4))) return ax;
Le59a:;
    return ax;
}


#undef int
int32_t w_ResetDrawLists(i16*a){  L_ResetDrawLists(); return ax; }
int32_t w_Joystick_ReadButtons(i16*a){ int32_t r= L_Joystick_ReadButtons(); return r; }
int32_t w_Tank_Pause(i16*a){  L_Tank_Pause(); return ax; }
int32_t w_Tank_Contact(i16*a){  L_Tank_Contact(); return ax; }
int32_t w_IceChunk_Think(i16*a){  L_IceChunk_Think(); return ax; }
int32_t w_Obj_Remove(i16*a){  L_Obj_Remove(); return ax; }
int32_t w_Obj_AccelX(i16*a){  L_Obj_AccelX(a[0]); return ax; }
int32_t w_Obj_AccelY(i16*a){  L_Obj_AccelY(a[0],a[1]); return ax; }
int32_t w_Obj_ApplyGravity(i16*a){  L_Obj_ApplyGravity(); return ax; }
int32_t w_Nop(i16*a){  L_Nop(); return ax; }
int32_t w_ShotSplash_Think(i16*a){  L_ShotSplash_Think(); return ax; }
int32_t w_Demo_StartRecording(i16*a){ int32_t r= L_Demo_StartRecording(); return r; }
int32_t w_EGA_SetLineCompare(i16*a){  L_EGA_SetLineCompare(a[0]); return ax; }
int32_t w_EGA_SetScreenStart(i16*a){ int32_t r= L_EGA_SetScreenStart(a[0]); return r; }
int32_t w_Text_DrawBox(i16*a){ int32_t r= L_Text_DrawBox(a[0],a[1],a[2],a[3]); return r; }
int32_t w_Text_DrawBoxCentered(i16*a){  L_Text_DrawBoxCentered(a[0],a[1]); return ax; }
int32_t w_Text_FillRect(i16*a){  L_Text_FillRect(a[0],a[1],a[2],a[3],a[4]); return ax; }
int32_t w_Text_OpenBoxAnimated(i16*a){  L_Text_OpenBoxAnimated(a[0],a[1]); return ax; }
int32_t w_Text_GrowWidth(i16*a){  L_Text_GrowWidth(a[0],a[1]); return ax; }
int32_t w_Text_GrowHeight(i16*a){  L_Text_GrowHeight(a[0],a[1]); return ax; }
int32_t w_Text_DrawStyledFrame(i16*a){  L_Text_DrawStyledFrame(a[0],a[1],a[2],a[3],a[4]); return ax; }
int32_t w_Text_WaitKey(i16*a){  L_Text_WaitKey(); return ax; }
int32_t w_Text_InputLine(i16*a){ int32_t r= L_Text_InputLine(a[0],a[1]); return r; }
int32_t w_RLEW_Compress_UNUSED(i16*a){ int32_t r= L_RLEW_Compress_UNUSED(a[0],a[1],a[2],a[3],a[4],a[5]); return r; }
int32_t w_RlewDecode(i16*a){  L_RlewDecode(a[0],a[1],a[2],a[3]); return ax; }
int32_t w_WorldMap_ScanMarkers(i16*a){  L_WorldMap_ScanMarkers(); return ax; }
int32_t w_Text_ClearLastBox(i16*a){  L_Text_ClearLastBox(); return ax; }
int32_t w_TextPager_ScrollEGA(i16*a){  L_TextPager_ScrollEGA(a[0],a[1],a[2]); return ax; }
int32_t w_TileCache_Invalidate(i16*a){  L_TileCache_Invalidate(); return ax; }
int32_t w_EGA_ComputePageOffsets(i16*a){  L_EGA_ComputePageOffsets(); return ax; }
int32_t w_EGA_DrawTile8_Latch(i16*a){ int32_t r= L_EGA_DrawTile8_Latch(a[0],a[1],a[2]); return r; }
int32_t w_Keyboard_GetKey(i16*a){ int32_t r= L_Keyboard_GetKey(a[0]); return r; }
int32_t w_Sound_Play(i16*a){  L_Sound_Play(a[0]); return ax; }
int32_t w_Sound_Stop(i16*a){ int32_t r= L_Sound_Stop(); return r; }
int32_t w_Sound_Push(i16*a){ int32_t r= L_Sound_Push(); return r; }
int32_t w_Sound_Pop(i16*a){ int32_t r= L_Sound_Pop(); return r; }
int32_t w_Rnd_Next(i16*a){ int32_t r= L_Rnd_Next(); return r; }
int32_t w_WaitVBL(i16*a){  L_WaitVBL(a[0]); return ax; }
int32_t w_EGA_SelectPlane(i16*a){ int32_t r= L_EGA_SelectPlane(a[0]); return r; }
int32_t w_Video_ProbeCRTC(i16*a){  L_Video_ProbeCRTC(); return ax; }
int32_t w_Rle_Init(i16*a){  L_Rle_Init(); return ax; }
int32_t w_atexit(i16*a){ int32_t r= L_atexit(a[0]); return r; }
int32_t w_printf(i16*a){  L_printf(a[0],a[1]); return ax; }
int32_t w_toupper(i16*a){ int32_t r= L_toupper(a[0]); return r; }
int32_t w_brk(i16*a){ int32_t r= L_brk(a[0]); return r; }
int32_t w___brk(i16*a){ int32_t r= L___brk(a[0]); return r; }
int32_t w_CritErr_Return(i16*a){  L_CritErr_Return(a[0]); return ax; }
int32_t w___IOerror(i16*a){ int32_t r= L___IOerror(a[0]); return r; }
int32_t w___vprinter(i16*a){  L___vprinter(); return ax; }
int32_t w__vprinter_putc(i16*a){  L__vprinter_putc(); return ax; }
int32_t w__far_split_block(i16*a){ int32_t r= L__far_split_block(); return r; }
int32_t w__F_SCOPY_prolog(i16*a){  L__F_SCOPY_prolog(); return ax; }
int32_t w_F_SCOPY(i16*a){  L_F_SCOPY(a[0],a[1],a[2],a[3],a[4]); return ax; }
int32_t w__near_unlink_free(i16*a){  L__near_unlink_free(); return ax; }
int32_t w__near_insert_free(i16*a){  L__near_insert_free(); return ax; }
int32_t w__fpnormalize(i16*a){  L__fpnormalize(); return ax; }
int32_t call_by_index(int i,i16*a){ switch(i){ case 0: return w_ResetDrawLists(a); case 1: return w_Joystick_ReadButtons(a); case 2: return w_Tank_Pause(a); case 3: return w_Tank_Contact(a); case 4: return w_IceChunk_Think(a); case 5: return w_Obj_Remove(a); case 6: return w_Obj_AccelX(a); case 7: return w_Obj_AccelY(a); case 8: return w_Obj_ApplyGravity(a); case 9: return w_Nop(a); case 10: return w_ShotSplash_Think(a); case 11: return w_Demo_StartRecording(a); case 12: return w_EGA_SetLineCompare(a); case 13: return w_EGA_SetScreenStart(a); case 14: return w_Text_DrawBox(a); case 15: return w_Text_DrawBoxCentered(a); case 16: return w_Text_FillRect(a); case 17: return w_Text_OpenBoxAnimated(a); case 18: return w_Text_GrowWidth(a); case 19: return w_Text_GrowHeight(a); case 20: return w_Text_DrawStyledFrame(a); case 21: return w_Text_WaitKey(a); case 22: return w_Text_InputLine(a); case 23: return w_RLEW_Compress_UNUSED(a); case 24: return w_RlewDecode(a); case 25: return w_WorldMap_ScanMarkers(a); case 26: return w_Text_ClearLastBox(a); case 27: return w_TextPager_ScrollEGA(a); case 28: return w_TileCache_Invalidate(a); case 29: return w_EGA_ComputePageOffsets(a); case 30: return w_EGA_DrawTile8_Latch(a); case 31: return w_Keyboard_GetKey(a); case 32: return w_Sound_Play(a); case 33: return w_Sound_Stop(a); case 34: return w_Sound_Push(a); case 35: return w_Sound_Pop(a); case 36: return w_Rnd_Next(a); case 37: return w_WaitVBL(a); case 38: return w_EGA_SelectPlane(a); case 39: return w_Video_ProbeCRTC(a); case 40: return w_Rle_Init(a); case 41: return w_atexit(a); case 42: return w_printf(a); case 43: return w_toupper(a); case 44: return w_brk(a); case 45: return w___brk(a); case 46: return w_CritErr_Return(a); case 47: return w___IOerror(a); case 48: return w___vprinter(a); case 49: return w__vprinter_putc(a); case 50: return w__far_split_block(a); case 51: return w__F_SCOPY_prolog(a); case 52: return w_F_SCOPY(a); case 53: return w__near_unlink_free(a); case 54: return w__near_insert_free(a); case 55: return w__fpnormalize(a); } return 0; }
void set_regs(i16*r){ DS=ES=SS=DSEG; sp=(i16)0xFF00; ax=r[0];bx=r[1];cx=r[2];dx=r[3];si=r[4];di=r[5];bp=r[6];carry=0; }
i16 get_ax(void){return ax;} i16 get_dx(void){return dx;} i16 get_si(void){return si;} i16 get_di(void){return di;}
