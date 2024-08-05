// NOTE: THIS WORKS IF const uint64_t IS 64 BITS!!!
#include <stdint.h>

#define FONT(ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8) ROW1 ## ROW2 ## ROW3 ## ROW4 ## ROW5 ## ROW6 ## ROW7 ## ROW8
#define TEMPLATE(...)

// Template
TEMPLATE(
0b0000000,
00000000,
00000000,
00000000,
00000000,
00000000,
00000000,
00000000
)

// No need for a space character since the renderer basically takes care of that for us


const uint64_t FONT_A =
     FONT(0b00111100,
            01100110,
            01100110,
            01111110,
            11000011,
            11000011,
            11000011,
            11000011);

const uint64_t FONT_B =
     FONT(0b11111100,
            11000011,
            11000011,
            11111100,
            11111100,
            11000011,
            11000011,
            11111100);

const uint64_t FONT_C =
     FONT(0b00111111,
            11000000,
            11000000,
            11000000,
            11000000,
            11000000,
            11000000,
            00111111);

const uint64_t FONT_D =
     FONT(0b11110000,
            11001100,
            11000011,
            11000011,
            11000011,
            11000011,
            11001100,
            11110000);

const uint64_t FONT_E =
     FONT(0b11111111,
            11111111,
            11000000,
            11111111,
            11111111,
            11000000,
            11111111,
            11111111);

const uint64_t FONT_F =
     FONT(0b11111111,
            11111111,
            11000000,
            11111111,
            11111111,
            11000000,
            11000000,
            11000000);

const uint64_t FONT_G =
     FONT(0b01111111,
            11111111,
            11000000,
            11011110,
            11011111,
            11000011,
            11000011,
            11111110);

                     
const uint64_t FONT_H =
    FONT(0b11000011,
            11000011,
            11000011,
            11111111,
            11111111,
            11000011,
            11000011,
            11000011);

const uint64_t FONT_I =
     FONT(0b11111111,
            11111111,
            00011000,
            00011000,
            00011000,
            00011000,
            11111111,
            11111111);

const uint64_t FONT_J =
     FONT(0b00000011,
            00000011,
            00000011,
            00000011,
            00000011,
            00000011,
            00000011,
            11111100);

const uint64_t FONT_K =
     FONT(0b11000111,
            11001100,
            11011000,
            11110000,
            11110000,
            11011000,
            11001100,
            11000111);

const uint64_t FONT_L =
     FONT(0b11000000,
            11000000,
            11000000,
            11000000,
            11000000,
            11000000,
            11000000,
            11111111);

const uint64_t FONT_M =
     FONT(0b11000011,
            11100111,
            11111111,
            11011011,
            11011011,
            11000011,
            11000011,
            11000011);

const uint64_t FONT_N =
     FONT(0b11000011,
            11100011,
            11110011,
            11110011,
            11011011,
            11011011,
            11001111,
            11000111);

const uint64_t FONT_O =
     FONT(0b01111110,
            11111111,
            11000011,
            11000011,
            11000011,
            11000011,
            11111111,
            01111110);

const uint64_t FONT_P =
     FONT(0b11111110,
            11111111,
            11000011,
            11000011,
            11111111,
            11111110,
            11000000,
            11000000);

const uint64_t FONT_Q =
     FONT(0b01111110,
            11111111,
            11000011,
            11000011,
            11000011,
            11111111,
            01111110,
            00000011);

const uint64_t FONT_R =
     FONT(0b11111110,
            11000011,
            11000011,
            11111110,
            11110000,
            11011000,
            11001100,
            11000111);

const uint64_t FONT_S =
     FONT(0b01111111,
            11000000,
            11000000,
            00111100,
            00000011,
            00000011,
            00000011,
            11111110);

const uint64_t FONT_T =
     FONT(0b11111111,
            11111111,
            00011000,
            00011000,
            00011000,
            00011000,
            00011000,
            00011000);

const uint64_t FONT_U =
     FONT(0b11000011,
            11000011,
            11000011,
            11000011,
            11000011,
            11000011,
            11000011,
            01111110);

