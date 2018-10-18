#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#include "gomoku.h"

char *colorString[] = {
	"None",
	"Black",
	"White"
};

void (*AIList[AI_LIST_LENGTH])(StoneLocation *to, GameEnvironment *env) = {
	ReijerAI_decideNextLocation,
	RandAI_decideNextLocation,
	TtwAI_decideNextLocation
};

char *playerTypeString[AI_LIST_LENGTH + 1] = {
	"Human",
	"ReijerAI",
	"RandAI",
	"TtwAI"
};

int inputStoneLocationOptional(StoneLocation *loc, int x, int y, char *board, char *message);
void printBoardOptional(char *board, int selectedX, int selectedY);
void revertTurn(GameEnvironment *env, int count);

int my_main(int argc, char **argv){
	
	GameEnvironment env;
	StoneLocation to;
	char message[100];
	int uiResult;
	
	printf("五目並べゲーム by yozu\n\n");
	
	// ゲームモードの選択
	selectGameMode(&env);
	
	srand(time(NULL));
	to.x = to.y = BOARD_SIZE / 2;

	for(env.gameCount = 0; env.gameCount < env.gameCountLimit; env.gameCount++){
		for(env.turnCount = 0; ; env.turnCount++){
			env.currentColor = (env.turnCount & 1) + 1;
			env.currentPlayerType = (env.currentColor == STATE_BLACK ? env.playerBlackType : env.playerWhiteType);
			printf("\n\n%3d-%3d, %s, %s\n", env.gameCount, env.turnCount, playerTypeString[env.currentPlayerType], colorString[env.currentColor]);
	
			// Think or Input.
			if(env.currentPlayerType == PLAYER_TYPE_HUMAN){
				sprintf(message, "%s's turn.\nPush [u] to revert.\n", colorString[env.currentColor]);
				while(uiResult = inputStoneLocationOptional(&to, to.x, to.y, env.mainBoard, message)){
					// ここでuiResultにキーコードが保管される。
					if(uiResult == 117){	// u
						printf("\n### REVERTING...\n");
						if((env.currentColor == STATE_BLACK ? env.playerWhiteType : env.playerBlackType) != PLAYER_TYPE_HUMAN){
							revertTurn(&env, 2);
						}else{
							revertTurn(&env, 1);
						}
						usleep(1000000);
						goto skipPutStone;
						continue;
					}
				}
			} else if(env.currentPlayerType <= AI_LIST_LENGTH){
				AIList[env.currentPlayerType - 1](&to, &env);
			} else{
				puts("Invalid player type. Abort.");
				return 1;
			}
			printf("%s->(%d, %d)\n", colorString[env.currentColor], to.x, to.y);
			
			// Check and put stone.
			if(putStone(env.mainBoard, to.x, to.y, env.currentColor)){
				puts("################################################################");
				if(env.currentPlayerType == PLAYER_TYPE_HUMAN){
					puts("You can't put stone there. Try other location.");
					env.turnCount--;
				} else{
					puts("COM tried to put stone to illegal location. Abort.");
					return 1;
				}
				puts("################################################################");
				usleep(1000000);
				//getchar();
				continue;
			}
skipPutStone:
			printBoardOptional(env.mainBoard, to.x, to.y);
			if(isGameEnd(&env)){
				break;
			}
		}
		
		if(env.gameCountLimit == 1){
			if(env.playerBlackWins > env.playerWhiteWins){
				printf("PLAYER %s WINS!!\n", playerTypeString[env.playerBlackType]);
			}else if(env.playerBlackWins < env.playerWhiteWins){
				printf("PLAYER %s WINS!!\n", playerTypeString[env.playerWhiteType]);
			}else{
				printf("DRAW!!");
			}
		}else{
			printf("BlackWins: %4d/%4d    (%s)\n", env.playerBlackWins, env.gameCount + 1, playerTypeString[env.playerBlackType]);
			printf("WhiteWins: %4d/%4d    (%s)\n", env.playerWhiteWins, env.gameCount + 1, playerTypeString[env.playerWhiteType]);
			printf("     Draw: %4d/%4d\n", env.gameCount - env.playerBlackWins - env.playerWhiteWins + 1, env.gameCount + 1);			
		}

		initBoard(env.mainBoard);
		env.rowList[0].length = 0;
	}

	return 0;
	
}

