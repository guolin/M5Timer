#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "../core/types.h"

// 输入处理任务
void inputTask(void *parameter);

// 全局队列声明
extern QueueHandle_t eventQueue; 