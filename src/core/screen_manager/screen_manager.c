#include "screen_manager.h"
#include "../../ui/main_ui.h"

void screen_manager_show(lv_obj_t *screen)
{
    if (screen != NULL) {
        lv_disp_load_scr(screen);
    }
}

void screen_manager_show_home(void)
{
    screen_manager_show(ui_start_screen);
}
