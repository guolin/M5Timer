#include "BatteryMode.h"
#include <M5Unified.h>

BatteryMode::BatteryMode() : Mode("Battery") {
    lastUpdateTime = 0;
}

void BatteryMode::begin() {
    M5.Display.fillScreen(BLACK);
    updateDisplay();
}

void BatteryMode::update() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 1000) {
        lastUpdateTime = currentTime;
        updateDisplay();
    }
}

void BatteryMode::exit() {
    M5.Display.fillScreen(BLACK);
}

void BatteryMode::handleEvent(EventType event) {
    if (event == EVENT_BUTTON_B) {
        updateDisplay();
    }
}

void BatteryMode::updateDisplay() {
    M5.Display.fillScreen(BLACK);
    
    // 获取电池信息
    int batteryLevel = M5.Power.getBatteryLevel();
    bool isCharging = M5.Power.isCharging();
    float batteryVoltage = M5.Power.getBatteryVoltage() / 1000.0f;  // 转换为V
    float current = M5.Power.getBatteryCurrent() / 1000.0f;  // 转换为mA
    float vbusVoltage = M5.Power.getVBUSVoltage() / 1000.0f;  // 转换为V
    
    // 显示电池百分比
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(isCharging ? 0xFFFF00 : 0x00FF00, BLACK);
    M5.Display.setCursor(15, 30);
    M5.Display.printf("%d%%", batteryLevel);
    
    // 显示电池电压
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(5, 80);
    M5.Display.printf("电池: %.2fV", batteryVoltage);
    
    // 显示USB电压
    M5.Display.setCursor(5, 110);
    M5.Display.printf("USB: %.2fV", vbusVoltage);
    
    // 显示充电状态和电流
    M5.Display.setCursor(5, 140);
    if (isCharging) {
        M5.Display.printf("充电: %.0fmA", current);
    } else {
        M5.Display.printf("放电: %.0fmA", -current);
    }
} 