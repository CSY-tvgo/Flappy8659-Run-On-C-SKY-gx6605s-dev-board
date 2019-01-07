// File:     flappyBirdByCSY.h
// Author:   Karbon
// Github:   https://github.com/CSY-tvgo/Flappy8659-Run-On-C-SKY-gx6605s-dev-board
// Date:     2019.1.3
// Brief:    a simple flappybird-like game run on C-SKY gx6605s dev board

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "CSKY_GX6605S_GPIO.h"

/*
 0         1         2         3         4         5         6         7
 01234567890123456789012345678901234567890123456789012345678901234567890123456789
9┌───P1:00000───P2:00000───P3:00000───P4:00000──────────────────────────────────┐9	 
8│                              X       X          X           X             X  │8
7│                              X       X          X           X             X  │7
6│                              X       X          X           X             X  │6
5│                              X       X          X           X             X  │5
4│                              X       X          X           X                │4
3│       (10,h)  (18,h)         X       X          X┌──6~12───┐X                │3
2│     8   6   5   9            X       X                      X                │2
1│   (6,h)   (14,h)             X                               ┐               │1
0│                                                              │            X  │0
9│                                                 X           3~6           X  │9
8│                                      X          X            │            X  │8
7│                              X       X          X            │            X  │7
6│                              X       X          X            ┘            X  │6
5│                              X       X          X           X  ┐          X  │5
4│                              X       X          X           X  │          X  │4
3│                              X       X          X           X 4~10        X  │3
2│                              X       X          X           X  │          X  │2
1│                              X       X          X           X  ┘          X  │1
0└──────────────────────────────────────────────────────────────────────────────┘0
 01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/

char graph[30][200];           // 实际图像是20*80
volatile int pipeSpace[78];    // 水管中间的空隙
volatile int pipeHeight[78];   // 下边水管的高度
volatile int nextPipeInterval; // 下一个水管距现在的间隔
volatile int nextPipeSpace;    // 下一个水管中间的空隙
volatile int nextPipeHeight;   // 下一个水管的高度

pthread_t tids[4];
int LEDPIN[4] = {10, 11, 12, 13};

volatile int birdAlive[4];  // 4个鸟的存活情况
volatile int birdHeight[4]; // 4个鸟当前的高度（0~18）
volatile int birdScore[4];  // 4个鸟的分数

void showMainMenu();           // 展示主菜单
void mainGame();               // 主要的游戏
void gameInput();              // 处理游戏过程中的输入
void gameProcess();            // 游戏进程（与输入无关的）
void refreshGraph();           // 刷新画面
int randIntFrom(int a, int b); // 生成[a,b]之间的随机整数
void msleep(long ms);          // 休眠毫秒

void *blink3times(void *args); // 闪3次灯

int main(void)
{
    int LEDPIN[4] = {10, 11, 12, 13};
    int selectNum = 0;
    GPIO_SETMODE(10, GPIO_OUT);
    GPIO_SETMODE(11, GPIO_OUT);
    GPIO_SETMODE(12, GPIO_OUT);
    GPIO_SETMODE(13, GPIO_OUT);
    GPIO_SETMODE(8, GPIO_IN);
    GPIO_SETMODE(6, GPIO_IN);
    GPIO_SETMODE(5, GPIO_IN);
    GPIO_SETMODE(9, GPIO_IN);
    GPIO_SETMODE(7, GPIO_IN);
    while (1)
    {
        while (GPIO_READ(7) == 0) // wait for releasing the button 7
            ;
        showMainMenu();
        while (1)
        {
            if (GPIO_READ(7) == 0)
            {
                selectNum = -1; // exit
                break;
            }
            if (GPIO_READ(8) == 0 || GPIO_READ(6) == 0 || GPIO_READ(5) == 0 || GPIO_READ(9) == 0)
            {
                selectNum = 1; // play game
                break;
            }
        }
        if (selectNum == -1)
        {
            printf("\x0c\nbye bye~\n");
            break;
        }
        else if (selectNum == 1)
        {
            mainGame();
        }
    }
    return 0;
}

void showMainMenu()
{
    printf("\x0c\n");
    printf(
        "┌───P8:00000───P6:00000───P5:00000───P9:00000──────────────────────────────────┐\n"
        "│           _____ _                          ___   __  ____  ___               │\n"
        "│          |  ___| | __ _ _ __  _ __  _   _ ( _ ) / /_| ___|/ _ \\              │\n"
        "│          | |_  | |/ _` | '_ \\| '_ \\| | | |/ _ \\| '_ \\___ \\ (_) |             │\n"
        "│          |  _| | | (_| | |_) | |_) | |_| | (_) | (_) |__) \\__, |             │\n"
        "│          |_|   |_|\\__,_| .__/| .__/ \\__, |\\___/ \\___/____/  /_/              │\n"
        "│                        |_|   |_|    |___/                                    │\n"
        "│     8   6   5   9                                                            │\n"
        "│                                                                              │\n"
        "│                                                                              │\n"
        "│                                                                              │\n"
        "│                        Press '8''6''5''9' to Start                           │\n"
        "│                               '7' to exit                                    │\n"
        "│                                                                              │\n"
        "│                                                                              │\n"
        "│                                                                              │\n"
        "│                                                                              │\n"
        "│                           Run on CSKY-GX6605S                                │\n"
        "│                        (C)Karbon Game Inc. 9102                              │\n"
        "└──────────────────────────────────────────────────────────────────────────────┘\n");
}

void mainGame()
{
    int i;

    nextPipeInterval = randIntFrom(6, 12); // 距下一个水管的间隔
    nextPipeHeight = randIntFrom(4, 10);   // 上下两水管之间的空隙
    nextPipeSpace = randIntFrom(3, 6);     // 下边水管的高度

    srand((unsigned)time(NULL)); // 改变随机数种子

    for (i = 0; i < 4; i++) // 初始化鸟
    {
        birdAlive[i] = 1;
        birdHeight[i] = 11;
        birdScore[i] = 0;
    }
    for (i = 0; i < 78; i++) // 初始化水管
    {
        pipeHeight[i] = 0;
        pipeSpace[i] = 18;
    }

    while (birdAlive[0] || birdAlive[1] || birdAlive[2] || birdAlive[3])
    {
        gameInput();
        gameProcess();
        refreshGraph();
        msleep(200); //sleeps
    }
    printf("\x0c\n\n\n\n\n\n");
    printf("                           =======GAME OVER=======\n");
    printf("                           ====PLAYER 8: %05d====\n", birdScore[0]);
    printf("                           ====PLAYER 6: %05d====\n", birdScore[1]);
    printf("                           ====PLAYER 5: %05d====\n", birdScore[2]);
    printf("                           ====PLAYER 9: %05d====\n", birdScore[3]);
    printf("                           ==Press 7 to continue==\n", birdScore[3]);
    while (1)
    {
        if (GPIO_READ(7) == 0)
        {
            printf("\x0c");
            break;
        }
    }
}

void gameInput()
{
    if (GPIO_READ(8) == 0)
    {
        birdHeight[0]--;
    }
    else
    {
        birdHeight[0]++;
    }
    if (GPIO_READ(6) == 0)
    {
        birdHeight[1]--;
    }
    else
    {
        birdHeight[1]++;
    }
    if (GPIO_READ(5) == 0)
    {
        birdHeight[2]--;
    }
    else
    {
        birdHeight[2]++;
    }
    if (GPIO_READ(9) == 0)
    {
        birdHeight[3]--;
    }
    else
    {
        birdHeight[3]++;
    }
}

void gameProcess()
{
    int i = 0;

    for (i = 1; i < 78; i++) // 水管左移
    {
        pipeSpace[i - 1] = pipeSpace[i];
        pipeHeight[i - 1] = pipeHeight[i];
    }
    if (nextPipeInterval == 0) // 生成新的水管
    {
        pipeHeight[77] = nextPipeHeight;
        pipeSpace[77] = nextPipeSpace;
        nextPipeInterval = randIntFrom(6, 12);
        nextPipeHeight = randIntFrom(4, 10);
        nextPipeSpace = randIntFrom(3, 6);
    }
    else
    {
        pipeHeight[77] = 0;
        pipeSpace[77] = 18;
        nextPipeInterval--;
    }

    for (i = 0; i < 4; i++) // 更新鸟的状态
    {
        if (birdAlive[i] == 0) // 鸟死了就不继续了
        {
            continue;
        }
        else
        {
            if (birdHeight[i] < 1 || birdHeight[i] > 18) // 超出上界或下界
            {
                birdAlive[i] = 0;
                birdHeight[i] = 20;
                pthread_create(&tids[i], NULL, blink3times, &LEDPIN[i]);
            }
            else if ((birdHeight[i] < pipeHeight[6 + 4 * i]) ||
                     (birdHeight[i] > pipeHeight[6 + 4 * i] + pipeSpace[6 + 4 * i])) // 撞到水管
            {
                birdAlive[i] = 0;
                birdHeight[i] = 20;
                pthread_create(&tids[i], NULL, blink3times, &LEDPIN[i]);
            }
            else // 加分
            {
                birdScore[i]++;
                GPIO_WRITE(10 + i, 1);
            }
        }
    }
}

void refreshGraph()
{
    int i = 0, j = 0;

    printf("\x0c\n"); // 绘制基本框架
    sprintf(graph[0], "┌───P8:%05d───P6:%05d───P5:%05d───P9:%05d──────────────────────────────────┐\n",
            birdScore[0], birdScore[1], birdScore[2], birdScore[3]);
    for (i = 1; i < 19; i++)
    {
        sprintf(graph[i], "│                                                                              │\n");
    }
    sprintf(graph[19], "└──────────────────────────────────────────────────────────────────────────────┘\n");

    if (birdHeight[0] >= 1 && birdHeight[0] <= 18) // 绘制鸟
        graph[birdHeight[0]][6] = '8';
    if (birdHeight[1] >= 1 && birdHeight[1] <= 18)
        graph[birdHeight[1]][10] = '6';
    if (birdHeight[2] >= 1 && birdHeight[2] <= 18)
        graph[birdHeight[2]][14] = '5';
    if (birdHeight[3] >= 1 && birdHeight[3] <= 18)
        graph[birdHeight[3]][18] = '9';

    for (i = 0; i < 78; i++) // 绘制水管
    {
        for (j = 0; j < pipeHeight[i]; j++)
        {
            graph[j + 1][i + 1] = 'X';
        }
        for (j = pipeHeight[i] + pipeSpace[i]; j < 18; j++)
        {
            graph[j + 1][i + 1] = 'X';
        }
    }

    for (i = 0; i < 20; i++)
    {
        printf("%s", graph[i]);
    }
}

int randIntFrom(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

void msleep(long ms)
{
    long i = 0;
    for (i = 0; i < ms; i++)
    {
        usleep(1000); //sleep 1 ms
    }
}

void *blink3times(void *args)
{
    int pin = (int)(*((int *)args));
    GPIO_WRITE(pin, 1);
    msleep(500);
    GPIO_WRITE(pin, 0);
    msleep(500);
    GPIO_WRITE(pin, 1);
    msleep(500);
    GPIO_WRITE(pin, 0);
    msleep(500);
    GPIO_WRITE(pin, 1);
    msleep(500);
    GPIO_WRITE(pin, 0);
    // 创建线程案例: pthread_create(&tids[3], NULL, blink3times, &LEDPIN[3]);
    // 等待所有线程运行完成再退出: pthread_exit(NULL);
}
