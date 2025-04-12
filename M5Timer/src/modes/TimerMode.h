#pragma once

#include "../core/Mode.h"

class TimerMode : public Mode {
public:
    TimerMode();  // 使用默认构造函数
    virtual void begin() override;
    virtual void update() override;
    virtual void exit() override;
    virtual void handleEvent(EventType event) override;

private:
    void updateDisplay();
    void updateLEDDisplay();  // 新增LED显示更新函数
    void startTimer();
    void pauseTimer();
    void resumeTimer();
    void resetTimer();
    void playSound(uint16_t track);  // 新增播放声音函数
    void showStopwatchIcon();
    void startCountdown();
    bool checkShaking();  // 新增：检测晃动
    void randomizeColors();  // 新增：随机改变颜色

    unsigned long startTime;
    unsigned long pauseTime;
    int remainingSeconds;
    int lastRemainingSeconds;  // 用于跟踪上一次的剩余时间
    bool isRunning;
    bool isPaused;
    bool isCountdown;  // 是否处于3秒倒计时状态
    int countdownSeconds;  // 倒计时秒数

    // 颜色配置
    uint32_t tensColor;   // 十位数颜色
    uint32_t onesColor;   // 个位数颜色
    
    // 可用颜色列表
    static const uint32_t availableColors[6];  // 预定义一些好看的颜色

    // 晃动检测相关变量
    float lastAccelX, lastAccelY, lastAccelZ;
    unsigned long lastShakeTime;
    const float SHAKE_THRESHOLD = 1.5f;  // 晃动阈值
    const unsigned long SHAKE_COOLDOWN = 1000;  // 晃动检测冷却时间（毫秒）
}; 