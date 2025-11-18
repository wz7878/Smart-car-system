#ifndef __MAIN_H
#define __MAIN_H

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
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
// #include <cjson/cJSON.h>
#include "./ui/ui.h"

#include "./modules/weather.h"
#include "./modules/QRcode.h"
#include "./modules/device.h"
#include "./modules/audio.h"


// 互斥锁（保护LVGL操作）
// pthread_mutex_t mutex;
void init_signals();

#endif /* __MAIN_H */