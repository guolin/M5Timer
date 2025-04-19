// 常量定义
const MATRIX_COLS = 4;  // 总矩阵列数
const MATRIX_ROWS = 3;  // 矩阵行数
const GAME_COLS = 3;    // 游戏显示区域列数
const LED_SIZE = 8;     // 每个矩阵的LED数量（8x8）
const TOTAL_WIDTH = MATRIX_COLS * LED_SIZE;  // 总宽度 32
const GAME_WIDTH = GAME_COLS * LED_SIZE;     // 游戏区域宽度 24
const TOTAL_HEIGHT = MATRIX_ROWS * LED_SIZE; // 总高度 24
const LED_UNIT = 1.0;   // 单个LED的尺寸（厘米）
const SCREEN_BORDER = 0.8;  // 屏幕边框宽度（厘米）
const OUTER_BORDER = 0.2;   // 外边框宽度（厘米）
const BUTTON_WIDTH = 4.8;   // 按钮宽度（厘米）
const BUTTON_HEIGHT = 1.0;  // 按钮高度（厘米）

// 颜色映射字典
const COLOR_MAP = {
    'black': 0,        // 0: 黑色（关闭）
    'red': 1,          // 1: 红色
    'green': 2,        // 2: 绿色
    'blue': 3,         // 3: 蓝色
    'yellow': 4,       // 4: 黄色
    'purple': 5,       // 5: 紫色
    'cyan': 6,         // 6: 青色
    'white': 7,        // 7: 白色
    'darkRed': 8,      // 8: 暗红色
    'darkGreen': 9,    // 9: 暗绿色
    'darkBlue': 10,    // 10: 暗蓝色
    'brown': 11,       // 11: 棕色
    'pink': 12,        // 12: 粉色
    'lightCyan': 13,   // 13: 淡青色
    'lightPink': 14,   // 14: 浅粉色
    'gray': 15         // 15: 灰色
};

// 游戏常量
const PLAYER_COLOR = [0, 255, 0];  // 玩家飞机颜色（绿色）
const ALIEN_COLORS = [
    [255, 0, 0],    // 第一排外星人颜色（红色）
    [255, 128, 0],  // 第二排外星人颜色（橙色）
    [255, 255, 0]   // 第三排外星人颜色（黄色）
];
const PLAYER_BULLET_COLOR = [255, 255, 255];  // 玩家子弹颜色（白色）
const ALIEN_BULLET_COLOR = [255, 0, 0];   // 外星人子弹颜色（红色）
const SHIELD_COLOR = [0, 255, 0];  // 掩体颜色（绿色）
const SCORE_COLOR = [255, 255, 0];  // 分数颜色（黄色）

// 游戏难度设置
const DIFFICULTY_LEVELS = {
    EASY: {
        alienSpeed: 0.5,
        alienMoveInterval: 8,
        alienShootInterval: 1500,
        playerColumnShootChance: 0.5,
        alienRows: 3,
        alienCols: 5
    },
    NORMAL: {
        alienSpeed: 1,
        alienMoveInterval: 5,
        alienShootInterval: 1000,
        playerColumnShootChance: 0.7,
        alienRows: 4,
        alienCols: 6
    },
    HARD: {
        alienSpeed: 1.5,
        alienMoveInterval: 3,
        alienShootInterval: 1000,
        playerColumnShootChance: 0.9,
        alienRows: 5,
        alienCols: 7
    }
};

// 当前难度设置
let currentDifficulty = DIFFICULTY_LEVELS.NORMAL;

// 声音效果
let playerShootSound;
let alienShootSound;
let explosionSound;
let audioActivated = false;

// 串口相关变量
let ports = new Array(12).fill(null);  // 存储所有串口
let writers = new Array(12).fill(null);  // 存储所有写入器
let readers = new Array(12).fill(null);  // 存储所有读取器
let connectedScreens = new Array(12).fill(false);  // 存储屏幕连接状态

// 连接串口
async function connectSerial(screenIndex) {
    try {
        // 请求用户选择串口
        const newPort = await navigator.serial.requestPort();
        
        // 打开串口
        await newPort.open({ baudRate: 115200 });
        
        // 获取写入器和读取器
        const writer = newPort.writable.getWriter();
        const reader = newPort.readable.getReader();
        
        // 存储串口信息
        ports[screenIndex] = newPort;
        writers[screenIndex] = writer;
        readers[screenIndex] = reader;
        connectedScreens[screenIndex] = true;
        
        // 更新界面状态
        updateScreenStatus(screenIndex, true);
        
        // 开始读取数据
        readLoop(screenIndex);
        
        // 连接成功后显示屏幕编号
        testSingleScreen(screenIndex);
        
        return true;
    } catch (err) {
        console.error(`屏幕${screenIndex + 1}串口连接失败:`, err);
        return false;
    }
}

// 断开串口
async function disconnectSerial(screenIndex) {
    try {
        if (writers[screenIndex]) {
            await writers[screenIndex].releaseLock();
            writers[screenIndex] = null;
        }
        if (readers[screenIndex]) {
            await readers[screenIndex].releaseLock();
            readers[screenIndex] = null;
        }
        if (ports[screenIndex]) {
            await ports[screenIndex].close();
            ports[screenIndex] = null;
        }
        connectedScreens[screenIndex] = false;
        
        // 更新界面状态
        updateScreenStatus(screenIndex, false);
        
        return true;
    } catch (err) {
        console.error(`屏幕${screenIndex + 1}串口断开失败:`, err);
        return false;
    }
}

// 更新屏幕状态显示
function updateScreenStatus(screenIndex, connected) {
    const screenButtons = document.querySelectorAll('.screen-button');
    if (screenButtons[screenIndex]) {
        if (connected) {
            screenButtons[screenIndex].classList.add('connected');
        } else {
            screenButtons[screenIndex].classList.remove('connected');
        }
    }
    
    // 检查所有屏幕是否都已连接
    updateConfigButtonStatus();
}

// 更新配置按钮状态
function updateConfigButtonStatus() {
    const allConnected = connectedScreens.every(status => status);
    const configButton = document.getElementById('configButton');
    
    if (allConnected) {
        configButton.classList.add('connected');
    } else {
        configButton.classList.remove('connected');
    }
    
    // 更新串口状态信息
    const connectedCount = connectedScreens.filter(status => status).length;
    document.getElementById('serialInfo').textContent = `串口状态: ${connectedCount}/12 屏幕已连接`;
}

