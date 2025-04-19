#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"  // 包含共享类型定义

// 声明全局变量
extern QueueHandle_t audioQueue;
extern SemaphoreHandle_t audioMutex;

// 音频任务函数
void audioTask(void *parameter);

// 帮助函数 - 发送音频停止命令并等待完成
void audioStop();

// 帮助函数 - 发送播放指定曲目命令并等待完成
void audioPlayTrack(uint16_t track);

// 帮助函数 - 发送播放指定曲目命令但不等待（非阻塞）
void audioPlayTrackNonBlocking(uint16_t track);

#endif // AUDIO_TASK_H 