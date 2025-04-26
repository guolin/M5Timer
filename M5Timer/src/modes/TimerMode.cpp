#include "TimerMode.h"
#include <M5Unified.h>
#include <math.h>  // 添加数学库以使用ceil()函数
#include "../core/LEDMatrix.h"
#include "../core/Player.h"
#include "../tasks/AudioTask.h"

// 声明外部全局变量
extern LEDMatrix ledMatrix;

// 添加播放器引脚定义
#define PIN_MP3_PLAYER     26      // MP3播放器控制引脚

// 省电模式相关定义
#define DIM_TIMEOUT        60000    // 1分钟后调暗LED矩阵
#define LED_NORMAL_BRIGHT  51       // 正常亮度 (20%)
#define LED_DIM_BRIGHT     3        // 调暗亮度 (约1%)
#define LED_SOUND_BRIGHT   25       // 声音播放时的亮度 (10%)

// UI相关常量 - 调整为适应135x240的屏幕
#define INFO_BAR_Y         100      // 信息条Y坐标
#define INFO_BAR_HEIGHT    35       // 信息条高度
#define BUTTON_SIZE        25       // 按钮大小
#define BUTTON_MARGIN      8        // 按钮间距
#define TIME_DISPLAY_X     10       // 时间显示X坐标
#define TIME_DISPLAY_Y     30       // 时间显示Y坐标
#define TIME_DISPLAY_HEIGHT 50      // 时间显示高度区域
#define TIME_DISPLAY_WIDTH  180     // 时间显示宽度区域
#define VERSION_TEXT       "v1.0"   // 版本号文本

// 定义常用颜色
#define BLACK 0x0000
#define WHITE 0xFFFF
#define LIGHT_GRAY 0x8410  // 浅灰色
#define DARK_GRAY 0x4208   // 深灰色
#define HIGHLIGHT_COLOR 0xFFE0  // 高亮黄色
#define GREEN 0x07E0
#define RED 0xF800
#define BLUE 0x001F
#define PURPLE 0x780F      // 紫色
#define YELLOW 0xFFE0      // 黄色

// 定义可用颜色列表
const uint32_t TimerMode::availableColors[6] = {
    0x0000FF,  // 蓝色
    0xFFFF00,  // 黄色
    0xFF0000,  // 红色
    0x00FF00,  // 绿色
    0xFF00FF,  // 紫色
    0x00FFFF   // 青色
};

// 定义LED颜色阶段
const uint32_t LED_PHASE1_COLOR = 0xFF00FF;  // 紫色 (60-35秒)
const uint32_t LED_PHASE2_COLOR = 0xFFFF00;  // 黄色 (35-25秒)
const uint32_t LED_PHASE3_COLOR = 0x0000FF;  // 蓝色 (25-10秒)
const uint32_t LED_PHASE4_COLOR = 0xFF0000;  // 红色 (10-0秒)

TimerMode::TimerMode() : Mode("Timer") {
    remainingSeconds = 60;
    lastRemainingSeconds = 60;
    isRunning = false;
    isPaused = false;
    isCountdown = false;
    countdownSeconds = 3;
    isStartSoundPlayed = false;
    startTime = 0;
    pauseTime = 0;
    lastDisplayedTime = 0;
    lastDisplayedSeconds = 60;
    lastDisplayedMilliseconds = 0;
    isPlayingSoundAtKeyTime = false;  // 初始化新添加的变量
    soundBrightnessLevel = 0;         // 初始化声音播放时的亮度级别
    
    // 初始化颜色 - 改为红色和绿色
    tensColor = 0xFF0000;  // 红色
    onesColor = 0x00FF00;  // 绿色
    
    // 初始化UI状态
    brightnessLevel = 2;  // 默认中等亮度
    isBrightnessSelected = false;
    isPlayButtonSelected = true;  // 默认选中START按钮
    
    // 初始化省电模式变量
    lastActivityTime = millis();
    isDimmed = false;
    originalBrightness = LED_NORMAL_BRIGHT;
    
    // 加载保存的亮度设置
    loadBrightness();
    
    Serial.println("TimerMode: 构造函数完成");
}

