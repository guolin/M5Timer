#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"
#include "../modes/BatteryMode.h"
#include "../modes/TimerMode.h"

extern BatteryMode batteryMode;
extern TimerMode timerMode;
extern QueueHandle_t modeQueue;
extern QueueHandle_t eventQueue;

void modeTask(void *parameter); 