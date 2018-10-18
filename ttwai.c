#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"
#include <math.h>

// 1: 味方, 2: 敵, 3: 置き場
char basicTypeList[19][5][5] = {
	{{0,0,3,0,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,3,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,3,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,3},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,3,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,3,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,0,3},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,3,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,0,0,3},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,3,0,0}},
	{{0,0,0,0,0},{0,0,2,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,3,0}},
	{{0,0,0,0,0},{0,0,0,2,3},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,3},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,0},{0,0,0,0,3},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,3,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,0},{0,0,0,3,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,3,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,0},{0,0,3,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,2,0},{0,0,1,0,0},{0,0,0,0,0},{0,3,0,0,0}}
};

void TtwAI_ApplyScoreToFiveSet(GameEnvironment *env, int board[], int *score[]){
	int i, j, k, hCount = 0, tCount = 0, point;
	for(i = 0; i < 5; i++){
		if(board[i] == env->currentColor) hCount++;
		else if(board[i] == STATE_NONE) tCount++;
	}
	if(!tCount){
		point = pow(2, hCount);
		if(point < 2) point = 0;
		for(i = 0; i < 5 && !board[i]; i++);
		for(j = 4; j >= 0 && !board[i]; j--);
		for(k = 0; k < 5; k ++) *score[k] += point;
		for(k = i + 1; k < j; k++) *score[k] += point;
	}
}

void TtwAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int score[BOARD_SIZE][BOARD_SIZE];
	int x, y, x1 = -1, x2 = -1, y1 = -1, y2 = -1, i;
	int bs[5], *ss[5];

	// 現在おかれている珠の存在範囲を計算
	for(i = 0; i < BOARD_SIZE * BOARD_SIZE; i++){
		if(!~x1 && env->mainBoard[(i / BOARD_SIZE) * (i % BOARD_SIZE)]) x1 = i / BOARD_SIZE;
		if(!~y1 && env->mainBoard[(i % BOARD_SIZE) * (i / BOARD_SIZE)]) y1 = i / BOARD_SIZE;
		if(~x2 && env->mainBoard[(BOARD_SIZE - 1 - i / BOARD_SIZE) * (i % BOARD_SIZE)]) x2 = BOARD_SIZE - 1 - i / BOARD_SIZE;
		if(~y2 && env->mainBoard[(i % BOARD_SIZE) * (BOARD_SIZE - 1 - i / BOARD_SIZE)]) y2 = BOARD_SIZE - 1 - i / BOARD_SIZE;
	}
	if(x1 < 0) x1 = 0;
	if(y1 < 0) y1 = 0;
	if(x1 >= BOARD_SIZE) x1 = BOARD_SIZE - 1;
	if(y1 >= BOARD_SIZE) y1 = BOARD_SIZE - 1;
	if(x2 < 0) x2 = 0;
	if(y2 < 0) y2 = 0;
	if(x2 >= BOARD_SIZE) x2 = BOARD_SIZE - 1;
	if(y2 >= BOARD_SIZE) y2 = BOARD_SIZE - 1;

	x1 = y1 = 0, x2 = y2 = BOARD_SIZE - 1;

	// 評価値の初期化
	for(x = 0; x < BOARD_SIZE; x++) for(y = 0; y < BOARD_SIZE; y++) score[x][y] = 0;

	printf("x1=%d,x2=%d,y1=%d,y2=%d\n", x1, x2, y1, y2);

	

	for(y = y1 - 4; y <= y2 + 4; y++){
		for(x = x1 - 4; x <= x2 + 4; x++){
			// 右方向の5つ組
			if(x <BOARD_SIZE - 4 && y < BOARD_SIZE){
				for(i = 0; i < 5; i++){
					bs[i] = env->mainBoard[(x + i)*y];
					ss[i] = &(score[x + i][y]);
				}
				TtwAI_ApplyScoreToFiveSet(env, bs, ss);
			}
			// 下方向
			if(y <BOARD_SIZE - 4 && x < BOARD_SIZE){
				for(i = 0; i < 5; i++){
					bs[i] = env->mainBoard[x*(y + i)];
					ss[i] = &(score[x][y + i]);
				}
				TtwAI_ApplyScoreToFiveSet(env, bs, ss);
			}
			// 右下方向
			if(x <BOARD_SIZE - 4 && y < BOARD_SIZE - 4){
				for(i = 0; i < 5; i++){
					bs[i] = env->mainBoard[(x + i)*(y + i)];
					ss[i] = &(score[x + i][y + i]);
				}
				TtwAI_ApplyScoreToFiveSet(env, bs, ss);
			}
			// 左下方向
			if(4 <= x && x <BOARD_SIZE && y < BOARD_SIZE - 4){
				for(i = 0; i < 5; i++){
					bs[i] = env->mainBoard[(x - i)*(y + i)];
					ss[i] = &(score[x - i][y + i]);
				}
				TtwAI_ApplyScoreToFiveSet(env, bs, ss);
			}
		}
	}
	int max = 0;
	for(x = 0; x < BOARD_SIZE; x++)
		for(y = 0; y < BOARD_SIZE; y++)
			if(score[x][y] > max) max = score[x][y];

	for(i = max; i >= 0; i--){
		for(y = 0; y < BOARD_SIZE; y++){
			for(x = 0; x < BOARD_SIZE; x++){
				if(score[x][y] == i) {
					to->x = x, to->y = y;
					return;
				}

			}
		}
	}
	 
}

