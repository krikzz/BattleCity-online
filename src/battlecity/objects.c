
#include "segalib.h"
#include "game.h"


#define PLAY_MAX 2
#define ENEMY_MAX 6
#define PLAY_BLT_MAX 4
#define ENEMY_BLT_MAX 6 

#define PLAY_BASE 0
#define ENEMY_BASE (PLAY_BASE + PLAY_MAX)
#define PLAY_BLT_BASE (ENEMY_BASE + ENEMY_MAX)
#define ENEMY_BLT_BASE (PLAY_BLT_BASE + PLAY_BLT_MAX)
#define BONUS_BASE (ENEMY_BLT_BASE + ENEMY_BLT_MAX)
#define MISK_BASE (BONUS_BASE + 1)


#define STATE_ACTIVE 1
#define STATE_EXPLODE 2
#define STATE_BORN 4
#define STATE_ANIMATION 8
#define STATE_ROTATE_DIR 16



#define MAX_OBJECTS 32
GameObject game_obj[MAX_OBJECTS + 1];

u16 objHandler_bullet(u16 idx);
u16 objHandler_enemy(u16 idx);
u16 objHandler_player(u16 idx);
u16 objHandler_tankExplode(u16 idx);
u16 objHandler_bltExplode(u16 idx);
u16 objHandler_bonus(u16 idx);
void objHandler_stabArm();
u16 bonusCheckCollision(u16 idx);
inline u16 objTankToMapCollision(u16 idx, u16 x, u16 y);
inline u16 objTankToTankCollision(u16 idx, u16 x, u16 y);
void objBltToObjCollision(u16 idx);
inline void objBltToMapCollision(u16 idx, u16 map_addr, u16 blt_mask);
u16 objHandler_score(u16 idx);

u16 game_tick;
u16 rnd_ptr;
u16 freeze;
u16 stab_armor;
u16 rnd_seed;
u8 rnd_ctr;

u16 rnd() {

    /*static u16 rnd;
    rnd += VDP_HVCNTR;
    return rnd;*/
    /*rnd_ptr++;
    if (game_tick % 4 == 2)return rnd_ptr ^ 0xffff;
    return game_tick + rnd_ptr;*/
    rnd_seed = (rnd_seed << 3) - rnd_seed + game_tick + ((u8*) 0x200)[rnd_ctr++];

    return rnd_seed;
}

void objInit() {

    u16 i;
    for (i = 0; i < MAX_OBJECTS; i++) {
        game_obj[i].state = 0;
        game_obj[i].blt_ctr = 0;
    }
    game_over = 0;
    game_tick = 0;
    rnd_ptr = 0;
    freeze = 0;
    stab_armor = 0;
    rnd_seed = 0;
    rnd_ctr = 0;
}

void objUpdate() {

    u16 i;
    for (i = 0; i < MAX_OBJECTS; i++) {
        if (game_obj[i].state == 0)continue;
        game_obj[i].handler(i);
    }
    objHandler_stabArm();
    game_tick++;
    if (freeze != 0)freeze--;
}

u16 objMake(u16 attrib) {

    if (attrib & OBJ_BULLET) {
        return objHandler_bullet(attrib);
    }

    if (attrib & OBJ_TANK) {

        if ((attrib & TEAM) > TEAM1) {
            return objHandler_enemy(attrib);
        } else {
            return objHandler_player(attrib);
        }
    }

    if (attrib & OBJ_EXPLODE) {
        return objHandler_tankExplode(attrib);
    }

    if (attrib & OBJ_BONUS) {
        return objHandler_bonus(attrib);
    }

    return MAX_OBJECTS;
}

void objDrawTank(u16 idx) {

    u16 i;

    if ((game_obj[idx].attrib & TEAM) > TEAM1) {

        if (game_obj[idx].attrib & OBJ_BONUS) {
            if (game_tick % 16 == 0)game_obj[idx].sprite &= ~TILE_PAL(3);
            if (game_tick % 16 == 8)game_obj[idx].sprite |= TILE_PAL(3);
        } else if (game_obj[idx].hitpoints > 1) {

            if (game_obj[idx].hitpoints == 2) {
                i = game_tick % 2 == 0 ? TILE_PAL(1) : TILE_PAL(2);
            } else if (game_obj[idx].hitpoints == 3) {
                i = game_tick % 2 == 0 ? TILE_PAL(1) : TILE_PAL(3);
            } else {
                i = game_tick % 2 == 0 ? TILE_PAL(2) : TILE_PAL(3);
            }

            game_obj[idx].sprite = (game_obj[idx].sprite & ~TILE_PAL(3)) | i;

        } else {
            game_obj[idx].sprite |= TILE_PAL(3);
        }
    }

    if (game_obj[idx].armor) {
        *spr_ptr++ = 0x80 + SCREEN_Y(game_obj[idx].y);
        *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
        *spr_ptr++ = 200 + game_obj[idx].armor / 2 % 2 * 4;
        *spr_ptr++ = 0x80 + SCREEN_X(game_obj[idx].x);
    }

    *spr_ptr++ = 0x80 + SCREEN_Y(game_obj[idx].y);
    *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
    *spr_ptr++ = game_obj[idx].sprite + game_obj[idx].rotate * 8;
    *spr_ptr++ = 0x80 + SCREEN_X(game_obj[idx].x);


}

