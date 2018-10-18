#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

void RandAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int pos;
	
	pos = rand();
	for(;;){
		pos %= (BOARD_SIZE * BOARD_SIZE);
		if(env->mainBoard[pos] == STATE_NONE){
			break;
		}
		pos++;
	}
	to->y = pos / BOARD_SIZE;
	to->x = pos % BOARD_SIZE;
}

#define EVAL_CANNOT_PUT		(-0xffffff)
#define EVAL_NEUTRAL		(0)
#define EVAL_CHECKMATE		(0xffffff)
#define EVAL_FACTOR_DIST_FROM_CENTER	5
#define EVAL_FACTOR_CAN_PUT_IN_ROW		5

void EasyAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int evalMap[BOARD_SIZE*BOARD_SIZE];
	int x, y, p, i;
	int bestEval;
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE){
				evalMap[y * BOARD_SIZE + x] = EVAL_CANNOT_PUT;
			} else{
				evalMap[y * BOARD_SIZE + x] = EVAL_NEUTRAL;
				p = 0;
				p += (x > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - x : x);
				p += (y > (BOARD_SIZE >> 1) ? BOARD_SIZE - 1 - y : y);
				p *= EVAL_FACTOR_DIST_FROM_CENTER;
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
	}
	// 自分の列を伸ばす戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col != env->currentColor){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length;
			p += ((env->rowList[i].length == 4) ? EVAL_CHECKMATE : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length;
			p += ((env->rowList[i].length == 4) ? EVAL_CHECKMATE : 0);
			evalMap[y * BOARD_SIZE + x] += p;
		}
	}
	// 相手の列を抑える戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col == env->currentColor){
			continue;
		}
		if(env->rowList[i].length < 3 || (env->rowList[i].length == 3 && env->rowList[i].endType != 3)){
			continue;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 2;
			evalMap[y * BOARD_SIZE + x] += p;
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 2;
			evalMap[y * BOARD_SIZE + x] += p;
		}
	}
	// 相手の列で間隔が一つのものを埋める戦略
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		if(env->rowList[i].col == env->currentColor){
			continue;;
		}
		if(env->rowList[i].endType & ENDTYPE_CAN_PUT){
			// can put before start point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, -2);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 3;
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
		if((env->rowList[i].endType >> 1) & ENDTYPE_CAN_PUT){
			// can put after end point.
			x = env->rowList[i].start.x;
			y = env->rowList[i].start.y;
			getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length + 1);
			p = EVAL_FACTOR_CAN_PUT_IN_ROW * env->rowList[i].length * 3;
			if(env->mainBoard[y * BOARD_SIZE + x] != STATE_NONE && env->mainBoard[y * BOARD_SIZE + x] != env->currentColor){
				x = env->rowList[i].start.x;
				y = env->rowList[i].start.y;
				getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
				evalMap[y * BOARD_SIZE + x] += p;
			}
		}
	}
	// Debug out.
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length <= 0){
			break;
		}
		printf("%s row ", colorString[env->rowList[i].col]);
		//
		x = env->rowList[i].start.x;
		y = env->rowList[i].start.y;
		getLocationOnDirection(&x, &y, env->rowList[i].direction, -1);
		p = evalMap[y * BOARD_SIZE + x];
		printf("[%d]", p);
		//
		printf("%c%d, %d%c", ((env->rowList[i].endType & 1) ? '(' : '['), x, y, (((env->rowList[i].endType >> 1) & 1) ? ')' : ']'));
		x = env->rowList[i].start.x;
		y = env->rowList[i].start.y;
		getLocationOnDirection(&x, &y, env->rowList[i].direction, env->rowList[i].length);
		p = evalMap[y * BOARD_SIZE + x];
		printf("[%d]", p);
		//
		printf("%d\n", env->rowList[i].length);
	}
	// Choose best location to put.
	bestEval = EVAL_CANNOT_PUT;
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(evalMap[y * BOARD_SIZE + x] > bestEval){
				bestEval = evalMap[y * BOARD_SIZE + x];
				to->x = x;
				to->y = y;
			}
		}
	}
}