const uint64_t FONT_V = 
     FONT(0b11000011,
            11000011,
            11000011,
            01100110,
            01100110,
            00111100,
            00111100,
            00011000);

const uint64_t FONT_W =
     FONT(0b11000011,
            11000011,
            11000011,
            11000011,
            11011011,
            11011011,
            11100111,
            11000011);

const uint64_t FONT_X =
     FONT(0b11000011,
            11000011,
            01100110,
            00111100,
            00111100,
            01100110,
            11000011,
            11000011);

const uint64_t FONT_Y =
     FONT(0b11000011,
            11000011,
            01100110,
            00111100,
            00011000,
            00011000,
            00011000,
            00011000);

const uint64_t FONT_Z =
     FONT(0b11111111,
            11111111,
            00000111,
            00011110,
            01111000,
            11100000,
            11111111,
            11111111);

const uint64_t FONT_0 =
     FONT(0b11111111,
            10000001,
            10000001,
            10000001,
            10000001,
            10000001,
            10000001,
            11111111);

const uint64_t FONT_1 =
     FONT(0b00011000,
            00011000,
            00011000,
            00011000,
            00011000,
            00011000,
            00011000,
            00011000);

const uint64_t FONT_2 =
     FONT(0b11111111,
            00000011,
            00000110,
            00001100,
            00011000,
            00110000,
            01100000,
            11111111);

const uint64_t FONT_3 =
     FONT(0b11111100,
            00000010,
            00000001,
            00001110,
            00001110,
            00000001,
            00000010,
            11111100);

const uint64_t FONT_4 =
     FONT(0b10000001,
            10000001,
            10000001,
            11111111,
            00000001,
            00000001,
            00000001,
            00000001);

const uint64_t FONT_5 =
     FONT(0b11111111,
            10000000,
            10000000,
            11111111,
            00000001,
            00000001,
            00000001,
            11111111);

const uint64_t FONT_6 =
     FONT(0b01111100,
            10000000,
            10000000,
            10000000,
            10111100,
            10000010,
            10000010,
            01111100);

const uint64_t FONT_7 =
     FONT(0b11111111,
            00000001,
            00000010,
            00000100,
            00001000,
            00010000,
            00100000,
            01000000);

const uint64_t FONT_8 =
     FONT(0b01111110,
            10000001,
            10000001,
            01111110,
            10000001,
            10000001,
            10000001,
            01111110);

const uint64_t FONT_9 =
     FONT(0b01111110,
            10000001,
            10000001,
            01111111,
            00000001,
            00000001,
            00000001,
            00000001);

const uint64_t FONT_COLON =
     FONT(0b00000000,
            00011000,
            00011000,
            00000000,
            00011000,
            00011000,
            00000000,
            00000000
);

const uint64_t FONT_CHAR_TABLE[256] = {
    0,  // Hopefully this sets the rest of the elements to zero (fingers crossed but probably not)
    ['A'] = FONT_A,
    ['B'] = FONT_B,
    ['C'] = FONT_C,
    ['D'] = FONT_D,
    ['E'] = FONT_E,
    ['F'] = FONT_F,
    ['G'] = FONT_G,
    ['H'] = FONT_H,
    ['I'] = FONT_I,
    ['J'] = FONT_J,
    ['K'] = FONT_K,
    ['L'] = FONT_L,
    ['M'] = FONT_M,
    ['N'] = FONT_N,
    ['O'] = FONT_O,
    ['P'] = FONT_P,
    ['Q'] = FONT_Q,
    ['R'] = FONT_R,
    ['S'] = FONT_S,
    ['T'] = FONT_T,
    ['U'] = FONT_U,
    ['V'] = FONT_V,
    ['W'] = FONT_W,
    ['X'] = FONT_X,
    ['Y'] = FONT_Y,
    ['Z'] = FONT_Z,
    ['0'] = FONT_0,
    ['1'] = FONT_1,
    ['2'] = FONT_2,
    ['3'] = FONT_3,
    ['4'] = FONT_4,
    ['5'] = FONT_5,
    ['6'] = FONT_6,
    ['7'] = FONT_7,
    ['8'] = FONT_8,
    ['9'] = FONT_9,
    [':'] = FONT_COLON
};