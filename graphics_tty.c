#include "graphics.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#ifdef _WINDOWS
    #include <windows.h>
    #define GET_TTY_SIZE(ptrc, ptrr) GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);*ptrc = csbi.srWindow.Right - csbi.srWindow.Left + 1;*ptrr = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
    #define GET_TTY_SIZE(ptrc, ptrr) struct winsize w;ioctl(0, TIOCGWINSZ, &w);*ptrc=w.ws_col;*ptrr=w.ws_row;
    struct termios orig_termios;
    void disable_raw_mode(){
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
    void enable_raw_mode(){
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disable_raw_mode);
        struct termios raw = orig_termios;
        raw.c_iflag &= ~(IXON); raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
#endif


glyph_t* buffer;    // will be used as 2d array in future, holds glyphs
unsigned short rows, columns;
unsigned short cursor_row = 0;
unsigned short cursor_column = 0;

glyph_t construct_glyph(unsigned char c, unsigned char f){
    glyph_t g;
    g.character = c;
    g.flags = f;
    return g;
}
int start_graphics(){
    GET_TTY_SIZE(&columns, &rows);
    buffer = malloc(columns*rows*sizeof(glyph_t));
    for(int i = 0; i < columns*rows; i++){
        buffer[i].character = ' ';
        buffer[i].flags = 0;
    }
    // initialise glyphs for text display
    // Just ascii characters, for text to work
    for(int i = 0; i < 128; i++){
        glyphs[i] = construct_glyph(i, 0);
    }
    fputs("\e[?47h\e[?25l\e[0;0H\e[0J", stdout);
    enable_raw_mode();
}
int move(unsigned short n_ccolumn, unsigned short n_crow){
    cursor_column = n_ccolumn;
    cursor_row = n_crow;
}
void outc(unsigned char g_id, unsigned char f){
    glyph_t g = glyphs[g_id];
    g.flags = f;
    buffer[cursor_row*columns+cursor_column] = g;
    if(cursor_column == columns-1){
        cursor_column = 0;
        cursor_row++;
    } else {
        cursor_column++;
    }
}
int poutc(unsigned char g_id, unsigned char f, unsigned short column, unsigned short row){
    move(column, row);
    outc(g_id, f);
}
void print(char* string, unsigned char f){
    size_t i = 0;
    while(string[i] != '\0'){
        outc(string[i], f);
        i++;
    }
}
void update(){
    for(int row = 0; row < rows; row++){
        for(int column = 0; column < columns; column++){
            glyph_t g = buffer[row*columns+column];
            if(check_flag(g.flags, 0)){
                fputs("\e[1m", stdout);
            }
            if(check_flag(g.flags, 1)){
                fputs("\e[3m", stdout);
            }
            if(check_flag(g.flags, 2)){
                fputs("\e[4m", stdout);
            }
            if(check_flag(g.flags, 3)){
                fputs("\e[7m", stdout);
            }
            if(check_flag(g.flags, 4)){
                fputs("\e[2m", stdout);
            }
            if(check_flag(g.flags, 5)){
                fputs("\e[5m", stdout);
            }
            fputc(g.character, stdout);
            fputs("\e[0m", stdout);
            //printf("%02X", g.character);
        }
    }
}
void clear(){
    for(int i = 0; i < columns*rows; i++){
        buffer[i].character = ' ';
        buffer[i].flags = 0;
    }
}
void end_graphics(){
    free(buffer);
    disable_raw_mode();
    fputs("\e[?47l\e[?25h", stdout);
}