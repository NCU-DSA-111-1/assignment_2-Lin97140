#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "player.h"
#include <ev.h>
#include <pthread.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

int flag = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ev_io stdin_watcher;
ev_timer timeout_watcher;

/*
9 8 7 6 5 4 3 2 1 棋盤位置座標表示
                2
                3
                4
                5
                6
                7
                8
                9
*/
void *child()
{
    time_t sec_s, sec_n, diff, diff_s = 0, diff_n = 0;
    sec_s = time(NULL);
    printf("\n");
    while (!flag)
    {
        sec_n = time(NULL);
        diff = sec_n - sec_s;
        diff_s = diff;
        if (diff_s != diff_n)
        {
            printf("\rtime: %ld      ", diff_s);
            diff_n = diff_s;
            fflush(stdout);
        }
    }
}

static void stdin_cb(EV_P_ ev_io *w, int revents)
{

    // for one-shot events, one must manually stop the watcher
    // with its corresponding stop function.
    // flag = 1;
    pthread_mutex_lock(&mutex);
    flag = 1;
    pthread_mutex_unlock(&mutex);
    ev_io_stop(EV_A_ w);
    // this causes all nested ev_run's to stop iterating
    ev_break(EV_A_ EVBREAK_ALL);
}

int isoccupied(node **stackptr, player *me, player *enemy, int position_x, int position_y, int kind)
{
    int type;
    int original_x, original_y;
    if (find_thesame(me, position_x, position_y) == -1 && find_enemy(enemy, position_x, position_y) == -1) //若目標位置沒有我方和敵方的棋
    {
        original_x = me->position[kind][0];
        original_y = me->position[kind][1];
        me->position[kind][0] = position_x;
        me->position[kind][1] = position_y;
        push(stackptr, me->name, kind, original_x, original_y, me->position[kind][0], me->position[kind][1], 0); //將原來玩家名稱、旗子種類、原來位置、目標位置、是否被吃push進stack
        return TRUE;                                                                                             //移動成功回傳1
    }
    //判斷是否目標位置上是敵方棋子
    else
    {

        type = find_enemy(enemy, position_x, position_y); //若有敵方棋子，紀錄種類
        if (type != -1)
        {
            //吃掉敵方棋子
            original_x = me->position[kind][0]; //先將我方的棋放入stack
            original_y = me->position[kind][1];
            me->position[kind][0] = position_x;
            me->position[kind][1] = position_y;
            push(stackptr, me->name, kind, original_x, original_y, me->position[kind][0], me->position[kind][1], 0);

            original_x = enemy->position[type][0]; //加入被吃敵方的棋
            original_y = enemy->position[type][1];
            enemy->position[type][0] = 0; //敵方被吃後的棋子設為(0,0)
            enemy->position[type][1] = 0;
            push(stackptr, enemy->name, type, original_x, original_y, enemy->position[type][0], enemy->position[type][1], 1);
            return TRUE;
        }
        //表示目標位置有我方棋子
        return FALSE; //移動失敗回傳0
    }
}

// 0->香0 1->桂0 2->銀0 3->金0 4->玉 5->金1 6->銀1 7->桂1 8->香1 9->飛 10->角 11 ~ 19->步

