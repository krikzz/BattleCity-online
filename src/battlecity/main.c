/*
 * File:   main.c
 * Author: krikzz
 *
 * Created on 23.06.2010
 */

#include "segalib.h"
#include "game.h"

vu16 frame_ctr;
vu16 vb_flag;

int main() {

    vu16 *ssf_bank_regs = (vu16 *) 0xA130F0;
    ssf_bank_regs[0] = 0xA000; // turn off ROM write protection
    ssf_bank_regs[4] = 28; //set bank at 0x200000 to 28. Mega OS uses bank 28 as save ram bank. We want to save hi score (:

    frame_ctr = 0;
    vb_flag = 0;

    gInit();
    gameInit();

    for (;;) {

        gameMainMenu();
    }

    return 0;
}

void hb() {
}

void in() {
}
