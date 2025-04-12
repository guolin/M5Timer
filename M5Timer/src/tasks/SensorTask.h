#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"

// 声明为外部变量，实际定义在main.cpp中
extern QueueHandle_t sensorQueue;

void sensorTask(void *parameter); 