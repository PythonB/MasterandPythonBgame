#ifndef BIOHAZARD_H_
#define BIOHAZARD_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include "graphics.h"

#define is_unbreakable(item) item.condition==-1 ? true ; false

typedef enum {BASIC, MELEEWEAPON, RANGEDWEAPON, ARMOR} item_type_t;
typedef enum {HEAD, ARMS, TORSO, LEGS, FEET} body_part_t;
typedef char percent_t;

typedef struct _coords {                         // Coordination triplet X Y Z
    unsigned short x;
    unsigned short y;
    char z;
} coords_t;
typedef struct _item {                          // All possible properties of item in game
    char label[48];
    unsigned int weight;
    item_type_t type;
    unsigned int price;
    unsigned int amount;
    percent_t condition; // -1 for unbreakable
    // Melee Weapon fields
    unsigned int damage;
    // Ranged fields
    percent_t accuracy;
    struct _item* ammo;
    struct _item* scope;
    struct _item* stock;
    // Armor fields
    body_part_t target_body_part;
    percent_t protection;
} item_t;
typedef struct _inventory {                     // Structure for storing inventory data
    item_t* items;
    size_t item_count;
} inventory_t;
typedef struct _container {                     // Container description, as in-game entity
    char label[48];
    bool is_locked;
    percent_t hardeness;
    coords_t position;
    inventory_t inventory;
} container_t;
typedef struct _stats {                         // Statblock for creatures
    int hp;
    int maxhp;
    unsigned char strength;
    unsigned char agility;
    unsigned char toughness;
    unsigned char intelegence;
} stats_t;
typedef struct _traits {
    // I need to fill it, but later
} traits_t;
typedef struct _plskill {                        // Player skill structure
    // I need to fill it too, but later, again
} plskill_t;
typedef struct _player {                        // Main structure for player
    char name[24];
    stats_t statblock;
    // Traits and skills fields
    coords_t position;
    inventory_t inventory;
} player_t;
typedef struct _tile {
    char label[32];
    glyph_t glyph;
    bool passable;
} tile_t;
// Inventory functions
void init_inventory(inventory_t* inventory);
void push_to_inventory(inventory_t* inventory, item_t* item);
void get_from_inventory(inventory_t* inventory, item_t* item, size_t i);
void pop_from_inventory(inventory_t* inventory, item_t* item, size_t i);
void free_inventory(inventory_t* inventory);
// Player functions
void init_player(player_t* player, char* name);
void move_player(player_t* player, unsigned short x, unsigned short y, char z);
// World function

#endif