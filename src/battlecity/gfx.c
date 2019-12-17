

#include "segalib.h"
#include "game.h"

u16 text_attrib;

void gInit() {

    VDP_CTRL16 = VDP_REG_MODE1 | M1_MDMODE;
    VDP_CTRL16 = VDP_REG_MODE2 | M2_DMA_ON | M2_MDMODE | M2_VINT_ON;
    VDP_CTRL16 = VDP_REG_APLAN | APLAN_ADDR(APLAN);
    VDP_CTRL16 = VDP_REG_WPLAN | WPLAN_ADDR(WPLAN);
    VDP_CTRL16 = VDP_REG_BPLAN | BPLAN_ADDR(BPLAN);
    VDP_CTRL16 = VDP_REG_SLIST | SLIST_ADDR(SLIST);
    VDP_CTRL16 = VDP_REG_MODE3 | 0;
    VDP_CTRL16 = VDP_REG_MODE4 | (SCREEN_W == 40 ? M4_HRES_40 : 0);
    VDP_CTRL16 = VDP_REG_PSIZE | PSIZE(PLAN_W, PLAN_H);
    text_attrib = 0;
}

void gScreenOff() {

    VDP_CTRL16 = VDP_REG_MODE2 | M2_DMA_ON | M2_MDMODE | M2_VINT_ON;
}

void gScreenOn() {

    VDP_CTRL16 = VDP_REG_MODE2 | M2_DMA_ON | M2_MDMODE | M2_VINT_ON | M2_DISP_ON;
}

void gSetPalColor(u16 val, u16 palnum, u16 colornum) {
    VDP_CTRL32 = VDP_CRAM_WRITE(palnum * 32 + colornum * 2);
    VDP_DATA16 = val;
}

void gSetPal(void *src, u16 palnum) {

    u16 i = 8;
    u32 *ptr = (u32 *) src;

    VDP_CTRL32 = VDP_CRAM_WRITE(palnum * 32);
    while (i--)VDP_DATA32 = *ptr++;
}

void gWriteVram(void *src, u16 dst, u16 len) {

    VDP_CTRL16 = VDP_REG_DMA_LEN_LO | ((len >> 1) & 0xff);
    VDP_CTRL16 = VDP_REG_DMA_LEN_HI | ((len >> 9) & 0xff);

    VDP_CTRL16 = VDP_REG_DMA_SRC_LO | (((u32) src >> 1) & 0xff);
    VDP_CTRL16 = VDP_REG_DMA_SRC_MI | (((u32) src >> 9) & 0xff);
    VDP_CTRL16 = VDP_REG_DMA_SRC_HI | (((u32) src >> 17) & 0xff);

    VDP_CTRL32 = VDP_VRAM_DMA(dst);
}

void gSrollV(u16 plan, u16 val) {

    if (plan == APLAN) {
        VDP_CTRL32 = VDP_VSRAM_WRITE(0);
    } else {
        VDP_CTRL32 = VDP_VSRAM_WRITE(2);
    }
    VDP_DATA16 = val;
}

void gSrollH(u16 plan, u16 val) {

    plan = plan == APLAN ? HSCRL : HSCRL + 2;

    VDP_CTRL32 = VDP_VRAM_WRITE(plan);
    VDP_DATA16 = val;
}

void gVsync() {

    vb_flag = 0;
    while (vb_flag == 0);
}

void gFillRect(u16 plan, u16 tile, u16 x, u16 y, u16 w, u16 h) {

    u16 i;
    u16 addr = plan + (x + y * PLAN_W) * 2;

    while (h--) {
        VDP_CTRL32 = VDP_VRAM_WRITE(addr);
        addr += PLAN_W * 2;
        for (i = 0; i < w; i++)VDP_DATA16 = tile;
    }
}

u16 g_str_y;

void gDrawString(u8 *str, u8 x, u8 y) {

    gSetXY(x, y);
    while (*str != 0)VDP_DATA16 = *str++ | text_attrib;
}

void gAppendString(u8 *str) {

    while (*str != 0)VDP_DATA16 = *str++ | text_attrib;
}

void gAppendHex8(u8 num) {

    u8 val;
    u8 buff[3];
    buff[2] = 0;

    val = num >> 4;
    if (val > 9)val += 7;
    buff[0] = val + '0';
    val = num & 0x0f;
    if (val > 9)val += 7;
    buff[1] = val + '0';

    gAppendString(buff);
}

void gDrawStringCX(u8 *str, u8 y) {

    u16 str_len = 0;
    u8 *ptr = str;
    while (*ptr++ != 0)str_len++;
    gDrawString(str, (SCREEN_W - str_len) / 2, y);

}

void gSetXY(u8 x, u8 y) {
    g_str_y = y;
    u16 addr = APLAN + x * 2 + y * PLAN_W * 2;
    VDP_CTRL32 = VDP_VRAM_WRITE(addr);
}

void gConsPrint(u8 *str) {
    g_str_y++;
    if (g_str_y >= SCREEN_H) {

        g_str_y = SCREEN_H - 1;
        gFillRect(APLAN, 0, 0, g_str_y, SCREEN_W, 1);
    }
    gDrawString(str, 2, g_str_y);
}

u16 *g_str_ptr;

void gSetXY_FB(u8 x, u8 y, u16 frame_buff[]) {

    g_str_ptr = &frame_buff[x + y * PLAN_W];
}

void gDrawStringFB(u8 *str, u16 frame_buff[], u16 x, u16 y) {

    gSetXY_FB(x, y, frame_buff);
    while (*str != 0)*g_str_ptr++ = *str++ | text_attrib;
}

void gAppendNumFB(u32 num) {

    u16 i;
    u8 buff[11];
    u8 *str = (u8 *) & buff[10];


    *str = 0;
    if (num == 0)*--str = '0';
    for (i = 0; num != 0; i++) {
        *--str = num % 10 + '0';
        num /= 10;
    }

    gAppendStringFB(str);
}

void gDrawNumFB(u32 num, u16 frame_buff[], u16 x, u16 y, u16 align) {

    u16 i;
    u8 buff[11];
    u8 *str = (u8 *) & buff[10];
    u16 len = 0;


    *str = 0;
    if (num == 0) {
        *--str = '0';
        len = 1;
    }
    for (i = 0; num != 0; i++) {
        *--str = num % 10 + '0';
        num /= 10;
        len++;
    }

    if (align)x -= (len - 1);
    gDrawStringFB(str, frame_buff, x, y);

}

void gSetTextAttrib(u16 attrib) {
    text_attrib = attrib;
}

void gAppendStringFB(u8 *str) {

    while (*str != 0)*g_str_ptr++ = *str++ | text_attrib;
}
