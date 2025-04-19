#include "MusicMode.h"
#include "../core/LEDMatrix.h"
#include <driver/i2s.h>

// 声明外部全局变量
extern LEDMatrix ledMatrix;

// 定义颜色值
const uint32_t MusicMode::colorValues[5] = {
    0xFF0000,  // 红色
    0xFFFF00,  // 黄色
    0x0000FF,  // 蓝色
    0x00FF00,  // 绿色
    0xFFFFFF   // 白色
};

// 颜色对应的名称
const char* MusicMode::audioColorNames[5] = {
    "Red",     // 红色
    "Yellow",  // 黄色
    "Blue",    // 蓝色
    "Green",   // 绿色
    "White"    // 白色
};

// 颜色对应的LCD颜色值 (16位RGB565格式)
const uint16_t MusicMode::audioLcdColors[5] = {
    0xF800,    // 红色
    0xFFE0,    // 黄色
    0x001F,    // 蓝色
    0x07E0,    // 绿色
    0xFFFF     // 白色
};

// 灵敏度系数
const float MusicMode::sensitivities[10] = {
    1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f
};

// 波形显示的旧Y值 (用于擦除)
static uint16_t oldY[160] = {0};

MusicMode::MusicMode() : Mode("Music") {
    sensitivityLevel = 4;  // 默认中等灵敏度
    colorMode = 2;        // 默认蓝色
    audioBuffer = (int16_t*)buffer;
}

void MusicMode::begin() {
    Serial.println("Entering Music Mode");
    
    // 初始化LED矩阵
    ledMatrix.begin();
    ledMatrix.clear();
    ledMatrix.update();
    
    // 初始化绘图区域
    for (int i = 0; i < 8; i++) {
        oldValues[i] = 0;
    }
    
    // 初始化I2S - 直接使用用户示例代码的方法
    i2sInit();
    
    // 在LCD上显示当前模式
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Music Mode");
    M5.Lcd.println("A: Sensitivity");
    M5.Lcd.printf("Level: %d", sensitivityLevel);
    M5.Lcd.setCursor(0, 60);
    M5.Lcd.println("B: Color");
    M5.Lcd.setTextColor(audioLcdColors[colorMode]);
    M5.Lcd.println(audioColorNames[colorMode]);
    
    // 创建任务用于音频处理
    xTaskCreate(
        micRecordTask,    // 任务函数
        "mic_record_task", // 任务名称
        2048,             // 堆栈大小
        this,             // 参数 (传递this指针)
        1,                // 优先级
        NULL              // 任务句柄
    );
}

void MusicMode::update() {
    // 注意：主要处理逻辑已移至micRecordTask
    // 此函数只处理非音频相关的更新
    delay(10);  // 短暂延迟，避免过快循环
}

void MusicMode::exit() {
    Serial.println("Exiting Music Mode");
    
    // 关闭I2S
    i2s_driver_uninstall(I2S_PORT);
    
    // 清除LED矩阵
    ledMatrix.clear();
    ledMatrix.update();
}

void MusicMode::handleEvent(EventType event) {
    switch (event) {
        case EVENT_BUTTON_A:
            // 按A键切换灵敏度
            sensitivityLevel = (sensitivityLevel + 1) % 10;
            Serial.printf("Sensitivity changed to: %d (%.1f)\n", sensitivityLevel, sensitivities[sensitivityLevel]);
            
            // 更新LCD显示
            M5.Lcd.fillRect(0, 32, 160, 20, BLACK);
            M5.Lcd.setCursor(0, 32);
            M5.Lcd.setTextColor(WHITE);
            M5.Lcd.printf("Level: %d", sensitivityLevel);
            break;
            
        case EVENT_BUTTON_B:
            // 按B键切换颜色
            colorMode = (colorMode + 1) % 5;
            Serial.printf("Color changed to: %s\n", audioColorNames[colorMode]);
            
            // 更新LCD显示
            M5.Lcd.fillRect(0, 80, 160, 20, BLACK);
            M5.Lcd.setCursor(0, 80);
            M5.Lcd.setTextColor(audioLcdColors[colorMode]);
            M5.Lcd.println(audioColorNames[colorMode]);
            break;
    }
}

// I2S初始化函数 - 直接从用户示例中移植
void MusicMode::i2sInit() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_pin_config_t pin_config;

#if (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 3, 0))
    pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
#endif

    pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num = 0;  // PIN_CLK
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = 34;  // PIN_DATA

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

// 麦克风记录任务 - 在独立任务中运行，类似用户示例
void MusicMode::micRecordTask(void* arg) {
    MusicMode* mode = (MusicMode*)arg;
    size_t bytesRead;
    
    while (1) {
        // 读取I2S数据 - 需要通过指针访问
        i2s_read(I2S_NUM_0, (char*)mode->buffer, mode->READ_LEN, &bytesRead, 
                 (100 / portTICK_RATE_MS));
        
        // 通过指针调用方法
        mode->showWaveform();
        mode->processAudio();
        mode->updateLEDs();
        
        // 添加延迟，避免过快更新
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void MusicMode::processAudio() {
    // 将音频数据分成8段
    int segmentSize = READ_LEN / 2 / 8;  // 每个段的大小
    
    for (int i = 0; i < 8; i++) {
        // 计算当前段的平均能量
        long sum = 0;
        
        for (int j = 0; j < segmentSize; j++) {
            int index = i * segmentSize + j;
            if (index < READ_LEN/2) {
                int16_t sample = audioBuffer[index];
                sum += abs(sample);
            }
        }
        
        // 计算平均能量并应用增益
        float avgEnergy = (float)sum / segmentSize;
        avgEnergy *= GAIN_FACTOR * sensitivities[sensitivityLevel];
        
        // 映射到LED高度 (1-7)
        int height = map(avgEnergy, 0, 32767, 1, 7);
        height = constrain(height, 1, 7);
        
        // 平滑处理
        height = (oldValues[i] * 0.7f) + (height * 0.3f);
        oldValues[i] = height;
    }
}

void MusicMode::updateLEDs() {
    // 清除LED
    ledMatrix.clear();
    
    // 获取当前选择的颜色
    uint32_t currentColor = colorValues[colorMode];
    
    // 为每列绘制音频柱状图
    for (int x = 0; x < 8; x++) {
        int height = oldValues[x];
        
        // 从底部开始绘制每一列
        for (int y = 7; y >= (8 - height); y--) {
            ledMatrix.setPixel(x, y, currentColor);
        }
    }
    
    // 更新LED显示
    ledMatrix.update();
}

void MusicMode::showWaveform() {
    // 完全模仿用户示例代码的方式绘制波形
    int y;
    for (int n = 0; n < 160; n++) {
        // 只处理LCD宽度内的像素
        if (n < 160) {
            // 获取并放大音频数据
            y = audioBuffer[n] * GAIN_FACTOR;
            
            // 映射到LCD坐标
            y = map(y, INT16_MIN, INT16_MAX, 110-25, 110+25);
            
            // 擦除旧点
            M5.Lcd.drawPixel(n, oldY[n], BLACK);
            
            // 绘制新点
            M5.Lcd.drawPixel(n, y, audioLcdColors[colorMode]);
            
            // 记录当前点位置
            oldY[n] = y;
        }
    }
} 