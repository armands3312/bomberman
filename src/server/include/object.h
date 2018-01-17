#ifndef BOMBERMAN_OBJECT_H
#define BOMBERMAN_OBJECT_H

#include <math.h>
#include "player.h"
#include "protocol.h"

#define DYNAMITE_TIMER 3
#define DYNAMITE_SLIDE_V 6
#define FLAME_TIMEOUT 0.5
#define PWRUP_TIMEOUT 20

typedef struct dyn {
    time_t created;
    player_t *owner;
    player_t *carrier;
    double x;
    double y;
    int remote_detonated;
    player_t *kicked_by;
    uint8_t slide_direction;
    uint8_t power;
} dyn_t;

extern dyn_t dynamites[256];
extern uint8_t dyn_cnt;

void dyn_create(time_t cur_time, player_t *owner);

void dyn_destroy(int i);

typedef struct flame {
    time_t created;
    player_t *owner;
    uint8_t x;
    uint8_t y;
    uint8_t spawn_pwrup_type;
} flame_t;

extern flame_t flames[256];
extern uint8_t flame_cnt;

int flame_create(time_t created, player_t *owner, uint8_t x, uint8_t y);

void flame_destroy(int i);

typedef struct pwrup {
    time_t created;
    uint8_t x;
    uint8_t y;
    uint8_t type;
} pwrup_t;

extern pwrup_t pwrups[256];
extern uint8_t pwrup_cnt;

void pwrup_create(time_t cur_time, uint8_t x, uint8_t y, uint8_t type);

void pwrup_destroy(int i);

typedef struct map_upd {
    uint8_t x;
    uint8_t y;
    uint8_t block;
    struct map_upd *next;
} map_upd_t;

extern map_upd_t *map_updates;
extern uint16_t map_upd_cnt;

void map_upd_create(uint8_t x, uint8_t y, uint8_t block);

void cleanup_objects(void);

void remove_player_objects(player_t *player);

#endif //BOMBERMAN_OBJECT_H
