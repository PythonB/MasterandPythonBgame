#include "biohazard.h"

void init_inventory(inventory_t* inventory){
    inventory->items = malloc(0);
    inventory->item_count = 0;
}
void push_to_inventory(inventory_t* inventory, item_t* item){
    inventory->item_count++;
    inventory->items = realloc(inventory->items, inventory->item_count*sizeof(item_t));
    printf("new inv size: %lu\n", inventory->item_count*sizeof(item_t));
    inventory->items[inventory->item_count-1] = *item;
}
void get_from_inventory(inventory_t* inventory, item_t* item, size_t i){
    if(i > inventory->item_count-1){
        item = NULL;
        return;
    }
    *item = inventory->items[i];
}
void pop_from_inventory(inventory_t* inventory, item_t* item, size_t i){
    if(i > inventory->item_count-1){
        item = NULL;
        return;
    }
    *item = inventory->items[i];
    item_t* move_to = inventory->items+i*sizeof(item_t);
    item_t* move_from = inventory->items+(i+1)*sizeof(item_t);
    size_t size_to_move = (inventory->item_count-i)*sizeof(item_t);
    memmove(move_to, move_from, size_to_move);
    inventory->item_count--;
    inventory->items = realloc(inventory, inventory->item_count*sizeof(item_t));
}
void free_inventory(inventory_t* inventory){
    free(inventory->items);
}