#ifndef FB_H
#define FB_H

#define FB_WIDTH            80
#define FB_HEIGHT           25
#define FB_TAB              8
#define FB_CELLS            (FB_WIDTH * FB_HEIGHT)

/* The Colors usable by text */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_BROWN   14
#define COLOR_WHITE         15

void fb_clear(void);
int fb_write(const char * buf, unsigned int len);

#endif