inline u16 getMapAddr(s16 x, s16 y) {

    return SCREEN_X(x) / 8 + SCREEN_Y(y) / 8 * SCREEN_W;
}

//fb fd

void objHandleBorn(u16 idx) {
    u16 tmp;

    if ((game_obj[idx].state & STATE_ANIMATION) == 0) {
        game_obj[idx].ctr = 0;
        game_obj[idx].state |= STATE_ANIMATION;
        return;
    }

    tmp = game_obj[idx].ctr / 2 % 8;
    if (tmp > 3)tmp = 7 - tmp;
    *spr_ptr++ = 0x80 + SCREEN_Y(game_obj[idx].y);
    *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
    *spr_ptr++ = 160 + tmp * 4;
    *spr_ptr++ = 0x80 + SCREEN_X(game_obj[idx].x);

    if (game_obj[idx].ctr++ > 32 + 16) {
        game_obj[idx].state = STATE_ACTIVE;
        game_obj[idx].ctr = 0;
    }
}

void objUpdatePlayerTankSprite(u16 idx) {

    u16 tmp = game_obj[idx].attrib & WEAPON;


    switch (tmp) {
        case WEAPON0:
            game_obj[idx].sprite = game_obj[idx].sprite & ~224;
            break;
        case WEAPON1:
            game_obj[idx].sprite = (game_obj[idx].sprite & ~224) | 1 * 32;
            break;
        case WEAPON2:
            game_obj[idx].sprite = (game_obj[idx].sprite & ~224) | 2 * 32;
            break;
        case WEAPON3:
            game_obj[idx].sprite = (game_obj[idx].sprite & ~224) | 3 * 32;
            break;
    }

}

