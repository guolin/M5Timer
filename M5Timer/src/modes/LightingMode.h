#pragma once

#include "../core/Mode.h"

class LightingMode : public Mode {
public:
    LightingMode();
    virtual void begin() override;
    virtual void update() override;
    virtual void exit() override;
    virtual void handleEvent(EventType event) override;
    
private:
    void updateDisplay();
    void updateLEDs();
    
    // 亮度等级 (0-9)
    uint8_t brightnessLevel;
    
    // 亮度值对应表 (10%-100%)，最高值降至72
    const uint8_t brightnessValues[10] = {7, 14, 22, 29, 36, 43, 50, 58, 65, 72};
    
    // 颜色切换相关
    uint8_t colorIndex;
    static const uint32_t colorValues[5];  // 红、黄、蓝、绿、白
    
    // 是否需要更新显示
    bool needDisplayUpdate;
}; 