// 析构函数
TimerMode::~TimerMode() {
    Serial.println("TimerMode: 析构函数被调用");
}

void TimerMode::begin() {
    // 设置屏幕旋转90度（横屏模式）
    M5.Display.setRotation(1);
    
    // 清除屏幕
    M5.Display.fillScreen(BLACK);
    
    // 确保LED矩阵已初始化
    ledMatrix.begin();
    
    // 设置正常亮度
    updateBrightness();
    isDimmed = false;
    lastDisplayedTime = 0; // 初始化显示时间戳
    
    // 确保有一个选项被选中
    if (!isPlayButtonSelected && !isBrightnessSelected) {
        isPlayButtonSelected = true;
    }
    
    // 绘制UI元素
    drawTimer();
    drawInfoBar();
    
    // 在右上角显示版本号
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARK_GRAY, BLACK);
    M5.Display.setCursor(210, 5);
    M5.Display.print(VERSION_TEXT);
    
    // 显示秒表图标
    showStopwatchIcon();
    ledMatrix.update();  // 确保更新显示
    
    // 重置活动时间
    lastActivityTime = millis();
    
    Serial.println("TimerMode: begin()完成");
}

void TimerMode::update() {
    // 更新省电模式状态（只保留亮度变暗功能）
    updatePowerSavingMode();
    
    // 获取当前时间
    unsigned long currentTime = millis();
    
    // 如果在充电，每500ms更新一次电池图标以展示充电动画
    bool isCharging = M5.Power.isCharging();
    static unsigned long lastBatteryUpdateTime = 0;
    if (isCharging && (currentTime - lastBatteryUpdateTime >= 500)) {
        drawBatteryIcon();
        lastBatteryUpdateTime = currentTime;
    }
    
    if (isCountdown) {
        // 倒计时是活动状态
        resetActivityTimer();
        
        unsigned long elapsedMillis = currentTime - startTime;
        int elapsedSeconds = elapsedMillis / 1000;
        int millisInCurrentSecond = elapsedMillis % 1000;
        countdownSeconds = 3 - elapsedSeconds;
        
        // 当倒计时还剩不到0.6秒时，提前播放开始声音并开始计时
        if (countdownSeconds == 1 && millisInCurrentSecond >= 400 && !isStartSoundPlayed) {
            Serial.println("TimerMode: 播放倒计时结束声音");
            // 使用AudioTask播放
            audioStop();
            audioPlayTrack(2);  // 倒计时结束声音
            isStartSoundPlayed = true;
        }
        
        if (countdownSeconds <= 0) {
            isCountdown = false;
            startTimer();
        } else {
            // 更新LCD上的倒计时显示
            updateTimeDisplay();
            
            // 添加对LED矩阵的更新
            updateLEDDisplay();
        }
    } else if (isRunning && !isPaused) {
        unsigned long elapsedMillis = currentTime - startTime;
        int elapsedSeconds = elapsedMillis / 1000;
        int millisInCurrentSecond = elapsedMillis % 1000;
        
        // 计算剩余秒数和毫秒
        float remainingTime = 60.0f - (elapsedMillis / 1000.0f);
        remainingSeconds = ceil(remainingTime);  // 使用向上取整而不是向下取整
        int remainingMillis = (int)((remainingTime - (int)remainingTime) * 1000);  // 小数部分是毫秒
        
        // 在关键时间点提前1秒播放声音并降低LED亮度
        if (lastRemainingSeconds > 35 && remainingSeconds <= 35) {
            // 从36变为35秒，播放003
            audioPlayTrack(3);
            // 降低LED亮度到当前亮度的两个级别
            soundBrightnessLevel = brightnessLevel >= 2 ? brightnessLevel - 2 : 0;
            int soundBrightness = map(soundBrightnessLevel, 0, 4, 5, LED_NORMAL_BRIGHT);
            ledMatrix.getStrip().setBrightness(soundBrightness);
            ledMatrix.getStrip().show();
            isPlayingSoundAtKeyTime = true;
            // 设置计时器，2秒后恢复原始亮度
            soundPlayStartTime = currentTime;
        } else if (lastRemainingSeconds > 25 && remainingSeconds <= 25) {
            // 从26变为25秒，播放003
            audioPlayTrack(3);
            // 降低LED亮度到当前亮度的两个级别
            soundBrightnessLevel = brightnessLevel >= 2 ? brightnessLevel - 2 : 0;
            int soundBrightness = map(soundBrightnessLevel, 0, 4, 5, LED_NORMAL_BRIGHT);
            ledMatrix.getStrip().setBrightness(soundBrightness);
            ledMatrix.getStrip().show();
            isPlayingSoundAtKeyTime = true;
            // 设置计时器，2秒后恢复原始亮度
            soundPlayStartTime = currentTime;
        } else if (lastRemainingSeconds > 0 && remainingSeconds <= 0) {
            // 从1变为0秒，播放004，但不降低亮度
            audioPlayTrack(4);
            // 不再降低LED亮度
            isPlayingSoundAtKeyTime = false; // 确保不会触发亮度恢复逻辑
        }
        lastRemainingSeconds = remainingSeconds;
        
        // 如果正在播放关键时间点的声音，检查是否需要恢复亮度
        if (isPlayingSoundAtKeyTime && (currentTime - soundPlayStartTime >= 2000)) {
            // 声音播放2秒后恢复原始亮度
            ledMatrix.getStrip().setBrightness(originalBrightness);
            ledMatrix.getStrip().show();
            isPlayingSoundAtKeyTime = false;
        }
        
        // 当计时器到达0秒时停止运行
        // 但不要立即重置，只标记为不运行，保持显示
        if (remainingTime <= 0) {  // 使用remainingTime而非remainingSeconds判断
            remainingSeconds = 0;  // 确保显示0.00秒
            isRunning = false;      // 标记为不运行
            resetActivityTimer();
            
            // 更新一次显示，确保显示0.00秒
            updateTimeDisplay();
            updateLEDDisplay();
        } else {
            // 限制屏幕更新频率，每50ms更新一次显示
            // 缩短更新间隔，使显示更流畅
            static unsigned long lastDisplayedTimeUpdate = 0;
            if (currentTime - lastDisplayedTimeUpdate >= 50) {
                // 更新时间显示
                updateTimeDisplay();
                lastDisplayedTimeUpdate = currentTime;
                lastDisplayedTime = currentTime;
            }
            
            // LED矩阵可以每帧都更新，不会引起闪烁
            updateLEDDisplay();
        }
    }
}

