#include <Arduino.h>
#include <M5Unified.h>
#include "modes/TimerMode.h"
#include "modes/ScreenMode.h"
#include "modes/LightingMode.h"
// #include "modes/MusicMode.h"
#include "tasks/InputTask.h"
#include "tasks/ModeTask.h"
#include "core/LEDMatrix.h"
#include "tasks/AudioTask.h"

// 硬件引脚定义
const uint8_t PIN_MP3_PLAYER = 26;  // MP3播放器控制引脚

// 全局对象
LEDMatrix ledMatrix;  // LED显示对象
TimerMode timerMode;
ScreenMode screenMode;
LightingMode lightingMode;
// MusicMode musicMode;

// 全局队列句柄
QueueHandle_t modeQueue;
QueueHandle_t eventQueue;
QueueHandle_t audioQueue;

// 音频互斥锁
SemaphoreHandle_t audioMutex;

// 全局事件组
EventGroupHandle_t systemEvents;
EventGroupHandle_t modeEvents;

void setup() {
    // 初始化串口
    Serial.begin(115200);
    Serial.println("M5Timer Starting...");
    
    // 初始化M5Stack
    auto cfg = M5.config();
    cfg.internal_imu = true;  // 启用内部IMU
    cfg.internal_mic = true;  // 确保启用内部麦克风
    M5.begin(cfg);
    Serial.println("M5Stack initialized");
    
    // 初始化LED矩阵
    ledMatrix.begin();
    Serial.println("LED Matrix initialized");
    
    // 创建消息队列
    modeQueue = xQueueCreate(5, sizeof(ModeMessage));
    eventQueue = xQueueCreate(10, sizeof(EventType));
    audioQueue = xQueueCreate(10, sizeof(AudioMessage));
    Serial.println("Queues created");
    
    // 创建事件组
    systemEvents = xEventGroupCreate();
    modeEvents = xEventGroupCreate();
    Serial.println("Event groups created");
    
    // 初始化音频互斥锁
    audioMutex = xSemaphoreCreateMutex();
    
    // 创建音频任务 - 通过参数传递引脚
    TaskHandle_t audioTaskHandle;
    xTaskCreate(
        audioTask,            // 任务函数
        "AudioTask",          // 任务名称
        4096,                 // 堆栈大小
        (void*)(intptr_t)PIN_MP3_PLAYER,  // 参数
        3,                    // 优先级 (提高到3)
        &audioTaskHandle      // 任务句柄
    );
    
    // 注册模式 - 使用ModeTask的模式管理功能
    registerMode(&timerMode);
    registerMode(&screenMode);
    registerMode(&lightingMode);
    // registerMode(&musicMode);
    initModeTask();
    Serial.println("Modes registered");
    
    // 创建任务
    xTaskCreate(inputTask, "InputTask", 4096, NULL, 1, NULL);
    xTaskCreate(modeTask, "ModeTask", 4096, NULL, 1, NULL);
    Serial.println("Tasks created");
}

void loop() {
    // 主循环为空，大部分工作由任务处理
    vTaskDelay(pdMS_TO_TICKS(5)); // 保留短延迟以避免看门狗问题
}