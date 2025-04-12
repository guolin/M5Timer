#include "TimerMode.h"
#include <M5Unified.h>
#include "../core/LEDMatrix.h"
#include "../tasks/AudioTask.h"

// 声明外部全局变量
extern LEDMatrix ledMatrix;
extern QueueHandle_t audioQueue;

// 定义可用颜色列表
const uint32_t TimerMode::availableColors[6] = {
    0x0000FF,  // 蓝色
    0xFFFF00,  // 黄色
    0xFF0000,  // 红色
    0x00FF00,  // 绿色
    0xFF00FF,  // 紫色
    0x00FFFF   // 青色
};

TimerMode::TimerMode() : Mode("Timer") {
    remainingSeconds = 60;
    lastRemainingSeconds = 60;
    isRunning = false;
    isPaused = false;
    isCountdown = false;
    countdownSeconds = 3;
    startTime = 0;
    pauseTime = 0;
    
    // 初始化颜色
    tensColor = availableColors[0];  // 默认蓝色
    onesColor = availableColors[1];  // 默认黄色
    
    // 初始化晃动检测相关变量
    lastAccelX = lastAccelY = lastAccelZ = 0.0f;
    lastShakeTime = 0;
}

void TimerMode::begin() {
    // 初始化显示
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Timer");
    M5.Display.setTextSize(3);
    
    // 确保LED矩阵已初始化
    ledMatrix.begin();
    
    // 显示秒表图标
    showStopwatchIcon();
    ledMatrix.update();  // 确保更新显示
}

void TimerMode::showStopwatchIcon() {
    // 在LED矩阵上显示一个简单的秒表图标
    // 这里使用一个简单的8x8图案表示秒表
    const uint8_t icon[8][8] = {
        {1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,0,1,0,1},
        {1,0,0,1,1,0,0,1},
        {1,0,0,1,1,0,0,1},
        {1,0,1,0,0,1,0,1},
        {1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1}
    };
    
    ledMatrix.clear();  // 先清除显示
    
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            if(icon[y][x]) {
                ledMatrix.setPixel(x, y, COLOR_BLUE);
            }
        }
    }
    
    ledMatrix.update();  // 确保更新显示
}

bool TimerMode::checkShaking() {
    float accX, accY, accZ;
    M5.Imu.getAccel(&accX, &accY, &accZ);
    
    // 计算加速度变化
    float deltaX = fabs(accX - lastAccelX);
    float deltaY = fabs(accY - lastAccelY);
    float deltaZ = fabs(accZ - lastAccelZ);
    
    // 更新上次的加速度值
    lastAccelX = accX;
    lastAccelY = accY;
    lastAccelZ = accZ;
    
    // 检查是否超过晃动阈值
    float totalDelta = deltaX + deltaY + deltaZ;
    unsigned long currentTime = millis();
    
    if (totalDelta > SHAKE_THRESHOLD && 
        currentTime - lastShakeTime > SHAKE_COOLDOWN) {
        lastShakeTime = currentTime;
        return true;
    }
    return false;
}

void TimerMode::update() {
    if (isCountdown) {
        unsigned long currentTime = millis();
        int elapsedSeconds = (currentTime - startTime) / 1000;
        countdownSeconds = 3 - elapsedSeconds;
        
        if (countdownSeconds <= 0) {
            isCountdown = false;
            startTimer();
            playSound(2);  // 倒计时结束后立即播放声音002
        } else {
            // 显示倒计时数字
            ledMatrix.showNumber(countdownSeconds, COLOR_BLUE);
        }
    } else if (isRunning && !isPaused) {
        unsigned long currentTime = millis();
        int elapsedSeconds = (currentTime - startTime) / 1000;
        remainingSeconds = 60 - elapsedSeconds;
        
        // 检查是否需要播放声音
        if (remainingSeconds != lastRemainingSeconds) {
            if (remainingSeconds == 35) {
                playSound(3);  // 35秒时播放声音003
            } else if (remainingSeconds == 25) {
                playSound(3);  // 25秒时播放声音003
            } else if (remainingSeconds == 0) {
                playSound(4);  // 0秒时播放声音004
            }
            lastRemainingSeconds = remainingSeconds;
        }
        
        if (remainingSeconds <= 0) {
            remainingSeconds = 0;
            isRunning = false;
        }
        
        updateDisplay();
        updateLEDDisplay();
    }
}

