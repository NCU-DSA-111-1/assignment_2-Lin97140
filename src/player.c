#include "player.h"
#include <stdio.h>

void initialize(player *ptr)
{
    if (ptr->name == 'x')
    {
        for (int i = 0; i < 9; i++)
        {
            ptr->position[i][0] = i + 1;
            ptr->position[i][1] = 9;
            ptr->position[i + 11][0] = i + 1;
            ptr->position[i + 11][1] = 7;
        }
        ptr->position[9][0] = 2;
        ptr->position[9][1] = 8;
        ptr->position[10][0] = 8;
        ptr->position[10][1] = 8;
    }
    else
    {
        for (int i = 0; i < 9; i++)
        {
            ptr->position[i][0] = i + 1;
            ptr->position[i][1] = 1;
            ptr->position[i + 11][0] = i + 1;
            ptr->position[i + 11][1] = 3;
        }
        ptr->position[9][0] = 8;
        ptr->position[9][1] = 2;
        ptr->position[10][0] = 2;
        ptr->position[10][1] = 2;
    }
}

int find_thesame(player *me, int position_x, int position_y)
{
    for (int i = 0; i < 20; i++)
    {
        if (me->position[i][0] == position_x && me->position[i][1] == position_y)
        {
            return i;
        }
    }
    return -1; //-1表示沒有找到目標位置有我方棋子
}

int find_enemy(player *enemy, int position_x, int position_y)
{
    for (int i = 0; i < 20; i++)
    {
        if (enemy->position[i][0] == position_x && enemy->position[i][1] == position_y)
        {
            return i; //回傳目標位置敵方棋子種類
        }
    }
    return -1; //-1表示沒有找到目標位置有敵方棋子
}

void show(const player *ptr)
{
    int color = 0;
    if (ptr->name == 'x')
    {
        color = 34;
    }
    else
    {
        color = 31;
    }

    printf("\033[%dm 王將: [%d %d] ", color, ptr->position[4][0], ptr->position[4][1]);
    printf("\033[%dm 金將1: [%d %d] ", color, ptr->position[3][0], ptr->position[3][1]);
    printf("\033[%dm 金將2: [%d %d] ", color, ptr->position[2][0], ptr->position[2][1]);
    printf("\033[%dm 銀將2: [%d %d] ", color, ptr->position[6][0], ptr->position[6][1]);
    printf("\033[%dm 桂馬1: [%d %d] ", color, ptr->position[1][0], ptr->position[1][1]);
    printf("\033[%dm 桂馬2: [%d %d] ", color, ptr->position[7][0], ptr->position[7][1]);
    printf("\033[%dm 香車1: [%d %d] ", color, ptr->position[0][0], ptr->position[0][1]);
    printf("\033[%dm 香車2: [%d %d] ", color, ptr->position[8][0], ptr->position[8][1]);
    printf("\033[%dm 飛車: [%d %d] ", color, ptr->position[9][0], ptr->position[9][1]);
    printf("\033[%dm 角行: [%d %d] ", color, ptr->position[10][0], ptr->position[10][1]);
    printf("\n");
    for (int i = 0; i < 9; i++)
    {
        printf("\033[%dm 步兵%d: [%d %d] ", color, i + 1, ptr->position[i + 11][0], ptr->position[i + 11][1]);
    }
    printf("\n");
}

void change(player **me, player **enemy)
{
    player *temp = *me;
    *me = *enemy;
    *enemy = temp;
}