#include "include/packet.h"

static void broadcast(void *msg, size_t size) {
    player *it;
    for (it = players; it; it = it->next) {
        send(it->fd, msg, size, MSG_DONTWAIT);
    }
}

void send_lobby_status(void) {
    size_t size = 2 + 25 * (size_t) player_count;
    uint8_t *msg = calloc(size, 1);
    msg[0] = LOBBY_STATUS;
    msg[1] = player_count;
    int i;
    player *it;
    for (i = 0, it = players; i < player_count && it; ++i, it = it->next) {
        msg[2 + i * 25] = it->id;
        memcpy(msg + 3 + i * 25, it->name, 23);
        msg[(i + 1) * 25 + 1] = it->ready;
    }
    broadcast(msg, size);
    free(msg);
}

void send_game_start(uint8_t *field, uint8_t w, uint8_t h) {
    size_t size = 2 + 29 * (size_t) player_count + 2 + w * h + 2;
    uint8_t *msg = calloc(size, 1);
    msg[0] = GAME_START;
    msg[1] = player_count;
    int i;
    player *it;
    for (i = 0, it = players; it; ++i, it = it->next) {
        msg[2 + i * 29] = it->id;
        memcpy(2 + msg + i * 29 + 1, it->name, 23);
        uint16_t x = (uint16_t) round(it->x * 10);
        uint16_t y = (uint16_t) round(it->y * 10);
        memcpy(2 + msg + i * 29 + 24, &x, 2);
        memcpy(2 + msg + i * 29 + 26, &y, 2);
        memcpy(2 + msg + i * 29 + 28, &it->direction, 1);
    }
    size_t offset = (size_t) (2 + 29 * player_count);
    msg[offset++] = w;
    msg[offset++] = h;
    memcpy(msg + offset, field, w * h);
    offset += w * h;

    // dynamite timer in s
    msg[offset++] = DYNAMITE_TIMER;

    // dynamite slide velocity in u/10s
    msg[offset] = DYNAMITE_SLIDE_V * 10;

    broadcast(msg, size);
    free(msg);
}

typedef struct {
    uint8_t id;
    uint8_t frags;
} score;

static int score_compar(const void *ptr1, const void *ptr2) {
    return ((score *) ptr1)->frags - ((score *) ptr2)->frags;
}

void send_game_over(void) {
    score *scores = malloc(player_count * sizeof(score));
    int i;
    player *it;
    for (i = 0, it = players; it; ++i, it = it->next) {
        scores[i].id = it->id;
        scores[i].frags = it->frags;
    }
    qsort(scores, player_count, sizeof(score), score_compar);
    size_t size = 2 + player_count;
    uint8_t *msg = malloc(size);
    msg[0] = GAME_OVER;
    msg[1] = player_count;
    for (i = 0; i < player_count; ++i) {
        msg[i + 2] = scores[i].id;
    }
    free(scores);
    broadcast(msg, size);
    free(msg);
}

void send_objects(uint16_t timer) {
    size_t max_size = (size_t) 7 + dyn_cnt * 4 + flame_cnt * 2 + pwrup_cnt * 3 + player_count * 11;
    size_t size = 0;
    uint8_t *msg = malloc(max_size);
    msg[size++] = OBJECTS;
    memcpy(msg + size, &timer, 2);
    size += 2;

    msg[size++] = dyn_cnt;
    dyn_t *dyn;
    for (dyn = dynamites; dyn; dyn = dyn->next) {
        uint16_t x = (uint16_t) round(dyn->x * 10);
        uint16_t y = (uint16_t) round(dyn->y * 10);
        memcpy(msg + size, &x, 2);
        memcpy(msg + size + 2, &y, 2);
        size += 4;
    }

    msg[size++] = flame_cnt;
    flame_t *flame;
    for (flame = flames; flame; flame = flame->next) {
        msg[size++] = flame->x;
        msg[size++] = flame->y;
    }

    msg[size++] = pwrup_cnt;
    pwrup_t *pwrup;
    for (pwrup = pwrups; pwrup; pwrup = pwrup->next) {
        msg[size++] = pwrup->x;
        msg[size++] = pwrup->y;
        msg[size++] = pwrup->type;
    }

    msg[size++] = player_count;
    player *it;
    for (it = players; it; it = it->next) {
        msg[size++] = it->id;
        msg[size++] = it->dead;
        if (!it->dead) {
            uint16_t x = (uint16_t) round(it->x * 10);
            uint16_t y = (uint16_t) round(it->y * 10);
            memcpy(msg + size, &x, 2);
            memcpy(msg + size + 2, &y, 2);
            size += 4;
            msg[size++] = it->direction;
            msg[size++] = it->active_pwrups;
            msg[size++] = it->power;
            msg[size++] = it->speed;
            msg[size++] = it->count;
        }
    }
    broadcast(msg, size);
    free(msg);
}

void send_map_update(void) {
    size_t size = (size_t) 3 + map_upd_cnt * 3;
    char *msg = malloc(size);
    msg[0] = MAP_UPDATE;
    memcpy(msg + 1, &map_upd_cnt, 2);
    int i = 3;
    map_upd_t *map_upd;
    while ((map_upd = map_updates)) {
        map_updates = map_upd->next;
        msg[i++] = map_upd->x;
        msg[i++] = map_upd->y;
        msg[i++] = map_upd->block;
        free(map_upd);
    }
    map_upd_cnt = 0;
    broadcast(msg, size);
    free(msg);
}