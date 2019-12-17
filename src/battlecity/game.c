
#include "segalib.h"
#include "game.h"




#define MAP_TYPE_GAME 0
#define MAP_TYPE_MENU 1

#define MAX_LEVELS 35
#define WATER_COLOR0 6
#define WATER_COLOR1 7
#define WATER_COLOR2 8

typedef struct {
    s16 live;
    u16 tank_type;
    u16 obj_idx;
    u16 score;
    u16 level_activity;
    u32 kills;
} Player;

typedef struct {
    u16 max_bots;
    u16 resp_ctr;
    u16 resp_time;
    u16 enemy_ctr;
    u16 stage;
    u16 active_bots;
    u16 replay;
    u16 bot_link[6];
} LevelState;

LevelState lvl;

#define T0 0x400
#define TS 0x500
#define TW 0x600
#define TA 0x700

static const u16 tank_list[] = {

    T0 | 18, TS | 2, T0 | 0, T0 | 0, /*01*/
    TA | 2, TS | 4, T0 | 14, T0 | 0, /*02*/
    T0 | 14, TS | 4, TA | 2, T0 | 0, /*03*/
    TW | 10, TS | 5, T0 | 2, TA | 3, /*04*/
    TW | 5, TA | 2, T0 | 8, TS | 5, /*05*/
    TW | 7, TS | 2, T0 | 9, TA | 2, /*06*/
    T0 | 3, TS | 4, TW | 6, T0 | 7, /*07*/
    TW | 7, TA | 2, TS | 4, T0 | 7, /*08*/
    T0 | 6, TS | 4, TW | 7, TA | 3, /*09*/
    T0 | 12, TS | 2, TW | 4, TA | 2, /*10*/
    TS | 10, TA | 6, TW | 4, T0 | 0, /*11*/
    TW | 8, TS | 6, TA | 6, T0 | 0, /*12*/
    TW | 8, TS | 8, TA | 4, T0 | 0, /*13*/
    TW | 10, TS | 4, TA | 6, T0 | 0, /*14*/
    T0 | 2, TS | 10, TA | 8, T0 | 0, /*15*/
    T0 | 16, TS | 2, TA | 2, T0 | 0, /*16*/
    TA | 8, TW | 2, TS | 8, T0 | 2, /*17*/
    T0 | 02, TS | 8, TW | 6, TA | 4, /*18*/
    T0 | 4, TS | 4, TW | 4, TA | 8, /*19*/
    TS | 8, T0 | 2, TW | 2, TA | 8, /*20*/
    TW | 8, TS | 2, T0 | 6, TS | 4, /*21*/
    TS | 8, T0 | 6, TW | 2, TA | 4, /*22*/
    6 + TA, 10 + TW, 4 + TS, 0 + T0, /*23*/
    4 + TW, 2 + TA, 4 + TS, 10 + T0, /*24*/
    2 + TW, 8 + TS, 10 + TA, 0 + T0, /*25*/
    6 + TS, 6 + TA, 4 + T0, 4 + TW, /*26*/
    2 + TW, 8 + TA, 8 + TS, 2 + T0, /*27*/
    2 + TS, 1 + TA, 15 + T0, 2 + TW, /*28*/
    10 + TW, 4 + TS, 6 + TA, 0 + T0, /*29*/
    4 + T0, 8 + TS, 4 + TW, 4 + TA, /*30*/
    6 + TW, 8 + TS, 6 + TA, 0 + T0, /*31*/
    8 + TA, 6 + T0, 2 + TW, 4 + TS, /*32*/
    8 + TS, 8 + TA, 4 + TW, 0 + T0, /*33*/
    4 + TW, 10 + TS, 6 + TA, 0 + T0, /*34*/
    4 + TW, 6 + TS, 10 + TA, 0 + T0, /*35*/
};

void gameDrawEnemyCounter(u16 val);
void gameDrawLifes(u16 player);
void gameDrawLevelNum(u16 val);
void gameCleanScreen(u16 val);
void gameMainMenu();

u16 gameSelectLevel(u16 construct, u16 stage, u16 change);
void gameStart(u16 play_num, u16 construct);
void gamePlayLoop();
void gamePause();
u16 gameOver();
void gameScoreScreen();
void gameIitPal(u16 pal_type);
void gameHiScoreSave();
void gameHiScoreLoad();
void gameOverScreen();
void gameMapEditor();

vu16 joy1;
vu16 joy2;
vu16 joy1_click;
vu16 joy2_click;
vu16 joy1_lock;
vu16 joy2_lock;

