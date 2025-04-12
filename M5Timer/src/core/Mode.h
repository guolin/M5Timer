#pragma once

#include <Arduino.h>
#include "types.h"

class Mode {
public:
    Mode(const char* name) : name(name) {}
    virtual ~Mode() {}
    
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void exit() = 0;
    virtual void handleEvent(EventType event) = 0;
    
    const char* getName() const { return name; }
    
protected:
    const char* name;
}; 