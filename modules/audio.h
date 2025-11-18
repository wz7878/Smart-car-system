// modules/audio.h
#ifndef AUDIO_H
#define AUDIO_H

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
#include "../ui/ui_events.h"


#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdatomic.h>

void init_song_list();
bool init_pipe();
void send_mplayer_cmd(const char *cmd);
FILE* start_mplayer();

extern void* play_music(void *arg);
extern void next_song();
extern void prev_song();
extern void toggle_play();
extern void adjust_volume(int delta);
extern void stop_play();


#endif