#include <GLPS/glps_window_manager.h>


int main()
{
    glps_WindowManager *wm = glps_wm_init();

    glps_wm_window_create(wm, "test x11", 400, 400);

    while(!glps_wm_should_close(wm)) {
        glps_wm_window_update(wm, 0);
    }

    glps_wm_destroy(wm);

    return 0;
}