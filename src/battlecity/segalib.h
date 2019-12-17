
#ifndef SEGALIB_H
#define	SEGALIB_H


#define s8      char
#define s16     short
#define s32     long

#define u8      unsigned char
#define u16     unsigned short
#define u32     unsigned long
#define u64     unsigned long long

#define vu8     volatile unsigned char
#define vu16    volatile unsigned short
#define vu32    volatile unsigned long
#define vu64    volatile unsigned long long

//****************************************************************************** VDP
#define SCREEN_W 32
#define SCREEN_H 28
#define PLAN_W 32
#define PLAN_H 64

#define APLAN           0xC000
#define BPLAN           0xE000
#define WPLAN           0xF000
#define HSCRL           0xF800
#define SLIST           0xFC00

#define VDP_DATA16 *((vu16 *)0xC00000)
#define VDP_DATA32 *((vu32 *)0xC00000)
#define VDP_CTRL16 *((vu16 *)0xC00004)
#define VDP_CTRL32 *((vu32 *)0xC00004)
#define VDP_HVCNTR *((vu16 *)0xC00008)
#define VDP_REGION *((vu16*) 0xa10000)

#define VDP_VSYNC \
while ((VDP_CTRL16 & VDP_STAT_VBL) != 0); \
while ((VDP_CTRL16 & VDP_STAT_VBL) == 0) 

#define REGION_J 0x00
#define REGION_U 0x80
#define REGION_E 0xC0
#define REGION_J_PAL 0x40


#define VDP_STAT_PALMODE        (1 << 0)
#define VDP_STAT_DMABUSY        (1 << 1)
#define VDP_STAT_HBL            (1 << 2)
#define VDP_STAT_VBL            (1 << 3)
#define VDP_STAT_ODDFRAME       (1 << 4)
#define VDP_STAT_SPRCOL         (1 << 5)
#define VDP_STAT_SPROVF         (1 << 6)
#define VDP_STAT_VBL_PND        (1 << 7)
#define VDP_STAT_FIFOFULL       (1 << 8)
#define VDP_STAT_FIFOEMPTY      (1 << 9)

#define BUTTON_DPAD (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT)
#define BUTTON_UP       0x0001
#define BUTTON_DOWN     0x0002
#define BUTTON_LEFT     0x0004
#define BUTTON_RIGHT    0x0008
#define BUTTON_A        0x0040
#define BUTTON_B        0x0010
#define BUTTON_C        0x0020
#define BUTTON_START    0x0080
#define BUTTON_X        0x0100
#define BUTTON_Y        0x0200
#define BUTTON_Z        0x0400
#define BUTTON_MODE     0x0800

#define VDP_VRAM_DMA(adr)       ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
#define VDP_CRAM_DMA(adr)       ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
#define VDP_VSRAM_DMA(adr)      ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x90)

#define VDP_VRAM_WRITE(adr)     ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define VDP_CRAM_WRITE(adr)     ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define VDP_VSRAM_WRITE(adr)    ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

#define VDP_VRAM_READ(adr)      ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define VDP_CRAM_READ(adr)      ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x20)
#define VDP_VSRAM_READ(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)



#define VDP_SET_REG(reg, val)   (VDP_CTRL16 = reg | val)
#define VDP_REG_MODE1 0x8000
#define VDP_REG_MODE2 0x8100
#define VDP_REG_APLAN 0x8200
#define VDP_REG_WPLAN 0x8300
#define VDP_REG_BPLAN 0x8400
#define VDP_REG_SLIST 0x8500
#define VDP_REG_BGCOL 0x8700
#define VDP_REG_HINT  0x8A00
#define VDP_REG_MODE3 0x8B00
#define VDP_REG_MODE4 0x8C00
#define VDP_REG_HSCRL 0x8D00
#define VDP_REG_AINCR 0x8F00
#define VDP_REG_PSIZE 0x9000
#define VDP_REG_WPOSX 0x9100
#define VDP_REG_WPOSY 0x9200
#define VDP_REG_DMA_LEN_LO 0x9300
#define VDP_REG_DMA_LEN_HI 0x9400
#define VDP_REG_DMA_SRC_LO 0x9500
#define VDP_REG_DMA_SRC_MI 0x9600
#define VDP_REG_DMA_SRC_HI 0x9700

