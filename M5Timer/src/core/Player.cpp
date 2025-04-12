#include "Player.h"
#include <M5Unified.h>

// JQ8900 命令定义
#define CMD_CLEAR 0x0A        // 清空数字
#define CMD_PLAY_SPECIFIED 0x0B // 选曲播放
#define CMD_VOLUME_SET 0x0C   // 设置音量
#define CMD_LOOP_MODE 0x0E    // 设置循环模式
#define CMD_PLAY 0x11         // 播放
#define CMD_PAUSE 0x12        // 暂停
#define CMD_STOP 0x13         // 停止
#define CMD_NEXT 0x15         // 下一曲

// 构造函数
JQ8900Player::JQ8900Player(uint8_t pin, uint8_t maxTracks) {
    _pin = pin;
    _currentTrack = 1;
    _maxTracks = maxTracks;
}

// 初始化播放器
void JQ8900Player::begin() {
    // 配置控制引脚
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
    
    // 等待JQ8900模块初始化
    delay(1000);
}

// 发送一个字节数据到JQ8900
void JQ8900Player::sendByte(uint8_t data) {
    // 开始信号
    digitalWrite(_pin, HIGH);
    delayMicroseconds(1000);
    
    // 引导码
    digitalWrite(_pin, LOW);
    delayMicroseconds(4000);  // 文档建议4ms
    
    // 发送8位数据 (从低位开始)
    for(uint8_t i = 0; i < 8; i++) {
        digitalWrite(_pin, HIGH);
        if(data & 0x01) {
            // 发送1 (高:低 = 3:1)
            delayMicroseconds(500);  // 高电平时间
            digitalWrite(_pin, LOW);
            delayMicroseconds(210);  // 低电平时间
        } else {
            // 发送0 (高:低 = 1:3)
            delayMicroseconds(210);  // 高电平时间
            digitalWrite(_pin, LOW);
            delayMicroseconds(500);  // 低电平时间
        }
        data >>= 1;  // 右移一位，准备发送下一位
    }
    
    // 结束信号
    digitalWrite(_pin, HIGH);
}

// 设置音量 (0-30)
void JQ8900Player::setVolume(uint8_t volume) {
    if(volume > 30) volume = 30;
    
    // 清空数字
    sendByte(CMD_CLEAR);
    delay(10);  // 两个字节之间延时建议10ms以上
    
    // 发送十位数
    sendByte(volume / 10);
    delay(10);
    
    // 发送个位数
    sendByte(volume % 10);
    delay(10);
    
    // 发送设置音量命令
    sendByte(CMD_VOLUME_SET);
    delay(50);
}

// 设置循环模式
void JQ8900Player::setLoopMode(uint8_t mode) {
    // 清空数字
    sendByte(CMD_CLEAR);
    delay(10);
    
    // 发送循环模式参数
    sendByte(mode);
    delay(10);
    
    // 发送设置循环模式命令
    sendByte(CMD_LOOP_MODE);
    delay(50);
}

// 停止播放
void JQ8900Player::stop() {
    sendByte(CMD_STOP);
    delay(50);
}

// 开始播放
void JQ8900Player::play() {
    sendByte(CMD_PLAY);
    delay(50);
}

// 暂停播放
void JQ8900Player::pause() {
    sendByte(CMD_PAUSE);
    delay(50);
}

// 播放下一曲
void JQ8900Player::next() {
    sendByte(CMD_NEXT);
    delay(50);
    
    // 更新当前曲目号
    _currentTrack++;
    if(_currentTrack > _maxTracks) _currentTrack = 1;
}

// 播放指定曲目 (1-9999)
void JQ8900Player::playTrack(uint16_t track) {
    _currentTrack = track;
    
    // 清空数字
    sendByte(CMD_CLEAR);
    delay(10);
    
    // 发送千位数
    sendByte((track / 1000) % 10);
    delay(10);
    
    // 发送百位数
    sendByte((track / 100) % 10);
    delay(10);
    
    // 发送十位数
    sendByte((track / 10) % 10);
    delay(10);
    
    // 发送个位数
    sendByte(track % 10);
    delay(10);
    
    // 发送选曲播放命令
    sendByte(CMD_PLAY_SPECIFIED);
    delay(50);
}

// 播放随机曲目
void JQ8900Player::playRandom() {
    // 生成1到_maxTracks之间的随机数
    uint8_t randomTrack = random(1, _maxTracks + 1);
    
    // 确保不重复播放同一首歌
    while(randomTrack == _currentTrack && _maxTracks > 1) {
        randomTrack = random(1, _maxTracks + 1);
    }
    
    // 先停止当前播放
    stop();
    delay(50);
    
    // 设置为不循环模式
    setLoopMode(LOOP_DISABLE);
    delay(50);
    
    // 播放随机选择的曲目
    playTrack(randomTrack);
}

// 获取当前播放的曲目
uint8_t JQ8900Player::getCurrentTrack() const {
    return _currentTrack;
} 