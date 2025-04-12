#pragma once

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

// LED配置
#define NUM_LEDS    64      // 8x8矩阵
#define LED_PIN     0       // LED数据引脚
#define BRIGHTNESS  51      // 亮度20% (255 * 0.2 ≈ 51)

// 定义颜色
#define COLOR_RED    0xFF0000
#define COLOR_GREEN  0x00FF00
#define COLOR_BLUE   0x0000FF
#define COLOR_YELLOW 0xFFFF00

class LEDMatrix {
public:
    LEDMatrix();
    void begin();
    void update();
    void clear();
    void clearAll();
    void showTestPattern();
    
    // 像素操作
    void setPixel(int x, int y, uint32_t color);
    void setPixel(int index, uint32_t color);
    
    // 数字显示相关方法
    void showNumber(int number, uint32_t color = COLOR_GREEN);  // 默认绿色
    void showTwoNumbers(int leftNum, int rightNum, uint32_t leftColor = COLOR_BLUE, uint32_t rightColor = COLOR_YELLOW);
    void startCountdown(int fromNumber);                     // 开始倒计时
    void updateCountdown();                                  // 更新倒计时

private:
    Adafruit_NeoPixel strip;
    int getIndex(int x, int y);  // 添加坐标转换方法
    void showDigit(int digit, int xOffset, uint32_t color);
    
    // 像素缓存
    uint32_t pixelCache[NUM_LEDS];    // 存储每个像素的当前颜色
    bool pixelChanged[NUM_LEDS];      // 标记每个像素是否发生变化
    bool needsFullUpdate;             // 是否需要完全更新
    
    // 倒计时相关变量
    int currentNumber;
    unsigned long lastUpdateTime;
    static const int COUNTDOWN_INTERVAL = 500;  // 500ms
}; 