// 测试单个屏幕 - 显示编号
function testSingleScreen(screenIndex) {
    if (!writers[screenIndex]) return;
    
    // 清空屏幕
    const clearData = new Array(64).fill(0);
    const clearDataString = clearData.join(',') + '\n';
    sendData(clearDataString, screenIndex);
    
    // 生成屏幕编号的显示数据
    setTimeout(() => {
        const displayNumber = screenIndex + 1;
        const data = new Array(64).fill(0);
        
        // 简单绘制数字
        switch(displayNumber) {
            case 1:
                // 绘制数字1
                for (let i = 1; i < 7; i++) {
                    data[8*i+3] = 7; // 白色垂直线
                }
                break;
            case 2:
                // 绘制数字2
                for (let i = 2; i <= 4; i++) data[8+i] = 7; // 顶部横线
                data[2*8+5] = 7;
                data[3*8+4] = 7;
                data[4*8+3] = 7;
                for (let i = 2; i <= 5; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 3:
                // 绘制数字3
                for (let i = 2; i <= 4; i++) data[8+i] = 7; // 顶部横线
                data[2*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[3*8+i] = 7; // 中间横线
                data[4*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 4:
                // 绘制数字4
                data[8+2] = 7;
                data[2*8+2] = 7;
                data[3*8+2] = 7;
                for (let i = 2; i <= 5; i++) data[3*8+i] = 7; // 中间横线
                data[8+5] = 7;
                data[2*8+5] = 7;
                data[3*8+5] = 7;
                data[4*8+5] = 7;
                data[5*8+5] = 7;
                break;
            case 5:
                // 绘制数字5
                for (let i = 2; i <= 5; i++) data[8+i] = 7; // 顶部横线
                data[2*8+2] = 7;
                for (let i = 2; i <= 4; i++) data[3*8+i] = 7; // 中间横线
                data[4*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 6:
                // 绘制数字6
                for (let i = 2; i <= 4; i++) data[8+i] = 7; // 顶部横线
                data[2*8+2] = 7;
                for (let i = 2; i <= 4; i++) data[3*8+i] = 7; // 中间横线
                data[4*8+2] = 7;
                data[4*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 7:
                // 绘制数字7
                for (let i = 2; i <= 5; i++) data[8+i] = 7; // 顶部横线
                data[2*8+5] = 7;
                data[3*8+4] = 7;
                data[4*8+3] = 7;
                data[5*8+2] = 7;
                break;
            case 8:
                // 绘制数字8
                for (let i = 2; i <= 4; i++) data[8+i] = 7; // 顶部横线
                data[2*8+2] = 7;
                data[2*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[3*8+i] = 7; // 中间横线
                data[4*8+2] = 7;
                data[4*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 9:
                // 绘制数字9
                for (let i = 2; i <= 4; i++) data[8+i] = 7; // 顶部横线
                data[2*8+2] = 7;
                data[2*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[3*8+i] = 7; // 中间横线
                data[4*8+5] = 7;
                for (let i = 2; i <= 4; i++) data[5*8+i] = 7; // 底部横线
                break;
            case 10:
                // 绘制数字10
                data[8+1] = 7; // 1的顶部
                for (let i = 1; i < 6; i++) data[8*i+1] = 7; // 1的竖线
                for (let i = 3; i <= 5; i++) data[8+i] = 7; // 0的顶部
                data[2*8+3] = 7; data[2*8+6] = 7; // 0的两侧
                data[3*8+3] = 7; data[3*8+6] = 7;
                data[4*8+3] = 7; data[4*8+6] = 7;
                for (let i = 3; i <= 5; i++) data[5*8+i] = 7; // 0的底部
                break;
            case 11:
                // 绘制数字11
                for (let i = 1; i < 6; i++) data[8*i+1] = 7; // 第一个1
                for (let i = 1; i < 6; i++) data[8*i+4] = 7; // 第二个1
                break;
            case 12:
                // 绘制数字12
                for (let i = 1; i < 6; i++) data[8*i+1] = 7; // 1的竖线
                for (let i = 3; i <= 5; i++) data[8+i] = 7; // 2的顶部
                data[2*8+6] = 7;
                data[3*8+5] = 7;
                data[4*8+4] = 7;
                for (let i = 3; i <= 6; i++) data[5*8+i] = 7; // 2的底部
                break;
        }
        
        const dataString = data.join(',') + '\n';
        sendData(dataString, screenIndex);
    }, 500);
}

// 测试所有已连接的屏幕
function testAllScreens() {
    for (let i = 0; i < connectedScreens.length; i++) {
        if (connectedScreens[i]) {
            testSingleScreen(i);
        }
    }
}

// 读取数据的循环
async function readLoop(screenIndex) {
    try {
        while (true) {
            const { value, done } = await readers[screenIndex].read();
            if (done) {
                readers[screenIndex].releaseLock();
                break;
            }
            // 将接收到的数据转换为字符串并添加前缀
            const receivedData = new TextDecoder().decode(value);
            const prefixedData = `SCREEN:${receivedData}`;
            console.log(`屏幕${screenIndex + 1}收到数据:`, prefixedData);
        }
    } catch (err) {
        console.error(`屏幕${screenIndex + 1}读取数据错误:`, err);
    }
}

// 发送数据到串口
async function sendData(data, screenIndex) {
    if (!writers[screenIndex]) return;
    
    try {
        // 添加屏幕前缀
        const prefixedData = `SCREEN:${data}`;
        
        // 将数据转换为Uint8Array
        const encoder = new TextEncoder();
        const encodedData = encoder.encode(prefixedData);
        await writers[screenIndex].write(encodedData);
        
        // 打印发送的数据到控制台
        console.log(`屏幕${screenIndex + 1}发送数据:`, prefixedData);
    } catch (err) {
        console.error(`屏幕${screenIndex + 1}发送数据错误:`, err);
    }
}

// 计算其他尺寸
const LED_MATRIX_SIZE = LED_SIZE * LED_UNIT;  // LED矩阵大小（8cm x 8cm）
const SCREEN_SIZE = LED_MATRIX_SIZE + 2 * SCREEN_BORDER;  // 屏幕大小（9.6cm x 9.6cm）
const DEVICE_SIZE = SCREEN_SIZE + 2 * OUTER_BORDER;  // 设备大小（10cm x 10cm）

// 缩放因子（像素/厘米）
const SCALE = 20;

// 计算像素尺寸
const LED_PIXEL_SIZE = LED_UNIT * SCALE;
const SCREEN_BORDER_PIXELS = SCREEN_BORDER * SCALE;
const OUTER_BORDER_PIXELS = OUTER_BORDER * SCALE;
const BUTTON_PIXEL_WIDTH = BUTTON_WIDTH * SCALE;
const BUTTON_PIXEL_HEIGHT = BUTTON_HEIGHT * SCALE;
const DEVICE_PIXEL_SIZE = DEVICE_SIZE * SCALE;
const LED_MATRIX_PIXEL_SIZE = LED_MATRIX_SIZE * SCALE;
const SCREEN_PIXEL_SIZE = SCREEN_SIZE * SCALE;

// 总画布尺寸
const CANVAS_WIDTH = MATRIX_COLS * DEVICE_PIXEL_SIZE;
const CANVAS_HEIGHT = MATRIX_ROWS * (DEVICE_PIXEL_SIZE + BUTTON_PIXEL_HEIGHT);

// 存储所有LED的状态
let leds = [];
let game;  // 只需要一个游戏实例

// 游戏状态
let gameState = {
    currentScreen: 'start',
    score: 0,
    highScore: 0,
    gameOver: false,
    explosions: [],  // 爆炸效果数组
    paused: false,  // 添加暂停标志
    isGameStarted: false,  // 游戏是否已开始
    countdown: 0,  // 开始倒计时
    lastCountdownUpdate: 0  // 上次倒计时更新时间
};

// 大心形图案定义 (7x7)
const BIG_HEART_PATTERN = [
    [0, 1, 0, 0, 0, 1, 0],
    [1, 1, 1, 0, 1, 1, 1],
    [1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1, 1],
    [0, 1, 1, 1, 1, 1, 0],
    [0, 0, 1, 1, 1, 0, 0],
    [0, 0, 0, 1, 0, 0, 0]
];

// 小心形图案定义 (5x5)
const SMALL_HEART_PATTERN = [
    [0, 1, 0, 1, 0],
    [1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1],
    [0, 1, 1, 1, 0],
    [0, 0, 1, 0, 0]
];

// 骷髅头图案定义 (8x8)
const SKULL_PATTERN = [
    [0, 1, 1, 1, 1, 1, 1, 0],
    [1, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 0, 1, 1, 0, 1, 1],
    [1, 1, 1, 1, 1, 1, 1, 1],
    [0, 1, 1, 1, 1, 1, 1, 0],
    [0, 0, 1, 0, 0, 1, 0, 0],
    [0, 1, 0, 1, 1, 0, 1, 0],
    [1, 0, 0, 0, 0, 0, 0, 1]
];

// 数字图案定义 (4x8)
const DIGIT_PATTERNS = [
    [ // 0
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ],
    [ // 1
        [0, 0, 1, 0],
        [0, 1, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 0],
        [0, 1, 1, 1]
    ],
    [ // 2
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [0, 0, 0, 1],
        [0, 0, 1, 0],
        [0, 1, 0, 0],
        [1, 0, 0, 0],
        [1, 0, 0, 0],
        [1, 1, 1, 1]
    ],
    [ // 3
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [0, 0, 0, 1],
        [0, 1, 1, 0],
        [0, 0, 0, 1],
        [0, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ],
    [ // 4
        [0, 0, 0, 1],
        [0, 0, 1, 1],
        [0, 1, 0, 1],
        [1, 0, 0, 1],
        [1, 1, 1, 1],
        [0, 0, 0, 1],
        [0, 0, 0, 1],
        [0, 0, 0, 1]
    ],
    [ // 5
        [1, 1, 1, 1],
        [1, 0, 0, 0],
        [1, 0, 0, 0],
        [1, 1, 1, 0],
        [0, 0, 0, 1],
        [0, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ],
    [ // 6
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 0],
        [1, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ],
    [ // 7
        [1, 1, 1, 1],
        [0, 0, 0, 1],
        [0, 0, 1, 0],
        [0, 0, 1, 0],
        [0, 1, 0, 0],
        [0, 1, 0, 0],
        [1, 0, 0, 0],
        [1, 0, 0, 0]
    ],
    [ // 8
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ],
    [ // 9
        [0, 1, 1, 0],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 1],
        [0, 0, 0, 1],
        [1, 0, 0, 1],
        [0, 1, 1, 0]
    ]
];

class Game {
    constructor() {
        this.reset();
    }

    reset() {
        // 初始化玩家
        this.player = {
            x: TOTAL_WIDTH / 2,
            y: TOTAL_HEIGHT - 2,
            lives: 2,  // 改为2条命
            bullets: [],
            lastShootTime: 0
        };
        
        // 初始化爆炸效果
        gameState.explosions = [];
        
        // 初始化其他游戏状态
        this.aliens = [];
        this.alienBullets = [];
        this.shields = [];
        this.alienDirection = 1;
        this.alienSpeed = currentDifficulty.alienSpeed;
        this.lastAlienMoveTime = 0;
        this.lastAlienShootTime = 0;
        this.moveTimer = 0;
        this.alienStepDown = false;
        this.score = 0;
        this.gameOver = false;
        
        // 初始化倒计时
        this.countdown = 60; // 60秒倒计时
        this.lastCountdownUpdate = Date.now();
        
        this.initAliens();
        this.initShields();
    }

    initAliens() {
        // 根据难度设置创建外星人
        for (let row = 0; row < currentDifficulty.alienRows; row++) {
            for (let col = 0; col < currentDifficulty.alienCols; col++) {
                this.aliens.push({
                    x: col * 3 + 4,
                    y: row * 2 + 2,
                    type: Math.floor(row / 2),
                    alive: true
                });
            }
        }
    }

    initShields() {
        // 创建5个掩体，每个掩体是3x2的矩形
        for (let i = 0; i < 5; i++) {
            let baseX = i * 6 + 3;
            // 上面一排掩体
            for (let x = 0; x < 3; x++) {
                this.shields.push({
                    x: baseX + x,
                    y: TOTAL_HEIGHT - 5,
                    health: 2
                });
            }
            // 下面一排掩体
            for (let x = 0; x < 3; x++) {
                this.shields.push({
                    x: baseX + x,
                    y: TOTAL_HEIGHT - 4,
                    health: 2
                });
            }
        }
    }

    update() {
        if (this.gameOver) return;

        // 更新倒计时
        const countdownCurrentTime = Date.now();
        if (countdownCurrentTime - this.lastCountdownUpdate >= 1000) { // 每秒更新一次
            this.countdown--;
            this.lastCountdownUpdate = countdownCurrentTime;
            
            // 倒计时结束，游戏结束
            if (this.countdown <= 0) {
                this.gameOver = true;
                return;
            }
        }

        // 更新外星人移动
        this.moveTimer++;
        if (this.moveTimer >= currentDifficulty.alienMoveInterval) {
            this.moveTimer = 0;
            this.moveAliens();
        }

        // 更新爆炸效果
        for (let i = gameState.explosions.length - 1; i >= 0; i--) {
            const explosion = gameState.explosions[i];
            
            // 更新闪烁状态 - 只闪烁一次
            if (explosion.flash === 0) {
                explosion.flash = 1;  // 第一帧显示，第二帧隐藏
            } else {
                // 闪烁完一次后移除爆炸效果
                gameState.explosions.splice(i, 1);
                continue;
            }
        }

        // 存储子弹的前一个位置用于碰撞检测
        for (let bullet of this.player.bullets) {
            bullet.prevY = bullet.y;
        }
        for (let bullet of this.alienBullets) {
            bullet.prevY = bullet.y;
        }

        // 更新玩家子弹
        for (let i = this.player.bullets.length - 1; i >= 0; i--) {
            let bullet = this.player.bullets[i];
            bullet.y--;

            // 检查是否与外星人子弹相撞 - 优化碰撞检测逻辑
            let bulletCollided = false;
            for (let j = this.alienBullets.length - 1; j >= 0; j--) {
                let alienBullet = this.alienBullets[j];
                
                // 检测多个点：
                // 1. 当前位置
                // 2. 上一个位置
                // 3. 中间位置
                if ((bullet.x === alienBullet.x && bullet.y === alienBullet.y) || 
                    (bullet.x === alienBullet.x && bullet.prevY === alienBullet.y) ||
                    (bullet.x === alienBullet.x && bullet.prevY === alienBullet.prevY) ||
                    (bullet.x === alienBullet.x && bullet.y === alienBullet.prevY) ||
                    (bullet.x === alienBullet.x && Math.abs(bullet.y - alienBullet.y) <= 1)) {
                    
                    // 子弹相撞，双方子弹都消失
                    this.player.bullets.splice(i, 1);
                    this.alienBullets.splice(j, 1);
                    bulletCollided = true;
                    
                    // 在碰撞位置添加子弹爆炸效果
                    let explosionY = Math.min(bullet.y, alienBullet.y);
                    gameState.explosions.push(createExplosion(bullet.x, explosionY, 'bullet'));
                    
                    // 播放爆炸声音
                    playSoundSafely(explosionSound);
                    
                    break;
                }
            }
            if (bulletCollided) continue;

            // 检查是否击中掩体 - 从下往上检查
            let hitShield = false;
            for (let shield of this.shields) {
                if (shield.health > 0 && bullet.x === shield.x && bullet.y === shield.y) {
                    // 如果是下面一排的掩体
                    if (shield.y === TOTAL_HEIGHT - 4) {
                        shield.health--;
                        hitShield = true;
                        if (shield.health > 0) {
                            this.player.bullets.splice(i, 1);
                            break;
                        }
                    }
                    // 如果是上面一排的掩体
                    else if (shield.y === TOTAL_HEIGHT - 5) {
                        // 检查下面一排的掩体是否已经被摧毁
                        let bottomShield = this.shields.find(s => 
                            s.x === shield.x && s.y === TOTAL_HEIGHT - 4 && s.health > 0);
                        if (bottomShield) {
                            // 如果下面一排的掩体还存在，子弹被阻挡
                            this.player.bullets.splice(i, 1);
                            break;
                        } else {
                            // 如果下面一排的掩体已经被摧毁，可以击中上面一排
                            shield.health--;
                            hitShield = true;
                            if (shield.health > 0) {
                                this.player.bullets.splice(i, 1);
                                break;
                            }
                        }
                    }
                }
            }

            if (hitShield) continue;

            // 检查是否击中外星人
            for (let alien of this.aliens) {
                if (alien.alive && bullet.x === alien.x && bullet.y === alien.y) {
                    // 检查这一列是否有掩体阻挡
                    let hasShield = this.shields.some(s => 
                        s.x === bullet.x && s.health > 0);
                    if (!hasShield) {
                        alien.alive = false;
                        this.player.bullets.splice(i, 1);
                        
                        // 根据外星人类型增加分数
                        switch(alien.type) {
                            case 0: // 第一排外星人
                                this.score += 1;
                                break;
                            case 1: // 第二排外星人
                                this.score += 2;
                                break;
                            case 2: // 第三排外星人
                                this.score += 4;
                                break;
                        }
                        
                        // 添加一个爆炸效果到gameState.explosions数组
                        gameState.explosions.push(createExplosion(alien.x, alien.y, 'alien'));
                        
                        // 播放爆炸声音
                        playSoundSafely(explosionSound);
                        
                        break;
                    }
                }
            }

            // 如果子弹超出屏幕顶部，移除它
            if (bullet.y < 0) {
                this.player.bullets.splice(i, 1);
            }
        }

        // 更新外星人子弹
        for (let i = this.alienBullets.length - 1; i >= 0; i--) {
            let bullet = this.alienBullets[i];
            bullet.y++;

            // 检查是否击中掩体 - 从上往下检查
            let hitShield = false;
            for (let shield of this.shields) {
                if (shield.health > 0 && bullet.x === shield.x && bullet.y === shield.y) {
                    // 如果是上面一排的掩体
                    if (shield.y === TOTAL_HEIGHT - 5) {
                        shield.health--;
                        hitShield = true;
                        if (shield.health > 0) {
                            this.alienBullets.splice(i, 1);
                            break;
                        }
                    }
                    // 如果是下面一排的掩体
                    else if (shield.y === TOTAL_HEIGHT - 4) {
                        // 检查上面一排的掩体是否已经被摧毁
                        let topShield = this.shields.find(s => 
                            s.x === shield.x && s.y === TOTAL_HEIGHT - 5 && s.health > 0);
                        if (topShield) {
                            // 如果上面一排的掩体还存在，子弹被阻挡
                            this.alienBullets.splice(i, 1);
                            break;
                        } else {
                            // 如果上面一排的掩体已经被摧毁，可以击中下面一排
                            shield.health--;
                            hitShield = true;
                            if (shield.health > 0) {
                                this.alienBullets.splice(i, 1);
                                break;
                            }
                        }
                    }
                }
            }

            if (hitShield) continue;

            // 检查是否击中玩家
            if (bullet.x === this.player.x && bullet.y === this.player.y) {
                // 检查这一列是否有掩体阻挡
                let hasShield = this.shields.some(s => 
                    s.x === bullet.x && s.health > 0);
                if (!hasShield) {
                    this.player.lives--;
                    this.alienBullets.splice(i, 1);
                    if (this.player.lives <= 0) {
                        this.gameOver = true;
                    }
                    break;
                }
            }

            // 如果子弹超出屏幕底部，移除它
            if (bullet.y >= TOTAL_HEIGHT) {
                this.alienBullets.splice(i, 1);
            }
        }

        // 外星人发射子弹
        const alienShootCurrentTime = Date.now();
        if (alienShootCurrentTime - this.lastAlienShootTime > currentDifficulty.alienShootInterval) {
            let aliveAliens = this.aliens.filter(a => a.alive);
            if (aliveAliens.length >= 4) {
                // 优先选择玩家所在列的外星人
                let playerColumnAliens = aliveAliens.filter(a => a.x === this.player.x);
                let shooter;
                
                if (playerColumnAliens.length > 0 && Math.random() < currentDifficulty.playerColumnShootChance) {
                    shooter = playerColumnAliens[Math.floor(Math.random() * playerColumnAliens.length)];
                } else {
                    // 选择其他列的外星人
                    let otherAliens = aliveAliens.filter(a => a.x !== this.player.x);
                    if (otherAliens.length > 0) {
                        shooter = otherAliens[Math.floor(Math.random() * otherAliens.length)];
                    } else {
                        shooter = aliveAliens[Math.floor(Math.random() * aliveAliens.length)];
                    }
                }
                
                this.alienBullets.push({
                    x: shooter.x,
                    y: shooter.y + 1
                });
                this.lastAlienShootTime = alienShootCurrentTime;
                
                // 播放外星人射击声音
                playSoundSafely(alienShootSound);
            }
        }

        // 检查是否胜利
        if (this.aliens.every(a => !a.alive)) {
            this.initAliens();
        }

        // 检查外星人是否到达底部
        if (this.aliens.some(a => a.alive && a.y >= TOTAL_HEIGHT - 3)) {
            this.gameOver = true;
        }
    }

    moveAliens() {
        let needsStepDown = false;
        
        // 检查是否需要改变方向
        for (let alien of this.aliens) {
            if (!alien.alive) continue;
            if (this.alienDirection > 0 && alien.x >= GAME_WIDTH - 2) {
                needsStepDown = true;
                break;
            }
            if (this.alienDirection < 0 && alien.x <= 1) {
                needsStepDown = true;
                break;
            }
        }

        if (needsStepDown) {
            this.alienDirection *= -1;
            // 所有外星人下移一行
            for (let alien of this.aliens) {
                if (alien.alive) {
                    alien.y++;
                }
            }
        } else {
            // 正常左右移动
            for (let alien of this.aliens) {
                if (alien.alive) {
                    alien.x += this.alienDirection;
                }
            }
        }
    }

    draw() {
        // 清空屏幕
        for (let row = 0; row < MATRIX_ROWS; row++) {
            for (let col = 0; col < MATRIX_COLS; col++) {
                for (let y = 0; y < LED_SIZE; y++) {
                    for (let x = 0; x < LED_SIZE; x++) {
                        leds[row][col][y][x] = color(30, 30, 30);
                    }
                }
            }
        }

        const setLED = (x, y, color) => {
            // 只处理游戏区域内的坐标
            if (x >= 0 && x < GAME_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
                const matrixRow = Math.floor(y / LED_SIZE);
                const matrixCol = Math.floor(x / LED_SIZE);
                const localY = y % LED_SIZE;
                const localX = x % LED_SIZE;
                if (matrixRow >= 0 && matrixRow < MATRIX_ROWS && 
                    matrixCol >= 0 && matrixCol < GAME_COLS) {
                    leds[matrixRow][matrixCol][localY][localX] = color;
                }
            }
        };

        // 绘制分数（显示在第4个屏幕，即第一行最后一列）
        const scoreStr = String(this.score).padStart(2, '0');
        const tensDigit = parseInt(scoreStr[0]);
        const onesDigit = parseInt(scoreStr[1]);
        
        // 绘制十位数（蓝色）
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 4; x++) {
                if (DIGIT_PATTERNS[tensDigit][y][x]) {
                    leds[0][3][y][x] = color(0, 0, 255); // 蓝色
                }
            }
        }
        
        // 绘制个位数（绿色）
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 4; x++) {
                if (DIGIT_PATTERNS[onesDigit][y][x]) {
                    leds[0][3][y][x + 4] = color(0, 255, 0); // 绿色
                }
            }
        }

        // 绘制倒计时（显示在第12个屏幕，即第3行第4列）
        const countdownStr = String(this.countdown).padStart(2, '0');
        const countdownTensDigit = parseInt(countdownStr[0]);
        const countdownOnesDigit = parseInt(countdownStr[1]);
        
        // 绘制十位数（红色）
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 4; x++) {
                if (DIGIT_PATTERNS[countdownTensDigit][y][x]) {
                    leds[2][3][y][x] = color(255, 0, 0); // 红色
                }
            }
        }
        
        // 绘制个位数（黄色）
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 4; x++) {
                if (DIGIT_PATTERNS[countdownOnesDigit][y][x]) {
                    leds[2][3][y][x + 4] = color(255, 255, 0); // 黄色
                }
            }
        }

        // 绘制生命值(在第8个屏幕,即第2行第4列)
        const lifeRow = 1; // 第2行
        const lifeCol = 3; // 第4列
        
        // 根据生命值状态选择图案和颜色
        if (this.player.lives > 0) {
            // 满血：红色大爱心
            if (this.player.lives === 2) {
                drawPattern(lifeRow, lifeCol, BIG_HEART_PATTERN, color(255, 0, 0), 1, 0);
            }
            // 残血：黄色小心
            else if (this.player.lives === 1) {
                drawPattern(lifeRow, lifeCol, SMALL_HEART_PATTERN, color(255, 255, 0), 1, 1);
            }
        } else {
            // 死亡：显示骷髅头
            drawPattern(lifeRow, lifeCol, SKULL_PATTERN, color(255, 255, 255));
        }

        // 绘制玩家
        setLED(this.player.x, this.player.y, color(...PLAYER_COLOR));

        // 绘制玩家子弹
        for (let bullet of this.player.bullets) {
            setLED(bullet.x, bullet.y, color(...PLAYER_BULLET_COLOR));
        }

        // 绘制外星人
        for (let alien of this.aliens) {
            if (alien.alive) {
                setLED(alien.x, alien.y, color(...ALIEN_COLORS[alien.type]));
            }
        }

        // 绘制外星人子弹
        for (let bullet of this.alienBullets) {
            setLED(bullet.x, bullet.y, color(...ALIEN_BULLET_COLOR));
        }

        // 绘制掩体
        for (let shield of this.shields) {
            if (shield.health > 0) {
                let shieldColor = [...SHIELD_COLOR];
                shieldColor[1] = shieldColor[1] * (shield.health / 3);  // 根据血量改变颜色
                setLED(shield.x, shield.y, color(...shieldColor));
            }
        }

        // 绘制爆炸效果
        for (const explosion of gameState.explosions) {
            // 只有flash为0时才显示
            if (explosion.flash !== 0) continue;
            
            // 根据爆炸类型决定显示范围
            if (explosion.type === 'alien') {
                // 敌机爆炸显示3x3
                for (let dx = -1; dx <= 1; dx++) {
                    for (let dy = -1; dy <= 1; dy++) {
                        const x = explosion.x + dx;
                        const y = explosion.y + dy;
                        if (x >= 0 && x < TOTAL_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
                            setLED(x, y, color(255, 255, 255)); // 纯白色
                        }
                    }
                }
            } else {
                // 子弹爆炸只显示1x1
                const x = explosion.x;
                const y = explosion.y;
                if (x >= 0 && x < TOTAL_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
                    setLED(x, y, color(255, 255, 255)); // 纯白色
                }
            }
        }

        // 如果游戏结束，显示"GAME OVER"
        if (this.gameOver) {
            // 简单显示一个X表示游戏结束
            setLED(TOTAL_WIDTH / 2 - 1, TOTAL_HEIGHT / 2, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2 + 1, TOTAL_HEIGHT / 2, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2, TOTAL_HEIGHT / 2 - 1, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2, TOTAL_HEIGHT / 2 + 1, color(255, 0, 0));
        }
        
        // 如果游戏暂停，显示暂停标志
        if (gameState.paused) {
            // 在屏幕中央显示两条竖线表示暂停
            const centerX = Math.floor(TOTAL_WIDTH / 2);
            const centerY = Math.floor(TOTAL_HEIGHT / 2);
            
            // 左侧暂停线
            setLED(centerX - 2, centerY - 2, color(255, 255, 255));
            setLED(centerX - 2, centerY - 1, color(255, 255, 255));
            setLED(centerX - 2, centerY, color(255, 255, 255));
            setLED(centerX - 2, centerY + 1, color(255, 255, 255));
            setLED(centerX - 2, centerY + 2, color(255, 255, 255));
            
            // 右侧暂停线
            setLED(centerX + 2, centerY - 2, color(255, 255, 255));
            setLED(centerX + 2, centerY - 1, color(255, 255, 255));
            setLED(centerX + 2, centerY, color(255, 255, 255));
            setLED(centerX + 2, centerY + 1, color(255, 255, 255));
            setLED(centerX + 2, centerY + 2, color(255, 255, 255));
        }
    }

    movePlayer(direction) {
        if (this.gameOver) return;
        
        if (direction === 'left' && this.player.x > 0) {
            this.player.x--;
        } else if (direction === 'right' && this.player.x < TOTAL_WIDTH - 1) {
            this.player.x++;
        }
    }

    shoot() {
        if (this.gameOver) return;

        // 检查发射冷却时间
        const playerShootCurrentTime = Date.now();
        if (playerShootCurrentTime - this.player.lastShootTime < 500) {  // 500ms冷却时间
            return;
        }

        this.player.bullets.push({
            x: this.player.x,
            y: this.player.y - 1
        });
        this.player.lastShootTime = playerShootCurrentTime;
        
        // 播放玩家射击声音
        playSoundSafely(playerShootSound);
    }
}

// 预加载资源
function preload() {
    // 检查p5.sound库是否可用
    if (typeof loadSound === 'function') {
        console.log('p5.sound库可用，开始加载声音');
        // 加载声音文件
        soundFormats('mp3', 'wav');
        try {
            // 获取音频上下文但不立即激活
            const audioContext = getAudioContext();
            console.log('音频上下文状态:', audioContext.state);
            
            playerShootSound = loadSound('sounds/player_shoot.wav', 
                () => console.log('玩家射击音效加载成功'),
                (err) => console.error('加载玩家射击音效失败', err));
            
            alienShootSound = loadSound('sounds/alien_shoot.wav',
                () => console.log('外星人射击音效加载成功'),
                (err) => console.error('加载外星人射击音效失败', err));
            
            explosionSound = loadSound('sounds/explosion.wav',
                () => console.log('爆炸音效加载成功'),
                (err) => console.error('加载爆炸音效失败', err));
        } catch (e) {
            console.error('加载音效文件时出错', e);
        }
    } else {
        console.warn('p5.sound库不可用，游戏将没有声音效果');
    }
}

function setup() {
    createCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
    
    // 初始化LED状态
    for (let row = 0; row < MATRIX_ROWS; row++) {
        leds[row] = [];
        for (let col = 0; col < MATRIX_COLS; col++) {
            leds[row][col] = [];
            for (let y = 0; y < LED_SIZE; y++) {
                leds[row][col][y] = [];
                for (let x = 0; x < LED_SIZE; x++) {
                    leds[row][col][y][x] = color(30, 30, 30);
                }
            }
        }
    }
    
    // 创建一个游戏实例
    game = new Game();
    
    // 初始化对话框控制
    initDialogControl();
    
    // 游戏主循环 - 提高更新频率
    setInterval(updateGame, 100);  // 改为每100ms更新一次，提高碰撞检测精度
}

// 初始化对话框控制
function initDialogControl() {
    console.log('初始化对话框控制...');
    
    const configButton = document.getElementById('configButton');
    const configDialog = document.getElementById('configDialog');
    const overlay = document.getElementById('overlay');
    const closeConfig = document.getElementById('closeConfig');
    const testButton = document.getElementById('testScreens');
    const screenButtons = document.querySelectorAll('.screen-button');
    const pauseGameButton = document.getElementById('pauseGame');
    const restartGameButton = document.getElementById('restartGame');
    
    console.log('暂停游戏按钮:', pauseGameButton);
    console.log('重启游戏按钮:', restartGameButton);
    
    // 暂停游戏按钮点击事件
    if (pauseGameButton) {
        pauseGameButton.addEventListener('click', () => {
            console.log('暂停游戏按钮被点击');
            if (gameState.isGameStarted) {
                gameState.paused = !gameState.paused;
                pauseGameButton.textContent = gameState.paused ? '继续游戏' : '暂停游戏';
            }
        });
    } else {
        console.error('找不到暂停游戏按钮');
    }
    
    // 重启游戏按钮点击事件
    if (restartGameButton) {
        restartGameButton.addEventListener('click', () => {
            console.log('重启游戏按钮被点击');
            if (gameState.isGameStarted) {
                // 如果游戏已经开始，点击后重新开始
                game.reset();
                gameState.paused = false;
                gameState.isGameStarted = false;
                restartGameButton.textContent = '开始';
                pauseGameButton.textContent = '暂停游戏';
            } else {
                // 如果游戏还没开始，点击后开始倒计时
                startCountdown();
            }
        });
    } else {
        console.error('找不到重启游戏按钮');
    }
    
    // 打开对话框
    if (configButton) {
        configButton.addEventListener('click', () => {
            configDialog.style.display = 'block';
            overlay.style.display = 'block';
            gameState.paused = true; // 打开对话框时暂停游戏
        });
    }
    
    // 关闭对话框
    if (closeConfig) {
        closeConfig.addEventListener('click', () => {
            configDialog.style.display = 'none';
            overlay.style.display = 'none';
            gameState.paused = false; // 关闭对话框时恢复游戏
        });
    }
    
    // 点击遮罩层关闭对话框
    if (overlay) {
        overlay.addEventListener('click', () => {
            configDialog.style.display = 'none';
            overlay.style.display = 'none';
            gameState.paused = false; // 关闭对话框时恢复游戏
        });
    }
    
    // 测试屏幕按钮
    if (testButton) {
        testButton.addEventListener('click', () => {
            testAllScreens();
        });
    }
    
    // 屏幕连接按钮
    screenButtons.forEach(button => {
        button.addEventListener('click', async () => {
            const screenIndex = parseInt(button.getAttribute('data-index'));
            if (connectedScreens[screenIndex]) {
                // 如果已连接，断开连接
                await disconnectSerial(screenIndex);
            } else {
                // 如果未连接，建立连接
                await connectSerial(screenIndex);
            }
        });
    });
    
    console.log('对话框控制初始化完成');
}

// 将颜色转换为对应的数字
function colorToNumber(r, g, b) {
    // 如果所有颜色分量都为0，返回黑色
    if (r === 0 && g === 0 && b === 0) return COLOR_MAP.black;
    
    // 如果所有颜色分量都接近最大值，返回白色
    if (r > 200 && g > 200 && b > 200) return COLOR_MAP.white;
    
    // 根据RGB值判断颜色
    if (r > g && r > b) {
        if (r < 100) return COLOR_MAP.darkRed;
        if (g > 100 && b > 100) return COLOR_MAP.pink;
        return COLOR_MAP.red;
    }
    if (g > r && g > b) {
        if (g < 100) return COLOR_MAP.darkGreen;
        if (r > 100 && b > 100) return COLOR_MAP.lightCyan;
        return COLOR_MAP.green;
    }
    if (b > r && b > g) {
        if (b < 100) return COLOR_MAP.darkBlue;
        if (r > 100 && g > 100) return COLOR_MAP.cyan;
        return COLOR_MAP.blue;
    }
    if (r > 100 && g > 100 && b < 100) return COLOR_MAP.yellow;
    if (r > 100 && b > 100 && g < 100) return COLOR_MAP.purple;
    if (g > 100 && b > 100 && r < 100) return COLOR_MAP.cyan;
    if (r > 50 && g > 50 && b > 50) return COLOR_MAP.gray;
    if (r > 100 && g > 50 && b > 50) return COLOR_MAP.brown;
    if (r > 150 && g > 100 && b > 100) return COLOR_MAP.lightPink;
    
    return COLOR_MAP.black;  // 默认返回黑色
}

// 修改sendLEDData函数
function sendLEDData() {
    // 为每个已连接的屏幕发送对应的LED数据
    for (let i = 0; i < 12; i++) {
        if (!connectedScreens[i] || !writers[i]) continue;
        
        // 计算这个屏幕对应的矩阵索引
        const row = Math.floor(i / MATRIX_COLS); 
        const col = i % MATRIX_COLS;
        
        // 如果索引超出范围，跳过
        if (row >= MATRIX_ROWS || col >= MATRIX_COLS) continue;
        
        let data = [];
        for (let y = 0; y < LED_SIZE; y++) {
            for (let x = 0; x < LED_SIZE; x++) {
                let c = leds[row][col][y][x];
                let colorNum = colorToNumber(red(c), green(c), blue(c));
                data.push(colorNum);
            }
        }
        let dataString = data.join(',') + '\n';
        sendData(dataString, i);
    }
}

// 添加开始倒计时函数
function startCountdown() {
    gameState.countdown = 3;  // 3秒倒计时
    gameState.lastCountdownUpdate = Date.now();
    gameState.isGameStarted = true;
    document.getElementById('restartGame').textContent = '重新开始';
}

// 修改updateGame函数
function updateGame() {
    // 处理开始倒计时
    if (gameState.isGameStarted && gameState.countdown > 0) {
        const currentTime = Date.now();
        if (currentTime - gameState.lastCountdownUpdate >= 1000) {  // 每秒更新一次
            gameState.countdown--;
            gameState.lastCountdownUpdate = currentTime;
            
            // 倒计时结束，开始游戏
            if (gameState.countdown <= 0) {
                game.reset();
                gameState.paused = false;
            }
        }
        return;  // 倒计时期间不更新游戏
    }
    
    if (!gameState.paused && gameState.isGameStarted) {
        game.update();
    }
    game.draw();
    
    // 只有在配置对话框关闭时才发送LED数据
    const configDialog = document.getElementById('configDialog');
    if (configDialog.style.display !== 'block') {
        sendLEDData();
    }
}

function keyPressed() {
    // P键切换暂停状态
    if (keyCode === 80) { // P键的keyCode是80
        gameState.paused = !gameState.paused;
        document.getElementById('pauseGame').textContent = gameState.paused ? '继续游戏' : '暂停游戏';
        return false;
    }
    
    if (gameState.paused) return false; // 游戏暂停时禁用其他控制
    
    if (keyCode === LEFT_ARROW) {
        game.movePlayer('left');
        return false;  // 阻止默认行为
    } else if (keyCode === RIGHT_ARROW) {
        game.movePlayer('right');
        return false;  // 阻止默认行为
    } else if (keyCode === 32) {  // 空格键
        game.shoot();
        return false;  // 阻止默认行为
    }
    return false;  // 阻止所有按键的默认行为，防止页面滚动
}

function draw() {
    background(0);
    
    // 绘制所有矩阵
    for (let row = 0; row < MATRIX_ROWS; row++) {
        for (let col = 0; col < MATRIX_COLS; col++) {
            drawMatrix(row, col);
        }
    }
    
    // 如果正在倒计时，在3x3的LED矩阵上显示倒计时数字
    if (gameState.isGameStarted && gameState.countdown > 0) {
        // 清空3x3游戏区域的LED
        for (let row = 0; row < MATRIX_ROWS; row++) {
            for (let col = 0; col < GAME_COLS; col++) {
                for (let y = 0; y < LED_SIZE; y++) {
                    for (let x = 0; x < LED_SIZE; x++) {
                        leds[row][col][y][x] = color(0, 0, 0);
                    }
                }
            }
        }
        
        // 绘制大数字（使用24x24的空间）
        const digit = gameState.countdown;
        const digitPattern = DIGIT_PATTERNS[digit];
        
        // 计算数字在24x24空间中的位置
        const startX = 6;  // 水平居中
        const startY = 0;  // 调整到最上方
        
        // 放大数字（3倍）
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 4; x++) {
                if (digitPattern[y][x]) {
                    // 绘制3x3的像素块
                    for (let dy = 0; dy < 3; dy++) {
                        for (let dx = 0; dx < 3; dx++) {
                            const pixelX = startX + x * 3 + dx;
                            const pixelY = startY + y * 3 + dy;
                            
                            // 计算对应的矩阵和局部坐标
                            const matrixRow = Math.floor(pixelY / LED_SIZE);
                            const matrixCol = Math.floor(pixelX / LED_SIZE);
                            const localY = pixelY % LED_SIZE;
                            const localX = pixelX % LED_SIZE;
                            
                            if (matrixRow >= 0 && matrixRow < MATRIX_ROWS && 
                                matrixCol >= 0 && matrixCol < GAME_COLS) {
                                leds[matrixRow][matrixCol][localY][localX] = color(255, 255, 255); // 白色
                            }
                        }
                    }
                }
            }
        }
    }
}

