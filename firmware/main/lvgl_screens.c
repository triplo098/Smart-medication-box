#include "lvgl_screens.h"
#include "display_driver.h"


static void see_reminders_btn_event_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED) {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press
    
        init_medicine_list_scr();

    } else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Code to handle button release
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }

}


void init_start_scr(lv_obj_t *scr)
{           
    if(scr == NULL){
        scr = lv_obj_create(NULL);
    }
    lv_obj_set_size(scr, 200, 200);
    lv_obj_center(scr);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_shadow_width(scr, 0, 0);

    // Time label
    time_label = lv_label_create(scr);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 40);
    lv_timer_create(lvgl_update_time_cb, 5000, NULL);
    lvgl_update_time_cb(); // Initial call to set time immediately

    lv_obj_t *btn1 = lv_btn_create(scr);
    lv_obj_set_size(btn1, 160, 30);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_radius(btn1, 8, 0);

    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "See reminders");
    lv_obj_center(label1);
    lv_obj_add_event_cb(btn1, see_reminders_btn_event_handler, LV_EVENT_ALL, NULL);
    

    lv_obj_t *btn2 = lv_btn_create(scr);
    lv_obj_set_size(btn2, 160, 30);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 45);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_radius(btn2, 8, 0);

    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Show medicine list");
    lv_obj_center(label2);

    lv_screen_load(scr);
}


void back_to_start_btn_event_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED) {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press
    
        init_start_scr(NULL);

    } else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }

}

void init_medicine_list_scr()
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t * label = lv_label_create(scr);
    lv_label_set_text(label, "Medicine List Screen");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);


    //Add back button
    lv_obj_t * back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 80, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, back_to_start_btn_event_handler, LV_EVENT_ALL, NULL);


    lv_screen_load(scr);
}

