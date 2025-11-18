#include "device.h"

// 线程互斥锁（保护LVGL UI操作和MQTT发布）
static pthread_mutex_t mqtt_lv_mutex = PTHREAD_MUTEX_INITIALIZER;
// 全局MQTT客户端（用于发布消息，需在初始化时赋值）
static struct mosquitto *g_mosq = NULL;

// MQTT发布消息函数（提取为通用接口）
int mqtt_publish_msg(const char *topic, const char *msg) 
{
    // if (g_mosq == NULL || !mosquitto_connect(g_mosq, "120.26.29.100", 1883, 60)) {
    //     printf("MQTT未连接，发布失败: %s\n", msg);
    //     return -1;
    // }
    if(mosquitto_connect(g_mosq, "120.26.29.100", 1883, 60) != MOSQ_ERR_SUCCESS) {
        printf("MQTT未连接，发布失败: %s\n", msg);
        return -1;
    }
    mosquitto_loop_start(g_mosq);
    if(mosquitto_publish(g_mosq, NULL, topic, strlen(msg), msg, 0, false)== MOSQ_ERR_SUCCESS) 
    {
        printf("MQTT发布成功: %s\n", msg);
        return 0;
    }
}

// ------------------------------
// MQTT接收消息处理（设备状态同步到UI）
// ------------------------------

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {  
    if (message->payloadlen == 0) {
        printf("%s (null)\n", message->topic);
        return;
    }

    char *payload = (char *)message->payload;
    printf("收到消息: 主题=%s, 内容=%s\n", message->topic, payload);

    pthread_mutex_lock(&mqtt_lv_mutex);
    // 空调状态同步（主题：temp）
    if (strcmp(message->topic, "temp") == 0) {
        lv_label_set_text(ui_aircondit_sta, payload);
        
        char *on_pos = strstr(payload, "开");
        char *off_pos = strstr(payload, "关");
        if (on_pos != NULL && strcmp(on_pos, "开") == 0) 
        {
            lv_obj_set_state(ui_airSwitch, LV_STATE_CHECKED, true);
        } 
        else if (off_pos != NULL && strcmp(off_pos, "关") == 0) 
        {
            lv_obj_set_state(ui_airSwitch, LV_STATE_CHECKED, false);
        }
    }
    // 灯光状态同步（主题：lightonoff）
    else if (strcmp(message->topic, "lightonoff") == 0) {
        lv_label_set_text(ui_lamp_sta, payload);
        
        char *on_pos = strstr(payload, "开");
        char *off_pos = strstr(payload, "关");
        if (on_pos != NULL && strcmp(on_pos, "开") == 0) 
        {
            lv_obj_set_state(ui_lightSwitch, LV_STATE_CHECKED, true);  // 修正为开关对象
        } 
        else if (off_pos != NULL && strcmp(off_pos, "关") == 0) 
        {
            lv_obj_set_state(ui_lightSwitch, LV_STATE_CHECKED, false); // 修正为开关对象
        }
    }
    // 电视状态同步（主题：tvonoff）
    else if (strcmp(message->topic, "tvonoff") == 0) {
        lv_label_set_text(ui_television_sta, payload);
        
        char *on_pos = strstr(payload, "开");
        char *off_pos = strstr(payload, "关");
        if (on_pos != NULL && strcmp(on_pos, "开") == 0) 
        {
            lv_obj_set_state(ui_telSwitch, LV_STATE_CHECKED, true);  // 修正为开关对象
        } 
        else if (off_pos != NULL && strcmp(off_pos, "关") == 0) 
        {
            lv_obj_set_state(ui_telSwitch, LV_STATE_CHECKED, false); // 修正为开关对象
        }
    }
    pthread_mutex_unlock(&mqtt_lv_mutex);
}

// ------------------------------
// MQTT初始化与线程
// ------------------------------

int mqtt_init_and_subscribe(void) 
{
    // MQTT初始化
    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
        printf("mosquitto库初始化失败\n");
        return -1;
    }

    // 创建MQTT客户端
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        printf("创建MQTT客户端失败\n");
        mosquitto_lib_cleanup();
        return -1;
    }

    // 保存全局客户端实例（供发布消息使用）
    g_mosq = mosq;

    // mosquitto_message_callback_set(mosq, on_message);

    int ret = mosquitto_connect(mosq, "120.26.29.100", 1883, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("连接MQTT服务器失败: %s\n", mosquitto_strerror(ret));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        g_mosq = NULL;
        return -1;
    }

    // 订阅设备状态主题（接收设备状态）
    const char *topics[] = {"temp", "lightonoff", "tvonoff"};
    for (int i = 0; i < 3; i++) {
        ret = mosquitto_subscribe(mosq, NULL, topics[i], 0);
        if (ret != MOSQ_ERR_SUCCESS) {
            printf("订阅主题 %s 失败: %s\n", topics[i], mosquitto_strerror(ret));
        } else {
            printf("已订阅主题: %s\n", topics[i]);
        }
    }

    // 设置消息回调函数
    mosquitto_message_callback_set(mosq, on_message);

    // 循环处理消息
    ret = mosquitto_loop_forever(mosq, -1, 1);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("MQTT消息循环异常: %s\n", mosquitto_strerror(ret));
    }

    // 清理资源
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    g_mosq = NULL;
    return ret;
}

void* get_device_status(void *arg) {
    mqtt_init_and_subscribe();
    return NULL;
}