void TimerMode::exit() {
    // 清除显示
    M5.Display.fillScreen(BLACK);
    ledMatrix.clear();  // 清除LED显示
    ledMatrix.update();
    
    // 停止声音播放
    audioStop();
    Serial.println("TimerMode: 退出时停止播放器");
}

void TimerMode::drawTimer() {
    // 清除计时器显示区域，但不包括右上角的版本号
    M5.Display.fillRect(0, 0, 205, INFO_BAR_Y, BLACK);
    
    // 更新时间显示
    updateTimeDisplay();
    
    // 重新绘制版本号
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARK_GRAY, BLACK);
    M5.Display.setCursor(210, 5);
    M5.Display.print(VERSION_TEXT);
}

void TimerMode::drawInfoBar() {
    // 绘制信息条背景
    M5.Display.fillRect(0, INFO_BAR_Y, 240, INFO_BAR_HEIGHT, DARK_GRAY);
    
    // 绘制播放/暂停按钮，传递当前运行状态
    drawPlayPauseButton(isRunning);
    
    // 绘制亮度按钮
    drawBrightnessButton();
    
    // 绘制电池图标
    drawBatteryIcon();
}

void TimerMode::drawPlayPauseButton(bool isPlaying) {
    // 调整按钮位置和大小
    int x = BUTTON_MARGIN;
    int y = INFO_BAR_Y + (INFO_BAR_HEIGHT - BUTTON_SIZE) / 2;
    
    // 修改选中样式，不使用反色
    uint16_t color = isPlayButtonSelected ? WHITE : LIGHT_GRAY;
    
    // 清除按钮区域，按钮现在要更宽些来容纳文本
    M5.Display.fillRect(x, y, BUTTON_SIZE * 2, BUTTON_SIZE, DARK_GRAY);
    
    // 设置文本大小和颜色
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(color, DARK_GRAY);
    
    // 设置文本位置并显示
    M5.Display.setCursor(x + 4, y + BUTTON_SIZE/2 - 10); // 调整居中对齐文本位置
    
    // 显示不同的文本基于当前状态
    if (isCountdown) {
        // 在倒计时阶段显示PAUSE，按下后会取消倒计时
        M5.Display.print("PAUSE");
    } else if (!isRunning && remainingSeconds == 0) {
        // 计时结束状态 - 显示RESET
        M5.Display.print("RESET");
    } else if (!isRunning && !isCountdown) {
        // 未运行状态 - 显示START
        M5.Display.print("START");
    } else if (isPaused) {
        // 暂停状态 - 显示RESUME
        M5.Display.print("RESUME");
    } else {
        // 运行状态 - 显示PAUSE
        M5.Display.print("PAUSE");
    }
}

