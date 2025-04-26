#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../core/types.h"
#include "../core/Mode.h"
#include <vector>

// 外部队列声明
extern QueueHandle_t modeQueue;
extern QueueHandle_t eventQueue;

// ModeTask函数声明
void modeTask(void *parameter);

// 模式管理函数
void registerMode(Mode* mode);
Mode* getCurrentMode();
void initModeTask();
int getRegisteredModeCount();
void switchToNextMode();
void switchToPreviousMode();
void switchToMode(int modeIndex);

// ScreenMode相关函数
bool isScreenMode(Mode* mode);
int getScreenModeIndex();
void setScreenModeAvailable(bool available);
bool switchToScreenMode(); 