u16 objHandler_player(u16 idx) {

    u16 joy;
    u16 joy_click;
    u16 tmp;


    if ((idx & 0xff00) != 0) {
        tmp = (idx & TEAM) == TEAM0 ? 0 : 1;
        game_obj[tmp].state = STATE_BORN;
        game_obj[tmp].sx = 0;
        game_obj[tmp].sy = 0;
        game_obj[tmp].sprite = 256 | TILE_PAL(tmp + 1);
        game_obj[tmp].x = tmp == 0 ? 64 : 128;
        game_obj[tmp].y = 192;
        game_obj[tmp].attrib = idx;
        game_obj[tmp].handler = objHandler_player;
        game_obj[tmp].rotate = 0;
        game_obj[tmp].armor = 60 * 3;
        game_obj[tmp].ctr2 = 0;
        game_obj[tmp].hitpoints = 1;
        game_obj[tmp].score = 0;
        objUpdatePlayerTankSprite(tmp);
        return tmp;
    }

    if (game_obj[idx].armor != 0)game_obj[idx].armor--;

    if (game_obj[idx].state & STATE_BORN) {
        objHandleBorn(idx);
        return 0;
    }

    if (idx == 0) {
        joy = joy1;
        joy_click = joy1_click;
    } else {
        joy = joy2;
        joy_click = joy2_click;
    }

    if (game_obj[idx].hitpoints < 1 && (game_obj[idx].state & STATE_ACTIVE))game_obj[idx].state = STATE_EXPLODE;

    if ((game_obj[idx].state & STATE_EXPLODE)) {
        objHandler_tankExplode(idx);
        return 0;
    }

    if (joy & (BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN)) {
        game_obj[idx].sprite = (game_obj[idx].sprite & ~4) | (game_tick & 4);
    }

    if (game_obj[idx].ctr2 != 0) {
        game_obj[idx].sx = 0;
        game_obj[idx].sy = 0;
    } else if (joy & BUTTON_LEFT) {
        game_obj[idx].sx = -1;
        game_obj[idx].sy = 0;
        game_obj[idx].rotate = 1;
    } else if (joy & BUTTON_RIGHT) {
        game_obj[idx].sx = 1;
        game_obj[idx].sy = 0;
        game_obj[idx].rotate = 3;
        //game_obj[idx].sprite = base_sprite + 24 + (frame_ctr & 4);
    } else if (joy & BUTTON_UP) {
        game_obj[idx].sx = 0;
        game_obj[idx].sy = -1;
        game_obj[idx].rotate = 0;
        //game_obj[idx].sprite = base_sprite + (frame_ctr & 4);
    } else if (joy & BUTTON_DOWN) {
        game_obj[idx].sx = 0;
        game_obj[idx].sy = 1;
        game_obj[idx].rotate = 2;
        // game_obj[idx].sprite = base_sprite + 16 + (frame_ctr & 4);
    } else if (game_obj[idx].ctr == 0) {

        game_obj[idx].sx = 0;
        game_obj[idx].sy = 0;
    }



    if (game_tick % 4 != idx) {

        s16 x = game_obj[idx].x;
        s16 y = game_obj[idx].y;
        s16 sx = game_obj[idx].sx;
        s16 sy = game_obj[idx].sy;
        u16 collision;




        if (sy != 0) {
            y += sy;
            tmp = x;
            x &= ~7;
            if (tmp % 8 > 3)x += 8;
        }
        if (sx != 0) {
            x += sx;
            tmp = y;
            y &= ~7;
            if (tmp % 8 > 3)y += 8;
        }


        collision = objTankToMapCollision(idx, x, y);


        if (collision & 1) {
            if (joy & (BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN)) {
                game_obj[idx].ctr = 16;
            } else {
                if (game_obj[idx].ctr != 0)game_obj[idx].ctr--;
            }
        } else {
            game_obj[idx].ctr = 0;
        }

        if (collision & 2) {
            x -= sx;
            y -= sy;
        } else if (objTankToTankCollision(idx, x, y)) {
            x -= sx;
            y -= sy;
        }



        game_obj[idx].x = x;
        game_obj[idx].y = y;

    }

    if (joy_click & (BUTTON_A | BUTTON_B | BUTTON_C)) {
        tmp = (game_obj[idx].attrib & WEAPON) >= WEAPON2 ? 2 : 1;
        if (game_obj[idx].blt_ctr < tmp)objMake(OBJ_BULLET | (game_obj[idx].attrib & 0xff00) | idx);
    }


    if (game_obj[idx].ctr2 == 0 || game_tick % 32 < 16) {
        objDrawTank(idx);
    }
    if (game_obj[idx].ctr2 != 0)game_obj[idx].ctr2--;



    return 0;
}

void objDrawBullet(u16 idx) {


    *spr_ptr++ = SCREEN_Y(game_obj[idx].y) + 0x80;
    if (game_obj[idx].sy != 0) {
        *spr_ptr++ = SPR_SIZE(2, 1) << 8 | ++spr_ctr;
    } else {
        *spr_ptr++ = SPR_SIZE(1, 2) << 8 | ++spr_ctr;
    }
    *spr_ptr++ = game_obj[idx].sprite;
    *spr_ptr++ = SCREEN_X(game_obj[idx].x) + 0x80;
}

