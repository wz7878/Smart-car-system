#include "main.h"

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "800");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "480");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

pthread_mutex_t mutex;

// 在程序初始化时调用（如 main 函数或启动播放线程前）
void init_signals() {
    // 忽略 SIGPIPE，防止管道破裂导致进程中断
    signal(SIGPIPE, SIG_IGN);
}

// 获取当前时间（用于显示在界面上）
void gain_time()
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    // 在界面上显示时间（假设有一个标签对象 ui_now_time）
    lv_label_set_text(ui_now_time, time_str);
}

int main(void)
{
    lv_init();
    if(pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("Mutex init failed\n");
        return -1;
    }
    /*Linux display device init*/
    lv_linux_disp_init();

#if LV_USE_SDL
    // init input device
    lv_sdl_mouse_create();
    lv_sdl_keyboard_create();
    lv_sdl_mousewheel_create();
#endif

#if LV_USE_LINUX_FBDEV
    // 创建输入设备
    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    // 校准输入设备屏幕坐标
    // lv_evdev_set_calibration(touch, 0, 0, 1024, 600); // 黑色边框的屏幕
    lv_evdev_set_calibration(touch, 0, 0, 800, 480);  // 蓝色边框的屏幕
#endif
    
    ui_init();

    gain_time(); // 初始化时间显示

    init_signals();

    //音乐
    pthread_t tid_music;
    if(pthread_create(&tid_music, NULL, play_music, NULL)!= 0)
    {
        printf("Failed to create music thread\n");
    }

    // 获取家居设备状态线程
    pthread_t tid_device;
    if(pthread_create(&tid_device, NULL, get_device_status, NULL)!= 0)
    {
        printf("Failed to create device thread\n");
    }

    // 创建获取二维码图片的线程
    pthread_t tid_qr;
    if(pthread_create(&tid_qr, NULL, get_qr, NULL)!= 0)
    {
        printf("Failed to create QR thread\n");
    }
    
    // 创建获取天气信息的线程
    pthread_t tid;
    if(pthread_create(&tid, NULL, http_get_weather, NULL)!= 0)
    {
        printf("Failed to create weather thread\n");
    }


    /*Handle LVGL tasks*/
    while (1)
    {
        pthread_mutex_lock(&mutex);
        lv_timer_handler();
        pthread_mutex_unlock(&mutex);
        usleep(5000);
    }
    return 0;
}


