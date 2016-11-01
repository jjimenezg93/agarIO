#ifndef _AGARIO_DEFS_H
#define _AGARIO_DEFS_H

const unsigned short int kSnapshotsDelay = 50;
const unsigned char kInitialPlayersSize = 20;

const unsigned char kMinInitialPickables = 2;
const unsigned char kMaxInitialPickables = 20;
const unsigned char kMinPickablesSize = 4;
const unsigned char kMaxPickablesSize = 12;

const unsigned short int kScreenWidth = 800;
const unsigned short int kScreenHeight = 600;

const double kClientInputMinTime = 0.05f;
const enet_uint16 kClientMovementRate = 8; //TODO: change positions to float

/* server commands */
#define C_PLAYER_CONNECTED 0
#define C_PLAYER_INIT_OWN 1
#define C_DESPAWN_ENTITY 2
#define C_DISCONNECT_PLAYER 3
#define C_SPAWN_PICKABLES 4
#define C_PICKABLES_SNAPSHOT 5
#define C_PLAYERS_SNAPSHOT 6
/* client commands*/
#define C_MOVE_RIGHT 7
#define C_MOVE_LEFT 8
#define C_MOVE_UP 9
#define C_MOVE_DOWN 10


#endif //!_AGARIO_DEFS_H