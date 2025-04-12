#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"

extern QueueHandle_t audioQueue;
extern SemaphoreHandle_t audioMutex;

void audioTask(void *parameter); 