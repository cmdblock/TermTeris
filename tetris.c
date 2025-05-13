#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>  // 添加布尔类型支持

// 游戏区域大小
#define WIDTH 10    // 修改为10列
#define HEIGHT 20   // 保持20行

// 方块形状定义
const int SHAPES[7][4][4] = {
    // I形
    {{0,1,0,0},
     {0,1,0,0},
     {0,1,0,0},
     {0,1,0,0}},
    // L形
    {{0,0,1,0},
     {1,1,1,0},
     {0,0,0,0},
     {0,0,0,0}},
    // J形
    {{1,0,0,0},
     {1,1,1,0},
     {0,0,0,0},
     {0,0,0,0}},
    // O形
    {{1,1,0,0},
     {1,1,0,0},
     {0,0,0,0},
     {0,0,0,0}},
    // S形
    {{0,1,1,0},
     {1,1,0,0},
     {0,0,0,0},
     {0,0,0,0}},
    // Z形
    {{1,1,0,0},
     {0,1,1,0},
     {0,0,0,0},
     {0,0,0,0}},
    // T形
    {{0,1,0,0},
     {1,1,1,0},
     {0,0,0,0},
     {0,0,0,0}}
};

// 游戏状态
int board[HEIGHT][WIDTH] = {0};
int currentShape[4][4] = {0};
int currentX = 0;
int currentY = 0;
int score = 0;
int gameOver = 0;

// 函数声明
void initGame();
void drawBoard();
void spawnBlock();
void moveBlock(int dx, int dy);
void rotateBlock();
int checkCollision(int x, int y);
void mergeBlock();
void clearLines();
void hideCursor();

// 隐藏控制台光标
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// 初始化游戏
void initGame() {
    // 设置控制台窗口和缓冲区大小
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = {0, 0, WIDTH * 2 + 10, HEIGHT + 3};  // 考虑中文字符宽度和额外信息
    COORD bufferSize = {WIDTH * 2 + 11, HEIGHT + 4};
    
    // 设置缓冲区大小
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    // 设置窗口大小
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = 0;
    srand(time(NULL));
    hideCursor();
}

// 在控制台绘制游戏界面
void drawBoard() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    static CHAR_INFO buffer[HEIGHT * (WIDTH * 2 + 1)];  // 创建缓冲区
    COORD bufferSize = {WIDTH * 2, HEIGHT};  // 设置缓冲区大小
    COORD bufferCoord = {0, 0};  // 缓冲区起始坐标
    SMALL_RECT writeRegion = {0, 2, WIDTH * 2 - 1, HEIGHT + 1};  // 写入区域
    
    // 准备缓冲区数据
    int index = 0;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int show = board[i][j];
            
            if (i >= currentY && i < currentY + 4 &&
                j >= currentX && j < currentX + 4) {
                if (currentShape[i - currentY][j - currentX] == 1) {
                    show = 1;
                }
            }
            
            // 设置方块字符（每个方块占用两个字符位置）
            wchar_t blockChar = show ? L'X' : L'.';  // 使用X表示实心方块，.表示空白
            WORD attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            
            // 写入方块的两个字符位置
            buffer[index].Char.UnicodeChar = blockChar;
            buffer[index].Attributes = attributes;
            index++;
            buffer[index].Char.UnicodeChar = L' ';  // 每个方块后面加一个空格，保持间距
            buffer[index].Attributes = attributes;
            index++;
        }
    }
    
    // 一次性写入整个游戏区域
    WriteConsoleOutputW(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);
    
    // 更新分数显示（使用单独的写入操作）
    COORD scorePos = {0, 0};
    SetConsoleCursorPosition(hConsole, scorePos);
    printf("Score: %d", score);
    fflush(stdout);  // 立即刷新输出缓冲区
}

// 生成新方块
void spawnBlock() {
    int type = rand() % 7;
    memcpy(currentShape, SHAPES[type], sizeof(currentShape));
    currentX = WIDTH / 2 - 2;
    currentY = HEIGHT - 1;  // 从顶部开始（Y=19）
    
    if (checkCollision(currentX, currentY)) {
        gameOver = 1;
    }
}

