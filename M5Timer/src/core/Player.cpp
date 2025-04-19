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
    _playerState = PLAYER_STATE_IDLE;
    _pendingTrack = 0;
    _lastCmdTime = 0;
    _needUpdate = false;
    _cmdStep = 0;
    Serial.println("JQ8900Player: 创建播放器, 引脚=" + String(pin));
}

// 初始化播放器
void JQ8900Player::begin() {
    // 配置控制引脚
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
    Serial.println("JQ8900Player: 初始化引脚 " + String(_pin));
    
    // 测试引脚工作状态
    testPin();
    
    // 等待JQ8900模块初始化
    delay(500);  // 减少初始化延迟
    Serial.println("JQ8900Player: 初始化完成");
}

// 测试引脚工作状态
void JQ8900Player::testPin() {
    Serial.println("JQ8900Player: 测试引脚 " + String(_pin));
    // 测试引脚能否正常切换状态 - 减少测试次数
    for(int i=0; i<3; i++) {
        digitalWrite(_pin, LOW);
        delay(50);
        digitalWrite(_pin, HIGH);
        delay(50);
    }
    Serial.println("JQ8900Player: 引脚测试完成");
}

// 发送一个字节数据到JQ8900
void JQ8900Player::sendByte(uint8_t data) {
    Serial.print("JQ8900Player: 发送字节 0x");
    Serial.println(data, HEX);
    
    // 开始信号，确保引脚开始于高电平
    digitalWrite(_pin, HIGH);
    delayMicroseconds(1000); // 减少开始信号时间
    
    // 引导码
    digitalWrite(_pin, LOW);
    delayMicroseconds(4000);  // 略微减少引导码时间
    
    // 发送8位数据 (从低位开始)
    for(uint8_t i = 0; i < 8; i++) {
        digitalWrite(_pin, HIGH);
        if(data & 0x01) {
            // 发送1 (高:低 = 3:1)
            delayMicroseconds(600);
            digitalWrite(_pin, LOW);
            delayMicroseconds(200);
        } else {
            // 发送0 (高:低 = 1:3)
            delayMicroseconds(200);
            digitalWrite(_pin, LOW);
            delayMicroseconds(600);
        }
        data >>= 1;  // 右移一位，准备发送下一位
    }
    
    // 结束信号，确保回到高电平状态
    digitalWrite(_pin, HIGH);
    delayMicroseconds(1000); // 减少结束信号时间
}