void TimerMode::exit() {
    // 清除显示
    M5.Display.fillScreen(BLACK);
    ledMatrix.clear();  // 清除LED显示
}

void TimerMode::handleEvent(EventType event) {
    switch (event) {
        case EVENT_BUTTON_A:
            // 短按A键 - 开始/暂停/继续
            if (!isRunning && !isCountdown) {
                startCountdown();
            } else if (isPaused) {
                resumeTimer();
            } else {
                pauseTimer();
            }
            break;
            
        case EVENT_BUTTON_A_LONG:
            // 长按A键 - 只重置到初始状态
            resetTimer();
            break;
            
        case EVENT_SHAKE:
            // 晃动事件 - 只改变颜色，不影响计时器状态
            if (isRunning && !isCountdown) {  // 只在计时器运行时改变颜色
                randomizeColors();
                updateLEDDisplay();  // 立即更新显示
            }
            break;
            
        case EVENT_BUTTON_B:
            // 短按B键 - 暂无功能
            break;
            
        case EVENT_BUTTON_B_LONG:
            // 长按B键 - 暂无功能
            break;
    }
}

void TimerMode::updateDisplay() {
    M5.Display.fillRect(10, 50, 200, 40, BLACK);  // 清除显示区域
    M5.Display.setCursor(10, 50);
    M5.Display.printf("%02d", remainingSeconds);
}

void TimerMode::updateLEDDisplay() {
    if (remainingSeconds >= 10) {
        // 两位数显示，十位和个位使用不同颜色
        int tens = remainingSeconds / 10;
        int ones = remainingSeconds % 10;
        ledMatrix.showTwoNumbers(tens, ones, tensColor, onesColor);
    } else {
        // 个位数显示
        ledMatrix.showNumber(remainingSeconds, onesColor);  // 使用个位数的颜色
    }
}

void TimerMode::startCountdown() {
    // 先播放声音001
    playSound(1);
    
    // 等待0.5秒
    delay(500);
    
    // 开始倒计时
    isCountdown = true;
    countdownSeconds = 3;
    startTime = millis();
}

void TimerMode::startTimer() {
    if (!isRunning) {
        startTime = millis();
        isRunning = true;
        isPaused = false;
    }
}

void TimerMode::pauseTimer() {
    if (isRunning && !isPaused) {
        pauseTime = millis();
        isPaused = true;
    }
}

void TimerMode::resumeTimer() {
    if (isRunning && isPaused) {
        unsigned long pauseDuration = millis() - pauseTime;
        startTime += pauseDuration;
        isPaused = false;
    }
}

void TimerMode::resetTimer() {
    remainingSeconds = 60;
    lastRemainingSeconds = 60;
    isRunning = false;
    isPaused = false;
    isCountdown = false;
    countdownSeconds = 3;
    startTime = 0;
    pauseTime = 0;
    updateDisplay();
    showStopwatchIcon();
}

void TimerMode::playSound(uint16_t track) {
    // 先发送停止命令
    AudioMessage stopMsg;
    stopMsg.type = MSG_AUDIO_STOP;
    xQueueSend(audioQueue, &stopMsg, 0);
    delay(50);  // 等待停止命令执行完成
    
    // 然后播放新的声音
    AudioMessage msg;
    msg.type = MSG_AUDIO_TRACK;
    msg.track = track;
    xQueueSend(audioQueue, &msg, 0);
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