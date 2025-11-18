#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <errno.h>
#include <signal.h>
#include "../main.h"
#include "audio.h"

#define MUSIC_DIR "/home/wz/lvgl/lv_port_linux_sdl_gec6818/music"  // 音乐文件目录
// #define MUSIC_DIR "/home/wz/music"
#define MPLAYER_PIPE "/home/wz/pipe"                               // 控制管道路径
#define MAX_SONGS 100                                               // 最大歌曲数

// 歌曲结构体（双向循环链表节点）
typedef struct Song {
    char name[100];   // 歌曲名
    char path[256];   // 歌曲路径
    struct Song *prev;// 上一首指针
    struct Song *next;// 下一首指针
} Song;

// 全局变量
Song *song_list = NULL;       // 歌曲链表头
Song *current_song = NULL;    // 当前播放歌曲
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER; // 链表互斥锁
atomic_bool is_playing = false; // 播放状态（原子变量保证线程安全）
int pipe_fd = -1;             // 管道文件描述符
pid_t mplayer_pid = -1;       // MPlayer进程ID
pthread_t player_thread;      // 播放线程ID


// ---------- 歌曲列表初始化 ----------
void init_song_list() {
    DIR *dir = opendir(MUSIC_DIR);
    if (!dir) {
        perror("无法打开音乐目录");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 仅处理普通文件且为.mp3后缀
        struct stat file_stat;
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", MUSIC_DIR, entry->d_name);

        if (stat(full_path, &file_stat) == 0 &&       // 获取文件状态成功
            S_ISREG(file_stat.st_mode) &&             // 是普通文件
            strstr(entry->d_name, ".mp3")) {          // 后缀为.mp3

            Song *new_song = malloc(sizeof(Song));
            if (!new_song) continue; // 内存不足则跳过

            // 填充歌曲信息
            snprintf(new_song->name, sizeof(new_song->name), "%s", entry->d_name);
            snprintf(new_song->path, sizeof(new_song->path), "%s/%s", MUSIC_DIR, entry->d_name);

            // 线程安全地插入链表
            pthread_mutex_lock(&list_mutex);
            if (!song_list) { // 链表为空，新节点为头
                song_list = new_song;
                new_song->prev = new_song;
                new_song->next = new_song;
            } else { // 插入到链表尾部（双向循环链表）
                new_song->prev = song_list->prev;
                new_song->next = song_list;
                song_list->prev->next = new_song;
                song_list->prev = new_song;
            }
            pthread_mutex_unlock(&list_mutex);

            printf("歌曲%s已添加到播放列表\n",new_song->name);
        }
    }
    closedir(dir);
}

// ---------- 管道初始化 ----------
bool init_pipe() {
    // 检查并创建管道（若不存在）
    if (access(MPLAYER_PIPE, F_OK) == -1) {
        if (mkfifo(MPLAYER_PIPE, 0666) == -1) {
            perror("创建管道失败");
            return false;
        }
    }
    
    // 以「读写+非阻塞」模式打开管道
    pipe_fd = open(MPLAYER_PIPE, O_RDWR | O_NONBLOCK);
    if (pipe_fd < 0) {
        perror("打开管道失败");
        return false;
    }
    return true;
}

void send_mplayer_cmd(const char *cmd) {
    // 检查 MPlayer 进程是否存活
    if (mplayer_pid != -1 && kill(mplayer_pid, 0) != 0 && errno == ESRCH) {
        printf("MPlayer 进程已退出，需重新启动\n");
        return;
    }

    if (pipe_fd < 0) {
        printf("管道未初始化\n");
        return;
    }

    // 命令需以换行符结尾（MPlayer 以此识别命令结束）
    char full_cmd[256];
    snprintf(full_cmd, sizeof(full_cmd), "%s\n", cmd);
    
    ssize_t bytes_written = write(pipe_fd, full_cmd, strlen(full_cmd));
    if (bytes_written < 0) {
        if (errno == EAGAIN) {
            // 非阻塞模式下暂时无法写入，属于正常现象
            printf("管道暂时忙碌（EAGAIN），稍后重试\n");
        } else if (errno == EPIPE) {
            // 管道破裂，重新初始化管道
            printf("管道破裂（EPIPE），正在重新初始化...\n");
            close(pipe_fd);
            init_pipe(); // 重新创建并打开管道
        } else {
            perror("发送命令到 MPlayer 失败");
            // 其他错误，尝试重建管道
            close(pipe_fd);
            init_pipe();
        }
    }
}

