#ifndef _AGARIO_DEFS_H
#define _AGARIO_DEFS_H

#define SNAPSHOTS_DELAY 50
#define INITIAL_PLAYERS_SIZE 20
#define INITIAL_PICKABLES_COUNT 20
#define INITIAL_PICKABLES_SIZE 5

const double kClientInputMinTime = 0.05f;
const enet_uint16 kClientMovementRate = 8; //change positions to float

/* server commands */
#define C_PLAYER_CONNECTED 0
#define C_PLAYER_INIT_OWN 1
#define C_INITIAL_PICKABLES 2
#define C_SPAWN_PICKABLES 3
#define C_PLAYERS_SNAPSHOT 4
/* client commands*/
#define C_MOVE_RIGHT 5
#define C_MOVE_LEFT 6
#define C_MOVE_UP 7
#define C_MOVE_DOWN 8


#endif //!_AGARIO_DEFS_H