#include "QRcode.h"

lv_image_dsc_t *qrcode_generate_lv91(const char *text, int scale) {
    if (text == NULL || scale <= 0) return NULL;

    // 1. 生成原始二维码数据（libqrencode核心调用）
    QRcode *qr = QRcode_encodeString(
        text,
        0,                  // 自动选择版本
        QR_ECLEVEL_M,       // 中等纠错等级（15%容错）
        QR_MODE_8,          // 8位数据模式（支持中文）
        1                   // 区分大小写
    );
    if (qr == NULL) return NULL;

    // 2. 计算图像尺寸（原始尺寸*放大倍数）
    int qr_size = qr->width;       // 原始二维码尺寸（如21x21）
    int img_size = qr_size * scale; // 放大后尺寸

    // 3. 分配LVGL 9.1图像描述符内存
    // 像素格式：LV_COLOR_FORMAT_RGB565（2字节/像素）
    lv_image_dsc_t *img = malloc(sizeof(lv_image_dsc_t) + img_size * img_size * 2);
    if (img == NULL) {
        QRcode_free(qr);
        return NULL;
    }

    // 4. 初始化图像描述符（LVGL 9.1格式）
    img->header.w = img_size;                      // 宽度
    img->header.h = img_size;                      // 高度
    img->header.cf = LV_COLOR_FORMAT_RGB565;       // 颜色格式（LVGL 9.x宏定义变更）
    img->data_size = img_size * img_size * 2;      // 数据总大小（字节）
    img->data = (uint8_t *)&img[1];                // 像素数据起始地址（紧接描述符之后）

    // 5. 填充像素数据（黑白二维码）
    uint16_t *pixel_buf = (uint16_t *)img->data;   // RGB565像素缓冲区
    for (int y = 0; y < img_size; y++) {
        for (int x = 0; x < img_size; x++) {
            // 映射到原始二维码坐标
            int qr_x = x / scale;
            int qr_y = y / scale;

            // 原始数据：0=白色背景，1=黑色码点
            uint8_t pixel = qr->data[qr_y * qr_size + qr_x] & 0x01;
            
            // RGB565颜色值（LVGL 9.x使用lv_color_t的新定义）
            uint16_t color = pixel ? 0x0000 : 0xFFFF;  // 黑/白
            pixel_buf[y * img_size + x] = color;
        }
    }

    // 6. 释放临时资源
    QRcode_free(qr);
    return img;
}

void* get_qr(void *arg)
{
    // 1. 生成套餐1的支付二维码（包含套餐信息）
    lv_image_dsc_t *qrcode1 = qrcode_generate_lv91(
        "年度套餐19.9/月充值完成",
        3  // 放大倍数
    );
    if (qrcode1 == NULL) return;
    
    lv_image_set_src(ui_QR_code, qrcode1);  // 设置二维码图像

    // 2. 显示套餐2二维码
    lv_image_dsc_t *qrcode2 = qrcode_generate_lv91("月度套餐59.9/月充值完成",3);
    if (qrcode2 == NULL) return;
    lv_image_set_src(ui_QR_code2, qrcode2);  // 设置二维码图像

    // 2. 显示套餐2二维码
    lv_image_dsc_t *qrcode3 = qrcode_generate_lv91("季度套餐109.9/月充值完成",3);
    if (qrcode3 == NULL) return;
    lv_image_set_src(ui_QR_code3, qrcode3);  // 设置二维码图像
}