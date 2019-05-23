#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <eggx.h>   //mac Ver.
//#include <wineggx.h>  //windows Ver.

#define ROW             20      //行
#define COLUMN          10      //列
#define CAMPUS_HEIGHT   700.0   //ウィンドウの大きさ       
#define ON              1
#define OFF             0
#define RIGHT_KEY       28
#define LEFT_KEY        29
#define DOWN_KEY        31
#define UP_KEY          30
#define A_KEY           97
#define D_KEY           100

int win, row, column, mainswitch, collision_val;
int board[ROW + 4][COLUMN + 4];
int motion[ROW + 4][COLUMN + 4];
int nextblock[8];
int numeric[7] = {1,2,3,4,5,6,7};
double CAMPUS_WIDTH = CAMPUS_HEIGHT * 1.618;
double blocksize = CAMPUS_HEIGHT / (ROW + 3);
double board_x = 50;//(CAMPUS_HEIGHT * 1.618 - (CAMPUS_HEIGHT / (ROW + 3)) * (COLUMN + 1)) / 2;
double board_y = (CAMPUS_HEIGHT - (CAMPUS_HEIGHT / (ROW + 3)) * (ROW + 2)) / 2;
double board_wid = (CAMPUS_HEIGHT / (ROW + 3)) * (COLUMN + 1);
double board_hei = (CAMPUS_HEIGHT / (ROW + 3)) * (ROW + 2);
int Tetrimino[8][4][2]  =   {
//  {{row,column},{...},{...},{...}}
    {{0,0},{0,0},{0,0},{0,0}},  //EMPTY
    {{2,1},{2,2},{2,3},{2,4}},  //TETRIS
    {{1,1},{1,2},{2,1},{2,2}},  //SQUARE
    {{1,1},{1,2},{2,2},{2,3}},  //KEY1
    {{1,3},{1,2},{2,2},{2,1}},  //KEY2
    {{1,1},{1,2},{1,3},{2,1}},  //L1
    {{1,1},{1,2},{1,3},{2,3}},  //L2
    {{1,1},{1,2},{1,3},{2,2}},  //T
};

typedef struct BlockInfo    {
    int addr[4][2];
    int color;
} BlockInfo;

struct BlockInfo DropBlock;

void debug()   {
    for(int i = 0; i < ROW + 4; i++)    {
        for(int j = 0; j < COLUMN + 4; j++)    {
            printf(" %d",board[i][j]);
        }
        putchar(' ');
        for(int j = 0; j < COLUMN + 4; j++)    {
            printf(" %d",motion[i][j]);
        }
        putchar('\n');
    }
    putchar('\n');
}

void shuffle(int array[])   {
    srand((unsigned int) time(NULL));
    for(int i = 0; i < 7; i++) {
        int j = rand() % 7;
        int t = array[i];
        array[i] = array[j];
        array[j] = t;
    }
}

void frame(double x,double y,double wid,double hei,double radius,int r, int g, int b)
{
    double frame_x = x + radius;
    double frame_y = y + radius;
    double frame_wid = wid - radius * 2;
    double frame_hei = hei - radius * 2;
    newrgbcolor(win, r, g, b);
    fillrect(win,x,frame_y,wid,frame_hei);
    fillrect(win,frame_x,y,frame_wid,hei);
    fillarc(win, frame_x, frame_y, radius, radius, 180, 270, 1);
    fillarc(win, frame_x, frame_y+frame_hei, radius, radius, 90, 180, 1);
    fillarc(win, frame_x+frame_wid, frame_y, radius, radius, 180, 360, 1);
    fillarc(win, frame_x+frame_wid, frame_y+frame_hei, radius, radius, 0, 90, 1);
}

void bitblock(int matrix_entry,double block_x, double block_y,double size)
{
    if(matrix_entry == 0) {
        // do nothing
    } else if(matrix_entry == 1)   {
        newrgbcolor(win, 0, 191, 255);  //skyblue
    } else if(matrix_entry == 2) {
        newrgbcolor(win, 255, 204, 0);  //yellow
    } else if(matrix_entry == 3) {
        newrgbcolor(win, 124, 252, 0);  //lightgreen
    } else if(matrix_entry == 4) {
        newrgbcolor(win, 204, 0, 0);    //red
    } else if(matrix_entry == 5) {
        newrgbcolor(win, 255, 102, 0);  //orenge
    } else if(matrix_entry == 6) {
        newrgbcolor(win, 0, 51, 204);   //blue
    } else if(matrix_entry == 7) {
        newrgbcolor(win, 148, 0, 211);  //purple
    }
    fillrect(win, block_x, block_y, size, size);
}

void draw_block()   {
    int matrix;
    for(int i = 2; i < ROW + 2; i++)    {
        for(int j = 2; j < COLUMN + 2; j++) {
            if(board[i][j] != 0 || motion[i][j] != 0)    {
                double x = board_x + blocksize * (j - 2) + blocksize / 2;
                double y = board_y + board_hei - blocksize * i;
                if(board[i][j] != 0)    {
                    matrix = board[i][j];
                } else if(motion[i][j] != 0)    {
                    matrix = motion[i][j];
                }
                bitblock(matrix, x, y, blocksize);
            }
        }
    }
}