void TimerMode::drawBrightnessButton() {
    // 调整按钮位置，增加与START按钮之间的间距
    int x = 240/2 - BUTTON_SIZE/2 + 20; // 向右移动20像素（之前是15，增加5像素）
    int y = INFO_BAR_Y + (INFO_BAR_HEIGHT - BUTTON_SIZE) / 2;
    
    // 修改选中样式，不使用反色
    uint16_t color = isBrightnessSelected ? WHITE : LIGHT_GRAY;
    
    // 确保按钮区域完全覆盖
    M5.Display.fillRect(x - 1, y - 1, BUTTON_SIZE + 2, BUTTON_SIZE + 2, DARK_GRAY);
    
    // 设置文本大小和颜色
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(color, DARK_GRAY);
    
    // 设置文本位置并显示
    int textX = x;
    // 对于亮度0-4，根据数字调整位置以保持居中
    if (brightnessLevel == 0) {
        textX += 2;  // L0需要微调
    }
    M5.Display.setCursor(textX, y + BUTTON_SIZE/2 - 10);
    
    // 显示L0-L4亮度文本
    char brightnessText[3];
    snprintf(brightnessText, sizeof(brightnessText), "L%d", brightnessLevel);
    M5.Display.print(brightnessText);
}

void TimerMode::drawBatteryIcon() {
    int batteryLevel = M5.Power.getBatteryLevel();
    bool isCharging = M5.Power.isCharging();
    
    // 电池图标位置，向右移动5像素
    int x = 240 - BUTTON_SIZE*2 - BUTTON_MARGIN + 5; // 向右移动5像素
    int y = INFO_BAR_Y + 6; // 几乎与信息栏一样高
    int battHeight = INFO_BAR_HEIGHT - 12; // 电池高度
    int battWidth = BUTTON_SIZE * 1.5; // 电池宽度
    
    // 电池外框，使用更粗的边框 - 明确加粗轮廓线
    // 先绘制一个粗边框作为底色
    M5.Display.fillRect(x - 2, y - 2, battWidth + 4, battHeight + 4, LIGHT_GRAY);
    // 然后在内部绘制黑色填充区域，形成粗边框效果
    M5.Display.fillRect(x, y, battWidth, battHeight, BLACK);
    
    // 电池凸起部分，更粗更大
    M5.Display.fillRect(x + battWidth, y + battHeight/2 - 6, 6, 12, LIGHT_GRAY);
    
    // 电池内部分成5格
    int segmentWidth = (battWidth - 6) / 5; // 每个格子的宽度
    int segmentSpacing = 2; // 格子之间的间距
    int activeSegments = map(batteryLevel, 0, 100, 0, 5); // 根据电量确定亮起几格
    
    // 绘制分隔线
    for (int i = 1; i < 5; i++) {
        int lineX = x + 3 + i * segmentWidth;
        M5.Display.drawLine(lineX, y + 3, lineX, y + battHeight - 3, DARK_GRAY);
    }
    
    // 确定额外亮起的格子（充电动画）
    int extraSegment = -1;
    if (isCharging && activeSegments < 5) {
        // 如果在充电，那么在当前电量的下一格闪烁
        unsigned long currentTime = millis();
        if ((currentTime % 1000) < 500) { // 每隔0.5秒闪烁一次
            extraSegment = activeSegments;
        }
    }
    
    // 填充电量格子，使用灰色
    for (int i = 0; i < 5; i++) {
        int segX = x + 3 + i * segmentWidth + 1;
        int segWidth = segmentWidth - 2;
        
        if (i < activeSegments) {
            // 正常亮起的格子，使用浅灰色
            M5.Display.fillRect(segX, y + 3, segWidth, battHeight - 6, LIGHT_GRAY);
        } else if (i == extraSegment) {
            // 充电动画闪烁的格子，仍然使用黄色
            M5.Display.fillRect(segX, y + 3, segWidth, battHeight - 6, YELLOW);
        }
    }
}