function drawMatrix(row, col) {
    // 计算矩阵的起始位置
    const startX = col * DEVICE_PIXEL_SIZE;
    const startY = row * (DEVICE_PIXEL_SIZE + BUTTON_PIXEL_HEIGHT);
    
    // 绘制屏幕区域（深灰色）
    fill(50);  // 深灰色屏幕
    noStroke();
    rect(startX + OUTER_BORDER_PIXELS, 
         startY + BUTTON_PIXEL_HEIGHT + OUTER_BORDER_PIXELS,
         SCREEN_PIXEL_SIZE,
         SCREEN_PIXEL_SIZE);
    
    // 绘制外边框（橙红色）- 只画四条边，不包含按钮区域
    fill(255, 69, 58);  // 橙红色外边框
    // 上边框（在按钮下方）
    rect(startX, startY + BUTTON_PIXEL_HEIGHT, DEVICE_PIXEL_SIZE, OUTER_BORDER_PIXELS);
    // 下边框
    rect(startX, startY + BUTTON_PIXEL_HEIGHT + DEVICE_PIXEL_SIZE - OUTER_BORDER_PIXELS, 
         DEVICE_PIXEL_SIZE, OUTER_BORDER_PIXELS);
    // 左边框
    rect(startX, startY + BUTTON_PIXEL_HEIGHT, 
         OUTER_BORDER_PIXELS, DEVICE_PIXEL_SIZE - OUTER_BORDER_PIXELS);
    // 右边框
    rect(startX + DEVICE_PIXEL_SIZE - OUTER_BORDER_PIXELS, startY + BUTTON_PIXEL_HEIGHT,
         OUTER_BORDER_PIXELS, DEVICE_PIXEL_SIZE - OUTER_BORDER_PIXELS);
    
    // 绘制按钮（橙色）- 在外边框之外
    fill(255, 165, 0);
    rect(startX + (DEVICE_PIXEL_SIZE - BUTTON_PIXEL_WIDTH) / 2, 
         startY,
         BUTTON_PIXEL_WIDTH, 
         BUTTON_PIXEL_HEIGHT);
    
    // 绘制LED矩阵区域
    const matrixStartX = startX + OUTER_BORDER_PIXELS + SCREEN_BORDER_PIXELS;
    const matrixStartY = startY + BUTTON_PIXEL_HEIGHT + OUTER_BORDER_PIXELS + SCREEN_BORDER_PIXELS;
    
    // 绘制LED
    for (let y = 0; y < LED_SIZE; y++) {
        for (let x = 0; x < LED_SIZE; x++) {
            fill(leds[row][col][y][x]);
            rect(matrixStartX + x * LED_PIXEL_SIZE, 
                 matrixStartY + y * LED_PIXEL_SIZE, 
                 LED_PIXEL_SIZE, 
                 LED_PIXEL_SIZE);
        }
    }
}