u16 objHandler_bullet(u16 idx) {


    u16 i;
    u16 master;
    u16 speed;
    u16 map_addr;

    if ((idx & 0xff00) != 0) {

        if ((idx & TEAM) < TEAM2) {
            for (i = PLAY_BLT_BASE; i < PLAY_BLT_BASE + PLAY_BLT_MAX; i++) {
                if (game_obj[i].state == 0)break;
            }
            if (i == PLAY_BLT_BASE + PLAY_BLT_MAX)return MAX_OBJECTS;
        } else {
            for (i = ENEMY_BLT_BASE; i < ENEMY_BLT_BASE + ENEMY_BLT_MAX; i++) {
                if (game_obj[i].state == 0)break;
            }
            if (i == ENEMY_BLT_BASE + ENEMY_BLT_MAX)return MAX_OBJECTS;
        }



        master = idx & 0xff;
        game_obj[i].attrib = idx;
        game_obj[i].state = STATE_ACTIVE;
        game_obj[i].sx = 0;
        game_obj[i].sy = 0;
        game_obj[i].sprite = 128 + 32 + 16 + game_obj[master].rotate * 2;

        speed = (idx & WEAPON) != 0 ? 4 : 2;
        game_obj[i].x = game_obj[master].x;
        game_obj[i].y = game_obj[master].y;
        //game_obj[i].blt_ctr = master;
        game_obj[master].blt_ctr++;


        switch (game_obj[master].rotate) {
            case 0://up
                game_obj[i].sy = -speed;
                //game_obj[i].y -= 4;
                break;
            case 1://left
                game_obj[i].sx = -speed;
                //game_obj[i].x -= 4;
                break;
            case 2://down
                game_obj[i].sy = speed;
                game_obj[i].y += 4;
                break;
            case 3://right
                game_obj[i].sx = speed;
                game_obj[i].x += 4;
                break;
        }


        game_obj[i].handler = objHandler_bullet;
        return i;
    }

    if (game_obj[idx].state == STATE_ACTIVE) {

        game_obj[idx].x += game_obj[idx].sx;
        game_obj[idx].y += game_obj[idx].sy;

        //if ((frame_ctr & 1) == (idx & 1) )
        if ((game_tick & 1) == (idx & 1) || (game_obj[i].attrib & WEAPON) != 0) {
            objBltToObjCollision(idx);
        }

        if (game_obj[idx].state != STATE_ACTIVE) return 0;


        s16 x = game_obj[idx].x;
        s16 y = game_obj[idx].y;
        s16 sx = game_obj[idx].sx;
        u16 blt_mask = 0;




        if (sx != 0) {
            x += 8;
            blt_mask = x % 8 < 4 ? 5 : 10;
        } else {
            y += 8;
            blt_mask = y % 8 < 4 ? 3 : 12;
        }



        map_addr = SCREEN_X(x) / 8 + SCREEN_Y(y) / 8 * SCREEN_W;
        objBltToMapCollision(idx, map_addr, blt_mask);
        if (sx == 0) {
            map_addr++;
        } else {
            map_addr += SCREEN_W;
        }
        objBltToMapCollision(idx, map_addr, blt_mask);
        objDrawBullet(idx);

    } else {

        objHandler_bltExplode(idx);
    }




    return 0;
}

u16 objHandler_bltExplode(u16 idx) {

    u8 master;

    if (game_obj[idx].state == STATE_EXPLODE) {
        game_obj[idx].sprite = 0;
        game_obj[idx].sx = 0;
        game_obj[idx].state |= STATE_ANIMATION;
    }

    *spr_ptr++ = SCREEN_Y(game_obj[idx].y) + 0x80;
    *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
    *spr_ptr++ = (256 - 16 + game_obj[idx].sprite * 4) | TILE_PRI(1);
    *spr_ptr++ = SCREEN_X(game_obj[idx].x) + 0x80;

    game_obj[idx].sx++;
    if (game_obj[idx].sx == 3) {
        game_obj[idx].sprite++;
        game_obj[idx].sx = 0;
    }

    if (game_obj[idx].sprite > 2) {
        game_obj[idx].state = 0;
        master = game_obj[idx].attrib;
        game_obj[master].blt_ctr--;
    }

    return 0;
}

u16 objHandler_tankExplode(u16 idx) {

    static const u16 exp_ani[] = {240, 244, 248, 208, 224, 248};
    static const u16 exp_siz[] = {2, 2, 2, 4, 4, 2};

    u16 i;
    if ((idx & 0xff00) != 0) {
        for (i = MISK_BASE; i < MAX_OBJECTS; i++) {
            if (game_obj[i].state == 0)break;
        }
        if (i == PLAY_BLT_BASE + PLAY_BLT_MAX)return MAX_OBJECTS;
        game_obj[i].state = STATE_EXPLODE;
        game_obj[i].handler = objHandler_tankExplode;
        return i;
    }


    if (game_obj[idx].state == STATE_EXPLODE) {
        game_obj[idx].sprite = 0;
        game_obj[idx].sx = 0;
        game_obj[idx].state |= STATE_ANIMATION;
    }

    u16 ani = game_obj[idx].sprite;
    u16 size = exp_siz[ani];
    u16 sprite = exp_ani[ani];
    u16 pos = size == 2 ? 0x80 : 0x78;

    *spr_ptr++ = SCREEN_Y(game_obj[idx].y) + pos;
    *spr_ptr++ = SPR_SIZE(size, size) << 8 | ++spr_ctr;
    *spr_ptr++ = sprite | TILE_PRI(1);
    *spr_ptr++ = SCREEN_X(game_obj[idx].x) + pos;

    game_obj[idx].sx++;
    if (game_obj[idx].sx == 6) {
        game_obj[idx].sprite++;
        game_obj[idx].sx = 0;
    }

    if (game_obj[idx].sprite > 5) {
        game_obj[idx].state = 0;
    }

    return 0;
}