void TimerMode::showStopwatchIcon() {
    // 在LED矩阵上显示一个香水瓶图标，宽6像素，高8像素，绿色轮廓和红色点缀
    const uint8_t icon[8][6] = {
        {0,1,1,1,1,0},
        {0,0,1,1,0,0},
        {1,1,1,1,1,1},
        {1,0,0,0,0,1},
        {1,0,0,2,0,1},
        {1,0,2,0,0,1},
        {1,0,0,0,0,1},
        {1,1,1,1,1,1}
    };
    
    // 设置颜色 - 使用明亮的绿色
    uint32_t bottleColor = 0x00FF00;     // 亮绿色瓶身
    uint32_t accentColor = 0xFF0000;     // 红色点缀
    
    // 先清除所有像素
    ledMatrix.clear();
    
    // 绘制图标，水平居中
    int offsetX = 1; // (8-6)/2 = 1，用于水平居中
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 6; x++) {
            if(icon[y][x] == 1) {
                ledMatrix.setPixel(x + offsetX, y, bottleColor);
            } else if(icon[y][x] == 2) {
                ledMatrix.setPixel(x + offsetX, y, accentColor);
            }
        }
    }
    
    ledMatrix.update();  // 更新显示
}

void TimerMode::handleEvent(EventType event) {
    // 任何事件发生时，都视为活动
    resetActivityTimer();
    
    // 从低功耗模式恢复
    wakeFromPowerSaving();
    
    switch (event) {
        case EVENT_BUTTON_A:
            if (isBrightnessSelected) {
                // 切换亮度等级
                brightnessLevel = (brightnessLevel + 1) % 5;
                updateBrightness();
                saveBrightness();
                drawInfoBar();
            } else {
                // 开始/暂停/继续/重置
                if (isCountdown) {
                    // 在倒计时阶段，按暂停则直接返回初始状态
                    resetTimer();
                } else if (!isRunning && remainingSeconds == 0) {
                    // 计时结束后，按下按钮重置计时器
                    resetTimer();
                } else if (!isRunning && !isCountdown) {
                    // 正常未运行状态，开始倒计时
                    startCountdown();
                } else if (isPaused) {
                    // 暂停状态，继续计时
                    resumeTimer();
                } else {
                    // 运行状态，暂停计时
                    pauseTimer();
                }
                drawInfoBar();
            }
            break;
            
        case EVENT_BUTTON_A_LONG:
            if (!isBrightnessSelected) {
                // 重置计时器
                resetTimer();
            }
            break;
            
        case EVENT_BUTTON_B:
            // 切换按钮选择状态
            isPlayButtonSelected = !isPlayButtonSelected;
            isBrightnessSelected = !isBrightnessSelected;
            drawInfoBar();
            break;
            
        case EVENT_SHAKE:
            // 晃动事件 - 改变颜色或从省电模式唤醒
            if (isRunning && !isCountdown) {
                // 只在计时器运行时改变颜色
                randomizeColors();
                updateLEDDisplay();
            }
            break;
            
        default:
            // 其他事件不处理
            break;
    }
}

