#include "SensorTask.h"
#include <M5Unified.h>
#include <Wire.h>

// 定义电池容量参数
#define BATTERY_CAPACITY 120.0f  // 电池容量 120mAh

// 定义电压阈值
#define VOLTAGE_MAX 4.2f   // 满电电压
#define VOLTAGE_MIN 3.3f   // 最低电压

// 声明外部队列变量（已在main.cpp中定义）
// extern QueueHandle_t sensorQueue; // 已在SensorTask.h中声明，这里不需要重复声明

// 传感器任务函数
void sensorTask(void *pvParameters) {
    Serial.println("传感器任务已启动");
    
    // 创建传感器数据结构
    SensorData sensorData;
    
    // 确保队列已创建
    if (sensorQueue == NULL) {
        Serial.println("错误: 传感器队列未初始化!");
        vTaskDelay(portMAX_DELAY); // 暂停任务
    }
    
    // 上次更新时间
    unsigned long lastUpdateTime = 0;
    
    // 任务主循环
    while (true) {
        // 获取当前时间
        unsigned long currentTime = millis();
        
        // 每500毫秒更新一次数据
        if (currentTime - lastUpdateTime >= 500) {
            lastUpdateTime = currentTime;
            
            // 读取AXP192电源管理数据
            if (M5.Power.Axp192.isEnabled()) {
                // 初始化消息类型
                sensorData.type = MSG_SENSOR_DATA;
                
                // 电池电压 (V)
                float rawBatteryVoltage = M5.Power.getBatteryVoltage();
                sensorData.batteryVoltage = rawBatteryVoltage / 1000.0f;  // 转换为V
                Serial.printf("原始电池电压: %.0fmV\n", rawBatteryVoltage);
                
                // 充电状态
                sensorData.isCharging = M5.Power.Axp192.isCharging();
                Serial.printf("充电状态: %s\n", sensorData.isCharging ? "充电中" : "放电中");
                
                // 充电/放电电流 (mA)
                if (sensorData.isCharging) {
                    sensorData.chargeCurrent = M5.Power.Axp192.getBatteryChargeCurrent() / 1000.0f;  // 转换为mA
                    sensorData.dischargeCurrent = 0;
                    Serial.printf("充电电流: %.1fmA\n", sensorData.chargeCurrent);
                } else {
                    sensorData.chargeCurrent = 0;
                    sensorData.dischargeCurrent = M5.Power.Axp192.getBatteryDischargeCurrent() / 1000.0f;  // 转换为mA
                    Serial.printf("放电电流: %.1fmA\n", sensorData.dischargeCurrent);
                }
                
                // 直接使用API获取电池电量百分比
                sensorData.batteryPercentage = M5.Power.Axp192.getBatteryLevel();
                Serial.printf("API获取的电量: %.1f%%\n", sensorData.batteryPercentage);
                
                // USB电压和电流
                float rawUsbVoltage = M5.Power.Axp192.getVBUSVoltage();
                sensorData.usbVoltage = rawUsbVoltage;  // 转换为V
                sensorData.usbCurrent = M5.Power.Axp192.getVBUSCurrent();  // 转换为mA
                Serial.printf("原始USB电压: %.0fV, USB电流: %.1fmA\n", rawUsbVoltage, sensorData.usbCurrent);
                
                // 芯片温度
                sensorData.temperature = M5.Power.Axp192.getInternalTemperature();
                Serial.printf("芯片温度: %.1f°C\n", sensorData.temperature);
                
                // 打印调试信息
                Serial.printf("电池信息: %.2fV %.1f%% %s ", 
                    sensorData.batteryVoltage, 
                    sensorData.batteryPercentage,
                    sensorData.isCharging ? "充电中" : "放电中");
                
                if (sensorData.isCharging) {
                    Serial.printf("充电电流: %.1fmA", sensorData.chargeCurrent);
                } else {
                    Serial.printf("放电电流: %.1fmA", sensorData.dischargeCurrent);
                }
                
                Serial.printf(" 温度: %.1f°C\n", sensorData.temperature);
                Serial.printf("USB: %.2fV %.1fmA\n", 
                    sensorData.usbVoltage, 
                    sensorData.usbCurrent);
            }
            else {
                // AXP192未启用，使用默认值
                sensorData.batteryVoltage = 0.0;
                sensorData.dischargeCurrent = 0.0;
                sensorData.chargeCurrent = 0.0;
                sensorData.batteryPercentage = 0.0;
                sensorData.isCharging = false;
                sensorData.usbVoltage = 0.0;
                sensorData.usbCurrent = 0.0;
                sensorData.temperature = 0.0;
                
                Serial.println("传感器任务: AXP192未启用，无法读取电池数据");
            }
            
            // 将数据发送到队列
            xQueueOverwrite(sensorQueue, &sensorData);
        }
        
        // 延迟一小段时间以减少CPU使用率
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
} 