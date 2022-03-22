#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "biohazard.h"
#include "graphics.h"

int main(void){
    start_graphics();
    char a = getchar();
    poutc(a, FLAG_ITALIC, 40, 10);
    update();
    getchar();
    end_graphics();
}