void TimerMode::updateDisplay() {
    drawTimer();
    drawInfoBar();
}

void TimerMode::updateLEDDisplay() {
    // 在LED矩阵上显示剩余时间（向上取整）
    // 计算向上取整的剩余秒数
    int ceiledSeconds;
    
    if (isCountdown) {
        // 倒计时状态，显示倒计时数字
        ledMatrix.showNumber(countdownSeconds, COLOR_BLUE);
        ledMatrix.update();
        return;
    } else if (isRunning && !isPaused) {
        // 计算精确的剩余时间
        unsigned long currentTime = millis();
        unsigned long elapsedMillis = currentTime - startTime;
        float remainingTime = 60.0f - (elapsedMillis / 1000.0f);
        
        // 使用向上取整而不是向下取整（转换为int类型）
        ceiledSeconds = ceil(remainingTime);
        
        // 确保不会出现负值
        if (ceiledSeconds < 0) ceiledSeconds = 0;
    } else {
        // 非运行状态下使用当前存储的remainingSeconds
        // 由于在resetTimer()中设置为60，不需要额外的向上取整
        ceiledSeconds = remainingSeconds;
    }
    
    // 显示向上取整后的时间
    if (ceiledSeconds >= 10) {
        // 两位数显示
        int tens = ceiledSeconds / 10;
        int ones = ceiledSeconds % 10;
        ledMatrix.showTwoNumbers(tens, ones, tensColor, onesColor);
    } else {
        // 个位数显示，不再闪烁
        ledMatrix.showNumber(ceiledSeconds, onesColor);
    }
    ledMatrix.update();
}

void TimerMode::startCountdown() {
    Serial.println("TimerMode: 开始倒计时声音");
    
    // 使用AudioTask来处理音频播放 - 先停止所有声音，然后播放倒计时开始声音
    audioStop();
    audioPlayTrack(1);  // 播放曲目1 (倒计时开始声音)
    
    // 延迟1秒后再开始倒计时，确保声音先播放
    delay(1000);
    
    // 开始倒计时
    isCountdown = true;
    countdownSeconds = 3;
    isStartSoundPlayed = false;  // 重置声音播放标志
    startTime = millis();
}

void TimerMode::startTimer() {
    if (!isRunning) {
        startTime = millis();
        isRunning = true;
        isPaused = false;
        lastRemainingSeconds = 60; // 确保初始状态正确
        lastDisplayedTime = 0; // 重置上次显示时间
        
        // 在计时开始时降低LED亮度到当前亮度的两个级别
        soundBrightnessLevel = brightnessLevel >= 2 ? brightnessLevel - 2 : 0;
        int soundBrightness = map(soundBrightnessLevel, 0, 4, 5, LED_NORMAL_BRIGHT);
        ledMatrix.getStrip().setBrightness(soundBrightness);
        ledMatrix.getStrip().show();
        isPlayingSoundAtKeyTime = true;
        // 设置计时器，2秒后恢复原始亮度
        soundPlayStartTime = millis();
        
        drawTimer(); // 完整重绘一次
        drawInfoBar();
        updateLEDDisplay();
    }
}

void TimerMode::pauseTimer() {
    if (isRunning && !isPaused) {
        pauseTime = millis();
        isPaused = true;
        updateDisplay();
        updateLEDDisplay();
    }
}