// 设置音量 (0-30)
void JQ8900Player::setVolume(uint8_t volume) {
    if(volume > 30) volume = 30;
    Serial.println("JQ8900Player: 设置音量 " + String(volume));
    
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
    Serial.println("JQ8900Player: 设置循环模式 " + String(mode));
    
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

// 停止播放 (阻塞版)
void JQ8900Player::stop() {
    Serial.println("JQ8900Player: 停止播放 (阻塞)");
    sendByte(CMD_STOP);
    delay(30);  // 减少等待时间
    _playerState = PLAYER_STATE_IDLE;
}

// 异步停止播放 (非阻塞版)
void JQ8900Player::stopAsync() {
    if (_playerState != PLAYER_STATE_STOPPING) {
        Serial.println("JQ8900Player: 停止播放 (异步)");
        _playerState = PLAYER_STATE_STOPPING;
        _needUpdate = true;
        _cmdStep = 0;
        _lastCmdTime = millis();
    }
}

// 开始播放 (阻塞版)
void JQ8900Player::play() {
    Serial.println("JQ8900Player: 开始播放");
    sendByte(CMD_PLAY);
    delay(50);
    _playerState = PLAYER_STATE_PLAYING;
}

// 暂停播放 (阻塞版)
void JQ8900Player::pause() {
    Serial.println("JQ8900Player: 暂停播放");
    sendByte(CMD_PAUSE);
    delay(50);
    _playerState = PLAYER_STATE_PAUSED;
}

// 播放下一曲 (阻塞版)
void JQ8900Player::next() {
    Serial.println("JQ8900Player: 下一曲");
    sendByte(CMD_NEXT);
    delay(50);
    
    // 更新当前曲目号
    _currentTrack++;
    if(_currentTrack > _maxTracks) _currentTrack = 1;
    _playerState = PLAYER_STATE_PLAYING;
}

// 播放指定曲目 (阻塞版)
void JQ8900Player::playTrack(uint16_t track) {
    _currentTrack = track;
    Serial.println("JQ8900Player: 播放曲目 " + String(track) + " (阻塞)");
    
    // 先确保停止当前播放
    stop();
    delay(50); // 减少停止后的等待时间
    
    // 清空数字
    sendByte(CMD_CLEAR);
    delay(10);  // 减少两字节之间的延时
    
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
    delay(50); // 减少命令后的等待时间
    
    _playerState = PLAYER_STATE_PLAYING;
    Serial.println("JQ8900Player: 播放命令已发送，曲目 " + String(track));
}

// 异步播放指定曲目 (非阻塞版)
void JQ8900Player::playTrackAsync(uint16_t track) {
    _pendingTrack = track;
    _currentTrack = track;
    Serial.println("JQ8900Player: 播放曲目 " + String(track) + " (异步)");
    
    // 设置状态为准备播放
    _playerState = PLAYER_STATE_PREPARING;
    _needUpdate = true;
    _cmdStep = 0;
    _lastCmdTime = millis();
}

// 播放随机曲目
void JQ8900Player::playRandom() {
    // 生成1到_maxTracks之间的随机数
    uint8_t randomTrack = random(1, _maxTracks + 1);
    
    // 确保不重复播放同一首歌
    while(randomTrack == _currentTrack && _maxTracks > 1) {
        randomTrack = random(1, _maxTracks + 1);
    }
    
    Serial.println("JQ8900Player: 随机播放曲目 " + String(randomTrack));
    
    // 使用异步方式播放
    playTrackAsync(randomTrack);
}

// 获取当前播放的曲目
uint8_t JQ8900Player::getCurrentTrack() const {
    return _currentTrack;
}

// 获取播放器状态
uint8_t JQ8900Player::getPlayerState() const {
    return _playerState;
}

// 更新播放器状态 (需要在主循环中定期调用)
void JQ8900Player::update() {
    if (!_needUpdate) return;
    
    unsigned long currentTime = millis();
    
    // 确保命令间隔足够长
    if (currentTime - _lastCmdTime < 20) return;
    
    if (_playerState == PLAYER_STATE_STOPPING) {
        // 异步停止
        if (_cmdStep == 0) {
            sendByte(CMD_STOP);
            _cmdStep = 1;
            _lastCmdTime = currentTime;
        } else {
            // 停止完成
            _playerState = PLAYER_STATE_IDLE;
            _needUpdate = false;
            _cmdStep = 0;
            Serial.println("JQ8900Player: 异步停止完成");
        }
    } else if (_playerState == PLAYER_STATE_PREPARING) {
        // 异步播放准备过程
        switch (_cmdStep) {
            case 0:  // 首先停止当前播放
                sendByte(CMD_STOP);
                _cmdStep = 1;
                _lastCmdTime = currentTime;
                break;
            
            case 1:  // 清空数字
                sendByte(CMD_CLEAR);
                _cmdStep = 2;
                _lastCmdTime = currentTime;
                break;
                
            case 2:  // 发送千位数
                sendByte((_pendingTrack / 1000) % 10);
                _cmdStep = 3;
                _lastCmdTime = currentTime;
                break;
                
            case 3:  // 发送百位数
                sendByte((_pendingTrack / 100) % 10);
                _cmdStep = 4;
                _lastCmdTime = currentTime;
                break;
                
            case 4:  // 发送十位数
                sendByte((_pendingTrack / 10) % 10);
                _cmdStep = 5;
                _lastCmdTime = currentTime;
                break;
                
            case 5:  // 发送个位数
                sendByte(_pendingTrack % 10);
                _cmdStep = 6;
                _lastCmdTime = currentTime;
                break;
                
            case 6:  // 发送播放命令
                sendByte(CMD_PLAY_SPECIFIED);
                _cmdStep = 7;
                _lastCmdTime = currentTime;
                break;
                
            case 7:  // 播放完成
                _playerState = PLAYER_STATE_PLAYING;
                _needUpdate = false;
                _cmdStep = 0;
                Serial.println("JQ8900Player: 异步播放完成，曲目 " + String(_pendingTrack));
                break;
        }
    }
} 