u16 game_state;
u16 scroll_v;
u16 *spr_ptr;
u16 spr_ctr;
u16 game_over;
u32 hi_score;
extern u8 def_cons_map[];

volatile Sprite sprite_list[128];
u16 screen_buff[SCREEN_W * SCREEN_H];
u8 construct_map[MAP_W * MAP_H];

Player players[2];


#define GAME_PLAN APLAN
#define PAL_GAME 1
#define PAL_SCOR 2

void gameInit() {


    gScreenOff();

    gameIitPal(PAL_GAME);

    gWriteVram(tileset + 16, 0, 16384);
    gWriteVram(tileset + 16 + 16 * 48, 16384, 32 * 10);
    gWriteVram(tileset + 16 + 16 * 65, 16384 + 320, 32 * 6);

    game_state = 0;
    scroll_v = 0;
    spr_ptr = (u16 *) sprite_list;
    spr_ctr = 0;
    netplay = 0;


    JOY_CTRL1 = JOY_DIR_BIT;
    JOY_CTRL2 = JOY_DIR_BIT;
    JOY_DATA1 = JOY_DIR_BIT;
    JOY_DATA2 = JOY_DIR_BIT;

    gScreenOn();

    gameHiScoreLoad();

    //for (i = 0; i < MAP_W * MAP_H; i++)construct_map[i] = maps[i];
}

void gameIitPal(u16 pal_type) {

    gSetPal(tileset, 0);

    if (pal_type == PAL_GAME) {

        gSetPalColor(0x06A, 1, 15);
        gSetPalColor(0x0AE, 1, 10);
        gSetPalColor(0xAEE, 1, 11);

        gSetPalColor(0x040, 2, 15);
        gSetPalColor(0x0A0, 2, 10);
        gSetPalColor(0xCE0, 2, 11);

        gSetPalColor(0x420, 3, 15);
        gSetPalColor(0xCCC, 3, 10);
        gSetPalColor(0xEEE, 3, 11);
    }

    if (pal_type == PAL_SCOR) {
        gSetPalColor(0x03E, 1, 11);
        gSetPalColor(0x08e, 2, 11);
    }
}

void gameDrawEnemyCounter(u16 val) {

    s16 i = 10;
    u16 x = 29;
    u16 y = 2;
    u16 *ptr = &screen_buff[x + y * SCREEN_W];


    while (i-- > 0) {
        *ptr++ = 17;
        *ptr++ = 17;
        ptr += SCREEN_W - 2;
    }

    i = val;
    ptr = &screen_buff[x + y * SCREEN_W];
    while (i > 0) {
        if (i-- > 0)*ptr++ = 106;
        if (i-- > 0)*ptr++ = 106;
        ptr += SCREEN_W - 2;
    }


}

void gameDrawLifes(u16 player) {

    u16 x = 29;
    u16 y = 15;
    u16 val = players[player].live;
    u16 *ptr = &screen_buff[x + y * SCREEN_W];

    if (player > 0)ptr += SCREEN_W * 3;

    if (val > 9)val = 9;
    ptr[0] = 88 + player * 2;
    ptr[1] = 19;
    ptr[SCREEN_W] = 20;
    ptr[SCREEN_W + 1] = 110 + val;

}

void gameDrawLevelNum(u16 val) {

    u16 x = 29;
    u16 y = 22;
    u16 *ptr = &screen_buff[x + y * SCREEN_W];

    ptr[0] = 108;
    ptr[1] = 44;
    ptr[SCREEN_W] = 109;
    ptr[SCREEN_W + 1] = 45;

    if (val < 10) {
        ptr[SCREEN_W * 2] = 17;
        ptr[SCREEN_W * 2 + 1] = 110 + val;
    } else {
        ptr[SCREEN_W * 2] = 110 + val / 10;
        ptr[SCREEN_W * 2 + 1] = 110 + val % 10;
    }

}

void gameCleanScreen(u16 val) {

    u16 *ptr = screen_buff;
    u16 i = SCREEN_W * SCREEN_H;
    while (i--)*ptr++ = val;

}

void gameSetMap(u8 *map) {

    u16 *ptr = &screen_buff[SCREEN_W + 2];
    u16 i = MAP_H;
    u16 u;
    u16 val;

    while (i--) {

        u = MAP_W;
        while (u--) {
            val = *map++;
            if (val == 34)val |= TILE_PRI(1);
            *ptr++ = val;
        }
        ptr += SCREEN_W - MAP_W;
    }
}