#define VDP_DMA_MODE_TO_VR0 0x00
#define VDP_DMA_MODE_TO_VR8 0x40
#define VDP_DMA_MODE_VR_COPY 0x80
#define VDP_DMA_MODE_VR_FILL 0xC0

#define PSIZE(w, h) ((w - 1) >> 5) | ((h - 1) >> 1 & 0x30)


#define APLAN_ADDR(addr) ((addr >> 10) & 0x38)
#define WPLAN_ADDR(addr) ((addr >> 10) & 0x3C)
#define BPLAN_ADDR(addr) ((addr >> 13) & 0x07)
#define SLIST_ADDR(addr) ((addr >>  9) & 0x7F)
#define HSCRL_ADDR(addr) ((addr >> 10) & 0x3F)

#define M1_HVCN_OFF     0x02
#define M1_MDMODE       0x04
#define M1_HINT_ON      0x10

#define M2_MDMODE       0x04
#define M2_VRES_30      0x08
#define M2_DMA_ON       0x10
#define M2_VINT_ON      0x20
#define M2_DISP_ON      0x40

#define M3_HSCRL_COL    0x02
#define M3_HSCRL_PIX    0x03
#define M3_VSCRL_COL    0x04
#define M3_EINT_ON      0x08

#define M4_HRES_40      0x81 /*horisontal resolution*/
#define M4_SHAD_HL      0x08 /*Shadow Highlight*/
#define M4_ILASE1X      0x02 /*interlace*/
#define M4_ILASE2X      0x06 /*interlce 2x*/

#define TILE_HFL(val) ((val) << 11)
#define TILE_VFL(val) ((val) << 12)
#define TILE_PAL(val) ((val) << 13)
#define TILE_PRI(val) ((val) << 15)

#define SPR_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

typedef struct {
    u16 y;
    u16 size;
    u16 tile;
    u16 x;
} Sprite;


//****************************************************************************** JOY
#define JOY_DATA1 *((vu8 *) 0xa10003)
#define JOY_DATA2 *((vu8 *) 0xa10005)
#define JOY_DATA3 *((vu8 *) 0xa10007)
#define JOY_CTRL1 *((vu8 *) 0xa10009)
#define JOY_CTRL2 *((vu8 *) 0xa1000b)
#define JOY_CTRL3 *((vu8 *) 0xa1000d)
#define JOY_DIR_BIT 0x40
//****************************************************************************** Z80 the king
#define Z80_HALT *((vu16 *)0xA11100)
#define Z80_RSET *((vu16 *)0xA11200)

#define Z80_BUSREQ_ON  Z80_HALT = 0x0100
#define Z80_BUSREQ_OFF Z80_HALT = 0x0000
#define Z80_IS_BUSREQ_OFF (Z80_HALT & 0x0100)

#define Z80_RESET_ON  Z80_RSET = 0
#define Z80_RESET_OFF Z80_RSET = 0x0100

#define Z80_RAM         ((u8*)0xA00000)
#define Z80_YM2612      ((u8*)0xA04000)
#define Z80_YM_A0       *((vu8*)0xA04000)
#define Z80_YM_D0       *((vu8*)0xA04001)
#define Z80_YM_A1       *((vu8*)0xA04002)
#define Z80_YM_D1       *((vu8*)0xA04003)
#define Z80_BANK        *((vu8*)0xA06000)
#define Z80_PSG         *((vu8*)0xA07F11)

#define YM2612_IS_BUSY (Z80_YM_A0 & 0x80)

#define PSG *((vu8 *)0xC00011)


#endif
