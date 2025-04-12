#include "LEDMatrix.h"

// 定义颜色
const uint32_t Red = 0xFF0000;
const uint32_t Green = 0x00FF00;
const uint32_t Blue = 0x0000FF;
const uint32_t Yellow = 0xFFFF00;

// 4x8数字点阵定义
const bool DIGITS[10][8][4] = {
    { // 0
        {1,1,1,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1}
    },
    { // 1
        {0,0,1,0},
        {0,1,1,0},
        {1,0,1,0},
        {0,0,1,0},
        {0,0,1,0},
        {0,0,1,0},
        {0,0,1,0},
        {1,1,1,1}
    },
    { // 2
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {1,1,1,1},
        {1,0,0,0},
        {1,0,0,0},
        {1,0,0,0},
        {1,1,1,1}
    },
    { // 3
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {1,1,1,1}
    },
    { // 4
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1}
    },
    { // 5
        {1,1,1,1},
        {1,0,0,0},
        {1,0,0,0},
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {1,1,1,1}
    },
    { // 6
        {1,1,1,1},
        {1,0,0,0},
        {1,0,0,0},
        {1,1,1,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1}
    },
    { // 7
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1}
    },
    { // 8
        {1,1,1,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1}
    },
    { // 9
        {1,1,1,1},
        {1,0,0,1},
        {1,0,0,1},
        {1,1,1,1},
        {0,0,0,1},
        {0,0,0,1},
        {0,0,0,1},
        {1,1,1,1}
    }
};

LEDMatrix::LEDMatrix() : strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800) {
    currentNumber = 0;
    lastUpdateTime = 0;
    needsFullUpdate = true;
    
    // 初始化缓存
    for (int i = 0; i < NUM_LEDS; i++) {
        pixelCache[i] = 0;
        pixelChanged[i] = false;
    }
}

void LEDMatrix::begin() {
    // 初始化NeoPixel
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    clear();
}

void LEDMatrix::update() {
    bool hasChanges = false;
    
    // 检查是否有像素变化
    for (int i = 0; i < NUM_LEDS; i++) {
        if (pixelChanged[i]) {
            hasChanges = true;
            pixelChanged[i] = false;  // 重置变化标记
        }
    }
    
    // 只有在有变化时才更新显示
    if (hasChanges || needsFullUpdate) {
        strip.show();
        needsFullUpdate = false;
    }
}

void LEDMatrix::clear() {
    bool needUpdate = false;
    for (int i = 0; i < NUM_LEDS; i++) {
        if (pixelCache[i] != 0) {
            pixelCache[i] = 0;
            pixelChanged[i] = true;
            strip.setPixelColor(i, 0);
            needUpdate = true;
        }
    }
    if (needUpdate) {
        strip.show();
    }
}

void LEDMatrix::clearAll() {
    clear();
}

void LEDMatrix::showTestPattern() {
    // 清除所有LED
    clear();
    delay(50);
    
    uint32_t Yellow = 0xFFFF00;
    
    // 遍历所有行
    for(int y = 0; y < 8; y++) {
        // 左侧3列显示绿色 (0-2)
        for(int x = 0; x < 3; x++) {
            int index = getIndex(x, y);
            strip.setPixelColor(index, Green);
        }
        
        // 中间3列显示黄色 (3-5)
        for(int x = 3; x < 6; x++) {
            int index = getIndex(x, y);
            strip.setPixelColor(index, Yellow);
        }
        
        // 右侧2列显示红色 (6-7)
        for(int x = 6; x < 8; x++) {
            int index = getIndex(x, y);
            strip.setPixelColor(index, Red);
        }
    }
    
    strip.show();
}

void LEDMatrix::setPixel(int x, int y, uint32_t color) {
    if(x >= 0 && x < 8 && y >= 0 && y < 8) {
        int index = getIndex(x, y);
        setPixel(index, color);
    }
}

void LEDMatrix::setPixel(int index, uint32_t color) {
    if(index >= 0 && index < NUM_LEDS) {
        if (pixelCache[index] != color) {
            pixelCache[index] = color;
            pixelChanged[index] = true;
            strip.setPixelColor(index, color);
        }
    }
}

int LEDMatrix::getIndex(int x, int y) {
    if(x >= 0 && x < 8 && y >= 0 && y < 8) {
        // 偶数行（0,2,4,6）从右到左
        if(y % 2 == 0) {
            return y * 8 + (7 - x);  // 从右到左
        } else {
            return y * 8 + x;        // 从左到右
        }
    }
    return 0;
}

void LEDMatrix::showDigit(int digit, int xOffset, uint32_t color) {
    if(digit < 0 || digit > 9) return;
    
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 4; x++) {
            if(DIGITS[digit][y][x]) {
                setPixel(x + xOffset, y, color);
            }
        }
    }
}

void LEDMatrix::showNumber(int number, uint32_t color) {
    clear();  // 先清除显示
    if(number >= 0 && number <= 99) {
        if(number < 10) {
            // 个位数，显示在中间
            showDigit(number, 2, color);
        } else {
            // 两位数，分别显示在左右两边
            showDigit(number / 10, 0, color);
            showDigit(number % 10, 4, color);
        }
    }
    update();  // 更新显示
}

void LEDMatrix::showTwoNumbers(int leftNum, int rightNum, uint32_t leftColor, uint32_t rightColor) {
    clear();  // 先清除显示
    if(leftNum >= 0 && leftNum <= 9) {
        showDigit(leftNum, 0, leftColor);  // 左边数字使用指定颜色
    }
    if(rightNum >= 0 && rightNum <= 9) {
        showDigit(rightNum, 4, rightColor);   // 右边数字使用指定颜色
    }
    update();  // 更新显示
}

void LEDMatrix::startCountdown(int fromNumber) {
    currentNumber = fromNumber;
    lastUpdateTime = millis();
    showNumber(currentNumber, Green);
}

void LEDMatrix::updateCountdown() {
    unsigned long currentTime = millis();
    if(currentTime - lastUpdateTime >= COUNTDOWN_INTERVAL) {
        lastUpdateTime = currentTime;
        if(currentNumber > 0) {
            currentNumber--;
            showNumber(currentNumber, Green);
        }
    }
} 