// 检查碰撞
int checkCollision(int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentShape[i][j]) {
                int newX = x + j;
                int newY = y - i;  // 反转Y轴方向
                
                if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT ||
                    (newY >= 0 && board[HEIGHT - 1 - newY][newX])) {  // 坐标系转换
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 移动方块
void moveBlock(int dx, int dy) {
    if (!checkCollision(currentX + dx, currentY - dy)) {  // 反转Y轴移动方向
        currentX += dx;
        currentY -= dy;  // 反转Y轴移动方向
        
        // 检查死亡判定线（Y ≥ 15）
        if (currentY <= HEIGHT - 15) {  // Y=15对应数组索引5
            gameOver = 1;
            return;
        }
    } else if (dy < 0) {  // 向下移动时碰撞
        mergeBlock();
        clearLines();
        spawnBlock();
    }
}

// 将方块合并到游戏板
void mergeBlock() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentShape[i][j]) {
                int boardY = HEIGHT - 1 - (currentY - i);  // 坐标系转换
                if (boardY >= 0 && boardY < HEIGHT) {
                    board[boardY][currentX + j] = currentShape[i][j];
                }
            }
        }
    }
}

// 清除已完成的行
void clearLines() {
    int linesCleared = 0;  // 记录一次性消除的行数
    int linesToClear[HEIGHT];  // 记录要消除的行
    
    // 先找出所有需要消除的行
    for (int i = HEIGHT - 1; i >= 0; i--) {
        int complete = 1;
        for (int j = 0; j < WIDTH; j++) {
            if (!board[i][j]) {
                complete = 0;
                break;
            }
        }
        
        if (complete) {
            linesToClear[linesCleared] = i;
            linesCleared++;
        }
    }
    
    // 根据消除的行数计算得分
    if (linesCleared > 0) {
        switch (linesCleared) {
            case 1:
                score += 100;  // 单行消除
                break;
            case 2:
                score += 300;  // 双行消除
                break;
            case 3:
                score += 500;  // 三行消除
                break;
            case 4:
                score += 800;  // 四行消除
                break;
        }
        
        // 执行消除操作
        for (int i = 0; i < linesCleared; i++) {
            int clearLine = linesToClear[i];
            for (int k = clearLine; k > 0; k--) {
                memcpy(board[k], board[k-1], sizeof(board[k]));
            }
            memset(board[0], 0, sizeof(board[0]));
        }
    }
}

// 旋转方块
void rotateBlock() {
    int temp[4][4];
    memcpy(temp, currentShape, sizeof(temp));
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            currentShape[i][j] = temp[3-j][i];
        }
    }
    
    if (checkCollision(currentX, currentY)) {
        memcpy(currentShape, temp, sizeof(currentShape));
    }
}

int main() {
    initGame();
    spawnBlock();
    
    DWORD lastRender = GetTickCount();
    DWORD lastDrop = GetTickCount();
    const DWORD RENDER_INTERVAL = 50;    // 降低帧率到20FPS
    const DWORD DROP_INTERVAL = 1000;    // 保持1秒下落一格
    
    while (!gameOver) {
        DWORD currentTime = GetTickCount();
        bool needRender = false;
        
        // 处理键盘输入
        if (_kbhit()) {
            char key = _getch();
            switch (key) {
                case 'a': moveBlock(-1, 0); needRender = true; break;
                case 'd': moveBlock(1, 0); needRender = true; break;
                case 's': moveBlock(0, 1); needRender = true; break;
                case 'w': rotateBlock(); needRender = true; break;
                case 'q': gameOver = 1; break;
            }
        }
        
        // 控制自动下落
        if (currentTime - lastDrop >= DROP_INTERVAL) {
            moveBlock(0, 1);
            lastDrop = currentTime;
            needRender = true;
        }
        
        // 只在需要时进行渲染
        if (needRender || currentTime - lastRender >= RENDER_INTERVAL) {
            drawBoard();
            lastRender = GetTickCount();  // 使用新的时间戳
        }
        
        Sleep(10);  // 增加休眠时间，减少CPU使用率
    }
    
    system("cls");
    printf("游戏结束！最终得分：%d\n", score);
    fflush(stdout);
    return 0;
}