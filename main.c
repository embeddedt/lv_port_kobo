
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl.h"

#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme_1.h"

/*********************
 *     PROTOTYPES
 *********************/

extern void kobo_force_update(lv_area_t *);
extern int kobo_initialize(void);

static void fbdev_monitor(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px);
static void my_feedback(lv_indev_drv_t * indev, uint8_t event);
static void kobo_flush(lv_disp_drv_t *disp, const lv_area_t * area, lv_color_t * color_p);

/*********************
 *      DEFINES
 *********************/

#define DISP_BUF_SIZE (80*LV_HOR_RES_MAX)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_area_t refresh_area;

/**********************
 *      MACROS
 **********************/
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /* Kobo HW specific init */
    kobo_initialize();
    /*Linux frame buffer device init*/
    fbdev_init();
    /*Linux input device init*/
    evdev_init();
    

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = kobo_flush;
    disp_drv.monitor_cb = fbdev_monitor;
    lv_disp_drv_register(&disp_drv);

    /*Initialize and register an input device*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv.feedback_cb = my_feedback;
    lv_indev_drv_register(&indev_drv);

    /*Create a Demo*/
    lv_theme_set_current(lv_theme_mono_init(210, NULL));
    lv_test_theme_1(lv_theme_get_current());

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void fbdev_monitor(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px) {
    /* The eInk screen on this hardware must be manually "kicked" after the framebuffer contents are changed */
    kobo_force_update(&refresh_area);
    /* Zero the dirty area (as it has now been updated) */
    lv_area_set(&refresh_area, 0, 0, 0, 0);
}

static void kobo_flush(lv_disp_drv_t *disp, const lv_area_t * area, lv_color_t * color_p) {
    /* Store the dirty area */
    lv_area_t tmp;
    memcpy(&tmp, &refresh_area, sizeof(lv_area_t));
    lv_area_join(&refresh_area, &tmp, area);
    /* Flush as usual */
    fbdev_flush(disp, area, color_p);
}

static void my_feedback(lv_indev_drv_t * indev, uint8_t event)
{
   if(event == LV_EVENT_RELEASED || event == LV_EVENT_PRESSED) {
       /* Force an immediate redraw of the screen (regardless of LV_REFR_PERIOD) */
	   lv_refr_now(lv_disp_get_default());
   }
}
