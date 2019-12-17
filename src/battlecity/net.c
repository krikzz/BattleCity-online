

#include "segalib.h"
#include "game.h"


#define SSF_CTRL_P 0x8000 //register accesss protection bit. should be set, otherwise register will ignore any attempts to write
#define SSF_CTRL_X 0x4000 //32x mode
#define SSF_CTRL_W 0x2000 //ROM memory write protection
#define SSF_CTRL_L 0x1000 //led

#define REG_CTRL *((vu16 *)0xA130F0)
#define REG_USB  *((vu16 *)0xA130E2)
#define REG_STE  *((vu16 *)0xA130E4)
#define STE_USB_WR_RDY 2
#define STE_USB_RD_RDY 4

#define USB_RD_BUSY while ((REG_STE & STE_USB_RD_RDY) == 0)
#define USB_WR_BUSY while ((REG_STE & STE_USB_WR_RDY) == 0)

u8 net_player_id;
u16 netplay;
u16 net_rd_timeout;
u16 net_wr_timeout;
extern vu16 joy1_lock;
extern vu16 joy2_lock;


u16 joy_buff_ptr;
vu16 vb_flag_net;
#define MAX_LATENCY 32
u16 joy_buff[MAX_LATENCY];
u16 net_latency;

#define NET_ERROR_RD_TIMEOUT    0x01
#define NET_ERROR_WR_TIMEOUT    0x02
#define NET_ERR_USB             0x03
#define NET_ERR_LAN1            0x04 
#define NET_ERR_LAN2            0x05
#define NET_ERR_LAN3            0x06
#define NET_ERROR_CANCEL        0x10
#define NET_TIMEOUT 0xffff

#define RD_TIMEOUT 30*60

u16 usbRdByte() {

    u16 timeout_ctr = 0;
    vb_flag_net = 0;

    USB_RD_BUSY
    {
        if (vb_flag_net) {
            timeout_ctr++;
            vb_flag_net = 0;
            if (timeout_ctr >= net_rd_timeout)return NET_TIMEOUT;
        }
    }
    return (u8) REG_USB;
}

u16 usbWrByte(u8 val) {

    u16 timeout_ctr = 0;
    vb_flag_net = 0;

    USB_WR_BUSY
    {
        if (vb_flag_net) {
            timeout_ctr++;
            vb_flag_net = 0;
            if (timeout_ctr >= net_wr_timeout)return NET_TIMEOUT;
        }
    }
    REG_USB = val;

    return 0;
}

u16 usbWrString(u8 *str) {
    u16 resp;

    while (*str != 0) {
        resp = usbWrByte(*str++);
        if (resp)return resp;
    }

    return 0;
}

void netTerminateUsbConnection() {

    u16 timeout = net_wr_timeout;
    net_wr_timeout = 30;
    usbWrString("+0");
    net_wr_timeout = timeout;
}

u8 netConnect_();

u8 netConnect() {

    u8 resp;

    resp = netConnect_();
    gFillRect(APLAN, 0, 0, 0, SCREEN_W, SCREEN_H);

    if (resp)netTerminateUsbConnection();

    if (resp == NET_ERROR_CANCEL) {
        return resp;
    }

    if (resp) {
        gConsPrint("CONNECTION ERROR ");
        gAppendHex8(resp);

        for (;;) {
            gameReadJoy();
            if (joy1_click == 0)break;
        }

        for (;;) {
            gameReadJoy();
            if (joy1_click != 0)break;
        }
    }

    return resp;
}

u8 netCancel() {
    gameReadJoy();
    if (joy1_click & BUTTON_B)return 1;
    return 0;
}

