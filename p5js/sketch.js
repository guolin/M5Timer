// 常量定义
const MATRIX_COLS = 4;  // 矩阵列数
const MATRIX_ROWS = 3;  // 矩阵行数
const LED_SIZE = 8;     // 每个矩阵的LED数量（8x8）
const TOTAL_WIDTH = MATRIX_COLS * LED_SIZE;  // 总宽度 32
const TOTAL_HEIGHT = MATRIX_ROWS * LED_SIZE; // 总高度 24
const LED_UNIT = 1.0;   // 单个LED的尺寸（厘米）
const SCREEN_BORDER = 0.8;  // 屏幕边框宽度（厘米）
const OUTER_BORDER = 0.2;   // 外边框宽度（厘米）
const BUTTON_WIDTH = 4.8;   // 按钮宽度（厘米）
const BUTTON_HEIGHT = 1.0;  // 按钮高度（厘米）

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
    explosionParticles: []  // 爆炸粒子
};

class Game {
    constructor() {
        this.reset();
    }

    reset() {
        // 初始化玩家
        this.player = {
            x: TOTAL_WIDTH / 2,
            y: TOTAL_HEIGHT - 2,
            lives: 3,
            bullets: [],
            lastShootTime: 0
        };
        
        // 创建爆炸效果
        gameState.explosionParticles = createExplosion(this.player.x, this.player.y);
        
        // 初始化其他游戏状态
        this.aliens = [];
        this.alienBullets = [];
        this.shields = [];
        this.alienDirection = 1;
        this.alienSpeed = 1;
        this.lastAlienMoveTime = 0;
        this.lastAlienShootTime = 0;
        this.moveTimer = 0;  // 添加移动计时器
        this.alienStepDown = false;  // 添加下移标志
        this.score = 0;  // 添加分数
        this.gameOver = false;  // 添加游戏结束标志
        this.initAliens();
        this.initShields();
    }

    initAliens() {
        // 创建5行8列的外星人
        for (let row = 0; row < 5; row++) {
            for (let col = 0; col < 8; col++) {
                this.aliens.push({
                    x: col * 3 + 4,  // 间隔排列
                    y: row * 2 + 2,
                    type: Math.floor(row / 2),  // 根据行数决定外星人类型
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

        // 更新外星人移动
        this.moveTimer++;
        if (this.moveTimer >= 5) {
            this.moveTimer = 0;
            this.moveAliens();
        }

        // 更新玩家子弹
        for (let i = this.player.bullets.length - 1; i >= 0; i--) {
            let bullet = this.player.bullets[i];
            bullet.y--;

            // 检查是否与外星人子弹相撞
            let bulletCollided = false;
            for (let j = this.alienBullets.length - 1; j >= 0; j--) {
                let alienBullet = this.alienBullets[j];
                if (bullet.x === alienBullet.x && bullet.y === alienBullet.y) {
                    // 子弹相撞，双方子弹都消失
                    this.player.bullets.splice(i, 1);
                    this.alienBullets.splice(j, 1);
                    bulletCollided = true;
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
                        this.score += (3 - alien.type) * 10;
                        // 创建爆炸效果
                        gameState.explosionParticles = createExplosion(alien.x, alien.y);
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
        const currentTime = Date.now();
        if (currentTime - this.lastAlienShootTime > 1000) {
            let aliveAliens = this.aliens.filter(a => a.alive);
            if (aliveAliens.length >= 4) {
                // 优先选择玩家所在列的外星人
                let playerColumnAliens = aliveAliens.filter(a => a.x === this.player.x);
                let shooter;
                
                if (playerColumnAliens.length > 0 && Math.random() < 0.7) {  // 70%概率选择玩家所在列
                    shooter = playerColumnAliens[Math.floor(Math.random() * playerColumnAliens.length)];
                } else {
                    // 30%概率选择其他列的外星人
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
                this.lastAlienShootTime = currentTime;
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
            if (this.alienDirection > 0 && alien.x >= TOTAL_WIDTH - 2) {
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
            const matrixRow = Math.floor(y / LED_SIZE);
            const matrixCol = Math.floor(x / LED_SIZE);
            const localY = y % LED_SIZE;
            const localX = x % LED_SIZE;
            if (matrixRow >= 0 && matrixRow < MATRIX_ROWS && 
                matrixCol >= 0 && matrixCol < MATRIX_COLS) {
                leds[matrixRow][matrixCol][localY][localX] = color;
            }
        };

        // 绘制分数（显示在最上方）
        let scoreStr = String(this.score).padStart(4, '0');
        for (let i = 0; i < scoreStr.length; i++) {
            setLED(i, 0, color(...SCORE_COLOR));
        }

        // 绘制生命值（显示在右上角）
        for (let i = 0; i < this.player.lives; i++) {
            setLED(TOTAL_WIDTH - 1 - i, 0, color(...PLAYER_COLOR));
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

        // 如果游戏结束，显示"GAME OVER"
        if (this.gameOver) {
            // 简单显示一个X表示游戏结束
            setLED(TOTAL_WIDTH / 2 - 1, TOTAL_HEIGHT / 2, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2 + 1, TOTAL_HEIGHT / 2, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2, TOTAL_HEIGHT / 2 - 1, color(255, 0, 0));
            setLED(TOTAL_WIDTH / 2, TOTAL_HEIGHT / 2 + 1, color(255, 0, 0));
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
        const currentTime = Date.now();
        if (currentTime - this.player.lastShootTime < 500) {  // 500ms冷却时间
            return;
        }

        this.player.bullets.push({
            x: this.player.x,
            y: this.player.y - 1
        });
        this.player.lastShootTime = currentTime;
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
    
    // 游戏主循环 - 提高更新频率
    setInterval(updateGame, 100);  // 改为每100ms更新一次，原来是200ms
}

function updateGame() {
    game.update();
    game.draw();
}

function keyPressed() {
    if (keyCode === LEFT_ARROW) {
        game.movePlayer('left');
    } else if (keyCode === RIGHT_ARROW) {
        game.movePlayer('right');
    } else if (keyCode === 32) {  // 空格键
        game.shoot();
    }
}

function draw() {
    background(0);
    
    // 更新和绘制爆炸效果
    updateExplosion();
    drawExplosion();
    
    // 绘制所有矩阵
    for (let row = 0; row < MATRIX_ROWS; row++) {
        for (let col = 0; col < MATRIX_COLS; col++) {
            drawMatrix(row, col);
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

// 创建爆炸粒子
function createExplosion(x, y) {
    const particles = [];
    const numParticles = 12;  // 粒子数量
    
    for (let i = 0; i < numParticles; i++) {
        const angle = (i / numParticles) * TWO_PI;
        const speed = random(1, 3);
        particles.push({
            x: x,
            y: y,
            vx: cos(angle) * speed,
            vy: sin(angle) * speed,
            size: random(2, 4),
            life: 1.0
        });
    }
    
    return particles;
}

// 更新爆炸粒子
function updateExplosion() {
    for (let i = gameState.explosionParticles.length - 1; i >= 0; i--) {
        const p = gameState.explosionParticles[i];
        p.x += p.vx;
        p.y += p.vy;
        p.life -= 0.05;
        
        if (p.life <= 0) {
            gameState.explosionParticles.splice(i, 1);
        }
    }
}

// 绘制爆炸粒子
function drawExplosion() {
    for (const p of gameState.explosionParticles) {
        const alpha = p.life * 255;
        fill(255, 255, 0, alpha);
        noStroke();
        circle(p.x, p.y, p.size);
    }
} 