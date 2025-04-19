#pragma once

#include "../core/Mode.h"
#include <M5Unified.h>
#include <driver/i2s.h>

// 音频可视化模式
class MusicMode : public Mode {
public:
    MusicMode();
    void begin() override;
    void update() override;
    void exit() override;
    void handleEvent(EventType event) override;
    
    // I2S相关静态方法
    static void micRecordTask(void* arg);
    
    // 音频处理 - 需要在任务中访问
    void processAudio();
    void updateLEDs();
    void showWaveform();
    
    // I2S配置 - 需要在任务中访问
    static const i2s_port_t I2S_PORT = I2S_NUM_0;
    static const int SAMPLE_RATE = 44100;
    static const int READ_LEN = 2 * 256;
    static const int GAIN_FACTOR = 3;
    
    // 音频缓冲区 - 需要在任务中访问
    uint8_t buffer[READ_LEN];
    
private:
    int16_t *audioBuffer;
    int16_t oldValues[8];  // 用于存储上一帧的LED高度
    
    // 显示参数
    uint8_t sensitivityLevel;   // 麦克风灵敏度等级 (0-9)
    uint8_t colorMode;         // 当前颜色模式
    
    // 颜色值
    static const uint32_t colorValues[5];  // 红、黄、蓝、绿、白
    static const char* audioColorNames[5];
    static const uint16_t audioLcdColors[5];
    static const float sensitivities[10];  // 灵敏度级别
    
    // I2S相关方法
    void i2sInit();
}; 