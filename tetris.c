#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

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
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = 0;
    srand(time(NULL));
    hideCursor();
}

// 在控制台绘制游戏界面
// 定义缓冲区
char buffer[HEIGHT][WIDTH * 2 + 1];  // *2是因为中文字符占2字节，+1是为了存放换行符

void drawBoard() {
    // 先在缓冲区中绘制
    sprintf(buffer[0], "Score: %d\n", score);
    
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int show = board[i][j];
            
            if (i >= currentY && i < currentY + 4 &&
                j >= currentX && j < currentX + 4) {
                if (currentShape[i - currentY][j - currentX] == 1) {
                    show = 1;
                }
            }
            
            // 写入缓冲区
            buffer[i+1][j*2] = show ? "■"[0] : "□"[0];
            buffer[i+1][j*2+1] = show ? "■"[1] : "□"[1];
        }
        buffer[i+1][WIDTH*2] = '\n';
    }
    buffer[HEIGHT+1][0] = '\0';
    
    // 移动光标到开始位置
    COORD coord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    
    // 一次性输出整个缓冲区
    printf("%s", buffer);
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
    for (int i = HEIGHT - 1; i >= 0; i--) {
        int complete = 1;
        for (int j = 0; j < WIDTH; j++) {
            if (!board[i][j]) {
                complete = 0;
                break;
            }
        }
        
        if (complete) {
            score += 100;
            for (int k = i; k > 0; k--) {
                memcpy(board[k], board[k-1], sizeof(board[k]));
            }
            memset(board[0], 0, sizeof(board[0]));
            i++; // 重新检查当前行
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
    
    while (!gameOver) {
        drawBoard();
        
        if (_kbhit()) {
            char key = _getch();
            switch (key) {
                case 'a': moveBlock(-1, 0); break;  // 左移
                case 'd': moveBlock(1, 0); break;   // 右移
                case 's': moveBlock(0, 1); break;   // 加速下落
                case 'w': rotateBlock(); break;     // 旋转
                case 'q': gameOver = 1; break;      // 退出
            }
        }
        
        // 自动下落
        static clock_t lastDrop = 0;
        if (clock() - lastDrop > CLOCKS_PER_SEC) {
            moveBlock(0, 1);
            lastDrop = clock();
        }
        
        Sleep(50); // 控制游戏速度
    }
    
    system("cls");
    printf("游戏结束！最终得分：%d\n", score);
    return 0;
}