int main(int argc, char **argv){
	return my_main(argc, argv);
}

int inputStoneLocationOptional(StoneLocation *loc, int x, int y, char *board, char *message){
	
	struct termios tms, tmsOrig;
	int fn, xt, yt;
	char c, line[100], *cl = line;
	
	*cl = '\0';

	tcgetattr(0, &tms);
	tcgetattr(0, &tmsOrig);
	tms.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &tms);
	fn = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, fn | O_NONBLOCK);

	while(getchar() != EOF);

	usleep(10000);

	printf("\n");
	printBoardOptional(board, x, y);
	printf("%s", message);
	printf("Place cursor then press [Enter], or input X [Space] Y : %s", line);
		
	for(;;){
		while((c = getchar()) != EOF){
			do {
				if(c == 10){	// enter
					if(cl != line){
						cl = line;
						xt = yt = 0;
						while('0' <= *cl && *cl <= '9') xt = xt * 10 + *(cl++) - '0';
						while(*cl == ' ') cl++;
						while('0' <= *cl && *cl <= '9') yt = yt * 10 + *(cl++) - '0';
						xt--; yt--;
						if(xt < 0 || yt < 0 || xt >= BOARD_SIZE || yt >= BOARD_SIZE || *cl != '\0'){
							printf("\nIllegal input\n");
							usleep(500000);
							cl = line;
							*cl = '\0';
							continue;
						}
						x = xt, y = yt;
					}
					goto exitLoop;
				}else if(48 <= c && c <= 57 || c == 32 || c == 127){	// 数字キーかSpace / BackSpace
					if(c == 127){
						cl--;
						*cl= '\0';
						if(cl < line) cl = line;
						continue;
					}
					if(c == 32) *cl = ' ';
					else *cl = '0' + (c - 48);
					cl++;
					*cl = '\0';
					continue;
				}else if(c == 27) {	// esc
					cl = line; *cl = '\0';
				}else if(65 <= c && c <= 68) {	// カーソルキー
					if(c == 68) x--;
					else if(c == 67) x++;
					else if(c == 65) y--;
					else if(c == 66) y++;
					if(x < 0) x = 0;
					if(y < 0) y = 0;
					if(x >= BOARD_SIZE) x = BOARD_SIZE - 1;
					if(y >= BOARD_SIZE) y = BOARD_SIZE - 1;		
				}else if(c != 27 && c != 91){
					loc->x = x, loc->y = y;
					return c;
				}
			}while((c = getchar()) != EOF);

			printf("\n");
			printBoardOptional(board, x, y);
			printf("%s", message);
			printf("Place cursor then press [Enter], or input X [Space] Y : %s", line);

		}
		usleep(100000);
	}
	
	exitLoop:
	tcsetattr(0, TCSANOW, &tmsOrig);
	fcntl(0, F_SETFL, fn);
	loc->x = x, loc->y = y;
	return 0;
}

void selectGameMode(GameEnvironment *env)
{	
	int i;
	
	env->playerBlackWins = 0;
	env->playerWhiteWins = 0;

	printf("0: 先手で遊ぶ\n1: 後手で遊ぶ\n2: 二人で遊ぶ\n3: ゲームのカスタマイズ(上級者向け)\n\n");
	i = scanIntegerRanged(0, 4, "? ");
	if(i == 0 || i == 1 || i == 2){
		env->playerBlackType = env->playerWhiteType = 1;
		if(!(i & 1)) env->playerBlackType = 0;
		if(i == 1 || i == 2) env->playerWhiteType = 0;
		env->gameCountLimit = 1;
		return;	
	}
	puts("\n\n\nPlayer type list.");
	for(i = 0; i < AI_LIST_LENGTH + 1; i++){
		printf("%d: %s\n", i, playerTypeString[i]);
	}
	env->playerBlackType = scanIntegerRanged(0, AI_LIST_LENGTH, "Select black player type: ");
	env->playerWhiteType = scanIntegerRanged(0, AI_LIST_LENGTH, "Select white player type: ");
	if(env->playerBlackType != PLAYER_TYPE_HUMAN && env->playerWhiteType != PLAYER_TYPE_HUMAN){
		env->gameCountLimit = scanIntegerRanged(1, 65535, "Input number of games (1-65535): ");
	} else{
		env->gameCountLimit = 1;
	}
	return;
}

