/* SPDX-License-Identifier: MIT */
#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/types.h>
#include "kobo_fb.h"

#include <sys/ioctl.h>
#include <stdint.h>
#include "lvgl.h"

static struct mxcfb_update_data fullUpdRegion;
/* static struct mxcfb_update_marker_data wait_for_update_data; */
static int fb0fd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

static uint32_t current_update = 0;

void kobo_force_update(lv_area_t *area)
{
	/* Setup the update region data structure. */
	if(area != NULL) {
		/*
		wait_for_update_data.update_marker = fullUpdRegion.update_marker;
		wait_for_update_data.collision_test = 0;
		ioctl(fb0fd,  MXCFB_WAIT_FOR_UPDATE_COMPLETE, (unsigned long int)&wait_for_update_data);
		*/
		fullUpdRegion.update_region.top = area->y1;
		fullUpdRegion.update_region.left = area->x1;
		fullUpdRegion.update_region.width = (area->x2 - area->x1 + 1);
		fullUpdRegion.update_region.height = (area->y2 - area->y1 + 1);
	}
	fullUpdRegion.update_mode = UPDATE_MODE_PARTIAL;
	fullUpdRegion.flags = 0;
	fullUpdRegion.update_marker = current_update++;
	/* Send the update request to the eInk chip */
	ioctl(fb0fd , MXCFB_SEND_UPDATE, (unsigned long int)&fullUpdRegion);
}

char oldrot;
void kobo_deinitialize(void)
{
	/* Restore the previous rotation */
	int sysfd = open("/sys/class/graphics/fb0/rotate", O_WRONLY);
	if(sysfd >= 0) {
		write(sysfd, &oldrot, 1);
		close(sysfd);
	}
}
int kobo_initialize(void)
{
	/* First set the screen rotation to be 0 (landscape on Aura) */
	int sysfd = open("/sys/class/graphics/fb0/rotate", O_RDWR);
	if(sysfd >= 0) {
		char c = '0';
		read(sysfd, &oldrot, 1);
		write(sysfd, &c, 1);
		close(sysfd);
	}
	/* Now open the framebuffer and collect the necessary information */
	fb0fd = open("/dev/fb0", O_RDWR);
	if(fb0fd == -1)
		return -1;
	
	ioctl(fb0fd, FBIOGET_FSCREENINFO, &finfo);

	/* Set common options in the update region data structure */
	fullUpdRegion.update_marker = 999;
    fullUpdRegion.update_region.top = 0;
    fullUpdRegion.update_region.left = 0;
    fullUpdRegion.update_region.width = vinfo.xres - 1;
    fullUpdRegion.update_region.height = vinfo.yres - 1;
    fullUpdRegion.waveform_mode = 3;
    fullUpdRegion.update_mode = UPDATE_MODE_FULL;
    fullUpdRegion.temp = TEMP_USE_AMBIENT;
    fullUpdRegion.flags = 0;

	/* Update the screen once */
    kobo_force_update(NULL);

	printf("kobo hw initialized\n");
    return fb0fd;
}