# 代码架构文档

## 1. 系统概述

本系统基于FreeRTOS操作系统，采用多任务设计，实现了VEX IQ比赛倒计时器的核心功能。系统设计注重模块化和可扩展性，为未来功能扩展预留了接口。

## 2. 任务设计

### 2.1 显示任务 (DisplayTask)
- 优先级：3
- 周期：50ms
- 职责：
  - 更新M5Stack LCD显示内容
  - 更新LED像素屏显示
  - 维护显示状态
- 通信：
  - 接收显示队列消息
  - 使用显示互斥锁

### 2.2 音频任务 (AudioTask)
- 优先级：2
- 周期：100ms
- 职责：
  - 控制MP3模块播放
  - 管理音频状态
  - 处理音频事件
- 通信：
  - 接收音频队列消息
  - 使用音频互斥锁

### 2.3 输入任务 (InputTask)
- 优先级：3
- 周期：20ms
- 职责：
  - 处理M5Stack按键输入
  - 检测长按事件
  - 发送按键事件到模式任务
- 通信：
  - 发送按键事件到模式队列

### 2.4 模式任务 (ModeTask)
- 优先级：2
- 周期：100ms
- 职责：
  - 管理当前运行模式
  - 处理模式切换
  - 执行模式逻辑
- 通信：
  - 接收模式控制消息
  - 发送显示和音频命令

### 2.5 传感器任务 (SensorTask)
- 优先级：1
- 周期：50ms
- 职责：
  - 读取传感器数据
  - 处理传感器事件
  - 发送传感器数据
- 通信：
  - 发送传感器数据到传感器队列

## 3. 核心类设计

### 3.1 DisplayManager类
```cpp
class DisplayManager {
    // LCD显示功能
    void showBatteryStatus(float percentage);
    void showCountdown(int seconds);
    void clearLCD();
    
    // LED像素屏功能
    void showLEDNumber(int number);
    void showLEDPattern(const uint8_t* pattern);
    void clearLED();
}
```

### 3.2 MP3Player类
```cpp
class MP3Player {
    // 音频控制
    void play(int trackNumber);
    void pause();
    void resume();
    void setVolume(int volume);
    
    // 状态查询
    bool isPlaying();
    int getCurrentTrack();
}
```

### 3.3 模式基类
```cpp
class BaseMode {
    // 模式生命周期
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void exit() = 0;
    
    // 事件处理
    virtual void handleEvent(EventType event) = 0;
}
```

## 4. 通信机制

### 4.1 消息队列
- displayQueue: 显示更新消息
- audioQueue: 音频控制消息
- modeQueue: 模式控制消息
- sensorQueue: 传感器数据消息

### 4.2 互斥锁
- displayMutex: 显示互斥锁
- audioMutex: 音频控制互斥锁

### 4.3 事件组
- systemEvents: 系统事件组
- modeEvents: 模式事件组

## 5. 文件结构
```
src/
├── main.cpp              // 主程序入口
├── tasks/               // 任务实现
│   ├── DisplayTask.h
│   ├── DisplayTask.cpp
│   ├── AudioTask.h
│   ├── AudioTask.cpp
│   ├── InputTask.h
│   ├── InputTask.cpp
│   ├── ModeTask.h
│   ├── ModeTask.cpp
│   ├── SensorTask.h
│   └── SensorTask.cpp
├── core/                // 核心类
│   ├── DisplayManager.h
│   ├── DisplayManager.cpp
│   ├── LEDMatrix.h
│   ├── LEDMatrix.cpp
├── modes/              // 功能模式
│   ├── BaseMode.h
│   ├── CountdownMode.h
│   ├── CountdownMode.cpp
│   ├── BatteryMode.h
│   └── BatteryMode.cpp
└── utils/              // 工具类
    ├── MP3Player.h
    └── MP3Player.cpp
```

## 6. 扩展性设计

### 6.1 新功能添加
1. 创建新的模式类，继承BaseMode
2. 实现必要的虚函数
3. 在ModeTask中注册新模式
4. 添加相应的显示和音频资源

### 6.2 预留功能
- 平衡球游戏（使用陀螺仪）
- 拾音器功能
- 自定义动画效果
- 更多显示模式

## 7. 调试支持

### 7.1 任务监控
- 使用FreeRTOS任务统计
- 监控任务运行时间
- 检测任务堆栈溢出

### 7.2 错误处理
- 异常捕获机制
- 错误日志记录
- 自动恢复机制

### 7.3 性能优化
- 任务优先级调整
- 内存使用优化
- 显示刷新率控制

## 8. 注意事项

1. 线程安全
   - 所有共享资源必须使用互斥锁保护
   - 避免长时间持有锁
   - 使用消息队列进行任务间通信

2. 内存管理
   - 合理分配任务栈大小
   - 注意内存碎片化
   - 优先使用静态内存分配

3. 电源管理
   - 合理控制LED亮度
   - 优化任务执行周期
   - 实现低功耗模式

4. 代码规范
   - 统一的命名规范
   - 清晰的注释
   - 模块化设计 