void TimerMode::resumeTimer() {
    if (isRunning && isPaused) {
        unsigned long pauseDuration = millis() - pauseTime;
        startTime += pauseDuration;
        isPaused = false;
        updateDisplay();
        updateLEDDisplay();
    }
}

void TimerMode::resetTimer() {
    // 使用AudioTask停止声音
    audioStop();
    
    remainingSeconds = 60;
    lastRemainingSeconds = 60;
    isRunning = false;
    isPaused = false;
    isCountdown = false;
    countdownSeconds = 3;
    isStartSoundPlayed = false;  // 重置声音播放标志
    isPlayingSoundAtKeyTime = false;  // 重置关键时间点声音播放标志
    startTime = 0;
    pauseTime = 0;
    updateDisplay();
    showStopwatchIcon();
}

void TimerMode::playSound(uint16_t track) {
    Serial.println("TimerMode: 准备播放声音 " + String(track));
    
    // 使用AudioTask异步播放
    audioPlayTrackNonBlocking(track);
    
    Serial.println("TimerMode: 已发送异步播放命令 " + String(track));
}

void TimerMode::randomizeColors() {
    // 随机选择两个不同的颜色
    int colorIndex1 = random(6);
    int colorIndex2;
    do {
        colorIndex2 = random(6);
    } while (colorIndex2 == colorIndex1);  // 确保两个颜色不同
    
    tensColor = availableColors[colorIndex1];
    onesColor = availableColors[colorIndex2];
}

void TimerMode::updateBrightness() {
    // 将亮度等级映射到实际亮度值
    int brightness = map(brightnessLevel, 0, 4, 5, LED_NORMAL_BRIGHT);
    
    // 只有在不是播放关键时间点的声音时才设置亮度
    if (!isPlayingSoundAtKeyTime) {
        ledMatrix.getStrip().setBrightness(brightness);
        ledMatrix.getStrip().show();
    }
    originalBrightness = brightness;
}

void TimerMode::saveBrightness() {
    preferences.begin("timer", false);
    preferences.putInt("brightness", brightnessLevel);
    preferences.end();
}

void TimerMode::loadBrightness() {
    preferences.begin("timer", true);
    brightnessLevel = preferences.getInt("brightness", 2);  // 默认中等亮度
    preferences.end();
}

// 重置活动计时器
void TimerMode::resetActivityTimer() {
    lastActivityTime = millis();
}

// 从省电模式唤醒
void TimerMode::wakeFromPowerSaving() {
    if (isDimmed) {
        // 恢复LED亮度，但要考虑是否正在关键时间点播放声音
        if (isPlayingSoundAtKeyTime) {
            // 如果正在播放关键时间点声音，保持降低的亮度
            int soundBrightness = map(soundBrightnessLevel, 0, 4, 5, LED_NORMAL_BRIGHT);
            ledMatrix.getStrip().setBrightness(soundBrightness);
        } else {
            // 否则恢复原始亮度
            ledMatrix.getStrip().setBrightness(originalBrightness);
        }
        isDimmed = false;
        ledMatrix.getStrip().show();
    }
}

// 更新省电模式状态 (简化，只保留亮度调暗功能)
void TimerMode::updatePowerSavingMode() {
    unsigned long currentTime = millis();
    unsigned long inactiveTime = currentTime - lastActivityTime;
    
    // 检查是否需要调暗LED矩阵 (1分钟无活动)
    if (inactiveTime >= DIM_TIMEOUT && !isDimmed) {
        Serial.println("省电模式: 1分钟无活动，调暗LED矩阵");
        ledMatrix.getStrip().setBrightness(LED_DIM_BRIGHT);
        ledMatrix.getStrip().show();
        isDimmed = true;
    }
}

