#pragma once

#include "../core/Mode.h"

class ScreenMode : public Mode {
public:
    ScreenMode();
    virtual void begin() override;
    virtual void update() override;
    virtual void exit() override;
    virtual void handleEvent(EventType event) override;
    
private:
    void updateDisplay();
    void generateFrameData();  // 生成并打印当前帧数据
    void parseSerialData();
    void parseSerialBinaryData();  // 添加二进制数据解析方法声明
    void parseSerialTextData();    // 添加文本数据解析方法声明
    
    // 屏幕数据
    uint8_t screenData[8][8];
    // 颜色字典 - 存储颜色编号对应的颜色值
    uint32_t colorMap[16];
    
    unsigned long animationStartTime;
    unsigned long lastFrameTime;
    bool isTestMode;          // 是否显示测试动画
    int testDurationMs;       // 测试动画持续时间（毫秒）
    int frameIntervalMs;      // 帧间隔（毫秒）
    int currentFrame;         // 当前帧
}; 