int move(player *me, player *enemy, int kind, int position_x, int position_y, node **stackptr)
{
    int type = 0; //接收函式回傳的敵方棋子種類
    int movecorrect = FALSE;
    //判斷是否超出棋盤或是沒有移動到棋子
    if (position_x < 1 || position_x > 9 || position_y < 1 || position_y > 9 || (position_x == me->position[kind][0] && position_y == me->position[kind][1]) || me->position[kind][0] == 0)
    {
        return FALSE;
    }
    else
    {
        //香車
        if (kind == 0 || kind == 8)
        {
            if (position_x - me->position[kind][0] != 0)
            {
                return FALSE;
            }
            else
            {
                if (me->name == 'x')
                {
                    if (position_y - me->position[kind][1] >= 0)
                    {
                        return FALSE; // 向後移動
                    }

                    else
                    {
                        //判斷前方是否有棋子阻擋
                        for (int i = 0; i < 20; i++)
                        {
                            if (i == kind)
                            {
                                continue;
                            }
                            if (me->position[i][0] == position_x && me->position[i][1] > position_y)
                            {
                                return FALSE;
                            }
                            if (enemy->position[i][0] == position_x && enemy->position[i][1] > position_y)
                            {
                                return FALSE;
                            }
                        }
                        //判斷目標位置是否已有棋子
                        if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                        {
                            return TRUE;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if (position_y - me->position[kind][1] <= 0)
                    {
                        return FALSE;
                    }
                    else
                    {
                        //判斷前方是否有棋子阻擋
                        for (int i = 0; i < 20; i++)
                        {
                            if (me->position[i][0] == position_x && me->position[i][1] < position_y)
                            {
                                return FALSE;
                            }
                            if (enemy->position[i][0] == position_x && enemy->position[i][1] < position_y)
                            {
                                return FALSE;
                            }
                        }
                        //判斷目標位置是否已有棋子
                        if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                        {
                            return TRUE;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }
        //桂馬
        else if (kind == 1 || kind == 7)
        {
            int y_move = 0;
            if (me->name == 'x')
            {
                y_move = -2;
            }
            else
            {
                y_move = 2;
            }
            if ((position_x - me->position[kind][0] == -1 || position_x - me->position[kind][0] == 1) && position_y - me->position[kind][1] == y_move)
            {
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        //銀
        else if (kind == 2 && kind == 6)
        {
            int kind2move_px[5][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 1}, {1, 1}}; // player_x可移動的方式
            int kind2move_py[5][2] = {{-1, 1}, {0, 1}, {1, 1}, {-1, -1}, {1, -1}};  // player_y可移動的方式
            if (me->name == 'x')
            {
                for (int i = 0; i < 5; i++)
                {
                    if (position_x - me->position[kind][0] == kind2move_px[i][0] && position_y - me->position[kind][1] == kind2move_px[i][1]) //判斷移動方式是否符合規則
                    {
                        movecorrect = TRUE;
                        break;
                    }
                }
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    if (position_x - me->position[kind][0] == kind2move_py[i][0] && position_y - me->position[kind][1] == kind2move_py[i][1]) //判斷移動方式是否符合規則
                    {
                        movecorrect = TRUE;
                        break;
                    }
                }
            }
            if (movecorrect) //符合規則
            {
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        //金
        else if (kind == 3 || kind == 5)
        {
            int kind3move_px[6][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {0, 1}}; // player_x可移動的方式
            int kind3move_py[6][2] = {{-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {0, -1}};   // player_y可移動的方式
            if (me->name == 'x')
            {
                //檢查是否符合上面六種走法
                for (int i = 0; i < 6; i++)
                {
                    if (position_x - me->position[kind][0] == kind3move_px[i][0] && position_y - me->position[kind][1] == kind3move_px[i][1]) //判斷移動方式是否符合規則
                    {
                        movecorrect = TRUE;
                        break;
                    }
                }
            }
            else
            {
                //檢查是否符合上面六種走法
                for (int i = 0; i < 6; i++)
                {
                    if (position_x - me->position[kind][0] == kind3move_py[i][0] && position_y - me->position[kind][1] == kind3move_py[i][1]) //判斷移動方式是否符合規則
                    {
                        movecorrect = TRUE;
                        break;
                    }
                }
            }

            if (movecorrect) //符合規則
            {
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        //王將
        else if (kind == 4)
        {
            int kind4move[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
            for (int i = 0; i < 8; i++)
            {
                if (position_x - me->position[kind][0] == kind4move[i][0] && position_y - me->position[kind][1] == kind4move[i][1]) //判斷移動方式是否符合規則
                {
                    movecorrect = TRUE;
                    break;
                }
            }
            if (movecorrect) //符合規則
            {
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        //飛車
        else if (kind == 9)
        {
            //判斷是否走直線
            if ((position_x - me->position[kind][0] == 0 && position_y - me->position[kind][1] != 0) || (position_x - me->position[kind][0] != 0 && position_y - me->position[kind][1] == 0))
            {
                //判斷是否有我方棋子阻擋
                for (int i = 0; i < 20; i++)
                {
                    if (i == kind)
                    {
                        continue;
                    }
                    if ((position_x < me->position[i][0] && me->position[i][0] < me->position[kind][0]) || (position_x > me->position[i][0] && me->position[i][0] > me->position[kind][0]) || (position_y > me->position[i][1] && me->position[i][1] > me->position[kind][1]) || (position_y < me->position[i][1] && me->position[i][1] < me->position[kind][1]))
                    {
                        return FALSE;
                    }
                }
                //判斷是否有敵方棋子阻擋
                for (int i = 0; i < 20; i++)
                {
                    if ((position_x < enemy->position[i][0] && enemy->position[i][0] < me->position[kind][0]) || (position_x > enemy->position[i][0] && enemy->position[i][0] > me->position[kind][0]) || (position_y > enemy->position[i][1] && enemy->position[i][1] > me->position[kind][1]) || (position_y < enemy->position[i][1] && enemy->position[i][1] < me->position[kind][1]))
                    {
                        return FALSE;
                    }
                }
                //判斷是否有我方棋子在目標位置
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        //角行
        else if (kind == 10)
        {
            int deltap2m_x = position_x - me->position[kind][0];
            int deltap2m_y = position_y - me->position[kind][1];
            //判斷是否移動為斜線
            if (position_x == me->position[kind][0] || position_y == me->position[kind][1]) //不是斜線
            {
                return FALSE;
            }
            else if (abs(deltap2m_x) == abs(deltap2m_y)) //是斜線
            {
                //判斷是否有我方棋子阻擋
                for (int i = 0; i < 20; i++)
                {
                    if (i == kind)
                    {
                        continue;
                    }
                    int delta_x = me->position[i][0] - me->position[kind][0];
                    int delta_y = me->position[i][1] - me->position[kind][1];

                    if (abs(delta_x) != abs(delta_y))
                    {
                        continue;
                    }

                    //判斷阻擋的方向
                    else
                    {
                        if ((deltap2m_x < 0 && deltap2m_y < 0) && (delta_x < 0 && delta_y < 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x > 0 && deltap2m_y < 0) && (delta_x > 0 && delta_y < 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x > 0 && deltap2m_y > 0) && (delta_x > 0 && delta_y > 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x < 0 && deltap2m_y > 0) && (delta_x < 0 && delta_y > 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                    }
                }
                //判斷是否有敵方棋子阻擋
                for (int i = 0; i < 20; i++)
                {
                    int delta_x = enemy->position[i][0] - me->position[kind][0];
                    int delta_y = enemy->position[i][1] - me->position[kind][1];

                    if (abs(delta_x) != abs(delta_y))
                    {
                        continue;
                    }
                    else
                    {
                        if ((deltap2m_x < 0 && deltap2m_y < 0) && (delta_x < 0 && delta_y < 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x > 0 && deltap2m_y < 0) && (delta_x > 0 && delta_y < 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x > 0 && deltap2m_y > 0) && (delta_x > 0 && delta_y > 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                        if ((deltap2m_x < 0 && deltap2m_y > 0) && (delta_x < 0 && delta_y > 0))
                        {
                            if (abs(delta_x) < abs(deltap2m_x))
                            {
                                return FALSE;
                            }
                        }
                    }
                }
                //判斷是否有我方棋子在目標位置
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            return FALSE;
        }
        //步兵
        else
        {
            int move_y = 1;
            if (me->name == 'x')
            {
                move_y = -1;
            }
            if (position_x - me->position[kind][0] == 0 && position_y - me->position[kind][1] == move_y) //移動符合規則
            {
                if (isoccupied(stackptr, me, enemy, position_x, position_y, kind))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            return FALSE;
        }
    }
}

int ask(player *play, int *position_x, int *position_y, int *original_x, int *original_y, struct ev_loop *loop, pthread_t *t)
{
    char action = '0';
    int back = 0;
    int kind = 0;

    printf("\033[0m 玩家%c", play->name);
    printf("\033[0m (0:悔棋 s:存檔 1:下棋): ");
    scanf(" %c", &action);
    if (action == 's')
    {
        return -2; //存檔並退出遊戲
    }
    if (action == '1') //下棋
    {
        flag = 0;
        ev_io_start(loop, &stdin_watcher);
        printf("\033[0m 原始座標-段: ");
        pthread_create(t, NULL, child, NULL);
        ev_run(loop, 0);
        scanf("%d", original_x);
        pthread_join(*t, NULL);
        printf("\033[0m 原始座標-筋: ");
        scanf("%d", original_y);
        kind = find_thesame(play, *original_x, *original_y);
        if (kind == -1)
        {
            return kind;
        }
        printf("\033[0m 目標座標-段: ");
        scanf("%d", position_x);
        printf("\033[0m 目標座標-筋: ");
        scanf("%d", position_y);

        return kind;
    }
    return 0; //悔棋
}

int go_back(node **ptr, player **game, int *chess, int *position_x, int *position_y)
{
    int player = 0;
    int p_x, p_y, original_x, original_y, die;
    char name;
    if ((*ptr)->next == NULL) //若目前只有下一手
    {
        if ((*ptr)->name == 'y') // pop一次
        {
            player = 1;
        }
        pop(ptr, chess, position_x, position_y, &original_x, &original_y, &die, &name);
        printf("chess_pop = %d  position_x_pop = %d position_y_pop = %d\n", *chess, original_x, original_y);
        game[player]->position[*chess][0] = original_x;
        game[player]->position[*chess][1] = original_y;
        return 0;
    }

    //若已經下多手
    else
    {
        for (int i = 0; i < 2; i++)
        {
            if ((*ptr)->die) //若遇到標註為死掉的棋，多做一次把被吃掉的棋復原
            {
                player = 0;
                if ((*ptr)->name == 'y')
                {
                    player = 1;
                }
                pop(ptr, chess, position_x, position_y, &original_x, &original_y, &die, &name);
                printf("chess_pop = %d  position_x_pop = %d position_y_pop = %d\n", *chess, original_x, original_y);
                game[player]->position[*chess][0] = original_x;
                game[player]->position[*chess][1] = original_y;
            }
            player = 0;

            //預設為一次復原兩手
            if ((*ptr)->name == 'y')
            {
                player = 1;
            }
            pop(ptr, chess, position_x, position_y, &original_x, &original_y, &die, &name);
            printf("chess_pop = %d  position_x_pop = %d position_y_pop = %d\n", *chess, original_x, original_y);
            game[player]->position[*chess][0] = original_x;
            game[player]->position[*chess][1] = original_y;
        }
        return 1;
    }
}

void save_file(node **stackptr, FILE **fptr)
{
    node *current = NULL;
    while (*stackptr != NULL)
    {
        //將stack內容寫入遊戲檔案
        current = *stackptr;
        fprintf(*fptr, "%c %d %d %d %d %d %d\n", current->name, current->chess, current->original_x, current->original_y, current->position_x, current->position_y, current->die);
        *stackptr = (*stackptr)->next;
        free(current);
    }
}

void load_file(node **stackptr, FILE **fptr)
{
    char name, enter;
    int type, original_x, original_y, position_x, position_y, die;
    while (fscanf(*fptr, " %c %d %d %d %d %d %d", &name, &type, &original_x, &original_y, &position_x, &position_y, &die) != EOF)
    {
        //讀入舊檔並push進stack
        push(stackptr, name, type, original_x, original_y, position_x, position_y, die);
    }
}

/*

int ask_oldgame(node **current, player **game, player *x, player *y)
{
    char action;
    printf("\033[0m f:下一手 b:上一手: e:結束: ");
    scanf(" %c", &action);
    int player = 0;
    printf("-----------------------------------\n");
    if (action == 'f')
    {
        printf("curret = %p\n", *current);
        if ((*current) != NULL)
        {
            for (int i = 0; i < 2; i++) //預設改變兩手
            {
                if ((*current)->name == 'y')
                {
                    player = 1;
                }
                else
                {
                    player = 0;
                }
                game[player]->position[(*current)->chess][0] = (*current)->position_x;
                game[player]->position[(*current)->chess][1] = (*current)->position_y;
                //若沒有遇到標註為死掉的棋，只改變一手棋
                if ((*current)->next == NULL)
                {
                    break;
                }
                *current = (*current)->next;
                if (!(*current)->die)
                {
                    break;
                }
            }
            show(x);
            show(y);
        }
        else
        {
            printf("已到最後一步\n");
        }

        return 1;
    }
    else if (action == 'b')
    {
        if ((*current)->previous != NULL)
        {
            *current = (*current)->previous;
            if ((*current)->name == 'y')
            {
                player = 1;
            }
            else
            {
                player = 0;
            }
            game[player]->position[(*current)->chess][0] = (*current)->original_x;
            game[player]->position[(*current)->chess][1] = (*current)->original_y;
            if ((*current)->die)
            {
                *current = (*current)->previous;
                if ((*current)->name == 'y')
                {
                    player = 1;
                }
                else
                {
                    player = 0;
                }
                game[player]->position[(*current)->chess][0] = (*current)->original_x;
                game[player]->position[(*current)->chess][1] = (*current)->original_y;
            }
            show(x);
            show(y);
        }
        else
        {
            printf("沒有之前的步\n");
        }

        return 1;
    }
    else if (action == 'e')
    {
        return 0;
    }
}

*/

int ask_oldgame(player **game, node **stackf, node **stackb)
{
    char action;
    printf("\033[0m f:下一手 b:上一手: e:結束: ");
    scanf(" %c", &action);
    int player = 0, chess, position_x, position_y, original_x, original_y, die;
    char name;
    printf("-----------------------------------\n");
    if (action == 'f')
    {
        if (*stackf != NULL)
        {
            printf("here\n");
            pop(stackf, &chess, &position_x, &position_y, &original_x, &original_y, &die, &name);
            printf("player: %c chess = %d o_x = %d o_y = %d p_x = %d p_y = %d die = %d\n", name, chess, original_x, original_y, position_x, position_y, die);
            push(stackb, name, chess, original_x, original_y, position_x, position_y, die);
            if (name == 'x')
            {
                player = 0;
            }
            else
            {
                player = 1;
            }
            game[player]->position[chess][0] = position_x;
            game[player]->position[chess][1] = position_y;
            if (*stackf != NULL)
            {
                if ((*stackf)->die == 1)
                {
                    pop(stackf, &chess, &position_x, &position_y, &original_x, &original_y, &die, &name);
                    push(stackb, name, chess, original_x, original_y, position_x, position_y, die);
                    if (name == 'x')
                    {
                        player = 0;
                    }
                    else
                    {
                        player = 1;
                    }
                    game[player]->position[chess][0] = position_x;
                    game[player]->position[chess][1] = position_y;
                }
            }
            show(game[0]);
            show(game[1]);
        }
        else
        {
            printf("已經最後一步\n");
        }

        return 1;
    }
    else if (action == 'b')
    {
        if (*stackb != NULL)
        {
            if ((*stackb)->die == 1)
            {
                pop(stackb, &chess, &position_x, &position_y, &original_x, &original_y, &die, &name);
                push(stackf, name, chess, original_x, original_y, position_x, position_y, die);
                if (name == 'x')
                {
                    player = 0;
                }
                else
                {
                    player = 1;
                }
                game[player]->position[chess][0] = original_x;
                game[player]->position[chess][1] = original_y;
            }
            pop(stackb, &chess, &position_x, &position_y, &original_x, &original_y, &die, &name);
            push(stackf, name, chess, original_x, original_y, position_x, position_y, die);
            if (name == 'x')
            {
                player = 0;
            }
            else
            {
                player = 1;
            }
            game[player]->position[chess][0] = original_x;
            game[player]->position[chess][1] = original_y;
            show(game[0]);
            show(game[1]);
        }
        else
        {
            printf("已經沒有之前的步\n");
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

int main(int argc, char *argv[])
{
    player x, y;
    player *game[2] = {&x, &y};
    x.name = 'x';
    y.name = 'y';
    int chess = 0, position_x = 0, position_y = 0, original_x = 0, original_y = 0, state = 0;
    initialize(&x);
    initialize(&y);
    node *stackptr = NULL, *stackb = NULL;
    player *me = &x, *enemy = &y;
    FILE *fptr = NULL;
    char path[50] = "game_file/";

    pthread_t t;
    struct ev_loop *loop = EV_DEFAULT;
    ev_io_init(&stdin_watcher, stdin_cb, 0, EV_READ);

    if (argc == 4) //若參數是 ./main -n -s new_file_name
    {
        char *file_name = argv[3]; //讀取建立的新檔名
        strcat(path, file_name);   //將路徑變成 "game_file/new_file_name"
        fptr = fopen(path, "w");
        while (1)
        {
            int chess_before = 0, position_y_before = 0, position_x_before = 0;
            if ((x.position[4][0] == 0 && x.position[4][1] == 0) || (y.position[4][0] == 0 && y.position[4][1] == 0))
            {
                break;
            }
            show(&x);
            show(&y);
            do
            {
                show_stack(stackptr);
                chess = ask(me, &position_x, &position_y, &original_x, &original_y, loop, &t); //回傳 -2:存檔 0 表示悔棋 >=1 整數 移動
                if (chess == -2)
                {
                    save_file(&stackptr, &fptr);
                    break;
                }
                if (!chess)
                {
                    if (isempty(stackptr))
                    {
                        continue;
                    }
                    state = go_back(&stackptr, game, &chess_before, &position_x_before, &position_y_before); //悔棋
                    if (!state)
                    {
                        break;
                    }
                    else
                    {
                        show(&x);
                        show(&y);
                        continue;
                    }
                }
            } while (!move(me, enemy, chess, position_x, position_y, &stackptr) || chess == -1); // 當移動方式符合規定
            if (chess == -2)
            {
                break;
            }
            change(&me, &enemy); //交換x y 身份
        }
    }
    else if (argc == 3)
    {
        char *file_name = argv[2];
        int flag = 1;
        strcat(path, file_name); //將讀取舊檔名的路徑變成 "game_file/old_file_name"
        fptr = fopen(path, "r");
        load_file(&stackptr, &fptr);
        show(&x);
        show(&y);
        show_stack(stackptr);
        while (flag)
        {
            flag = ask_oldgame(game, &stackptr, &stackb); //回復舊檔
        }
    }
    free_stack(&stackptr);
    free_stack(&stackb);
    fclose(fptr);
    return 0;
}