// 创建爆炸效果
function createExplosion(x, y, type = 'alien') {
    // 返回一个爆炸对象，包含位置、类型和生命周期
    return {
        x: x,
        y: y,
        type: type,  // 'alien'表示击中敌机，'bullet'表示击中子弹
        life: 1.0,   // 生命周期，会随时间减少
        flash: 0     // 闪烁阶段：0显示，1不显示
    };
}

// 绘制爆炸效果
function drawExplosion() {
    for (const explosion of gameState.explosions) {
        // 只有flash为0时才显示
        if (explosion.flash !== 0) continue;
        
        // 根据爆炸类型决定显示范围
        if (explosion.type === 'alien') {
            // 敌机爆炸显示3x3
            for (let dx = -1; dx <= 1; dx++) {
                for (let dy = -1; dy <= 1; dy++) {
                    const x = explosion.x + dx;
                    const y = explosion.y + dy;
                    if (x >= 0 && x < TOTAL_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
                        const matrixRow = Math.floor(y / LED_SIZE);
                        const matrixCol = Math.floor(x / LED_SIZE);
                        const localY = y % LED_SIZE;
                        const localX = x % LED_SIZE;
                        if (matrixRow >= 0 && matrixRow < MATRIX_ROWS && 
                            matrixCol >= 0 && matrixCol < MATRIX_COLS) {
                            leds[matrixRow][matrixCol][localY][localX] = color(255, 255, 255); // 纯白色
                        }
                    }
                }
            }
        } else {
            // 子弹爆炸只显示1x1
            const x = explosion.x;
            const y = explosion.y;
            if (x >= 0 && x < TOTAL_WIDTH && y >= 0 && y < TOTAL_HEIGHT) {
                const matrixRow = Math.floor(y / LED_SIZE);
                const matrixCol = Math.floor(x / LED_SIZE);
                const localY = y % LED_SIZE;
                const localX = x % LED_SIZE;
                if (matrixRow >= 0 && matrixRow < MATRIX_ROWS && 
                    matrixCol >= 0 && matrixCol < MATRIX_COLS) {
                    leds[matrixRow][matrixCol][localY][localX] = color(255, 255, 255); // 纯白色
                }
            }
        }
    }
}

