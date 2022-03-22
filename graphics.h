// Header file to wrap all graphics functions, to make it backend independent
// Different implementations (SDL or curses) supported
#ifndef GRAPHICS_H_
#define GRAPHICS_H_
#include <stdbool.h>

#define check_flag(byte, num) (byte & (1 << num)) != 0

typedef enum _Flags {
    FLAG_NONE = 0,
    FLAG_BOLD = 0b00000001,
    FLAG_ITALIC = 0b00000010,
    FLAG_UNDERLINED = 0b00000100,
    FLAG_INVERSE = 0b00001000,
    FLAG_DIM = 0b00010000,
    FLAG_BLINKING = 0b00100000
} Flags;
typedef enum _Colors {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    B_BLACK,
    B_RED,
    B_GREEN,
    B_YELLOW,
    B_BLUE,
    B_MAGENTA,
    B_CYAN,
    B_WHITE
} Colors;
typedef struct _glyph {
    unsigned char character;    // 00-7F for text mode, and 80-FF for pictures in graphical mode
    unsigned char flags;        // From LSB to MSB
                                // 0 - bold
                                // 1 - italic
                                // 2 - underlined
                                // 3 - inverse
                                // 4 - dim
                                // 5 - strikethrough
    Colors fg;
    Colors bg;
} glyph_t;
glyph_t construct_glyph(unsigned char c, unsigned char f);
glyph_t glyphs[256];
void set_graphics_config();
int start_graphics();
int move(unsigned short n_ccolumn, unsigned short n_crow);
void outc(unsigned char g_id, unsigned char f);
int poutc(unsigned char g_id, unsigned char f, unsigned short column, unsigned short row);
void print(char* string, unsigned char f);
void clear();
void update();
void end_graphics();
#endif