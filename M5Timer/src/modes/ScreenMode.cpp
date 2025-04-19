#include "ScreenMode.h"
#include <M5Unified.h>
#include "../core/LEDMatrix.h"

// 声明外部全局变量
extern LEDMatrix ledMatrix;

// 定义动画帧数
const int ANIMATION_FRAMES = 16;  // 16个不同的帧，每行按顺序轮换

// 全局 ScreenMode 实例指针
static ScreenMode* screenModeInstance = nullptr;

// 24位RGB颜色定义 (标准RGB格式，适用于NeoPixel)
#define NEOPIXEL_BLACK   0x000000
#define NEOPIXEL_RED     0xFF0000
#define NEOPIXEL_GREEN   0x00FF00
#define NEOPIXEL_BLUE    0x0000FF
#define NEOPIXEL_YELLOW  0xFFFF00
#define NEOPIXEL_PURPLE  0xFF00FF
#define NEOPIXEL_CYAN    0x00FFFF
#define NEOPIXEL_WHITE   0xFFFFFF
#define NEOPIXEL_ORANGE  0xFF8000
#define NEOPIXEL_DGREEN  0x008000
#define NEOPIXEL_DBLUE   0x000080
#define NEOPIXEL_BROWN   0x964B00
#define NEOPIXEL_PINK    0xFF69B4
#define NEOPIXEL_LCYAN   0xE0FFFF
#define NEOPIXEL_LPINK   0xFFB6C1
#define NEOPIXEL_GRAY    0x808080

#define FRAME_HEADER 0xAA
#define FRAME_TAIL 0x55
#define FRAME_LENGTH 66  // 帧头(1) + 数据(64) + 校验和(1)

ScreenMode::ScreenMode() : Mode("Screen") {
    // 保存实例指针
    screenModeInstance = this;
    
    // 初始化颜色映射表 - 使用24位RGB值
    colorMap[0] = NEOPIXEL_BLACK;   // 0: 黑色（关闭）
    colorMap[1] = NEOPIXEL_RED;     // 1: 红色
    colorMap[2] = NEOPIXEL_GREEN;   // 2: 绿色
    colorMap[3] = NEOPIXEL_BLUE;    // 3: 蓝色
    colorMap[4] = NEOPIXEL_YELLOW;  // 4: 黄色
    colorMap[5] = NEOPIXEL_PURPLE;  // 5: 紫色
    colorMap[6] = NEOPIXEL_CYAN;    // 6: 青色
    colorMap[7] = NEOPIXEL_WHITE;   // 7: 白色
    colorMap[8] = NEOPIXEL_ORANGE;  // 8: 橙色
    colorMap[9] = NEOPIXEL_DGREEN;  // 9: 暗绿色
    colorMap[10] = NEOPIXEL_DBLUE;  // 10: 暗蓝色
    colorMap[11] = NEOPIXEL_BROWN;  // 11: 棕色
    colorMap[12] = NEOPIXEL_PINK;   // 12: 粉色
    colorMap[13] = NEOPIXEL_LCYAN;  // 13: 淡青色
    colorMap[14] = NEOPIXEL_LPINK;  // 14: 浅粉色
    colorMap[15] = NEOPIXEL_GRAY;   // 15: 灰色
    
    // 初始化屏幕数据
    memset(screenData, 0, sizeof(screenData));
    
    // 初始化动画参数
    isTestMode = false;
    currentFrame = 0;
    animationStartTime = 0;
    frameIntervalMs = 200; // 200ms 每帧
    lastFrameTime = 0;
}

void ScreenMode::begin() {
    Serial.println("进入屏幕模式 (ScreenMode)");
    
    // 初始化LED矩阵
    ledMatrix.begin();
    
    // 清空矩阵
    ledMatrix.clear();
    ledMatrix.update();
    Serial.println("LED矩阵已初始化并清空");
    
    // 初始化屏幕数据
    memset(screenData, 0, sizeof(screenData));
    
    // 显示提示信息
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Screen Mode");
}

void ScreenMode::update() {
    // 处理串行数据命令
    parseSerialData();
    
    if (isTestMode) {
        // 更新动画帧
        unsigned long currentTime = millis();
        if (currentTime - lastFrameTime >= frameIntervalMs) {
            lastFrameTime = currentTime;
            currentFrame = (currentFrame + 1) % ANIMATION_FRAMES;
            
            // 生成并显示动画帧
            generateFrameData();
        }
    }
}

void ScreenMode::exit() {
    // 清除显示
    ledMatrix.clear();
    ledMatrix.update();
}

