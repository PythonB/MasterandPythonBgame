#include "biohazard.h"

void init_player(player_t* player, char* name){
    init_inventory(&player->inventory);
    strcpy(player->name, name);
}
void move_player(player_t* player, unsigned short x, unsigned short y, char z){
    player->position.x = x;
    player->position.y = y;
    player->position.z = z;
}