void drawing()
{
    //stage
    double stage_x = board_x + blocksize / 2;
    double stage_y = board_y + blocksize;
    double stage_wid = blocksize * COLUMN;
    double stage_hei = blocksize * ROW;
    newrgbcolor(win, 255, 255, 224);
    fillrect(win,stage_x, stage_y, stage_wid, stage_hei);
    //block
    draw_block();
    //grid
    double line_x0 = board_x + board_y;
    double line_x1 = line_x0 + CAMPUS_HEIGHT / (ROW + 3) * COLUMN;
    double line_y0 = board_y + CAMPUS_HEIGHT / (ROW + 3);
    double line_y1 = line_y0 + (CAMPUS_HEIGHT / (ROW + 3)) * ROW;
    newrgbcolor(win, 211, 211, 211);
    for(int x = 0; x <= COLUMN; x++) {
        double line_x  = board_x + board_y + (blocksize * x);
        drawline(win,line_x, line_y0, line_x, line_y1);
    }
    for(int y = 1; y <= ROW + 1; y++)    {
        double line_y = board_y + blocksize * y;
        drawline(win, line_x0, line_y, line_x1, line_y);
    }
    
}

void create_newblock()
{
    if(nextblock[0] == 0)   {
        int array[7];
        memcpy(array, numeric, sizeof(numeric));
        shuffle(array);
        for(int i = 0; i < 7; i++) {
            nextblock[i] = array[i];
        }
    }
    // ドロップブロックを更新
    int num = nextblock[0];
    for(int i = 0; i < 4; i++) {
        DropBlock.addr[i][0] = Tetrimino[num][i][0] - 1;
        DropBlock.addr[i][1] = Tetrimino[num][i][1] + (COLUMN / 2);
    }
    DropBlock.color = num;
    for(int i = 0; i < 7; i++) {
        nextblock[i] = nextblock[i+1];
    }
}

void setting()
{
    for(int i = 2; i < COLUMN + 2; i++)    {
        board[ROW + 2][i] = 9;
        board[ROW + 3][i] = 9;
    }
    for(int i = 0; i < ROW + 4; i++)    {
        board[i][0] = 9;
        board[i][1] = 9;
        board[i][COLUMN + 2] = 9;
        board[i][COLUMN + 3] = 9;
    }
    mainswitch = ON;
    newrgbcolor(win, 255, 255, 255);
    fillrect(win,0,0,board_wid+100,CAMPUS_HEIGHT);
    frame(board_x,board_y,board_wid,board_hei,14.0,32,178,170);
    create_newblock();
}

void deleteline()
{
    mainswitch = OFF;
    for (int row = ROW+1; row > 1; row--)    {
        int num = 1;
        for (int column = 2; column < COLUMN+2; column++)    {
            num *= board[row][column];
        }
        if (num != 0)   {
            for(int i = row; i > 1; i--)    {
                for(int j = 2; j < COLUMN+2; j++)   {
                    board[i][j] = board[i-1][j];
                }
            }
            msleep(1000);
            drawing();
            row = ROW+1;
        }
        if(row == 2)  {
            break;
        }
    }
    mainswitch = ON;
}

void transcribe_addr()
{
    for(int i = 0; i < ROW + 4; i++)    {
        for(int j = 0; j < COLUMN + 4; j++)    {
            motion[i][j] = 0;
        }
    }
    for(int i = 0; i < 4; i++)  {
        row = DropBlock.addr[i][0];
        column = DropBlock.addr[i][1];
        motion[row][column] = DropBlock.color;
    }
}

void RightShift_process()
{
    // 衝突値の計算
    for(int i = 0; i < 4; i++)  {
        row = DropBlock.addr[i][0];
        column = DropBlock.addr[i][1] + 1;
        collision_val += board[row][column];
    }

    // 衝突検査
    if(collision_val == 0)    {   //非衝突
        // アドレスを更新
        for(int i = 0; i < 4; i++)  {
            DropBlock.addr[i][1] += 1;
        }
        transcribe_addr();
    } else  {           //衝突
        collision_val = 0;   //衝突値を初期化
    }

    debug();
    drawing();
}

void LeftShift_process()
{
    // 衝突値の計算
    for(int i = 0; i < 4; i++)  {
        row = DropBlock.addr[i][0];
        column = DropBlock.addr[i][1] - 1;
        collision_val += board[row][column];
    }

    // 衝突検査
    if(collision_val == 0)    {   //非衝突
        // アドレスを更新
        for(int i = 0; i < 4; i++)  {
            DropBlock.addr[i][1] -= 1;
        }
        transcribe_addr();
    } else  {           //衝突
        collision_val = 0;   //衝突値を初期化
    }

    debug();
    drawing();
}

