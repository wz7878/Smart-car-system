#ifndef __MODULES_WEATHER_H
#define __MODULES_WEATHER_H

#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include <cjson/cJSON.h>
#include "../ui/ui.h"

void *http_get_weather(char *arg);
void choice_weather(lv_obj_t *img, char *typ);
void choice_week(lv_obj_t *label, char *wk,
                lv_obj_t *tmph ,char *temh,
                lv_obj_t *tmpl, char *teml);

#endif // __MODULES_WEATHER_H