u16 objHandler_enemy(u16 idx) {

    static const u16 scor[] = {0, 0, 0, 0, 100, 200, 300, 400};
    //const u16 scor[100, 200, 300, 400];
    u16 i;
    u16 tmp;
    static u16 resp_pos;

    if ((idx & 0xff00) != 0) {

        resp_pos++;
        if (resp_pos > 2)resp_pos = 0;
        for (i = ENEMY_BASE; i < ENEMY_BASE + ENEMY_MAX; i++) {
            if (game_obj[i].state == 0)break;
        }
        if (i == PLAY_BLT_BASE + PLAY_BLT_MAX)return MAX_OBJECTS;

        game_obj[i].state = STATE_BORN;
        game_obj[i].armor = 0;
        game_obj[i].attrib = idx;
        game_obj[i].handler = objHandler_enemy;
        game_obj[i].rotate = 2;
        game_obj[i].x = resp_pos == 1 ? 96 : resp_pos == 2 ? 192 : 0;
        game_obj[i].y = 0;
        game_obj[i].sprite = 256 + (idx & 7) * 32;
        //game_obj[i].blt_ctr = 0;
        game_obj[i].ctr2 = 0;
        game_obj[i].hitpoints = 1;
        tmp = (game_obj[i].attrib & 7);
        game_obj[i].score = scor[tmp];
        if (tmp == 7)game_obj[i].hitpoints = 4;
        if (tmp == 6)game_obj[i].attrib |= WEAPON1;


        return i;
    }


    if ((game_obj[idx].state & STATE_BORN)) {
        objHandleBorn(idx);
        return 0;
    }

    if (game_obj[idx].hitpoints < 1 && (game_obj[idx].state & STATE_ACTIVE))game_obj[idx].state = STATE_EXPLODE;

    if ((game_obj[idx].state & STATE_EXPLODE)) {
        objHandler_tankExplode(idx);
        return 0;
    }





    if ((game_tick % 2 == idx % 2 || (game_obj[idx].attrib & 7) == 5) && freeze == 0) {

        s16 sx;
        s16 sy;
        s16 speed = 1;
        game_obj[idx].sprite = (game_obj[idx].sprite & ~4) | (game_tick & 4);
        //if ((game_obj[idx].attrib & 7) == 5)speed = 2;
        switch (game_obj[idx].rotate) {
            case 0:
                sx = 0;
                sy = -speed;
                break;
            case 1:
                sx = -speed;
                sy = 0;
                break;
            case 2:
                sx = 0;
                sy = speed;
                break;
            default:
                sx = speed;
                sy = 0;
                break;
        }

        s16 x = game_obj[idx].x;
        s16 y = game_obj[idx].y;
        u16 collision;
        u16 tmp;
        u16 rotate = 0;


        if (sy != 0) {
            y += sy;
            tmp = x;
            x &= ~7;
            if (tmp % 8 > 3)x += 8;
        }
        if (sx != 0) {
            x += sx;
            tmp = y;
            y &= ~7;
            if (tmp % 8 > 3)y += 8;
        }

        game_obj[idx].sx = sx;
        game_obj[idx].sy = sy;
        collision = objTankToMapCollision(idx, x, y);

        collision |= objTankToTankCollision(idx, x, y);

        if (collision & 2) {
            x -= sx;
            y -= sy;

            if (game_obj[idx].ctr == 0) {
                game_obj[idx].ctr = 6;
            } else {
                game_obj[idx].ctr--;
                if (game_obj[idx].ctr == 1) rotate = 1;
            }


        } else {
            //if (rnd() % 256 == 0)rotate = 1;
            if (x % 8 == 0 && y % 8 == 0 && rnd() % 16 == 0)rotate = 1;
            game_obj[idx].ctr = 0;
        }

        if (rotate) {

            if (rnd() % 4 == 0)game_obj[idx].state ^= STATE_ROTATE_DIR;

            if ((game_obj[idx].state & STATE_ROTATE_DIR) == 0) {
                game_obj[idx].rotate--;
            } else {
                game_obj[idx].rotate++;
            }
            game_obj[idx].rotate %= 4;
        }

        game_obj[idx].x = x;
        game_obj[idx].y = y;

        /* if (game_obj[idx].ctr2 != 0) {
             game_obj[idx].ctr2--;
         }*/
        if (game_obj[idx].blt_ctr == 0 && rnd() % 32 == 0) {

            // game_obj[idx].ctr2 = 40 + rnd() % 16;

            i = OBJ_BULLET | (game_obj[idx].attrib & 0xff00) | idx;
            objMake(i);
        }
    }



    objDrawTank(idx);
    return 0;
}


