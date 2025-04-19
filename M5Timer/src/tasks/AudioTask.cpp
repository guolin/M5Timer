#include "AudioTask.h"
#include "../core/Player.h"

// 移除全局变量定义，只在main.cpp中定义
// 这里只使用extern定义的外部变量

// 音频任务函数
void audioTask(void *parameter) {
    // 获取播放器引脚
    uint8_t pin = (uint8_t)(intptr_t)parameter;
    AudioMessage msg;
    
    // 创建播放器实例 - 直接创建不使用单例
    JQ8900Player player(pin);
    
    // 初始化播放器
    player.begin();
    player.setVolume(30);  // 设置默认音量为最大值30
    player.setLoopMode(LOOP_DISABLE);  // 设置默认循环模式为不循环
    
    Serial.println("AudioTask: 任务启动，准备处理消息");
    
    while (true) {
        // 处理异步命令更新
        player.update();
        
        // 等待音频消息，使用较短的超时时间，而不是无限等待
        if (xQueueReceive(audioQueue, &msg, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 获取音频互斥锁
            if (xSemaphoreTake(audioMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                // 处理音频消息 - 提高优先级
                switch (msg.type) {
                    case MSG_AUDIO_PLAY:
                        Serial.println("AudioTask: 播放");
                        player.play();
                        break;
                        
                    case MSG_AUDIO_PAUSE:
                        Serial.println("AudioTask: 暂停");
                        player.pause();
                        break;
                        
                    case MSG_AUDIO_STOP:
                        Serial.println("AudioTask: 停止");
                        player.stop();
                        break;
                        
                    case MSG_AUDIO_NEXT:
                        Serial.println("AudioTask: 下一曲");
                        player.next();
                        break;
                        
                    case MSG_AUDIO_VOLUME:
                        Serial.println("AudioTask: 设置音量 " + String(msg.volume));
                        player.setVolume(msg.volume);
                        break;
                        
                    case MSG_AUDIO_TRACK:
                        Serial.println("AudioTask: 播放曲目 " + String(msg.track));
                        player.playTrack(msg.track);
                        break;
                        
                    case MSG_AUDIO_RANDOM:
                        Serial.println("AudioTask: 随机播放");
                        player.playRandom();
                        break;
                }
                
                // 释放音频互斥锁
                xSemaphoreGive(audioMutex);
            }
        }
    }
}

// 帮助函数 - 发送音频停止命令并等待完成
void audioStop() {
    AudioMessage msg;
    msg.type = MSG_AUDIO_STOP;
    
    // 先销毁队列中的其他请求
    if (audioQueue != NULL) {
        xQueueReset(audioQueue);
    }
    
    // 发送停止命令，设置最高优先级
    if (audioQueue != NULL) {
        xQueueSendToFront(audioQueue, &msg, 0);
        
        // 短暂延迟确保命令开始执行
        delay(20);
    }
}

// 帮助函数 - 发送播放指定曲目命令
void audioPlayTrack(uint16_t track) {
    // 先停止所有声音
    audioStop();
    
    // 然后播放指定曲目
    AudioMessage msg;
    msg.type = MSG_AUDIO_TRACK;
    msg.track = track;
    
    if (audioQueue != NULL) {
        Serial.println("发送播放命令，曲目: " + String(track) + " (阻塞)");
        xQueueSendToFront(audioQueue, &msg, 0);  // 使用最高优先级，不等待
        
        // 短暂延迟确保命令开始执行
        delay(20);
    }
}

// 帮助函数 - 发送播放指定曲目命令但不等待（非阻塞）
void audioPlayTrackNonBlocking(uint16_t track) {
    AudioMessage msg;
    msg.type = MSG_AUDIO_TRACK;
    msg.track = track;
    
    if (audioQueue != NULL) {
        Serial.println("发送播放命令，曲目: " + String(track) + " (非阻塞)");
        xQueueSendToFront(audioQueue, &msg, 0);  // 使用最高优先级，不等待
    }
} 