int scanIntegerRanged(int from, int to, char *message)
{	
	int tmp;
	for(;;){
		printf("%s", message);
		if(scanf("%d", &tmp) == 1){
			if(from <= tmp && tmp <= to){
				break;
			}
			puts("Out of range. Input again.");
		} else{
			scanf("%*s");
		}
	}
	return tmp;
}

void revertTurn(GameEnvironment *env, int count)
{
	int i;
	
	if(env->turnCount < count){
		puts("################################################################");
		printf("%d %d You can't back before starting this game.", env->turnCount, count);
		env->turnCount--;
		return;
	}
	for(i = 0; i < count; i++){
		putStone(env->mainBoard, env->history[env->turnCount - 1 - i].x, env->history[env->turnCount - 1 - i].y, STATE_NONE);
	}
	env->turnCount -= (count + 1);
	printf("Back to turn %d.", env->turnCount + 1);
}

void initBoard(char *board)
{
	int x, y;

	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			board[y * BOARD_SIZE + x] = STATE_NONE;
		}
	}
	return;
}

void printBoardOptional(char *board, int selectedX, int selectedY){
	int x, y, s;
	printf("\033[2J");
	
	printf("  \033[30;42m   ");
	for(x = 0; x < BOARD_SIZE; x++){
		if(selectedX == x) printf("\033[30;45m");
		printf("%2d ", x + 1);
		if(selectedX == x) printf("\033[30;42m");
	}
	printf("  \033[m\n");
	for(y = 0; y < BOARD_SIZE; y++){
		
		printf("  \033[30;42m");
		if(selectedY == y) printf("\033[30;45m");
		printf("%2d ", y + 1);
		if(selectedY == y) printf("\033[30;42m");
		for(x = 0; x < BOARD_SIZE; x++){
			s = board[y * BOARD_SIZE + x];
			if(selectedX == x && selectedY == y) printf("\033[30;45m");
			if(s == 0){
				if(!x && !y) printf(" \u250f\u2501");
				else if(!x && y == BOARD_SIZE - 1) printf(" \u2517\u2501");
				else if(x == BOARD_SIZE - 1 && !y) printf("\u2501\u2513 ");
				else if(x == BOARD_SIZE - 1 && y == BOARD_SIZE - 1) printf("\u2501\u251b ");
				else if(!x) printf(" \u2523\u2501");
				else if(!y) printf("\u2501\u2533\u2501");
				else if(x == BOARD_SIZE - 1) printf("\u2501\u252b ");
				else if(y == BOARD_SIZE - 1) printf("\u2501\u253b\u2501");
				else printf("\u2501\u254b\u2501");
				
			}
			else if(s == 1) {
			       x ? printf("\u2501") : printf(" "); printf("●"); x < BOARD_SIZE - 1 ? printf("\u2501"): printf(" "); // black
			} else if(s == 2) {
				if(selectedX == x && selectedY == y) {
					x ? printf("\u2501") : printf(" ");
				        printf("\033[37;45m●");
					x < BOARD_SIZE - 1 ? printf("\033[30;45m\u2501") : printf(" ");
				       printf("\033[30;42m");
				} else {
				       x ? printf("\u2501") : printf(" ");
				       printf("\033[37;42m●\033[30;42m");
				       x < BOARD_SIZE - 1 ? printf("\u2501") : printf(" ");
				}
			}// white
			if(selectedX == x && selectedY == y) printf("\033[30;42m");
		}
		printf("  \033[m\n");
	}
}

void printBoard(char *board)
{
	printBoardOptional(board, -1, -1);
	return;
}

int putStone(char *board, int x, int y, int state)
{
	// 0: success
	// 1: out of range
	// 2: already exist.
	if(x < 0 || BOARD_SIZE <= x || y < 0 || BOARD_SIZE <= y){
		return 1;
	}
	if(board[y * BOARD_SIZE + x]){
		return 2;
	}
	board[y * BOARD_SIZE + x] = state;
	return 0;
}

