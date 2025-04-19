#include "InputTask.h"
#include <M5Unified.h>

// 按键长按时间阈值（毫秒）
const unsigned long LONG_PRESS_TIME = 1000;

// 晃动检测相关常量
const float SHAKE_THRESHOLD = 1.5f;  // 晃动阈值
const unsigned long SHAKE_COOLDOWN = 1000;  // 晃动检测冷却时间（毫秒）

// 蜂鸣器音调频率
const uint16_t BEEP_FREQUENCY = 2000;  // 更高的蜂鸣器频率，使声音更清脆
const uint8_t BEEP_VOLUME = 64;  // 音量控制，范围0-255
const uint16_t BEEP_DURATION = 50;  // 蜂鸣持续时间（毫秒）

void inputTask(void *parameter) {
    EventMessage eventMsg;
    unsigned long buttonATime = 0;  // A按钮按下时间
    unsigned long buttonBTime = 0;  // B按钮按下时间
    bool buttonAPressed = false;
    bool buttonBPressed = false;
    bool buttonALongPressHandled = false;  // A按钮长按处理标记
    bool buttonBLongPressHandled = false;  // B按钮长按处理标记
    bool isBeeperOn = false;  // 蜂鸣器状态
    
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
        
        // 检测按键A - 长按立即触发事件
        if (M5.BtnA.wasPressed()) {
            buttonATime = millis();
            buttonAPressed = true;
            buttonALongPressHandled = false;
            Serial.println("Button A Pressed");
            // 按下A键时开始蜂鸣
            M5.Speaker.setVolume(BEEP_VOLUME);
            M5.Speaker.tone(BEEP_FREQUENCY, BEEP_DURATION);  // 短促的蜂鸣声
            isBeeperOn = true;
        } else if (M5.BtnA.wasReleased()) {
            // 松开A键时停止蜂鸣
            if (isBeeperOn) {
                M5.Speaker.stop();
                isBeeperOn = false;
            }
            
            if (buttonAPressed && !buttonALongPressHandled) {
                // 只有在没有处理过长按的情况下才处理短按
                Serial.println("Short Press A");
                eventMsg.type = EVENT_BUTTON_A;
                xQueueSend(eventQueue, &eventMsg, 0);
            }
            buttonAPressed = false;
            buttonALongPressHandled = false;
        } else if (buttonAPressed && M5.BtnA.isPressed()) {
            // 检查是否达到长按阈值但尚未处理
            unsigned long pressDuration = millis() - buttonATime;
            if (pressDuration >= LONG_PRESS_TIME && !buttonALongPressHandled) {
                // 长按A键 - 立即触发事件，无需等待松开按钮
                Serial.println("Long Press A");
                // 不需要再次触发蜂鸣器，因为按下时已经开始蜂鸣
                eventMsg.type = EVENT_BUTTON_A_LONG;
                xQueueSend(eventQueue, &eventMsg, 0);
                buttonALongPressHandled = true;
            }
        }
        
        // 检测按键B - 长按立即触发事件
        if (M5.BtnB.wasPressed()) {
            buttonBTime = millis();
            buttonBPressed = true;
            buttonBLongPressHandled = false;
            Serial.println("Button B Pressed");
            // 按下B键时开始蜂鸣
            M5.Speaker.setVolume(BEEP_VOLUME);
            M5.Speaker.tone(BEEP_FREQUENCY, BEEP_DURATION);  // 短促的蜂鸣声
            isBeeperOn = true;
        } else if (M5.BtnB.wasReleased()) {
            // 松开B键时停止蜂鸣
            if (isBeeperOn) {
                M5.Speaker.stop();
                isBeeperOn = false;
            }
            
            if (buttonBPressed && !buttonBLongPressHandled) {
                // 只有在没有处理过长按的情况下才处理短按
                Serial.println("Short Press B");
                eventMsg.type = EVENT_BUTTON_B;
                xQueueSend(eventQueue, &eventMsg, 0);
            }
            buttonBPressed = false;
            buttonBLongPressHandled = false;
        } else if (buttonBPressed && M5.BtnB.isPressed()) {
            // 检查是否达到长按阈值但尚未处理
            unsigned long pressDuration = millis() - buttonBTime;
            if (pressDuration >= LONG_PRESS_TIME && !buttonBLongPressHandled) {
                // 长按B键 - 立即切换模式，无需等待松开按钮
                Serial.println("Long Press B - Mode Switch");
                // 不需要再次触发蜂鸣器，因为按下时已经开始蜂鸣
                eventMsg.type = EVENT_BUTTON_B_LONG;
                xQueueSend(eventQueue, &eventMsg, 0);
                buttonBLongPressHandled = true;
            }
        }
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(20));
    }
} 