void ScreenMode::handleEvent(EventType event) {
    switch (event) {
        case EVENT_BUTTON_A:
            // 按A键切换测试模式
            isTestMode = !isTestMode;
            if (isTestMode) {
                animationStartTime = millis();
                lastFrameTime = animationStartTime;
                currentFrame = 0;
                
                // 更新显示文本
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(10, 10);
                M5.Display.println("Screen Mode");
                M5.Display.setCursor(10, 40);
                M5.Display.println("Test Mode");
                
                // 立即显示第一帧
                generateFrameData();
            } else {
                ledMatrix.clear();
                ledMatrix.update();
                
                // 更新显示文本
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(10, 10);
                M5.Display.println("Screen Mode");
            }
            break;
            
        default:
            // 其他事件暂时不处理
            break;
    }
}

void ScreenMode::updateDisplay() {
    // 根据屏幕数据更新LED矩阵
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // 获取当前像素的颜色编号，并确保它在有效范围内
            uint8_t colorIndex = screenData[y][x] & 0x0F; // 只使用低4位作为颜色索引
            
            // 确保colorMap中有这个索引的颜色，如果超出范围就使用默认颜色
            uint32_t color = (colorIndex < 16) ? colorMap[colorIndex] : 0xFF0000; // 默认红色
            
            // 设置像素颜色
            ledMatrix.setPixel(x, y, color);
        }
    }
    
    // 更新LED矩阵显示
    ledMatrix.update();
}

void ScreenMode::generateFrameData() {
    // 生成当前帧 - 每行一种颜色，从1到15再到0，并向上移动
    for (int y = 0; y < 8; y++) {
        uint8_t colorIndex = (y + currentFrame) % 16;
        for (int x = 0; x < 8; x++) {
            screenData[y][x] = colorIndex;
        }
    }
    
    // 更新显示
    updateDisplay();
    
    // 输出当前帧数据到串口，便于验证解析功能
    Serial.print("SCREEN:");
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Serial.print(screenData[y][x]);
            if (!(y == 7 && x == 7)) {
                Serial.print(",");
            }
        }
    }
    Serial.println();
}

void ScreenMode::parseSerialData() {
    if (Serial.available() > 0) {
        // 检查是否是二进制数据（以0xAA开头）
        if (Serial.peek() == FRAME_HEADER) {
            parseSerialBinaryData();
        } else {
            parseSerialTextData();
        }
    }
}

void ScreenMode::parseSerialBinaryData() {
    static uint8_t buffer[FRAME_LENGTH];
    static int bufferIndex = 0;
    static bool frameStarted = false;
    
    while (Serial.available() > 0) {
        uint8_t inByte = Serial.read();
        
        // 检测帧头
        if (!frameStarted && inByte == FRAME_HEADER) {
            frameStarted = true;
            bufferIndex = 0;
            continue;
        }
        
        if (frameStarted) {
            buffer[bufferIndex++] = inByte;
            
            // 检查是否接收到完整帧
            if (bufferIndex == FRAME_LENGTH) {
                // 验证校验和
                uint8_t checksum = 0;
                for (int i = 0; i < FRAME_LENGTH - 1; i++) {
                    checksum ^= buffer[i];  // 使用异或作为简单的校验方式
                }
                
                // 如果校验和正确
                if (checksum == buffer[FRAME_LENGTH - 1]) {
                    // 更新屏幕数据
                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            screenData[y][x] = buffer[y * 8 + x] & 0x0F;  // 确保值在0-15范围内
                        }
                    }
                    // 更新显示
                    updateDisplay();
                }
                
                // 重置接收状态
                frameStarted = false;
                bufferIndex = 0;
            }
        }
    }
}

void ScreenMode::parseSerialTextData() {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    // 处理命令字符串
    if (data.startsWith("TEST")) {
        // 启动测试模式
        isTestMode = true;
        animationStartTime = millis();
        lastFrameTime = animationStartTime;
        currentFrame = 0;
        
        // 更新显示文本
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(WHITE, BLACK);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(10, 10);
        M5.Display.println("Screen Mode");
        M5.Display.setCursor(10, 40);
        M5.Display.println("Test Mode");
        
        // 立即显示第一帧
        generateFrameData();
    }
    else if (data.startsWith("SCREEN:")) {
        // 解析屏幕数据格式：SCREEN:0,1,2,...
        String payload = data.substring(7); // 跳过"SCREEN:"前缀
        int index = 0;
        int startPos = 0;
        int commaPos;
        
        while ((commaPos = payload.indexOf(',', startPos)) != -1 && index < 64) {
            String valueStr = payload.substring(startPos, commaPos);
            int value = valueStr.toInt();
            
            int y = index / 8;
            int x = index % 8;
            screenData[y][x] = value & 0x0F; // 确保值在0-15范围内
            
            startPos = commaPos + 1;
            index++;
        }
        
        // 处理最后一个值
        if (index < 64) {
            String valueStr = payload.substring(startPos);
            int value = valueStr.toInt();
            
            int y = index / 8;
            int x = index % 8;
            screenData[y][x] = value & 0x0F;
        }
        
        // 更新显示
        updateDisplay();
    }
} 