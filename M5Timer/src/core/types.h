#pragma once

#include <Arduino.h>

// 音频消息类型
enum AudioMessageType {
    MSG_AUDIO_PLAY,
    MSG_AUDIO_PAUSE,
    MSG_AUDIO_STOP,
    MSG_AUDIO_NEXT,
    MSG_AUDIO_VOLUME,
    MSG_AUDIO_TRACK,
    MSG_AUDIO_RANDOM
};

// 音频消息结构
struct AudioMessage {
    AudioMessageType type;
    union {
        uint8_t volume;  // 音量值 (0-30)
        uint16_t track;  // 曲目号 (1-9999)
    };
};

// 显示消息类型
enum DisplayMessageType {
    MSG_DISPLAY_UPDATE,  // 更新显示
    MSG_DISPLAY_CLEAR,   // 清除显示
    MSG_DISPLAY_BATTERY, // 显示电池状态
    MSG_DISPLAY_COUNTDOWN // 显示倒计时
};

// 显示消息结构
struct DisplayMessage {
    DisplayMessageType type;
    union {
        float batteryPercentage;
        int countdownSeconds;
    };
};

// 传感器数据类型
enum SensorDataType {
    MSG_SENSOR_DATA
};

// 扩展传感器数据结构，包含完整的电池信息
struct SensorData {
    SensorDataType type;
    
    // 内部电池信息
    float batteryVoltage;     // 电池电压(V)
    float batteryPercentage;  // 电池百分比(%)
    bool isCharging;          // 充电状态
    float chargeCurrent;      // 充电电流(mA)
    float dischargeCurrent;   // 放电电流(mA)
    
    // USB信息
    float usbVoltage;         // USB电压(V)
    float usbCurrent;         // USB电流(mA)
    
    // 温度信息
    float temperature;        // 温度(°C)
    
    // 外部电池信息
    float extBatteryVoltage;  // 外部电池电压(V)
    float extBatteryPercentage; // 外部电池百分比(%)
    int extBatteryCurrent;    // 外部电池电流(mA)

    // 陀螺仪信息
    float roll;              // 横滚角度
    float pitch;             // 俯仰角度
    float yaw;               // 偏航角度
};

// 模式消息类型
enum ModeMessageType {
    MSG_MODE_CHANGE
};

// 模式消息结构
struct ModeMessage {
    ModeMessageType type;
    uint8_t modeId;
    bool active;
};

// 事件类型定义
enum EventType {
    EVENT_BUTTON_A,
    EVENT_BUTTON_B,
    EVENT_BUTTON_C,
    EVENT_BUTTON_A_LONG,
    EVENT_BUTTON_B_LONG,
    EVENT_BUTTON_C_LONG,
    EVENT_SHAKE,          // 晃动事件
    EVENT_TILT_LEFT,      // 向左倾斜事件
    EVENT_TILT_RIGHT,     // 向右倾斜事件
    EVENT_TILT_CENTER     // 恢复中间位置事件
};

// 模式类型定义
enum ModeType {
    MODE_BATTERY,
    MODE_TIMER,
    MODE_SCREEN,
    MODE_GAME            // 游戏模式
};

// 事件消息结构
struct EventMessage {
    EventType type;
}; 