#define BONUS_HELM 0
#define BONUS_TIME 1
#define BONUS_STAB 2
#define BONUS_STAR 3
#define BONUS_BOMB 4
#define BONUS_TANK 5

u16 objHandler_bonus(u16 idx) {

    u16 i;
    s16 y;
    s16 x;
    u16 tmp;

    static const u8 bonus[] = {BONUS_TANK, BONUS_STAR, BONUS_BOMB, BONUS_TIME, BONUS_HELM, BONUS_STAB, BONUS_BOMB, BONUS_TIME};

    if ((idx & 0xff00) != 0) {


        idx = BONUS_BASE;
        game_obj[idx].state = 1;
        game_obj[idx].attrib = bonus[rnd() % 8];
        //game_obj[idx].attrib = BONUS_STAB;
        game_obj[idx].handler = objHandler_bonus;


        for (i = 0; i < 16; i++) {
            x = rnd() % 16;
            y = rnd() % 16;
            if (x >= MAP_W / 2)x -= MAP_W / 2;
            if (y >= MAP_H / 2)y -= MAP_H / 2;
            x *= 16;
            y *= 16;
            //if (x > MAP_W * 8 - 16)x -= MAP_W / 2 * 8 - 16;
            //if (y > MAP_H * 8 - 16)y -= MAP_H / 2 * 8 - 16;
            tmp = screen_buff[getMapAddr(x, y)] & 255;

            if (tmp != 0) {
                if (tmp < 19)continue;
                if (tmp > 251)continue;
            }

            game_obj[idx].x = x;
            game_obj[idx].y = y;
            if (bonusCheckCollision(idx) != 0xffff)continue;
            break;
        }


        return idx;
    }

    i = bonusCheckCollision(idx);
    if (i != 0xffff) {

        gameIncScore(i, 500);
        objHandler_score(0xff00 | idx);

        game_obj[idx].state = 0;

        switch (game_obj[idx].attrib) {
            case BONUS_HELM:
                game_obj[i].armor = 10 * 60;
                break;
            case BONUS_TIME:
                freeze = 10 * 60;
                break;
            case BONUS_STAB:
                stab_armor = 0xffff;
                break;
            case BONUS_STAR:
                gameIncTankType(i);
                objUpdatePlayerTankSprite(i);
                break;
            case BONUS_BOMB:
                for (i = ENEMY_BASE; i < ENEMY_BASE + ENEMY_MAX; i++) {
                    if (game_obj[i].state & STATE_ACTIVE)game_obj[i].state = STATE_EXPLODE;
                }
                break;
            case BONUS_TANK:
                gameIncPlayerLive(i);
                break;
        }
    }


    if (game_tick % 32 < 16)return 0;
    *spr_ptr++ = 0x80 + SCREEN_Y(game_obj[idx].y);
    *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
    *spr_ptr++ = (128 + game_obj[idx].attrib * 4) | TILE_PRI(1);
    *spr_ptr++ = 0x80 + SCREEN_X(game_obj[idx].x);


    return 0;
}

void objHandler_stabArm() {

    static const u16 tile_map[] = {0, 1, 2, 3, SCREEN_W, SCREEN_W * 2, SCREEN_W + 3, SCREEN_W * 2 + 3};
    u16 init = 0;

    if (stab_armor == 0)return;
    if (stab_armor == 0xffff) {
        stab_armor = 20 * 60;
        init = 1;
    }

    u16 i;
    u16 arm_tile = 16;
    u16 *ptr = &screen_buff[getMapAddr(MAP_W / 2 * 8 - 16, MAP_H * 8 - 24)];
    stab_armor--;

    if (stab_armor < 4 * 60 && game_tick % 32 < 16) {
        arm_tile = 15;
    }

    if (stab_armor == 0)arm_tile = 15;
    if (stab_armor < 4 * 60)init = 1;

    if (init) {

        for (i = 0; i < 8; i++) {
            ptr[tile_map[i]] = arm_tile;
        }
    }


}

