#ifndef __DEVICE_H
#define __DEVICE_H

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
// #include <cjson/cJSON.h>
#include "../ui/ui.h"
#include "../main.h"

#include <mosquitto.h>

// pthread_mutex_t mqtt_lv_mutex = PTHREAD_MUTEX_INITIALIZER;
// struct mosquitto *g_mosq = NULL;
// extern struct mosquitto *g_mosq;

void* get_device_status(void *arg);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
int mqtt_init_and_subscribe(void);
int mqtt_publish_msg(const char *topic, const char *msg);

#endif // __DEVICE_H