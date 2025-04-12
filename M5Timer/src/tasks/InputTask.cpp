#include "InputTask.h"
#include <M5Unified.h>

// 按键长按时间阈值（毫秒）
const unsigned long LONG_PRESS_TIME = 1000;

// 晃动检测相关常量
const float SHAKE_THRESHOLD = 1.5f;  // 晃动阈值
const unsigned long SHAKE_COOLDOWN = 1000;  // 晃动检测冷却时间（毫秒）

void inputTask(void *parameter) {
    EventMessage eventMsg;
    unsigned long buttonPressTime = 0;
    bool buttonPressed = false;
    
    // 晃动检测相关变量
    float lastAccelX = 0.0f, lastAccelY = 0.0f, lastAccelZ = 0.0f;
    unsigned long lastShakeTime = 0;
    
    Serial.println("InputTask started");
    
    while (true) {
        M5.update();
        
        // 检测晃动
        float accX, accY, accZ;
        if (M5.Imu.getAccel(&accX, &accY, &accZ)) {  // 如果成功读取加速度数据
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
                Serial.println("Shake detected");
                eventMsg.type = EVENT_SHAKE;
                xQueueSend(eventQueue, &eventMsg, 0);
            }
        }
        
        // 检测按键A
        if (M5.BtnA.wasPressed()) {
            buttonPressTime = millis();
            buttonPressed = true;
            Serial.println("Button A Pressed");
        } else if (M5.BtnA.wasReleased()) {
            if (buttonPressed) {
                unsigned long pressDuration = millis() - buttonPressTime;
                if (pressDuration >= LONG_PRESS_TIME) {
                    // 长按A键
                    Serial.println("Long Press A");
                    eventMsg.type = EVENT_BUTTON_A_LONG;
                    xQueueSend(eventQueue, &eventMsg, 0);
                } else {
                    // 短按A键
                    Serial.println("Short Press A");
                    eventMsg.type = EVENT_BUTTON_A;
                    xQueueSend(eventQueue, &eventMsg, 0);
                }
            }
            buttonPressed = false;
        }
        
        // 检测按键B
        if (M5.BtnB.wasPressed()) {
            buttonPressTime = millis();
            buttonPressed = true;
            Serial.println("Button B Pressed");
        } else if (M5.BtnB.wasReleased()) {
            if (buttonPressed) {
                unsigned long pressDuration = millis() - buttonPressTime;
                if (pressDuration >= LONG_PRESS_TIME) {
                    // 长按B键 - 切换模式
                    Serial.println("Long Press B - Mode Switch");
                    eventMsg.type = EVENT_BUTTON_B_LONG;
                    if (xQueueSend(eventQueue, &eventMsg, 0) == pdTRUE) {
                        Serial.println("Mode switch event sent successfully");
                    } else {
                        Serial.println("Failed to send mode switch event");
                    }
                } else {
                    // 短按B键
                    Serial.println("Short Press B");
                    eventMsg.type = EVENT_BUTTON_B;
                    xQueueSend(eventQueue, &eventMsg, 0);
                }
            }
            buttonPressed = false;
        }
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(20));
    }
} 