#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>

// 循环模式定义
#define LOOP_SINGLE 0x00      // 单曲循环
#define LOOP_FOLDER 0x01      // 文件夹循环
#define LOOP_DISABLE 0x02     // 不循环

// 播放器状态定义
#define PLAYER_STATE_IDLE        0  // 空闲状态
#define PLAYER_STATE_PLAYING     1  // 正在播放
#define PLAYER_STATE_STOPPING    2  // 正在停止
#define PLAYER_STATE_PREPARING   3  // 准备播放
#define PLAYER_STATE_PAUSED      4  // 暂停状态

class JQ8900Player {
private:
    uint8_t _pin;                // 控制引脚
    uint8_t _currentTrack;       // 当前播放的曲目
    uint8_t _maxTracks;          // 最大曲目数量
    
    // 异步操作相关
    uint8_t _playerState;        // 播放器状态
    uint16_t _pendingTrack;      // 待播放的曲目号
    unsigned long _lastCmdTime;  // 上次命令发送时间
    bool _needUpdate;            // 是否需要更新状态
    uint8_t _cmdStep;            // 命令执行步骤
    
    // 发送一个字节数据到JQ8900
    void sendByte(uint8_t data);
    
    // 测试引脚工作状态
    void testPin();

public:
    // 构造函数
    JQ8900Player(uint8_t pin, uint8_t maxTracks = 9);
    
    // 初始化播放器
    void begin();
    
    // 设置音量 (0-30)
    void setVolume(uint8_t volume);
    
    // 设置循环模式
    void setLoopMode(uint8_t mode);
    
    // 停止播放 (阻塞版)
    void stop();
    
    // 异步停止播放 (非阻塞版)
    void stopAsync();
    
    // 开始播放 (阻塞版)
    void play();
    
    // 暂停播放 (阻塞版)
    void pause();
    
    // 播放下一曲 (阻塞版)
    void next();
    
    // 播放指定曲目 (1-9999) (阻塞版)
    void playTrack(uint16_t track);
    
    // 异步播放指定曲目 (非阻塞版)
    void playTrackAsync(uint16_t track);
    
    // 播放随机曲目
    void playRandom();
    
    // 获取当前播放的曲目
    uint8_t getCurrentTrack() const;
    
    // 获取播放器状态
    uint8_t getPlayerState() const;
    
    // 更新播放器状态 (需要在主循环中定期调用)
    void update();
    
    // 删除拷贝构造函数和赋值操作符
    JQ8900Player(const JQ8900Player&) = delete;
    JQ8900Player& operator=(const JQ8900Player&) = delete;
};

#endif // PLAYER_H 