void ClockwiseRotation_process()
{
    // 衝突値の計算
    int info[4][2];
    for(int i = 0;i < 4;i++){
        info[i][0] = DropBlock.addr[1][0] - DropBlock.addr[1][1] + DropBlock.addr[i][1];
        info[i][1] = DropBlock.addr[1][1] + DropBlock.addr[1][0] - DropBlock.addr[i][0];
        row = info[i][0];
        column = info[i][1];
        collision_val += board[row][column];
    }

    // 衝突検査
    if(collision_val == 0)    {   //非衝突
        // Update address
        for(int i = 0; i < 4; i++)  {
            DropBlock.addr[i][0] = info[i][0];
            DropBlock.addr[i][1] = info[i][1];
        }
        transcribe_addr();
    } else  {           //衝突
        collision_val = 0;   //衝突値を初期化
        //shift
    }

    debug();
    drawing();
}

void AnticlockwiseRotation_process()
{
    // 衝突値の計算
    int info[4][2]; 
    for(int i = 0;i < 4;i++)   {
        info[i][0] = DropBlock.addr[1][1] + DropBlock.addr[1][0] - DropBlock.addr[i][1];
        info[i][1] = DropBlock.addr[1][1] - DropBlock.addr[1][0] + DropBlock.addr[i][0];
        row = info[i][0];
        column = info[i][1];
        collision_val += board[row][column];
    }

    // 衝突検査
    if(collision_val == 0)    {   //非衝突
        // Update address
        for(int i = 0; i < 4; i++)  {
            DropBlock.addr[i][0] = info[i][0];
            DropBlock.addr[i][1] = info[i][1];
        }
        transcribe_addr();
    } else  {           //衝突
        collision_val = 0;   //衝突値を初期化
        //shift
    }

    debug();
    drawing();
}

void quickdrop_process()    {
    while( 1 )  {
        //衝突値の計算
        for(int i = 0; i < 4; i++)  {
            row = DropBlock.addr[i][0] + 1;
            column = DropBlock.addr[i][1];
            collision_val += board[row][column];
        }

        // 衝突検査
        if(collision_val == 0)    {   //非衝突
            //Update adress
            for(int i = 0; i < 4; i++)  {
                DropBlock.addr[i][0]++;
            }
            transcribe_addr();
        } else  {           //衝突:
            collision_val = 0;   //衝突値を初期化
            for(int i = 0; i < 4; i++)  {   //fix block
                row = DropBlock.addr[i][0];
                column = DropBlock.addr[i][1];
                board[row][column] = DropBlock.color;
            }
            deleteline();    //削除行を検査:
            create_newblock();  //新しいブロックを設定
            break;
        }
    }
    debug();
    drawing();
}

void dropping_process()
{
    //衝突値の計算
    for(int i = 0; i < 4; i++)  {
        row = DropBlock.addr[i][0] + 1;
        column = DropBlock.addr[i][1];
        collision_val += board[row][column];
    }

    // 衝突検査
    if(collision_val == 0)    {   //非衝突
        //Update adress
        for(int i = 0; i < 4; i++)  {
            DropBlock.addr[i][0]++;
        }
        transcribe_addr();
    } else  {           //衝突
        collision_val = 0;   //衝突値を初期化
        for(int i = 0; i < 4; i++)  {   //fix block
            row = DropBlock.addr[i][0];
            column = DropBlock.addr[i][1];
            board[row][column] = DropBlock.color;
        }
        deleteline();    //削除行を検査:
        create_newblock();  //新しいブロックを設定
    }
    
    debug();
    drawing();
}

void SignalHandler(int signum)      //時間制御
{
    static unsigned long msec_cnt = 0;
    msec_cnt++;
    if(!(msec_cnt % 100)){      //１秒経てばドロップ
        dropping_process();
    }
    return;
}

void main(void)
{
    struct sigaction action;
    struct itimerval timer;
    win=gopen(board_wid+100,CAMPUS_HEIGHT);

    memset(&action, 0, sizeof(action));

    // set signal handler 
    action.sa_handler = SignalHandler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    if(sigaction(SIGALRM, &action, NULL) < 0)   {
        perror("sigaction error");
        exit(1);
    }
    // set intarval timer (10ms)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &timer, NULL) < 0)    {
        perror("setitimer error");
        exit(1);
    }
    setting();
    while( 1 ) {
        if(mainswitch == ON)    {
            switch(ggetch()) {
                case LEFT_KEY:
                    LeftShift_process();
                    break;
                case RIGHT_KEY:
                    RightShift_process();
                    break;
                case D_KEY:
                    ClockwiseRotation_process();
                    break;
                case A_KEY:
                    AnticlockwiseRotation_process();
                    break;
                case DOWN_KEY:
                    dropping_process();
                    break;
                case UP_KEY:
                    quickdrop_process();
                    break;
            }
        }
    }
}