// ---------- 启动MPlayer进程（内部函数） ----------
FILE* start_mplayer() {
    int pipefd[2];
    if (pipe(pipefd) == -1) { // 创建父子进程通信管道
        perror("pipe创建失败");
        return NULL;
    }

    mplayer_pid = fork(); // 创建子进程
    if (mplayer_pid == -1) {
        perror("fork失败");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    } else if (mplayer_pid == 0) { // 子进程：启动MPlayer
        close(pipefd[0]); // 关闭读端
        // 重定向标准输出/错误到管道（便于父进程监听状态）
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        // 启动MPlayer（-nolirc禁用红外遥控，避免无意义错误）
        execlp("mplayer", "mplayer", "-slave", "-quiet", "-idle", "-nolirc", 
               "-input", "file=/home/wz/pipe", NULL);
        _exit(1); // execlp失败时退出子进程
    } else { // 父进程：返回文件流供监听
        close(pipefd[1]); // 关闭写端
        FILE *mplayer = fdopen(pipefd[0], "r");
        if (!mplayer) {
            perror("fdopen失败");
            close(pipefd[0]);
        }
        return mplayer;
    }
    return NULL;
}

// ---------- 播放控制线程 ----------
void* play_music(void *arg) {
    // 1. 初始化歌曲列表
    init_song_list();
    
    // 2. 初始化管道
    if (!init_pipe()) {
        return NULL;
    }

    // 3. 启动MPlayer进程
    FILE *mplayer = start_mplayer();
    if (!mplayer) {
        perror("无法启动mplayer");
        close(pipe_fd);
        return NULL;
    }

    // 4. 首次播放第一首歌曲
    pthread_mutex_lock(&list_mutex);
    if (song_list) {
        current_song = song_list;
        char play_cmd[512];
        snprintf(play_cmd, sizeof(play_cmd), "loadfile \"%s\"", current_song->path);
        send_mplayer_cmd(play_cmd);
        is_playing = true;
    }
    pthread_mutex_unlock(&list_mutex);

    // 5. 监听MPlayer输出（处理播放完成、状态等）
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), mplayer)) {
        // 播放完成时，自动切换下一首
        if (strstr(buffer, "Exiting... (End of file)")) {
            pthread_mutex_lock(&list_mutex);
            if (current_song && is_playing) {
                current_song = current_song->next;
                char play_cmd[512];
                snprintf(play_cmd, sizeof(play_cmd), "loadfile \"%s\"", current_song->path);
                send_mplayer_cmd(play_cmd);
            }
            pthread_mutex_unlock(&list_mutex);
        }
    }

    system("echo 'pause' >   /home/wz/pipe");

    // 6. 清理资源
    fclose(mplayer);
    close(pipe_fd);
    pipe_fd = -1;
    mplayer_pid = -1;
    is_playing = false;
    return NULL;
}

// ---------- 启动播放线程（对外接口） ----------
void start_player() {
    if (pthread_create(&player_thread, NULL, play_music, NULL) != 0) {
        perror("创建播放线程失败");
    }
}

// ---------- 切换下一首 ----------
void next_song() {
    pthread_mutex_lock(&list_mutex);
    if (current_song && is_playing) {
        current_song = current_song->next;
        char play_cmd[512];
        snprintf(play_cmd, sizeof(play_cmd), "loadfile \"%s\"", current_song->path);
        send_mplayer_cmd(play_cmd);
    }
    pthread_mutex_unlock(&list_mutex);
}

// ---------- 切换上一首 ----------
void prev_song() {
    pthread_mutex_lock(&list_mutex);
    if (current_song && is_playing) {
        current_song = current_song->prev;
        char play_cmd[512];
        snprintf(play_cmd, sizeof(play_cmd), "loadfile \"%s\"", current_song->path);
        send_mplayer_cmd(play_cmd);
    }
    pthread_mutex_unlock(&list_mutex);
}

// ---------- 暂停/继续播放 ----------
void toggle_play() {
    send_mplayer_cmd("pause");
    is_playing = !is_playing;
}

// ---------- 调节音量（delta为正数增大，负数减小） ----------
void adjust_volume(int delta) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "volume %d 1", delta);
    send_mplayer_cmd(cmd);
}

// ---------- 停止播放 ----------
void stop_play() {
    send_mplayer_cmd("stop");
    is_playing = false;
}