u8 netConnect_() {


    u16 resp;
    u16 cmd;
    u16 i;
    net_rd_timeout = 2;
    net_wr_timeout = 60 * 3;

    gFillRect(APLAN, 0, 0, 0, SCREEN_W, SCREEN_H);
    gDrawStringCX("WAITING FOR USB CONNECTION", 12);

    netTerminateUsbConnection();

    for (;;) {



        cmd = usbRdByte();
        if (cmd == NET_TIMEOUT && netCancel())return NET_ERROR_CANCEL;
        if (cmd != '+')continue;


        for (;;) {
            cmd = usbRdByte();
            if (cmd == NET_TIMEOUT && netCancel())return NET_ERROR_CANCEL;
            if (cmd != NET_TIMEOUT)break;
        }

        if (cmd == 'T') {
            resp = usbWrByte('k');
            if (resp)return NET_ERROR_WR_TIMEOUT;
        }

        if (cmd == 'm') {
            break;
        }
    }

    gFillRect(APLAN, 0, 0, 0, SCREEN_W, SCREEN_H);
    gSetXY(0, 3);
    gConsPrint("CONNECT TO SERVERiii");
    if ((VDP_REGION & REGION_E) == REGION_E || (VDP_REGION & REGION_J_PAL) == REGION_J_PAL) {
        resp = usbWrString("+1p");
    } else {
        resp = usbWrString("+1n");
    }

    if (resp)return NET_ERROR_WR_TIMEOUT;
    for (;;) {
        cmd = usbRdByte();
        if (cmd == NET_TIMEOUT && netCancel())return NET_ERROR_CANCEL;
        if (cmd != NET_TIMEOUT)break;
    }
    if (cmd != 'k') {
        gConsPrint("ERROR");
        return NET_ERR_USB;
    }
    gAppendString("OK");

    resp = usbWrString("+2");
    if (resp)return NET_ERROR_WR_TIMEOUT;
    gConsPrint("WAITING FOR PLAYERSiii");



    for (;;) {
        cmd = usbRdByte();
        if (cmd == NET_TIMEOUT && netCancel())return NET_ERROR_CANCEL;
        if (cmd == 'p' || cmd == 0 || cmd == NET_TIMEOUT)continue;
        net_rd_timeout = 30 * 60;

        if (cmd != '+')return NET_ERR_LAN1;
        cmd = usbRdByte();
        if (cmd == NET_TIMEOUT)return NET_ERROR_RD_TIMEOUT;
        if (cmd != 'j')return NET_ERR_LAN2;
        break;
    }


    net_player_id = usbRdByte();
    if (cmd == NET_TIMEOUT)return NET_ERROR_RD_TIMEOUT;


    gAppendString("OK");
    gConsPrint("PLAYER ID ");
    gAppendHex8(net_player_id);


    gConsPrint("TEST LATENCYiii ");
    net_latency = 2;
    if (net_player_id == 0) {

        for (i = 0; i < 16; i++) {
            USB_WR_BUSY;
            REG_USB = 'p';
            USB_RD_BUSY;
            cmd = REG_USB;
        }

        for (i = 0; i < 32; i++) {
            vb_flag_net = 0;
            while (vb_flag_net == 0);
            frame_ctr = 0;
            USB_WR_BUSY;
            REG_USB = 'p';
            USB_RD_BUSY;
            cmd = REG_USB;
            if (net_latency < frame_ctr)net_latency = frame_ctr;
        }
        //net_latency /= 32;
        net_latency++;
        if (net_latency < 2)net_latency = 3;
        if (net_latency > MAX_LATENCY)net_latency = MAX_LATENCY;

        resp = usbWrByte(net_latency >> 8);
        if (resp)return NET_ERR_LAN3;
        resp = usbWrByte(net_latency & 0xff);
        if (resp)return NET_ERR_LAN3;

    } else {
        for (i = 0; i < 16 + 32; i++) {
            USB_RD_BUSY;
            cmd = REG_USB;
            USB_WR_BUSY;
            REG_USB = cmd;
        }

        net_latency = usbRdByte() << 8;
        if (net_latency == 0xff00)return NET_ERR_LAN3;
        net_latency |= usbRdByte();
        if (net_latency == 0xffff)return NET_ERR_LAN3;
    }

    gAppendHex8(net_latency);
    for (i = 0; i < 60 * 3; i++)gVsync();



    //net_latency = 4;

    for (i = 0; i < net_latency; i++) {
        joy_buff[i] = 0;
        resp = usbWrByte(0);
        if (resp)return NET_ERROR_WR_TIMEOUT;
    }





    /*
    if (net_player_id == 1) {
        for (i = 0; i < JOY_BUFF_LEN * 2; i++) usbRdByte();
    } else {
         for (i = 0; i < JOY_BUFF_LEN; i++)usbWrByte(0);
    }*/

    joy_buff_ptr = 0;
    netplay = 1;
    joy1_lock = 0;
    joy2_lock = 0;
    joy1 = 0;
    joy2 = 0;
    joy1_click = 0;
    joy2_click = 0;
    game_tick = 0;
    frame_ctr = 0;

    return 0;
}

void netSyncError(u8 code) {

    u16 i;
    netplay = 0;
    gameRepaint(1);
    gameRepaint(1);
    gFillRect(APLAN, 0, 0, 0, SCREEN_W, SCREEN_H);
    gDrawStringCX("NET SYNC TIMEOUT ", 12);
    gAppendHex8(code);

    netTerminateUsbConnection();

    for (i = 0; i < 60; i++)gVsync();
    netplay = 0;

    for (;;) {
        gVsync();
        gameReadJoy();
        if ((joy1 & BUTTON_START) == BUTTON_START)break;
    }

    for (;;) {
        gVsync();
        gameReadJoy();
        if ((joy1 & BUTTON_START) != BUTTON_START)break;
    }


    asm("move.l 0, %sp");
    asm("move.l 4, %a0");
    asm("jmp (%a0)");

}

void netReadJoy() {

    u16 joy;
    u16 resp;

    JOY_DATA1 = JOY_DIR_BIT;
    JOY_DATA2 = JOY_DIR_BIT;

    joy = JOY_DATA1 & 0x3f;

    JOY_DATA1 = 0;
    JOY_DATA2 = 0;

    joy |= (JOY_DATA1 & 0x30) << 2;

    joy ^= 0xff;

    if (joy == 0xff)joy &= ~BUTTON_START;
    //gFillRect(APLAN, 0, 0, 0, SCREEN_W, SCREEN_H);
    //gConsPrint("DT");
    //for (;;)gAppendHex8(usbRdByte());

    //usbWrByte(joy_buff_ptr);
    resp = usbWrByte(joy);
    if (resp)netSyncError(NET_ERROR_WR_TIMEOUT);

    /*if(usbRdByte() != joy_buff_ptr){
        netSyncError();
    }*/
    if (net_player_id == 0) {
        joy1 = joy_buff[joy_buff_ptr];
        joy2 = usbRdByte();
        if ((joy2 & 0xff) == 0xff)netSyncError(NET_ERROR_RD_TIMEOUT);
    } else {
        joy1 = usbRdByte();
        joy2 = joy_buff[joy_buff_ptr];
        if ((joy1 & 0xff) == 0xff)netSyncError(NET_ERROR_RD_TIMEOUT);
    }

    joy_buff[joy_buff_ptr] = joy;
    joy_buff_ptr++;
    if (joy_buff_ptr >= net_latency)joy_buff_ptr = 0;

    joy1_click = joy1 & ~joy1_lock;
    joy2_click = joy2 & ~joy2_lock;
    joy1_lock = joy1;
    joy2_lock = joy2;
}
