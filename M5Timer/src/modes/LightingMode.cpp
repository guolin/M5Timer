#include "LightingMode.h"
#include <M5Unified.h>
#include "../core/LEDMatrix.h"

// 声明外部全局变量
extern LEDMatrix ledMatrix;

// 定义颜色值
const uint32_t LightingMode::colorValues[5] = {
    0xFF0000,  // 红色
    0xFFFF00,  // 黄色
    0x0000FF,  // 蓝色
    0x00FF00,  // 绿色
    0xFFFFFF   // 白色
};

// 颜色对应的名称
const char* colorNames[5] = {
    "Red",     // 红色
    "Yellow",  // 黄色
    "Blue",    // 蓝色
    "Green",   // 绿色
    "White"    // 白色
};

// 颜色对应的LCD颜色值 (16位RGB565格式)
const uint16_t lcdColors[5] = {
    0xF800,    // 红色
    0xFFE0,    // 黄色
    0x001F,    // 蓝色
    0x07E0,    // 绿色
    0xFFFF     // 白色
};

LightingMode::LightingMode() : Mode("Lighting") {
    // 初始化为最低亮度和默认颜色(白色)
    brightnessLevel = 0;
    colorIndex = 4;  // 默认白色
    needDisplayUpdate = true;
}

void LightingMode::begin() {
    Serial.println("Entering Lighting Mode");
    
    // 初始化LED矩阵
    ledMatrix.begin();
    
    // 设置为最低亮度
    brightnessLevel = 0;
    
    // 更新LCD显示
    updateDisplay();
    
    // 更新LED矩阵
    updateLEDs();
    
    // 确保亮度设置生效
    ledMatrix.getStrip().setBrightness(brightnessValues[brightnessLevel]);
    ledMatrix.getStrip().show();
}

void LightingMode::update() {
    if (needDisplayUpdate) {
        updateDisplay();
        needDisplayUpdate = false;
    }
}

void LightingMode::exit() {
    // 清除LED矩阵
    ledMatrix.clear();
    ledMatrix.update();
    
    // 恢复默认亮度
    ledMatrix.getStrip().setBrightness(BRIGHTNESS);
    ledMatrix.getStrip().show(); // 直接调用show确保亮度设置生效
}

void LightingMode::handleEvent(EventType event) {
    switch (event) {
        case EVENT_BUTTON_A:
            // 按A键切换亮度
            brightnessLevel = (brightnessLevel + 1) % 10;
            
            // 先设置LED像素
            updateLEDs();
            
            // 然后设置亮度并立即显示 - 确保亮度设置不被覆盖
            ledMatrix.getStrip().setBrightness(brightnessValues[brightnessLevel]);
            ledMatrix.getStrip().show();
            
            Serial.print("Brightness changed to level ");
            Serial.print(brightnessLevel + 1);
            Serial.print("/10 (");
            Serial.print(brightnessValues[brightnessLevel]);
            Serial.println(")");
            
            // 标记需要更新显示
            needDisplayUpdate = true;
            break;
            
        case EVENT_BUTTON_B:
            // 按B键切换颜色
            colorIndex = (colorIndex + 1) % 5;
            
            Serial.print("Color changed to ");
            Serial.print(colorNames[colorIndex]);
            Serial.print(" (0x");
            Serial.print(colorValues[colorIndex], HEX);
            Serial.println(")");
            
            // 更新LED显示
            updateLEDs();
            
            // 确保亮度设置不被覆盖
            ledMatrix.getStrip().setBrightness(brightnessValues[brightnessLevel]);
            ledMatrix.getStrip().show();
            
            // 标记需要更新显示
            needDisplayUpdate = true;
            break;
            
        default:
            // 其他事件暂时不处理
            break;
    }
}

void LightingMode::updateDisplay() {
    // 清空显示
    M5.Display.fillScreen(BLACK);
    
    // 获取当前颜色
    uint16_t currentColor = lcdColors[colorIndex];
    
    // 设置文本样式 - 使用4号字体（大尺寸）
    M5.Display.setTextColor(currentColor, BLACK);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(4, 30);
    M5.Display.println("Lighting");
    
    // 切换回标准字体大小
    M5.Display.setTextSize(2);
    
    // 显示当前颜色
    M5.Display.setCursor(4, 110);
    M5.Display.print("Color: ");
    M5.Display.print(colorNames[colorIndex]);
    
    // 显示亮度百分比
    int brightnessPercent = (brightnessLevel + 1) * 10; // 10% 到 100%
    
    
    // 绘制亮度条
    int barWidth = 200;
    int barHeight = 20;
    int fillWidth = barWidth * brightnessPercent / 100;
    
    // 绘制边框 - 使用当前颜色
    M5.Display.drawRect(4, 80, barWidth, barHeight, currentColor);
    
    // 填充亮度条 - 使用当前颜色
    M5.Display.fillRect(4, 80, fillWidth, barHeight, currentColor);
}

void LightingMode::updateLEDs() {
    // 先清除所有像素
    // ledMatrix.clear();
    
    // 获取当前选择的颜色
    uint32_t currentColor = colorValues[colorIndex];
    
    // 将所有LED设置为当前颜色
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            ledMatrix.setPixel(x, y, currentColor);
        }
    }
    
    // 只更新像素值，不更新亮度
    ledMatrix.update();
} 