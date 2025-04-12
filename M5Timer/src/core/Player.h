#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>

// 循环模式定义
#define LOOP_SINGLE 0x00      // 单曲循环
#define LOOP_FOLDER 0x01      // 文件夹循环
#define LOOP_DISABLE 0x02     // 不循环

class JQ8900Player {
private:
    uint8_t _pin;             // 控制引脚
    uint8_t _currentTrack;    // 当前播放的曲目
    uint8_t _maxTracks;       // 最大曲目数量
    
    // 发送一个字节数据到JQ8900
    void sendByte(uint8_t data);

public:
    // 构造函数
    JQ8900Player(uint8_t pin, uint8_t maxTracks = 9);
    
    // 初始化播放器
    void begin();
    
    // 设置音量 (0-30)
    void setVolume(uint8_t volume);
    
    // 设置循环模式
    void setLoopMode(uint8_t mode);
    
    // 停止播放
    void stop();
    
    // 开始播放
    void play();
    
    // 暂停播放
    void pause();
    
    // 播放下一曲
    void next();
    
    // 播放指定曲目 (1-9999)
    void playTrack(uint16_t track);
    
    // 播放随机曲目
    void playRandom();
    
    // 获取当前播放的曲目
    uint8_t getCurrentTrack() const;
};

#endif // PLAYER_H 