// 安全播放声音（带错误处理）
function playSoundSafely(sound) {
    if (!sound) {
        console.warn("声音对象不存在");
        return;
    }
    
    if (typeof sound.isLoaded !== 'function') {
        console.warn("声音对象没有isLoaded方法");
        return;
    }
    
    if (!sound.isLoaded()) {
        console.warn("声音未加载完成");
        return;
    }
    
    if (typeof sound.play !== 'function') {
        console.warn("声音对象没有play方法");
        return;
    }

    // 检查音频上下文状态
    const audioContext = getAudioContext();
    if (audioContext.state === 'suspended') {
        console.warn("音频上下文被暂停");
        
        // 尝试恢复音频上下文
        audioContext.resume().then(() => {
            console.log("音频上下文恢复成功");
            try {
                sound.play();
            } catch (e) {
                console.error("播放声音时出错:", e);
            }
        }).catch(err => {
            console.error("恢复音频上下文失败:", err);
        });
        
        return;
    }

    try {
        console.log("播放声音");
        sound.play();
    } catch (e) {
        console.error("播放声音时出错:", e);
    }
}

// 绘制图案
function drawPattern(row, col, pattern, color, offsetX = 0, offsetY = 0, scaleX = 1, scaleY = 1) {
    const patternHeight = pattern.length;
    const patternWidth = pattern[0].length;
    
    for (let y = 0; y < patternHeight; y++) {
        for (let x = 0; x < patternWidth; x++) {
            if (pattern[y][x]) {
                const drawX = x * scaleX + offsetX;
                const drawY = y * scaleY + offsetY;
                if (drawX >= 0 && drawX < 8 && drawY >= 0 && drawY < 8) {
                    leds[row][col][drawY][drawX] = color;
                }
            }
        }
    }
} 