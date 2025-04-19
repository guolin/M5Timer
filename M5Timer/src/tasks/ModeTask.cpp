#include "ModeTask.h"
#include <M5Unified.h>

// 模式管理静态变量
static std::vector<Mode*> modes;
static int currentModeIndex = 0;

// 注册模式函数
void registerMode(Mode* mode) {
    if (mode != nullptr) {
        modes.push_back(mode);
        Serial.println("ModeTask: 注册模式: " + String(mode->getName()));
    }
}

// 获取当前模式
Mode* getCurrentMode() {
    if (currentModeIndex >= 0 && currentModeIndex < modes.size()) {
        return modes[currentModeIndex];
    }
    return nullptr;
}

// 初始化ModeTask
void initModeTask() {
    if (!modes.empty()) {
        currentModeIndex = 0;
        Serial.println("ModeTask: 初始化第一个模式: " + String(modes[currentModeIndex]->getName()));
        modes[currentModeIndex]->begin();
    } else {
        Serial.println("ModeTask: 警告 - 没有注册模式");
    }
}

// 获取已注册模式数量
int getRegisteredModeCount() {
    return modes.size();
}

// 切换到下一个模式
void switchToNextMode() {
    if (modes.empty()) return;
    
    // 退出当前模式
    if (currentModeIndex >= 0 && currentModeIndex < modes.size()) {
        Serial.println("ModeTask: 退出模式: " + String(modes[currentModeIndex]->getName()));
        modes[currentModeIndex]->exit();
    }
    
    // 切换到下一个模式
    currentModeIndex = (currentModeIndex + 1) % modes.size();
    
    // 进入新模式
    Serial.println("ModeTask: 切换到模式: " + String(modes[currentModeIndex]->getName()));
    modes[currentModeIndex]->begin();
}

// 切换到上一个模式
void switchToPreviousMode() {
    if (modes.empty()) return;
    
    // 退出当前模式
    if (currentModeIndex >= 0 && currentModeIndex < modes.size()) {
        Serial.println("ModeTask: 退出模式: " + String(modes[currentModeIndex]->getName()));
        modes[currentModeIndex]->exit();
    }
    
    // 切换到上一个模式
    currentModeIndex = (currentModeIndex - 1 + modes.size()) % modes.size();
    
    // 进入新模式
    Serial.println("ModeTask: 切换到模式: " + String(modes[currentModeIndex]->getName()));
    modes[currentModeIndex]->begin();
}

// 切换到指定索引的模式
void switchToMode(int modeIndex) {
    if (modes.empty() || modeIndex < 0 || modeIndex >= modes.size() || modeIndex == currentModeIndex) return;
    
    // 退出当前模式
    if (currentModeIndex >= 0 && currentModeIndex < modes.size()) {
        Serial.println("ModeTask: 退出模式: " + String(modes[currentModeIndex]->getName()));
        modes[currentModeIndex]->exit();
    }
    
    // 设置新的当前模式
    currentModeIndex = modeIndex;
    
    // 进入新模式
    Serial.println("ModeTask: 切换到模式: " + String(modes[currentModeIndex]->getName()));
    modes[currentModeIndex]->begin();
}

// ModeTask主函数
void modeTask(void *parameter) {
    EventMessage eventMsg;
    
    // 确保至少有一个模式注册并初始化
    if (!modes.empty()) {
        Serial.println("ModeTask: 启动, 初始模式: " + String(modes[currentModeIndex]->getName()));
    } else {
        Serial.println("ModeTask: 警告 - 无可用模式");
    }
    
    while (true) {
        // 检查事件消息
        if (xQueueReceive(eventQueue, &eventMsg, 0) == pdTRUE) {
            // 处理事件
            switch (eventMsg.type) {
                case EVENT_BUTTON_B_LONG:
                    // 长按B键 - 切换到下一个模式
                    Serial.println("ModeTask: 收到模式切换事件");
                    switchToNextMode();
                    break;
                    
                default:
                    // 其他事件发送给当前模式处理
                    Mode* currentMode = getCurrentMode();
                    if (currentMode != nullptr) {
                        currentMode->handleEvent(eventMsg.type);
                    }
                    break;
            }
        }
        
        // 更新当前模式
        Mode* currentMode = getCurrentMode();
        if (currentMode != nullptr) {
            currentMode->update();
        }
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(50));
    }
} 