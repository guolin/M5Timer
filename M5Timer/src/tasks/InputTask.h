#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"

extern QueueHandle_t modeQueue;
extern QueueHandle_t audioQueue;
extern QueueHandle_t eventQueue;

void inputTask(void *parameter); 