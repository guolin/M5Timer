#include "ModeTask.h"
#include <M5Unified.h>

static uint8_t currentMode = MODE_TIMER;  // 修改默认模式为倒计时模式

void modeTask(void *parameter) {
    ModeMessage modeMsg;
    EventMessage eventMsg;
    
    // 初始化倒计时模式
    timerMode.begin();
    
    while (true) {
        // 检查事件消息
        if (xQueueReceive(eventQueue, &eventMsg, 0) == pdTRUE) {
            // 处理事件
            switch (eventMsg.type) {
                case EVENT_BUTTON_B_LONG:
                    // 长按B键 - 切换模式
                    Serial.println("ModeTask: Received mode switch event");
                    // 退出当前模式
                    if (currentMode == MODE_BATTERY) {
                        Serial.println("ModeTask: Exiting Battery mode");
                        batteryMode.exit();
                        // 切换到Timer模式
                        currentMode = MODE_TIMER;
                        Serial.println("ModeTask: Starting Timer mode");
                        timerMode.begin();
                    } else if (currentMode == MODE_TIMER) {
                        Serial.println("ModeTask: Exiting Timer mode");
                        timerMode.exit();
                        // 切换到Battery模式
                        currentMode = MODE_BATTERY;
                        Serial.println("ModeTask: Starting Battery mode");
                        batteryMode.begin();
                    }
                    break;
                    
                default:
                    // 其他事件发送给当前模式处理
                    if (currentMode == MODE_BATTERY) {
                        batteryMode.handleEvent(eventMsg.type);
                    } else if (currentMode == MODE_TIMER) {
                        timerMode.handleEvent(eventMsg.type);
                    }
                    break;
            }
        }
        
        // 更新当前模式
        if (currentMode == MODE_BATTERY) {
            batteryMode.update();
        } else if (currentMode == MODE_TIMER) {
            timerMode.update();
        }
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(50));
    }
} 