u16 objHandler_score(u16 idx) {

    u16 i;
    u16 master;

    if ((idx & 0xff00) != 0) {

        for (i = MISK_BASE; i < MAX_OBJECTS; i++) {
            if (game_obj[i].state == 0)break;
        }
        if (i == PLAY_BLT_BASE + PLAY_BLT_MAX)return MAX_OBJECTS;

        master = game_obj[i].attrib = idx & 0xff;
        game_obj[i].x = game_obj[master].x;
        game_obj[i].y = game_obj[master].y;
        if ((game_obj[master].attrib & OBJ_TANK)) {
            game_obj[i].sprite = 184 + (game_obj[master].attrib & 3) * 4;
        } else {
            game_obj[i].sprite = 58;
        }
        game_obj[i].handler = objHandler_score;
        game_obj[i].state = STATE_ACTIVE;
        game_obj[i].ctr = 60 * 1;
        game_obj[i].sprite |= TILE_PRI(1);

        switch (game_obj[idx & 0xff].attrib & 0xff00) {

        }

        return i;
    }

    *spr_ptr++ = 0x80 + SCREEN_Y(game_obj[idx].y);
    *spr_ptr++ = SPR_SIZE(2, 2) << 8 | ++spr_ctr;
    *spr_ptr++ = game_obj[idx].sprite;
    *spr_ptr++ = 0x80 + SCREEN_X(game_obj[idx].x);

    if (game_obj[idx].ctr-- == 0) {
        game_obj[idx].state = 0;
    }

    return 0;
}


//****************************************************************************** collision
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************

inline u16 objTankToTankCollision(u16 idx, u16 x, u16 y) {

    u16 i;
    s16 col_x;
    s16 col_y;
    s16 xx;
    s16 yy;


    col_x = x & ~7;
    if (x % 8 > 3)col_x += 8;
    col_y = y & ~7;
    if (y % 8 > 3)col_y += 8;

    for (i = PLAY_BASE; i < ENEMY_BASE + ENEMY_MAX; i++) {
        if (idx == i)continue;
        if ((game_obj[i].state & STATE_ACTIVE) == 0)continue;
        xx = game_obj[i].x & ~7;
        if (game_obj[i].x % 8 > 3)xx += 8;
        if (xx + 16 > col_x && xx < col_x + 16) {
            yy = game_obj[i].y & ~7;
            if (game_obj[i].y % 8 > 3)yy += 8;
            if (yy + 16 > col_y && yy < col_y + 16) {
                x -= game_obj[idx].sx;
                y -= game_obj[idx].sy;
                col_x = x & ~7;
                if (x % 8 > 3)col_x += 8;
                col_y = y & ~7;
                if (y % 8 > 3)col_y += 8;
                if (xx + 16 > col_x && xx < col_x + 16 && yy + 16 > col_y && yy < col_y + 16)return 0;
                //if(xx > 2 || yy > 2)return 0;
                return 2;
            }
        }
    }

    return 0;

}

u16 bonusCheckCollision(u16 idx) {

    u16 i;
    for (i = PLAY_BASE; i < PLAY_BASE + PLAY_MAX; i++) {

        if (game_obj[idx].x + 16 > game_obj[i].x && game_obj[idx].x < game_obj[i].x + 16) {
            if (game_obj[idx].y + 16 > game_obj[i].y && game_obj[idx].y < game_obj[i].y + 16) {
                return i;
            }
        }
    }

    return 0xffff;
}

inline u16 objTankToMapCollision(u16 idx, u16 x, u16 y) {

    u16 map_addr;
    s16 sx = game_obj[idx].sx;
    s16 sy = game_obj[idx].sy;
    u16 tile_idx1 = 0;
    u16 tile_idx2 = 0;
    u16 resp = 0;

    if (sx < 0) {
        map_addr = getMapAddr(x, y);
        tile_idx1 = screen_buff[map_addr];
        map_addr = getMapAddr(x, y + 15);
        tile_idx2 = screen_buff[map_addr];
    } else
        if (sx > 0) {
        map_addr = getMapAddr(x + 15, y);
        tile_idx1 = screen_buff[map_addr];
        map_addr = getMapAddr(x + 15, y + 15);
        tile_idx2 = screen_buff[map_addr];
    } else
        if (sy < 0) {
        map_addr = getMapAddr(x, y);
        tile_idx1 = screen_buff[map_addr];
        map_addr = getMapAddr(x + 15, y);
        tile_idx2 = screen_buff[map_addr];
    } else
        if (sy > 0) {
        map_addr = getMapAddr(x, y + 15);
        tile_idx1 = screen_buff[map_addr];
        map_addr = getMapAddr(x + 15, y + 15);
        tile_idx2 = screen_buff[map_addr];
    }

    if (tile_idx1 != 0 || tile_idx2 != 0) {
        if (tile_idx1 == 33 || tile_idx2 == 33)resp |= 1;
        if ((tile_idx1 != 0 && tile_idx1 < 19) || (tile_idx2 != 0 && tile_idx2 < 19))resp |= 2;
        if((tile_idx1 & 0xff) > 251 || (tile_idx2 & 0xff) > 251)resp |= 2;
    }

    return resp;
}