void gameReadJoy() {

    if (netplay) {
        netReadJoy();
        return;
    }

    JOY_DATA1 = JOY_DIR_BIT;
    JOY_DATA2 = JOY_DIR_BIT;

    joy1 = JOY_DATA1 & 0x3f;
    joy2 = JOY_DATA2 & 0x3f;

    JOY_DATA1 = 0;
    JOY_DATA2 = 0;

    joy1 |= (JOY_DATA1 & 0x30) << 2;
    joy2 |= (JOY_DATA2 & 0x30) << 2;

    joy1 ^= 0xff;
    joy2 ^= 0xff;


    joy1_click = joy1 & ~joy1_lock;
    joy2_click = joy2 & ~joy2_lock;
    joy1_lock = joy1;
    joy2_lock = joy2;

}

void gameMainMenu() {

    u16 i;
    u16 construct = 0;
    u16 selector = 0;
    u16 sel_x = 64;
    u16 sel_y = 124;
    u8 resp;
    u8 menu_num = 4;
    scroll_v = 256 + 32;
    gameCleanScreen(0);
    gameSetMap(main_menu);
    gDrawNumFB(hi_score, screen_buff, 15, 2, 0);
    for (i = 0; i < MAP_W * MAP_H; i++)construct_map[i] = def_cons_map[i];

    if (netplay) {
        menu_num = 3;
        gDrawStringFB("        ", screen_buff, 11, 22);
    }

    while (scroll_v != 0) {

        gameRepaint(1);
        scroll_v++;
        if (scroll_v == 512)scroll_v = 0;
        if (joy1_click & BUTTON_START)scroll_v = 0;
    }




    for (;;) {


        gameRepaint(1);

        if (joy1_click & BUTTON_DOWN) {
            selector++;
            selector %= menu_num;
        }
        if (joy1_click & BUTTON_UP) {
            selector = selector == 0 ? menu_num - 1 : selector - 1;
        }

        if ((joy1_click & BUTTON_START) && selector < 2) {
            gameStart(selector + 1, construct);
            return;
        }

        if ((joy1_click & BUTTON_START) && selector == 2) {
            gameMapEditor();
            construct = 1;
        }

        if ((joy1_click & BUTTON_START) && selector == 3) {

            gameRepaint(1);
            resp = netConnect();
            if (resp == 0)return;
        }

        *spr_ptr++ = 0x80 + sel_y + selector * 16;
        *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
        *spr_ptr++ = 256 + 4 * 6 + (frame_ctr & 4) + TILE_PAL(1);
        *spr_ptr++ = 0x80 + sel_x;

    }
}


void dbgDrawNum(u16 val, u16 x, u16 y);

void dbgDrawHV() {

    u16 x = 224 / 8;
    u16 y = 1;
    static u16 ctr;
    if (frame_ctr % 16 == 0)ctr = VDP_HVCNTR;

    dbgDrawNum(ctr, x, y);
}

