#include "ModeTask.h"
#include <M5Unified.h>

// 模式管理静态变量
static std::vector<Mode*> modes;
static int currentModeIndex = 0;
static bool screenModeAvailable = false; // 标记ScreenMode是否可用

// 检查是否是ScreenMode
bool isScreenMode(Mode* mode) {
    return (mode != nullptr && String(mode->getName()) == "Screen");
}

// 获取ScreenMode的索引
int getScreenModeIndex() {
    for (int i = 0; i < modes.size(); i++) {
        if (isScreenMode(modes[i])) {
            return i;
        }
    }
    return -1; // 未找到
}

// 注册模式函数
void registerMode(Mode* mode) {
    if (mode != nullptr) {
        modes.push_back(mode);
        Serial.println("ModeTask: 注册模式: " + String(mode->getName()));
        
        // 检查是否是ScreenMode
        if (isScreenMode(mode)) {
            Serial.println("ModeTask: 检测到ScreenMode");
        }
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

// 设置ScreenMode是否可用
void setScreenModeAvailable(bool available) {
    screenModeAvailable = available;
    Serial.println("ModeTask: ScreenMode可用性: " + String(available ? "可用" : "不可用"));
}

// 切换到下一个模式
void switchToNextMode() {
    if (modes.empty()) return;
    
    // 退出当前模式
    if (currentModeIndex >= 0 && currentModeIndex < modes.size()) {
        Serial.println("ModeTask: 退出模式: " + String(modes[currentModeIndex]->getName()));
        modes[currentModeIndex]->exit();
    }
    
    // 切换到下一个模式，如果ScreenMode不可用且下一个是ScreenMode，则跳过
    int nextIndex = (currentModeIndex + 1) % modes.size();
    int screenModeIndex = getScreenModeIndex();
    
    // 当ScreenMode不可用且下一个模式是ScreenMode时，跳过该模式
    if (!screenModeAvailable && nextIndex == screenModeIndex) {
        nextIndex = (nextIndex + 1) % modes.size();
    }
    
    currentModeIndex = nextIndex;
    
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
    
    // 切换到上一个模式，如果ScreenMode不可用且上一个是ScreenMode，则跳过
    int prevIndex = (currentModeIndex - 1 + modes.size()) % modes.size();
    int screenModeIndex = getScreenModeIndex();
    
    // 当ScreenMode不可用且上一个模式是ScreenMode时，跳过该模式
    if (!screenModeAvailable && prevIndex == screenModeIndex) {
        prevIndex = (prevIndex - 1 + modes.size()) % modes.size();
    }
    
    currentModeIndex = prevIndex;
    
    // 进入新模式
    Serial.println("ModeTask: 切换到模式: " + String(modes[currentModeIndex]->getName()));
    modes[currentModeIndex]->begin();
}

// 切换到指定索引的模式
void switchToMode(int modeIndex) {
    if (modes.empty() || modeIndex < 0 || modeIndex >= modes.size() || modeIndex == currentModeIndex) return;
    
    // 如果ScreenMode不可用且目标是ScreenMode，则取消切换
    int screenModeIndex = getScreenModeIndex();
    if (!screenModeAvailable && modeIndex == screenModeIndex) {
        Serial.println("ModeTask: ScreenMode当前不可用，取消切换");
        return;
    }
    
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

// 切换到ScreenMode
bool switchToScreenMode() {
    int screenModeIndex = getScreenModeIndex();
    if (screenModeIndex >= 0) {
        setScreenModeAvailable(true);  // 启用ScreenMode
        switchToMode(screenModeIndex);
        return true;
    }
    return false;
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
    
    // 初始状态下，ScreenMode不可用（除非接收到串口数据）
    setScreenModeAvailable(false);
    
    // 上次检查串口的时间
    unsigned long lastSerialCheckTime = 0;
    
    while (true) {
        // 检查串口是否有数据，每100ms检查一次，避免频繁检查
        unsigned long currentTime = millis();
        if (currentTime - lastSerialCheckTime > 100) {
            lastSerialCheckTime = currentTime;
            
            if (Serial.available() > 0) {
                // 检测到串口有数据，切换到ScreenMode
                Mode* currentMode = getCurrentMode();
                if (!isScreenMode(currentMode)) {
                    Serial.println("ModeTask: 检测到串口数据，切换到ScreenMode");
                    switchToScreenMode();
                }
            }
        }
        
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