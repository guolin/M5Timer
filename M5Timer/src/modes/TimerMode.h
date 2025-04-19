#pragma once

#include "../core/Mode.h"
#include <Preferences.h>
#include "../core/Player.h"

class TimerMode : public Mode {
public:
    TimerMode();  // 使用默认构造函数
    virtual ~TimerMode();  // 添加析构函数
    virtual void begin() override;
    virtual void update() override;
    virtual void exit() override;
    virtual void handleEvent(EventType event) override;

private:
    void updateDisplay();
    void updateLEDDisplay();  // LED显示更新函数
    void startTimer();
    void pauseTimer();
    void resumeTimer();
    void resetTimer();
    void playSound(uint16_t track);  // 播放声音函数
    void showStopwatchIcon();  // 显示秒表图标
    void startCountdown();  // 开始倒计时
    void randomizeColors();  // 随机改变颜色
    void drawPlayPauseButton(bool isPlaying);  // 绘制播放/暂停按钮
    void drawBrightnessButton();  // 绘制亮度按钮
    void drawBatteryIcon();  // 绘制电池图标
    void drawInfoBar();  // 绘制信息条
    void drawTimer();  // 绘制计时器
    void updateBrightness();  // 更新亮度
    void saveBrightness();  // 保存亮度设置
    void loadBrightness();  // 加载亮度设置
    
    // 省电模式相关方法
    void resetActivityTimer();        // 重置活动计时器
    void wakeFromPowerSaving();       // 从省电模式唤醒
    void updatePowerSavingMode();     // 更新省电模式状态

    unsigned long startTime;
    unsigned long pauseTime;
    int remainingSeconds;
    int lastRemainingSeconds;  // 用于跟踪上一次的剩余时间
    bool isRunning;
    bool isPaused;
    bool isCountdown;  // 是否处于3秒倒计时状态
    int countdownSeconds;  // 倒计时秒数
    bool isStartSoundPlayed;  // 是否已经播放了开始声音
    unsigned long lastDisplayedTime; // 上次显示更新的时间戳
    int lastDisplayedSeconds;      // 新增：上次显示的秒数
    int lastDisplayedMilliseconds; // 新增：上次显示的毫秒数

    // UI相关变量
    static const uint16_t LIGHT_GRAY = 0x8410;  // 浅灰色
    static const uint16_t DARK_GRAY = 0x4208;   // 深灰色
    static const uint16_t HIGHLIGHT_COLOR = 0xFFE0;  // 高亮黄色
    
    // 亮度控制
    int brightnessLevel;  // 当前亮度等级 (0-4)
    bool isBrightnessSelected;  // 亮度按钮是否被选中
    Preferences preferences;  // 用于保存设置
    
    // 按钮状态
    bool isPlayButtonSelected;  // 播放按钮是否被选中
    
    // 颜色配置
    uint32_t tensColor;   // 十位数颜色
    uint32_t onesColor;   // 个位数颜色
    
    // 可用颜色列表
    static const uint32_t availableColors[6];  // 预定义一些好看的颜色
    
    // 屏幕自动调节亮度和关机
    unsigned long lastActivityTime;  // 上次活动时间
    bool isDimmed;                   // 屏幕是否已调暗
    bool isLEDOff;                   // LED矩阵是否已关闭
    int originalBrightness;          // 原始亮度值
    
    // 私有方法
    void updateTimeDisplay(); // 只更新时间显示部分，减少闪烁
}; 