#ifndef LVGL_DEFAULT_SCR_H
#define LVGL_DEFAULT_SCR_H

#include "lvgl.h"

#define LVGL_WHITE_COLOR lv_color_hex(0xFFFFFF)
#define LVGL_BLACK_COLOR lv_color_hex(0x000000)
#define LVGL_DARK_BLUE_COLOR lv_color_hex(0x195592)
#define LVGL_DARK_RED_COLOR lv_color_hex(0x8B0000)

/*
Initialize the start screen with time display and buttons    
*/
void init_lvgl_default_scr();
void add_back_btn(lv_obj_t *scr);

#endif // LVGL_DEFAULT_SCR_H