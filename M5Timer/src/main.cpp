#include <Arduino.h>
#include <M5Unified.h>
#include "modes/BatteryMode.h"
#include "modes/TimerMode.h"
#include "tasks/AudioTask.h"
#include "tasks/InputTask.h"
#include "tasks/ModeTask.h"
#include "tasks/SensorTask.h"
#include "core/LEDMatrix.h"

// 硬件引脚定义
const uint8_t PIN_MP3_PLAYER = 26;  // MP3播放器控制引脚

// 全局对象
LEDMatrix ledMatrix;  // LED显示对象
BatteryMode batteryMode;
TimerMode timerMode;

// 全局队列句柄
QueueHandle_t audioQueue;
QueueHandle_t modeQueue;
QueueHandle_t sensorQueue;
QueueHandle_t eventQueue;

// 全局互斥锁
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
    M5.begin(cfg);
    Serial.println("M5Stack initialized");
    
    // 初始化LED矩阵
    ledMatrix.begin();
    Serial.println("LED Matrix initialized");
    
    // 创建互斥锁
    audioMutex = xSemaphoreCreateMutex();
    Serial.println("Mutexes created");
    
    // 创建消息队列
    audioQueue = xQueueCreate(5, sizeof(AudioMessage));
    modeQueue = xQueueCreate(5, sizeof(ModeMessage));
    sensorQueue = xQueueCreate(1, sizeof(SensorData));
    eventQueue = xQueueCreate(10, sizeof(EventType));
    Serial.println("Queues created");
    
    // 创建事件组
    systemEvents = xEventGroupCreate();
    modeEvents = xEventGroupCreate();
    Serial.println("Event groups created");
    
    // 创建任务，使用 intptr_t 进行安全的整数到指针的转换
    xTaskCreate(audioTask, "AudioTask", 4096, (void*)(intptr_t)PIN_MP3_PLAYER, 1, NULL);
    xTaskCreate(inputTask, "InputTask", 4096, NULL, 1, NULL);
    xTaskCreate(modeTask, "ModeTask", 4096, NULL, 1, NULL);
    xTaskCreate(sensorTask, "SensorTask", 4096, NULL, 1, NULL);
    Serial.println("Tasks created");
}

void loop() {
    // 主循环为空，所有工作由任务处理
    vTaskDelay(pdMS_TO_TICKS(100));
}