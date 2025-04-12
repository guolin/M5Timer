#include "AudioTask.h"
#include "core/Player.h"

void audioTask(void *parameter) {
    // 使用 intptr_t 进行安全的指针到整数的转换
    uint8_t pin = (uint8_t)(intptr_t)parameter;
    AudioMessage msg;
    
    // 创建播放器实例
    JQ8900Player player(pin);
    
    // 初始化播放器
    player.begin();
    player.setVolume(30);  // 设置默认音量为最大值30
    player.setLoopMode(LOOP_DISABLE);  // 设置默认循环模式为不循环
    
    while (true) {
        // 等待音频消息
        if (xQueueReceive(audioQueue, &msg, portMAX_DELAY) == pdTRUE) {
            // 获取音频互斥锁
            if (xSemaphoreTake(audioMutex, portMAX_DELAY) == pdTRUE) {
                // 处理音频消息
                switch (msg.type) {
                    case MSG_AUDIO_PLAY:
                        player.play();
                        break;
                    case MSG_AUDIO_PAUSE:
                        player.pause();
                        break;
                    case MSG_AUDIO_STOP:
                        player.stop();
                        break;
                    case MSG_AUDIO_NEXT:
                        player.next();
                        break;
                    case MSG_AUDIO_VOLUME:
                        player.setVolume(msg.volume);
                        break;
                    case MSG_AUDIO_TRACK:
                        player.playTrack(msg.track);
                        break;
                    case MSG_AUDIO_RANDOM:
                        player.playRandom();
                        break;
                }
                
                // 释放音频互斥锁
                xSemaphoreGive(audioMutex);
            }
        }
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(100));
    }
} 