inline void objBltToMapCollision(u16 idx, u16 map_addr, u16 blt_mask) {

    u16 tile_idx = screen_buff[map_addr];
    if (tile_idx > 0) {

        if (tile_idx < 16) {
            if ((tile_idx & blt_mask) != 0) {
                game_obj[idx].state = STATE_EXPLODE;
                screen_buff[map_addr] &= (blt_mask ^ 0x0f);
            }
            return;
        }

        if (tile_idx == 16 && (game_obj[idx].attrib & WEAPON) == WEAPON3) screen_buff[map_addr] = 0;

        if (tile_idx < 18) {
            game_obj[idx].state = STATE_EXPLODE;
            return;
        }

        if (tile_idx > 251 && tile_idx < 256) {
            game_over = 1;
            game_obj[idx].state = STATE_EXPLODE;
            return;
        }
    }
}

void objBltToObjCollision(u16 idx) {

    u16 x = game_obj[idx].x;
    u16 y = game_obj[idx].y;
    u16 sx = game_obj[idx].sx;
    u16 x2;
    u16 y2;
    u16 i;
    u16 max;

    u8 master;

    if ((game_obj[idx].attrib & TEAM) < TEAM2) {
        max = PLAY_BLT_BASE + PLAY_BLT_MAX + ENEMY_BLT_MAX;
    } else {
        max = PLAY_BLT_BASE + PLAY_BLT_MAX;
    }

    if (sx != 0) {
        y += 4;
    } else {
        x += 4;
    }


    for (i = PLAY_BLT_BASE; i < max; i++) {
        if (idx == i)continue;
        if ((game_obj[i].state & STATE_ACTIVE) == 0)continue;
        x2 = game_obj[i].x;
        y2 = game_obj[i].y;

        if (sx != 0) {
            y2 += 4;
        } else {
            x2 += 4;
        }

        if (x + 4 > x2 && x < x2 + 4) {
            if (y + 4 > y2 && y < y2 + 4) {
                game_obj[idx].state = 0;
                game_obj[i].state = 0;
                game_obj[(u8) game_obj[idx].attrib].blt_ctr--;
                game_obj[(u8) game_obj[i].attrib].blt_ctr--;
                return;
            }
        }
    }

    if ((game_obj[idx].attrib & TEAM) < TEAM2) {
        max = PLAY_BASE + PLAY_MAX + ENEMY_MAX;
    } else {
        max = PLAY_BASE + PLAY_MAX;
    }


    master = game_obj[idx].attrib;

    if (sx != 0) {
        x += 6;
    } else {
        y += 6;
    }

    for (i = PLAY_BASE; i < max; i++) {
        if (master == i)continue;
        if ((game_obj[i].state & STATE_ACTIVE) == 0)continue;


        if (x + 5 > game_obj[i].x && x < game_obj[i].x + 16) {
            if (y + 5 > game_obj[i].y && y < game_obj[i].y + 16) {


                if ((game_obj[master].attrib & TEAM) < TEAM2 && (game_obj[i].attrib & TEAM) < TEAM2) {
                    game_obj[idx].state = STATE_EXPLODE;
                    if (game_obj[i].armor == 0)game_obj[i].ctr2 = 3 * 60;

                } else {
                    game_obj[idx].state = STATE_EXPLODE;
                    if (game_obj[i].armor == 0) {
                        game_obj[i].hitpoints--;
                        if (game_obj[i].attrib & OBJ_BONUS) {
                            objMake(OBJ_BONUS);
                            game_obj[i].attrib &= ~OBJ_BONUS;
                        }

                        if (game_obj[i].hitpoints == 0 && (game_obj[i].attrib & TEAM) > TEAM1) {

                            objHandler_score(0xff00 | i);
                            gameIncKills(master, game_obj[i].attrib & 3);
                        }
                    }
                }
                return;
            }
        }
    }

}



