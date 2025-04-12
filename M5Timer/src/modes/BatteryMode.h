#pragma once

#include "../core/Mode.h"
#include "../core/types.h"

class BatteryMode : public Mode {
public:
    BatteryMode();
    virtual void begin() override;
    virtual void update() override;
    virtual void exit() override;
    virtual void handleEvent(EventType event) override;
    
private:
    void updateDisplay();
    unsigned long lastUpdateTime;  // 上次更新时间
}; 