void dbgDrawNum(u16 val, u16 x, u16 y) {


    /*u16 *ptr = &screen_buff[x + y * PLAN_W];

     *ptr++ = 512 + ((val >> 12) & 15);
     *ptr++ = 512 + ((val >> 8) & 15);
     *ptr++ = 512 + ((val >> 4) & 15);
     *ptr++ = 512 + ((val >> 0) & 15);*/


    x *= 8;
    y *= 8;
    *spr_ptr++ = 0x80 + y;
    *spr_ptr++ = SPR_SIZE(1, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 512 + ((val >> 12) & 15);
    *spr_ptr++ = 0x80 + x + 0;

    *spr_ptr++ = 0x80 + y;
    *spr_ptr++ = SPR_SIZE(1, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 512 + ((val >> 8) & 15);
    *spr_ptr++ = 0x80 + x + 8;

    *spr_ptr++ = 0x80 + y;
    *spr_ptr++ = SPR_SIZE(1, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 512 + ((val >> 4) & 15);
    *spr_ptr++ = 0x80 + x + 16;

    *spr_ptr++ = 0x80 + y;
    *spr_ptr++ = SPR_SIZE(1, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 512 + ((val >> 0) & 15);
    *spr_ptr++ = 0x80 + x + 24;

}

u16 gameSelectLevel(u16 construct, u16 stage, u16 change) {

    u16 i;
    u16 *ptr;
    u16 addr;

    stage++;
    //gameCleanScreen(0);
    //if (construct)gameSetMap(construct_map);
    gameRepaint(1);

    for (i = 0; i < SCREEN_H / 2; i++) {

        gVsync();
        gFillRect(GAME_PLAN, 17, 0, i, SCREEN_W, 1);
        gFillRect(GAME_PLAN, 17, 0, SCREEN_H - 1 - i, SCREEN_W, 1);
    }

    gameCleanScreen(17);

    ptr = &screen_buff[(SCREEN_H - 1) / 2 * SCREEN_W + (SCREEN_W - 9) / 2];
    for (i = 0; i < 5; i++)*ptr++ = 35 + i;

    for (;;) {

        if (stage < 10) {
            ptr[1] = 17;
            ptr[2] = 110 + stage;
        } else {
            ptr[1] = 110 + stage / 10;
            ptr[2] = 110 + stage % 10;
        }
        gameRepaint(1);

        if (!change)break;
        if (joy1_click & BUTTON_B)stage++;
        if (joy1_click & BUTTON_C)stage--;
        if (joy1_click & BUTTON_START)break;
        if (stage == 0)stage = 1;
        if (stage > MAX_LEVELS)stage = MAX_LEVELS;



    }

    for (i = 0; i < 30; i++) {
        gVsync();
    }

    if (construct) {
        gameSetMap(construct_map);
    } else {
        gameSetMap(&maps[(stage - 1) * MAP_W * MAP_H]);
    }
    for (i = 0; i < SCREEN_H / 2; i++) {
        gVsync();
        addr = (SCREEN_H / 2 - i) * SCREEN_W;
        gWriteVram(&screen_buff[addr], GAME_PLAN + addr * 2, SCREEN_W * 2);
        addr = (SCREEN_H / 2 + i + 1) * SCREEN_W;
        gWriteVram(&screen_buff[addr], GAME_PLAN + addr * 2, SCREEN_W * 2);
    }

    //gameRepaint(1);

    gameDrawLevelNum(stage);
    return stage - 1;

}

void gameStart(u16 play_num, u16 construct) {


    u16 change_level = 1;
    u16 i;
    for (i = 0; i < 2; i++) {
        players[i].live = 2;
        players[i].score = 0;
    }
    players[0].tank_type = OBJ_TANK | TEAM0; // | WEAPON3;
    players[1].tank_type = OBJ_TANK | TEAM1;
    players[0].level_activity = 1;
    players[1].level_activity = 1;
    if (play_num != 2) {
        players[1].level_activity = 0;
        players[1].live = -1;
    }
    lvl.replay = 0;
    lvl.stage = 0;

    for (;;) {

        lvl.stage = gameSelectLevel(construct, lvl.stage, change_level);
        change_level = 0;
        construct = 0;
        gamePlayLoop();
        gameScoreScreen();
        if (game_over)break;

        lvl.stage++;
        if (lvl.stage >= MAX_LEVELS) {
            lvl.stage = 0;
            lvl.replay = 16;
        }
    }

    gameOverScreen();

}



u8 current_tank_table[20];

u16 gameNextTank(u16 tank_num) {

    u16 bonus = 0;

    tank_num = 20 - tank_num;
    if (tank_num == 3 || tank_num == 10 || tank_num == 17)bonus = OBJ_BONUS;



    return OBJ_TANK | TEAM2 | current_tank_table[tank_num] | bonus;
}

void gameRepaint(u16 clean_sprite) {



    //if (clean_sprite)dbgDrawHV();

    if (spr_ctr > 80) {
        spr_ctr = 80;
    }
    if (spr_ctr == 0) {
        spr_ctr = 1;
        sprite_list[0].x = 0;
    }
    sprite_list[spr_ctr - 1].size &= 0xff00;

    if (clean_sprite) {
        spr_ctr = 0;
        spr_ptr = (u16 *) & sprite_list;
    }



    gameReadJoy();


    gVsync();
    gSrollV(GAME_PLAN, scroll_v);
    gWriteVram(screen_buff, GAME_PLAN, SCREEN_W * SCREEN_H * 2);
    gWriteVram(&sprite_list, SLIST, 64 * 8);

    if (frame_ctr % 64 == 32) {
        gSetPalColor(tileset[WATER_COLOR0], 0, WATER_COLOR1);
        gSetPalColor(tileset[WATER_COLOR1], 0, WATER_COLOR2);
    }

    if (frame_ctr % 64 == 0) {
        gSetPalColor(tileset[WATER_COLOR1], 0, WATER_COLOR1);
        gSetPalColor(tileset[WATER_COLOR0], 0, WATER_COLOR2);
    }


    //if (clean_sprite)dbgDrawNum(game_tick, 224 / 8, 25);

}

void gamePause() {


    if (((joy1_click | joy2_click) & BUTTON_START) == 0)return;


    spr_ptr[0] = 0x80 + 113;
    spr_ptr[1] = SPR_SIZE(4, 1) << 8 | ++spr_ctr;
    spr_ptr[2] = 22 | TILE_PRI(1);
    spr_ptr[3] = 0x80 + 100;

    spr_ptr[4] = spr_ptr[0];
    spr_ptr[5] = SPR_SIZE(1, 1) << 8;
    spr_ptr[6] = spr_ptr[2] + 4;
    spr_ptr[7] = spr_ptr[3] + 32;



    for (;;) {

        gVsync();
        gWriteVram(&sprite_list, SLIST, 64 * 8);
        gameReadJoy();
        if (frame_ctr % 32 == 0) {
            spr_ptr[0] = 0;
            spr_ptr[4] = 0;
        }

        if (frame_ctr % 32 == 16) {
            spr_ptr[0] = 0x80 + 113;
            spr_ptr[4] = spr_ptr[0];
        }

        if (((joy1_click | joy2_click) & BUTTON_START))return;
    }


}

u16 gameOver() {

    u16 idx;
    s16 y;

    if (game_over == 0)return 0;

    joy1 = 0;
    joy1_click = 0;
    joy2 = 0;
    joy2_click = 0;

    if (game_over > 240)return 1;

    if (game_over == 1) {
        screen_buff[14 + 25 * SCREEN_W + 0] = 156;
        screen_buff[14 + 25 * SCREEN_W + 1] = 158;
        screen_buff[14 + 26 * SCREEN_W + 0] = 157;
        screen_buff[14 + 26 * SCREEN_W + 1] = 159;

        idx = objMake(OBJ_EXPLODE);
        game_obj[idx].x = 104 - 8;
        game_obj[idx].y = 200 - 8;
    }

    y = 256 - game_over;
    if (y < 98)y = 98;


    *spr_ptr++ = 0x80 + y;
    *spr_ptr++ = SPR_SIZE(4, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 120 | TILE_PRI(1);
    *spr_ptr++ = 0x80 + 104;

    *spr_ptr++ = 0x88 + y;
    *spr_ptr++ = SPR_SIZE(4, 1) << 8 | ++spr_ctr;
    *spr_ptr++ = 124 | TILE_PRI(1);
    *spr_ptr++ = 0x80 + 104;

    game_over++;


    return 0;
}

void gameIncPlayerLive(u16 idx) {

    if (idx == players[0].obj_idx) {
        players[0].live++;
        gameDrawLifes(0);
    } else {
        players[1].live++;
        gameDrawLifes(1);
    }
}

void gameIncTankType(u16 idx) {


    u16 weapon = 0;
    idx = idx == players[0].obj_idx ? 0 : 1;

    weapon = players[idx].tank_type & WEAPON;

    if (weapon == WEAPON0)weapon = WEAPON1;
    else
        if (weapon == WEAPON1)weapon = WEAPON2;
    else
        if (weapon == WEAPON2)weapon = WEAPON3;

    players[idx].tank_type &= ~WEAPON;
    players[idx].tank_type |= weapon;


    game_obj[players[idx].obj_idx].attrib &= ~WEAPON;
    game_obj[players[idx].obj_idx].attrib |= weapon;

}

void gameIncScore(u16 idx, u16 val) {

    idx = idx == players[0].obj_idx ? 0 : 1;
    players[idx].score += val;
}

void gameIncKills(u16 idx, u16 tank_type) {

    idx = idx == players[0].obj_idx ? 0 : 1;
    players[idx].kills += 1 << tank_type * 8;
    players[idx].score += (tank_type + 1) * 100;
}

void gameInitTankList() {

    u16 i;
    u16 stage = lvl.stage + lvl.replay;
    u16 *ptr;
    s8 ctr;

    if (stage >= MAX_LEVELS)stage -= lvl.replay;
    ptr = (u16 *) & tank_list[stage];


    ctr = *ptr & 0xff;
    for (i = 0; i < 20; i++) {

        current_tank_table[i] = *ptr >> 8;
        ctr--;
        if (ctr < 1) {
            ptr++;
            ctr = *ptr & 31;
        }

    }
}

void gameRespawnUpdate() {

    u16 i;

    for (i = 0; i < 2; i++) {
        if (players[i].live >= 0 && game_obj[players[i].obj_idx].state == 0) {
            players[i].live--;

            if (players[i].live >= 0) {
                gameDrawLifes(i);
                players[i].tank_type = OBJ_TANK | (i == 0 ? TEAM0 : TEAM1);
                objMake(players[i].tank_type);
            }
        }
    }

    if (lvl.resp_ctr != 0) lvl.resp_ctr--;

    lvl.active_bots = 0;
    for (i = 0; i < lvl.max_bots; i++) {

        if (lvl.bot_link[i] != 0xffff && game_obj[lvl.bot_link[i]].state != 0) {
            lvl.active_bots++;
            continue;
        }
        if (lvl.resp_ctr != 0 || lvl.enemy_ctr == 0)continue;

        lvl.bot_link[i] = objMake(gameNextTank(lvl.enemy_ctr--));
        gameDrawEnemyCounter(lvl.enemy_ctr);
        lvl.resp_ctr = lvl.resp_time;
        lvl.active_bots++;
        //break;
    }
}

void gameInitLevel() {

    u16 i;
    game_over = 0;
    objInit();
    gameInitTankList();
    for (i = 0; i < 6; i++)lvl.bot_link[i] = 0xffff;

    lvl.enemy_ctr = 20;
    lvl.resp_time = 190 - lvl.stage * 4;
    lvl.max_bots = 4;
    lvl.active_bots = 0;
    lvl.resp_ctr = 0;

    if (players[0].live >= 0 && players[1].live >= 0) {
        lvl.resp_time -= 20;
        lvl.max_bots = 6;
    }

    if (lvl.replay) lvl.resp_time = 20;

    for (i = 0; i < 2; i++) {
        if (players[i].live >= 0) {
            players[i].obj_idx = objMake(players[i].tank_type);
            gameDrawLifes(i);
        }
        players[i].kills = 0;
    }

    gameDrawEnemyCounter(lvl.enemy_ctr);
}

void gamePlayLoop() {


    u16 level_end_delay = 0;
    gameInitLevel();

    for (;;) {

        gameRepaint(1);
        if (gameOver())return;

        gameRespawnUpdate();


        if (lvl.active_bots == 0 && lvl.enemy_ctr == 0) {
            level_end_delay++;
            if (level_end_delay > 60 * 3)break;
        } else {
            level_end_delay = 0;
        }
        if (players[0].live < 0 && players[1].live < 0 && game_over == 0)game_over = 2;

        objUpdate();
        gamePause();

    }



}

void gameScoreScreen() {

    u16 act[2];
    u16 u;
    u16 i;
    gameCleanScreen(0);


    act[0] = players[0].level_activity;
    act[1] = players[1].level_activity;

    if (players[0].live < 0)players[0].level_activity = 0;
    if (players[1].live < 0)players[1].level_activity = 0;

    //players[0].kills = 0x01020304;
    //players[1].kills = 0x01020304;

    gVsync();
    gameIitPal(PAL_SCOR);

    gSetTextAttrib(TILE_PAL(1));
    gDrawStringFB("HIkSCORE   ", screen_buff, 8, 2);
    gSetTextAttrib(TILE_PAL(2));
    gAppendNumFB(hi_score);
    gSetTextAttrib(TILE_PAL(0));

    gDrawStringFB("STAGE  ", screen_buff, 12, 4);
    gAppendNumFB(lvl.stage + 1);

    gSetTextAttrib(TILE_PAL(1));

    if (act[0])gDrawStringFB("^kPLAYER", screen_buff, 3, 6);
    if (act[1])gDrawStringFB("_kPLAYER", screen_buff, 21, 6);

    gSetTextAttrib(TILE_PAL(2));
    if (act[0])gDrawNumFB(players[0].score, screen_buff, 3 + 7, 8, 1);
    if (act[1])gDrawNumFB(players[1].score, screen_buff, 21 + 7, 8, 1);

    gSetTextAttrib(TILE_PAL(0));

    u16 pts_x = 8;
    u16 pts_y = 11;

    for (i = 0; i < 4; i++) {

        if (act[0]) {
            gDrawStringFB("PTS", screen_buff, pts_x, pts_y + i * 3);
            gDrawStringFB("[", screen_buff, pts_x + 6, pts_y + i * 3);
        }
        if (act[1]) {
            gDrawStringFB("PTS", screen_buff, pts_x + 18, pts_y + i * 3);
            gDrawStringFB("]", screen_buff, pts_x + 9, pts_y + i * 3);
        }



        *spr_ptr++ = 0x80 + 85 + i * 24;
        *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
        *spr_ptr++ = (256 + 128 + i * 32) | TILE_PAL(3);
        *spr_ptr++ = 0x80 + 256 / 2 - 7;

    }

    gDrawStringFB("\\\\\\\\\\\\\\\\", screen_buff, 12, 21);
    gDrawStringFB("TOTAL", screen_buff, 6, 22);



    u16 scor;
    u16 kills[2];
    u16 ctr[2];
    u16 total_kills[2];



    total_kills[0] = 0;
    total_kills[1] = 0;

    for (i = 0; i < 4; i++) {

        kills[0] = players[0].kills >> i * 8 & 0xff;
        kills[1] = players[1].kills >> i * 8 & 0xff;
        total_kills[0] += kills[0];
        total_kills[1] += kills[1];
        ctr[0] = 0;
        ctr[1] = 0;
        for (;;) {

            if (act[0]) {
                scor = ctr[0] * (i * 100 + 100);
                gDrawNumFB(ctr[0], screen_buff, pts_x + 5, pts_y, 1);
                gDrawNumFB(scor, screen_buff, pts_x - 2, pts_y, 1);
            }

            if (act[1]) {
                scor = ctr[1] * (i * 100 + 100);
                gDrawNumFB(ctr[1], screen_buff, pts_x + 11, pts_y, 1);
                gDrawNumFB(scor, screen_buff, pts_x + 16, pts_y, 1);
            }

            gameRepaint(0);
            for (u = 0; u < 10; u++)gVsync();
            if (ctr[0] >= kills[0] && ctr[1] >= kills[1])break;
            if (ctr[0] < kills[0])ctr[0]++;
            if (ctr[1] < kills[1])ctr[1]++;
        }

        for (u = 0; u < 16; u++)gVsync();
        pts_y += 3;
    }


    pts_y--;
    if (act[0])gDrawNumFB(total_kills[0], screen_buff, pts_x + 5, pts_y, 1);
    if (act[1])gDrawNumFB(total_kills[1], screen_buff, pts_x + 11, pts_y, 1);

    gameRepaint(0);


    for (u = 0; u < 20; u++)gVsync();

    if (act[0] && act[1]) {
        gSetTextAttrib(TILE_PAL(1));
        if (total_kills[0] > total_kills[1]) {
            gDrawStringFB("BONUS", screen_buff, 3, 24);
            gSetTextAttrib(TILE_PAL(0));
            gDrawStringFB("1000 PTS", screen_buff, 3, 25);
            players[0].score += 1000;
            gSetTextAttrib(TILE_PAL(2));
            gDrawNumFB(players[0].score, screen_buff, 3 + 7, 8, 1);
        }

        if (total_kills[1] > total_kills[0]) {
            gDrawStringFB("BONUS", screen_buff, 22, 24);
            gSetTextAttrib(TILE_PAL(0));
            gDrawStringFB("1000 PTS", screen_buff, 22, 25);
            players[1].score += 1000;
            gSetTextAttrib(TILE_PAL(2));
            gDrawNumFB(players[1].score, screen_buff, 21 + 7, 8, 1);
        }
    }



    gameRepaint(0);
    for (u = 0; u < 30; u++)gVsync();


    for (u = 0; u < 60 * 5; u++) {
        gameRepaint(0);
        if (joy1_click != 0 || joy2_click != 0)break;
    }



    gameIitPal(PAL_GAME);
    gSetTextAttrib(0);
    gameRepaint(1);
    gameRepaint(1);
}

void gameHiScoreSave() {

    u16 i;
    u16 *ptr = (u16 *) 0x200000;

    ptr[0] = (hi_score >> 24) & 0xff;
    ptr[1] = (hi_score >> 16) & 0xff;
    ptr[2] = (hi_score >> 8) & 0xff;
    ptr[3] = (hi_score) & 0xff;

    for (i = 0; i < 4; i++)ptr[i + 4] = ptr[i] ^ 0xff;

}

void gameHiScoreLoad() {
    u16 i;
    u32 not_val = 0;
    u16 *ptr = (u16 *) 0x200000;

    hi_score = 0;
    for (i = 0; i < 4; i++) {
        hi_score <<= 8;
        not_val <<= 8;
        hi_score |= ptr[i] & 0xff;
        not_val |= ptr[i + 4] & 0xff;
    }

    not_val ^= 0xffffffff;
    if (not_val != hi_score) {
        hi_score = 20000;
        gameHiScoreSave();
    }
}

void gameOverScreen() {

    u16 i;
    for (i = 0; i < 2; i++) {
        if (hi_score < players[i].score) {
            hi_score = players[i].score;
            gameHiScoreSave();
        }
    }

    gameCleanScreen(0);
    gameSetMap(game_over_scr);


    gameRepaint(1);
    gameRepaint(1);

    for (i = 0; i < 3 * 60; i++)gVsync();
}

static const u16 map_objects[] = {
    0, 0,
    0, 0,

    0, 15,
    0, 15,

    0, 0,
    15, 15,

    15, 0,
    15, 0,

    15, 15,
    0, 0,

    15, 15,
    15, 15,

    0, 16,
    0, 16,

    0, 0,
    16, 16,

    16, 0,
    16, 0,

    16, 16,
    0, 0,

    16, 16,
    16, 16,

    18, 18,
    18, 18,

    34, 34,
    34, 34,

    33, 33,
    33, 33,


};

void gameSetBird(u8 *map) {

    map[12 + 24 * MAP_W + 0] = 252;
    map[12 + 24 * MAP_W + 1] = 254;
    map[12 + 25 * MAP_W + 0] = 253;
    map[12 + 25 * MAP_W + 1] = 255;
}

void gameMapEditor() {

    s16 x = 0;
    s16 y = 0;
    u16 i;
    u16 joy;
    u16 joy_click;
    u16 joy_lock = 0;
    u16 joy_lock_ctr = 0;
    s16 object = 0;
    u16 inc_object = 1;
    u16 map_addr;



    gameSetBird(construct_map);

    gameCleanScreen(17);
    gameSetMap(construct_map);

    for (;;) {

        joy_click = joy1_click;

        if ((joy1 & BUTTON_DPAD) == 0) {
            joy_lock = 0;
            joy_lock_ctr = 0;
        } else {
            joy_lock_ctr++;
        }

        if (joy_lock) {
            joy = 0;
            joy_lock--;
        } else {
            joy = joy1;
            if ((joy1 & BUTTON_DPAD) != 0) {
                joy_lock = joy_lock_ctr == 1 ? 32 : 4;
            }
        }

        if (joy & BUTTON_LEFT)x -= 2;
        if (joy & BUTTON_RIGHT)x += 2;
        if (joy & BUTTON_UP)y -= 2;
        if (joy & BUTTON_DOWN)y += 2;
        if (x < 0)x = 0;
        if (x >= MAP_W)x = MAP_W - 2;
        if (y < 0)y = 0;
        if (y >= MAP_W)y = MAP_H - 2;

        if (joy & BUTTON_DPAD)inc_object = 0;

        if ((joy_click & BUTTON_A) && inc_object)object--;
        if ((joy_click & BUTTON_B) && inc_object)object++;
        if (object < 0)object = 13;
        if (object >= 14)object = 0;

        if (joy & (BUTTON_A | BUTTON_B)) {
            map_addr = getMapAddr(x * 8, y * 8);
            screen_buff[map_addr] = map_objects[object * 4];
            screen_buff[map_addr + 1] = map_objects[object * 4 + 1];
            screen_buff[map_addr + SCREEN_W] = map_objects[object * 4 + 2];
            screen_buff[map_addr + SCREEN_W + 1] = map_objects[object * 4 + 3];
            inc_object = 1;
        }

        if (frame_ctr % 32 < 16) {
            *spr_ptr++ = 0x80 + SCREEN_Y(y * 8);
            *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
            *spr_ptr++ = 256 | TILE_PAL(1);
            *spr_ptr++ = 0x80 + SCREEN_X(x * 8);
        }


        gameRepaint(1);
        if (joy1_click & BUTTON_START)break;

    }

    map_addr = getMapAddr(0, 0);
    u16 *ptr1;
    u8 *ptr2 = construct_map;
    u16 u;

    for (i = 0; i < MAP_H; i++) {

        ptr1 = (u16 *) & screen_buff[map_addr];

        for (u = 0; u < MAP_W; u++) {
            *ptr2++ = *ptr1++;
        }
        map_addr += SCREEN_W;
    }

    gameSetBird(construct_map);

    construct_map[0] = 0;
    construct_map[1] = 0;
    construct_map[0 + MAP_W] = 0;
    construct_map[1 + MAP_W] = 0;

    construct_map[0 + MAP_W / 2 - 1] = 0;
    construct_map[1 + MAP_W / 2 - 1] = 0;
    construct_map[0 + MAP_W + MAP_W / 2 - 1] = 0;
    construct_map[1 + MAP_W + MAP_W / 2 - 1] = 0;

    construct_map[0 + MAP_W - 2] = 0;
    construct_map[1 + MAP_W - 2] = 0;
    construct_map[0 + MAP_W + MAP_W - 2] = 0;
    construct_map[1 + MAP_W + MAP_W - 2] = 0;

    
    gameCleanScreen(0);
    gameSetMap(main_menu);

}