void checkMapRows(char *board, StoneRow *rowList, int rowListLen)
{
	char checkMap[BOARD_SIZE*BOARD_SIZE];
	int x, y, count, col, d, rowIndex, endType;

	// initialize.
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			checkMap[y * BOARD_SIZE + x] = 0;
		}
	}
	rowList[0].length = 0;
	rowIndex = 0;
	for(col = 1; col <= 2; col++){
		for(d = 1; d <= 8; d <<= 1){
			for(y = 0; y < BOARD_SIZE; y++){
				for(x = 0; x < BOARD_SIZE; x++){
					if(board[y * BOARD_SIZE + x] == STATE_NONE){
						continue;
					}
					if(checkMap[y * BOARD_SIZE + x] & d){
						continue;
					}
					count = 0;
					checkMapRowsSub(board, checkMap, x, y, d, col, &count);
					if(count > 1){
						endType = checkMapRows_getEndType(board, x, y, d, count);
						//printf("%s row %c%d, %d%c %d %d %d\n", colorString[col], ((endType & 1) ? '(' : '['), x, y, (((endType >> 1) & 1) ? ')' : ']'), d, count, endType);
						rowList[rowIndex].col = col;
						rowList[rowIndex].start.x = x;
						rowList[rowIndex].start.y = y; 
						rowList[rowIndex].direction = d;
						rowList[rowIndex].length = count;
						rowList[rowIndex].endType = endType;
						rowIndex++;
						if(rowIndex >= rowListLen){
							puts("RowList overflow.");
							return;
						}
					}
				}
			}
		}
	}
	rowList[rowIndex].length = 0;
}

void checkMapRowsSub(char *board, char *checkMap, int nx, int ny, int d, int col, int *count)
{
	if(nx < 0 || BOARD_SIZE <= nx || ny < 0 || BOARD_SIZE <= ny){
		return;
	}
	if(board[ny * BOARD_SIZE + nx] != col){
		return;
	}
	(*count)++;
	checkMap[ny * BOARD_SIZE + nx] |= d;
	getLocationOnDirection(&nx, &ny, d, 1);
	checkMapRowsSub(board, checkMap, nx, ny, d, col, count);
}

int checkMapRows_getEndType(char *board, int x, int y, int d, int count)
{
	int endType = 0;
	int px, py;
	
	// start
	px = x;
	py = y;
	getLocationOnDirection(&px, &py, d, -1);
	if(px != -1 && board[py * BOARD_SIZE + px] == STATE_NONE){
		endType |= ENDTYPE_CAN_PUT;
	}
	
	// end
	px = x;
	py = y;
	getLocationOnDirection(&px, &py, d, count);
	if(px != -1 && board[py * BOARD_SIZE + px] == STATE_NONE){
		endType |= (ENDTYPE_CAN_PUT << 1);
	}
	return endType;
}

void getLocationOnDirection(int *x, int *y, int d, int count)
{
	// (*x, *y) is set (-1, -1) when location after moving is out of bound.
	int dx = 0, dy = 0;
	if(d == DIRECTION_X){
		dx++;
	} else if(d == DIRECTION_Y){
		dy++;
	} else if(d == DIRECTION_LDOWN){
		dx--;
		dy++;
	} else if(d == DIRECTION_RDOWN){
		dx++;
		dy++;
	}
	*x += dx * count;
	*y += dy * count;
	if(*x < 0 || BOARD_SIZE <= *x || *y < 0 || BOARD_SIZE <= *y){
		*x = -1;
		*y = -1;
	}
}

int isGameEnd(GameEnvironment *env)
{
	int i, x, y;

	checkMapRows(env->mainBoard, env->rowList, ROW_LIST_LENGTH);
	for(i = 0; i < ROW_LIST_LENGTH; i++){
		if(env->rowList[i].length == 0){
			break;
		}
		if(env->rowList[i].length == 5){
			break;
		}
	}
	if(i != ROW_LIST_LENGTH && env->rowList[i].length >= 5){
		puts("Game end.");
		printf("WINNER is %s !\n", colorString[env->currentColor]);
		if(env->currentColor == STATE_BLACK){
			env->playerBlackWins++;
		} else{
			env->playerWhiteWins++;
		}
		return 1;
	}
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			if(env->mainBoard[y * BOARD_SIZE + x] == STATE_NONE){
				break;
			}
		}
		if(x != BOARD_SIZE){
			break;
		}
	}
	if(y == BOARD_SIZE){
		puts("Game end.");
		printf("Game ended in draw... \n");
		return 1;
	}

	return 0;
}