// 更新时间显示部分，减少闪烁并增加颜色变化
void TimerMode::updateTimeDisplay() {
    // 计算剩余时间，包括毫秒
    unsigned long currentTime = millis();
    unsigned long elapsedMillis = 0;
    float remainingTime = 60.0f;
    
    if (isCountdown) {
        // 倒计时状态，显示3、2、1（居中）
        unsigned long elapsedMillis = currentTime - startTime;
        float elapsedSeconds = elapsedMillis / 1000.0f;
        int countdownValue = 3 - ceil(elapsedSeconds);  // 使用向上取整确保平滑过渡
        if (countdownValue > 0) {
            // 清除整个显示区域
            M5.Display.fillRect(TIME_DISPLAY_X, TIME_DISPLAY_Y, TIME_DISPLAY_WIDTH, TIME_DISPLAY_HEIGHT, BLACK);
            
            // 显示倒计时数字（居中）
            M5.Display.setTextSize(6);
            M5.Display.setTextColor(LIGHT_GRAY, BLACK);
            
            // 计算居中位置
            // 单个数字宽度约为6*6=36像素
            int digitWidth = 36;
            int centerX = TIME_DISPLAY_X + (TIME_DISPLAY_WIDTH - digitWidth) / 2;
            
            M5.Display.setCursor(centerX, TIME_DISPLAY_Y);
            M5.Display.printf("%d", countdownValue);
            return;
        }
    } else if (isRunning) {
        if (isPaused) {
            // 暂停状态，保持当前时间
            elapsedMillis = pauseTime - startTime;
        } else {
            // 运行状态，计算经过时间
            elapsedMillis = currentTime - startTime;
        }
        remainingTime = 60.0f - (elapsedMillis / 1000.0f);
    } else if (!isCountdown && remainingSeconds == 0) {
        // 计时结束但未重置的特殊状态，显示0.00
        remainingTime = 0.0f;
    }
    
    // 确保不会出现负值
    if (remainingTime < 0) remainingTime = 0;
    
    // 分离秒数和毫秒
    int seconds = (int)remainingTime;
    int milliseconds = (int)((remainingTime - seconds) * 100);
    
    // 确保当到达0秒时，显示精确的0.00而不是四舍五入
    if (seconds == 0 && !isRunning) {
        milliseconds = 0;
    }
    
    // 清除整个显示区域
    M5.Display.fillRect(TIME_DISPLAY_X, TIME_DISPLAY_Y, TIME_DISPLAY_WIDTH, TIME_DISPLAY_HEIGHT, BLACK);
    
    // 设置显示颜色为固定的浅灰色，不再根据阶段变化
    uint16_t timeColor = LIGHT_GRAY;
    
    // 最后10秒显示红色但不闪烁
    if (seconds <= 10 && seconds > 0) {
        timeColor = RED;    // 10-0秒: 红色
    } else if (seconds == 0) {
        // 0秒时使用红色
        timeColor = RED;
    }
    
    // 显示秒数
    M5.Display.setTextSize(6);
    M5.Display.setTextColor(timeColor, BLACK);
    M5.Display.setCursor(TIME_DISPLAY_X, TIME_DISPLAY_Y);
    M5.Display.printf("%02d", seconds);
    
    // 显示毫秒
    M5.Display.setCursor(TIME_DISPLAY_X + 64, TIME_DISPLAY_Y);
    M5.Display.printf(".%02d", milliseconds);
    
    // 显示"sec"，现在使用左对齐
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARK_GRAY, BLACK);
    
    // 只在正常计时模式下显示sec，倒计时时不显示
    if (!isCountdown) {
        // 计算sec文本的左侧位置，不再需要计算宽度
        int secX = TIME_DISPLAY_X + TIME_DISPLAY_WIDTH; // 直接指定偏移量
        // 计算sec文本的y坐标，使其与数字底部对齐（数字高度约48像素）
        int secY = TIME_DISPLAY_Y + 48 - 16; // 16是sec文本的高度
        M5.Display.setCursor(secX, secY);
        M5.Display.print("sec");
    }
    
    lastDisplayedSeconds = seconds;
    lastDisplayedMilliseconds = milliseconds;
} 