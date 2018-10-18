#define BOARD_SIZE	15
#define ROW_LIST_LENGTH (BOARD_SIZE * BOARD_SIZE * 4)

#define STATE_NONE	0
#define STATE_BLACK	1
#define STATE_WHITE	2

#define PLAYER_TYPE_HUMAN		0
#define PLAYER_TYPE_AI_RAND		1
#define AI_LIST_LENGTH			3

#define DIRECTION_X		1
#define DIRECTION_Y		2
#define DIRECTION_LDOWN	4
#define DIRECTION_RDOWN 8

#define ENDTYPE_CAN_PUT		1

typedef struct STONE_LOCATION StoneLocation;
struct STONE_LOCATION {
	int x, y;
};

typedef struct STONE_ROW StoneRow;
struct STONE_ROW {
	int col;
	StoneLocation start;
	int direction;
	int length; // > 1.
	int endType;
	// bit 0: start
	// bit 1: end
};

typedef struct GAME_ENVIRONMENT GameEnvironment;
struct GAME_ENVIRONMENT {
	int playerBlackType;
	int playerWhiteType;
	int playerBlackWins;
	int playerWhiteWins;
	char mainBoard[BOARD_SIZE*BOARD_SIZE];
	StoneRow rowList[ROW_LIST_LENGTH];
	StoneLocation history[BOARD_SIZE*BOARD_SIZE];
	int turnCount;	// even: black, odd: white
	int gameCount;
	int gameCountLimit;
	int currentColor;
	int currentPlayerType;
};

extern char *stateString[];
extern char *colorString[];
extern char *playerTypeString[];

// @gomoku.c
void inputStoneLocation(StoneLocation *loc);
void selectGameMode(GameEnvironment *env);
int scanIntegerRanged(int from, int to, char *message);
void initBoard(char *board);
void printBoard(char *board);
int putStone(char *board, int x, int y, int state);
void checkMapRows(char *board, StoneRow *rowList, int rowListLen);
void checkMapRowsSub(char *board, char *checkMap, int nx, int ny, int d, int col, int *count);
int checkMapRows_getEndType(char *board, int x, int y, int d, int count);
void getLocationOnDirection(int *x, int *y, int d, int count);
int isGameEnd(GameEnvironment *env);

// @gomokuai.c
void RandAI_decideNextLocation(StoneLocation *to, GameEnvironment *env);
void EasyAI_decideNextLocation(StoneLocation *to, GameEnvironment *env);
void TtwAI_decideNextLocation(StoneLocation *to, GameEnvironment *env);

// ReigerAI.c
void ReijerAI_decideNextLocation(StoneLocation *to, GameEnvironment *env);
