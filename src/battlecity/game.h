/* 
 * File:   game.h
 * Author: krik
 *
 * Created on November 24, 2014, 4:02 PM
 */

#ifndef GAME_H
#define	GAME_H

#define MAP_W 26
#define MAP_H 26

#define SCREEN_X(x) ((x) + 16)
#define SCREEN_Y(y) ((y) + 8)

extern vu16 vb_flag;
extern vu16 frame_ctr;

extern u16 tileset[];
extern u8 main_menu[];
extern u8 maps[];
extern u8 game_over_scr[];


extern volatile Sprite sprite_list[];
extern u16 *spr_ptr;
extern u16 spr_ctr;
extern u16 screen_buff[];

extern vu16 joy1;
extern vu16 joy2;
extern vu16 joy1_click;
extern vu16 joy2_click;
extern u16 game_over;
extern u16 game_tick;
//extern u16 team_score[4];
//extern u32 team_kills[4];
inline u16 getMapAddr(s16 x, s16 y);

void gameInit();
void gameMainMenu();
void gameReadJoy();
void gameIncPlayerLive(u16 idx);
void gameIncTankType(u16 idx);
void gameIncScore(u16 idx, u16 val);
void gameIncKills(u16 idx, u16 tank_type);
void gameRepaint(u16 clean_sprite);

void gInit();
void gScreenOff();
void gScreenOn();
void gSetPal(void *src, u16 palnum);
void gSetPalColor(u16 val, u16 palnum, u16 colornum);
void gWriteVram(void *src, u16 dst, u16 len);
void gSrollV(u16 plan, u16 val);
void gSrollH(u16 plan, u16 val);
void gVsync();
void gFillRect(u16 plan, u16 tile, u16 x, u16 y, u16 w, u16 h);
void gDrawString(u8 *str, u8 x, u8 y);
void gAppendString(u8 *str);
void gDrawStringCX(u8 *str, u8 y);
void gSetXY(u8 x, u8 y);
void gConsPrint(u8 *str);
void gAppendHex8(u8 num);
void gDrawStringFB(u8 *str, u16 frame_buff[], u16 x, u16 y);
void gSetXY_FB(u8 x, u8 y, u16 frame_buff[]);
void gAppendNumFB(u32 num);
void gAppendStringFB(u8 *str);
void gDrawNumFB(u32 num, u16 frame_buff[], u16 x, u16 y, u16 align);
void gSetTextAttrib(u16 attrib);

#define OBJ_EXPLODE     0x0100
#define OBJ_BONUS       0x0200
#define OBJ_TANK        0x0400
#define OBJ_BULLET      0x0800



#define TEAM0           0x0000
#define TEAM1           0x1000
#define TEAM2           0x2000
#define TEAM3           0x3000
#define TEAM            0x3000

#define WEAPON0         0x0000
#define WEAPON1         0x4000
#define WEAPON2         0x8000
#define WEAPON3         0xC000
#define WEAPON          0xC000


#define OBJ_WEAPON_MASK 0x7000

typedef struct {
    u16(*handler)(u16 idx);
    s16 x;
    s16 y;
    u16 state;
    u16 attrib;
    u16 sprite;
    u16 armor;
    u16 ctr;
    u16 ctr2;
    s8 sx;
    s8 sy;
    u8 rotate;
    s8 hitpoints;
    u8 blt_ctr;
    u8 score;
} GameObject;
extern GameObject game_obj[];


void objInit();
void objUpdate();
u16 objMake(u16 attrib);


extern u16 netplay;
u8 netConnect();
void netReadJoy();

#endif	/* GAME_H */

