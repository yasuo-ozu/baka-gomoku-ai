#include <stdio.h>
#include <stdlib.h>
#include "gomoku.h"

#define INFINITYVAL		32000	// 無限大
#define BOARDSIZE		15
#define MAXDEPTH		2
#define MAXRANDBONUS	5
#define EMPTY_SQUARE	0		// 空きマス
#define WINNING			30000	// 勝ち

// マスの情報
#define EMPTY_SQUARE	0		// 空きマス
#define BLACK_STONE		1		// 黒い石
#define WHITE_STONE		2		// 白い石

//　手番
#define BLACK			1		// 黒の手番
#define WHITE			2		// 白の手番

// 評価値配列の中、評価特徴の場所
#define CLOSED_FOUR_VALUE	0		// 1番目の値
#define OPEN_THREE_VALUE	1		// 2番目の値
#define CLOSED_THREE_VALUE	2		// 3番目の値
#define OPEN_TWO_VALUE		3		// 4番目の値
#define CLOSED_TWO_VALUE	4		// 5番目の値
#define INITIATIVE			5		// 6番目の値

#define HUMAN				1		// 人間
#define BEST_AI				2		// 現在の一番強い五目AI
#define TEST_AI				3		// テストしたいAI

// 現在の最強の評価値
const int bestEvalValues[]	= { 50, 120, 30, 50, 30, 90 };
// テストしたい評価値
const int testValues[]		= { 10, 10, 10, 10, 10, 10 };



int rootToMove;		// 探索の初期局面の手番
int saveData;
FILE *dataFile;
int gomokuBoard[ BOARDSIZE + 1 ][ BOARDSIZE + 1 ];  // 五目並べの盤
int nextMoveX;										// 次の手のx軸
int nextMoveY;										// 次の手のy軸
int blackOpenFour;		// 黒い石を四つ並びの数、穴なし、ブロックなし： * B B B B *
int blackClosedFour;	// 黒い石を四つ並びの数、穴あり (B B * B B) か白い石のブロック (W B B B B *) か盤の端にブロック (E B B B B *)
int blackOpenThree;		// 黒い石を三つ並びの数、穴なし、ブロックなし： * B B B *
int blackClosedThree;	// 黒い石を三つ並びの数、穴あり (B * B B）か白い石のブロック (W B B B *) か盤の端にブロック (E B B B *)
int blackOpenTwo;		// 黒い石を二つ並びの数、穴なし、ブロックなし： * B B *
int blackClosedTwo;		// 黒い石を二つ並びの数、穴あり (B * B) か白い石のブロック (W B B *) か盤の端にブロック (E B B *)
int whiteOpenFour;		// 白い石を四つ並びの数、黒と同様
int whiteClosedFour;	// 白い石を四つ並びの数、黒と同様
int whiteOpenThree;		// 白い石を三つ並びの数、黒と同様
int whiteClosedThree;	// 白い石を三つ並びの数、黒と同様
int whiteOpenTwo;		// 白い石を二つ並びの数、黒と同様
int whiteClosedTwo;		// 白い石を二つ並びの数、黒と同様

int blackPlayer;
int whitePlayer;

int potentialEvaluation[ BOARDSIZE + 1 ][ BOARDSIZE + 1 ] = 
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0 },
};

int alphaBetaSearch( int depth, int toMove, int alpha, int beta );
int evaluate( int side, int nextToMove, int depth ) ;
int fiveInRowCheck( int lastX, int lastY, int color ) ;
int flip( int toMove ) ;
void blackConnectionAdmin( int connectNo, int blocked, int openSquare ) ;
void whiteConnectionAdmin( int connectNo, int blocked, int openSquare ) ;
int decideComputerMove( int toMove );

void ReijerAI_decideNextLocation(StoneLocation *to, GameEnvironment *env)
{
	int x, y;
	
	for(y = 0; y < BOARD_SIZE; y++){
		for(x = 0; x < BOARD_SIZE; x++){
			gomokuBoard[ x + 1 ][ y + 1 ] = env->mainBoard[y * BOARD_SIZE + x];
		}
	}
	
	decideComputerMove(env->currentColor);
	to->y = nextMoveY - 1;
	to->x = nextMoveX - 1;
}

// COMの手を決める．これは五目並べプログラムのAIの部分．
int decideComputerMove( int toMove ) 
{
	// toMove: 最終的に動かす決定をしたい石の色
	int score;

	rootToMove = toMove;

	score = alphaBetaSearch( 0, rootToMove, -INFINITYVAL, INFINITYVAL );
	printf( "Best move evaluation: %d\n", score );

	return 1;
}

int alphaBetaSearch( int depth, int toMove, int alpha, int beta ) 
{
	int score, eval;
	int x, y;

	// 最大深さになったら局面を評価する
	if( depth == MAXDEPTH )
		return evaluate( rootToMove, toMove, depth );

	// MaxプレイヤーとMinプレイヤーの最高評価を初期化
	if( toMove == rootToMove )
		score = -INFINITYVAL;
	else
		score = INFINITYVAL;

	// Generate all the moves by putting stones of the right color on the empty squares
	// 空いているマスに正しい色の石を置いて、すべての手を作成
	for( x = 1; x <= BOARDSIZE; x++ ) 
	{
		for( y = 1; y <= BOARDSIZE; y++ ) 
		{
			if( gomokuBoard[ x ][ y ] == EMPTY_SQUARE ) 
			{
				gomokuBoard[ x ][ y ] = toMove;
				// この手で五目並べになったかどうかをチェック
				if( fiveInRowCheck( x, y, toMove ) ) 
				{
					if( rootToMove == toMove ) 
					{	
						// Maxプレイヤーの五目並べ
						// 手を戻す
						gomokuBoard[ x ][ y ] = EMPTY_SQUARE;
						// 勝ちになった手は探索の初期局面にあったので手を保存する
						if( depth == 0 ) 
						{
							nextMoveX = x;
							nextMoveY = y;
						}
						return WINNING - depth;			// 浅い探索の勝ちは深い探索の勝ちより良い
					}
					else 
					{		
						// Minプレイヤーの五目並べ
						// 手を戻す
						gomokuBoard[ x ][ y ] = EMPTY_SQUARE;
						return -( WINNING - depth );	// 浅い探索の勝ちは深い探索の勝ちより良い
					}
				}
				else 
				{
					// alpha-beta探索を再帰的に呼ぶ
					eval = alphaBetaSearch( depth + 1, flip( toMove ), alpha, beta );

					// 手を戻す
					gomokuBoard[ x ][ y ] = EMPTY_SQUARE;

					if( rootToMove == toMove ) 
					{
						// この局面はMaxプレイヤーの手番．探索の結果は現在の最大評価より高いならば最大評価を更新
						if( eval > score ) 
						{
							score = eval;
							// 最善手は探索の初期局面にあったので手を保存する
							if( depth == 0 ) 
							{
								nextMoveX = x;
								nextMoveY = y;
							}
						}

						// Beta枝刈り
						if( score >= beta )
							return score;
						// alphaを更新
						if( score > alpha )
							alpha = score;
					}
					else 
					{
						// この局面はMinプレイヤーの手番．探索の結果は現在の最低評価より低いならば最低評価を更新
						if( eval < score )
							score = eval;

						// Alpha枝刈り
						if( score <= alpha )
							return score;
						// Betaを更新
						if( beta < score )
							beta = score;
					}
				}
			}
		}
	}
	return score;
}

int evaluate( int side, int nextToMove, int depth ) 
{
	int eval = 0;
	int x, y;
	int connectNo;			// 連続の同じ色の石の数
	int openSquare;
	int emptySqCon;			// 石の連結に穴がある
	int blocked;			// 石の連結はブロックされている
	int countX, countY;

	blackOpenFour = 0;
	blackClosedFour = 0;
	blackOpenThree = 0;
	blackClosedThree = 0;
	blackOpenTwo = 0;
	blackClosedTwo = 0;
	whiteOpenFour = 0;
	whiteClosedFour = 0;
	whiteOpenThree = 0;
	whiteClosedThree = 0;
	whiteOpenTwo = 0;
	whiteClosedTwo = 0;

	for( x = 1; x <= BOARDSIZE; x++ ) 
	{
		for( y = 1; y <= BOARDSIZE; y++ ) 
		{

			if(gomokuBoard[ x ][ y ] == BLACK_STONE ) 
			{
				// このマスから黒い石を数える

				// 北西方向
				if( ( x > 1 && y > 1 && gomokuBoard[ x - 1 ][ y - 1 ] == BLACK_STONE ) ||
					( x > 2 && y > 2 && gomokuBoard[ x - 1 ][ y - 1 ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y - 2 ] == BLACK_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || y == BOARDSIZE || gomokuBoard[ x + 1 ][ y + 1 ] == WHITE_STONE ) 
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					for( countX = x - 1, countY = y - 1; countX >= 1 && countY >= 1; countX--, countY-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countX < 1 || countY < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					blackConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 北方向
				if( ( y > 1 && gomokuBoard[ x ][ y - 1 ] == BLACK_STONE ) ||
					( y > 2 && gomokuBoard[ x ][ y - 1 ] == EMPTY_SQUARE && gomokuBoard[ x ][ y - 2 ] == BLACK_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( y == BOARDSIZE || gomokuBoard[ x ][ y + 1 ] == WHITE_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					countX = x;
					for( countY = y - 1; countY >= 1; countY-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
							break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countY < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					blackConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 北東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || y == BOARDSIZE || gomokuBoard[ x - 1 ][ y + 1 ] == WHITE_STONE )
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				for( countX = x + 1, countY = y - 1; countX <= BOARDSIZE && countY >= 1; countX++, countY-- ) 
				{
					if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE || countY < 1 )
					blocked++;
				// 石の連結の種類の数を更新
				blackConnectionAdmin( connectNo, blocked, emptySqCon );

				// 東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || gomokuBoard[ x - 1 ][ y ] == WHITE_STONE )
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				countY = y;
				for( countX = x + 1; countX <= BOARDSIZE; countX++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				blackConnectionAdmin(connectNo, blocked, emptySqCon);

				// 南東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || y == 1 || gomokuBoard[ x - 1 ][ y - 1 ] == WHITE_STONE )
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				for( countX = x + 1, countY = y + 1; countX <= BOARDSIZE && countY <= BOARDSIZE; countX++, countY++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE )
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE || countY > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				blackConnectionAdmin( connectNo, blocked, emptySqCon );

				// 南方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( y == 1 || gomokuBoard[ x ][ y - 1 ] == WHITE_STONE )
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				countX = x;
				for( countY = y + 1; countY <= BOARDSIZE; countY++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}
				
				// 石の連結は端までになっているかどうか
				if( countY > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				blackConnectionAdmin( connectNo, blocked, emptySqCon );

				// 南西方向
				if( ( x > 1 && y < BOARDSIZE && gomokuBoard[ x - 1 ][ y + 1 ] == BLACK_STONE ) ||
					( x > 2 && y < BOARDSIZE - 1 && gomokuBoard[ x - 1 ][ y + 1 ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y + 2 ] == BLACK_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || y == 1 || gomokuBoard[ x + 1 ][ y - 1 ] == WHITE_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					for( countX = x - 1, countY = y + 1; countX >= 1 && countY <= BOARDSIZE; countX--, countY++ ) 
					{
						if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}
					// 石の連結は端までになっているかどうか
					if( countX < 1 || countY > BOARDSIZE ) 
						blocked++;
					// 石の連結の種類の数を更新
					blackConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 西方向
				if( ( x > 1 && gomokuBoard[ x - 1 ][ y ] == BLACK_STONE ) ||
					( x > 2 && gomokuBoard[ x - 1 ][ y ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y ] == BLACK_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || gomokuBoard[ x + 1 ][ y ] == WHITE_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					countY = y;
					for( countX = x - 1; countX >= 1; countX-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}
					// 石の連結は端までになっているかどうか
					if( countX < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					blackConnectionAdmin( connectNo, blocked, emptySqCon );
				}

			}
			else if( gomokuBoard[ x ][ y ] == WHITE_STONE ) 
			{
				// このマスから黒い石を数える

				// 北西方向
				if( ( x > 1 && y > 1 && gomokuBoard[ x - 1 ][ y - 1 ] == WHITE_STONE ) ||
					( x > 2 && y > 2 && gomokuBoard[ x - 1 ][ y - 1 ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y - 2 ] == WHITE_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || y == BOARDSIZE || gomokuBoard[ x + 1 ][ y + 1 ] == BLACK_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					for( countX = x - 1, countY = y - 1; countX >= 1 && countY >= 1; countX--, countY-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countX < 1 || countY < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					whiteConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 北方向
				if( ( y > 1 && gomokuBoard[ x ][ y - 1 ] == WHITE_STONE ) ||
					( y > 2 && gomokuBoard[ x ][ y - 1 ] == EMPTY_SQUARE && gomokuBoard[ x ][ y - 2 ] == WHITE_STONE ) )
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( y == BOARDSIZE || gomokuBoard[ x ][ y + 1 ] == BLACK_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					countX = x;
					for( countY = y - 1; countY >= 1; countY-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countY < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					whiteConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 北東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || y == BOARDSIZE || gomokuBoard[ x - 1 ][ y + 1 ] == BLACK_STONE )
					blocked = 1;
				else
					blocked = 0;
				
				// 石の連結を数える
				for( countX = x + 1, countY = y - 1; countX <= BOARDSIZE && countY >= 1; countX++, countY-- ) 
				{
					if( gomokuBoard[ countX ][ countY ] == WHITE_STONE) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE || countY < 1 )
					blocked++;
				// 石の連結の種類の数を更新
				whiteConnectionAdmin( connectNo, blocked, emptySqCon );

				// 東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || gomokuBoard[ x - 1 ][ y ] == BLACK_STONE ) 
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				countY = y;
				for( countX = x + 1; countX <= BOARDSIZE; countX++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				whiteConnectionAdmin( connectNo, blocked, emptySqCon );

				// 南東方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( x == 1 || y == 1 || gomokuBoard[ x - 1 ][ y - 1 ] == BLACK_STONE )
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				for( countX = x + 1 , countY = y + 1; countX <= BOARDSIZE && countY <= BOARDSIZE; countX++, countY++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countX > BOARDSIZE || countY > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				whiteConnectionAdmin( connectNo, blocked, emptySqCon );

				// 南方向（この場合にダブルカウントはないので確認しなくてよい）
				connectNo = 1;
				openSquare = 0;
				emptySqCon = 0;

				// 石の連結は盤の端か相手の石にブロックされている
				if( y == 1 || gomokuBoard[ x ][ y - 1 ] == BLACK_STONE ) 
					blocked = 1;
				else
					blocked = 0;

				// 石の連結を数える
				countX = x;
				for( countY = y + 1; countY <= BOARDSIZE; countY++ ) 
				{
					if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
					{
						connectNo++;
						if( openSquare == 1 )
							emptySqCon = 1;
					}
					else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
					{
						blocked++;
						break;
					}
					else 
					{
						if( openSquare != 1 )
							openSquare++;
							break;
					}
				}

				// 石の連結は端までになっているかどうか
				if( countY > BOARDSIZE )
					blocked++;
				// 石の連結の種類の数を更新
				whiteConnectionAdmin( connectNo, blocked, emptySqCon );

				// 南西方向
				if( ( x > 1 && y < BOARDSIZE && gomokuBoard[ x - 1 ][ y + 1 ] == WHITE_STONE ) ||
					( x > 2 && y < BOARDSIZE - 1 && gomokuBoard[ x - 1 ][ y + 1 ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y + 2 ] == WHITE_STONE ) )  
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || y == 1 || gomokuBoard[ x + 1 ][ y - 1 ] == BLACK_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					for( countX = x - 1, countY = y + 1; countX >= 1 && countY <= BOARDSIZE; countX--, countY++ ) 
					{
						if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countX < 1 || countY > BOARDSIZE )
						blocked++;
					// 石の連結の種類の数を更新
					whiteConnectionAdmin( connectNo, blocked, emptySqCon );
				}

				// 西方向
				if( ( x > 1 && gomokuBoard[ x - 1 ][ y ] == WHITE_STONE ) ||
					( x > 2 && gomokuBoard[ x - 1 ][ y ] == EMPTY_SQUARE && gomokuBoard[ x - 2 ][ y ] == WHITE_STONE ) ) 
				{
					// この方向の石は既にカウントされたので何もしない
				}
				else 
				{
					connectNo = 1;
					openSquare = 0;
					emptySqCon = 0;

					// 石の連結は盤の端か相手の石にブロックされている
					if( x == BOARDSIZE || gomokuBoard[ x + 1 ][ y ] == BLACK_STONE )
						blocked = 1;
					else
						blocked = 0;

					// 石の連結を数える
					countY = y;
					for( countX = x - 1; countX >= 1; countX-- ) 
					{
						if( gomokuBoard[ countX ][ countY ] == WHITE_STONE ) 
						{
							connectNo++;
							if( openSquare == 1 )
								emptySqCon = 1;
						}
						else if( gomokuBoard[ countX ][ countY ] == BLACK_STONE ) 
						{
							blocked++;
							break;
						}
						else 
						{
							if( openSquare != 1 )
								openSquare++;
								break;
						}
					}

					// 石の連結は端までになっているかどうか
					if( countX < 1 )
						blocked++;
					// 石の連結の種類の数を更新
					whiteConnectionAdmin( connectNo, blocked, emptySqCon );
				}

			}

			// 盤の中央に近い石の評価を高くする
			if( side == BLACK && gomokuBoard[ x ][ y ] == BLACK_STONE )
				eval += potentialEvaluation[ x ][ y ];
			else if( side == WHITE && gomokuBoard[ x ][ y ] == WHITE_STONE ) 
				eval -= potentialEvaluation[ x ][ y ];
		}
	}

	// 連結に値を付ける
	if( ( blackOpenFour || blackClosedFour ) && nextToMove == BLACK )
		eval = WINNING - ( depth + 1 );				// 黒の手番と長さ４の連結あり、浅い勝ちは深い勝ちより良い
	else if( ( whiteOpenFour || whiteClosedFour ) && nextToMove == WHITE )
		eval = -( WINNING - ( depth + 1 ) );		// 白の手番と長さ４の連結あり、浅い勝ちは深い勝ちより良い
	else if( blackOpenFour >= 1 )
		eval = WINNING - ( depth + 2 );				// 黒のOpen Four、浅い勝ちは深い勝ちより良い
	else if( whiteOpenFour >= 1 )
		eval = -( WINNING - ( depth + 2 ) );		// 白のOpen Four、浅い勝ちは深い勝ちより良い
	else if( blackClosedFour >= 2 )
		eval = WINNING - ( depth + 2 );				// 黒は連結４を二つ以上がある、浅い勝ちは深い勝ちより良い
	else if( whiteClosedFour >= 2 )
		eval = -( WINNING - ( depth + 2 ) );		// 白は連結４を二つ以上がある、浅い勝ちは深い勝ちより良い
	else {
		// 最強のAIかテストAIの評価値を使うか
		if( (rootToMove == BLACK && blackPlayer == BEST_AI) ||
			(rootToMove == WHITE && whitePlayer == BEST_AI ) ) {
			//　勝ち以外の連結の評価（黒）
			eval += blackClosedFour * bestEvalValues[ CLOSED_FOUR_VALUE ];
			eval += blackOpenThree * bestEvalValues[ OPEN_THREE_VALUE ];
			eval += blackClosedThree * bestEvalValues[ CLOSED_THREE_VALUE ];
			eval += blackOpenTwo * bestEvalValues[ OPEN_TWO_VALUE ];
			eval += blackClosedTwo * bestEvalValues[ CLOSED_TWO_VALUE ];

			//　勝ち以外の連結の評価（白）
			eval -= whiteClosedFour * bestEvalValues[ CLOSED_FOUR_VALUE ];
			eval -= whiteOpenThree * bestEvalValues[ OPEN_THREE_VALUE ];
			eval -= whiteClosedThree * bestEvalValues[ CLOSED_THREE_VALUE ];
			eval -= whiteOpenTwo * bestEvalValues[ OPEN_TWO_VALUE ];
			eval -= whiteClosedTwo * bestEvalValues[ CLOSED_TWO_VALUE ];

			// 手番にボーナス
			if( nextToMove == BLACK )
			{
				eval += bestEvalValues[ INITIATIVE ];
			}
			else
			{
				eval -= bestEvalValues[ INITIATIVE ];
			}

		}
		else {
			//　勝ち以外の連結の評価（黒）
			eval += blackClosedFour * testValues[ CLOSED_FOUR_VALUE ];
			eval += blackOpenThree * testValues[ OPEN_THREE_VALUE ];
			eval += blackClosedThree * testValues[ CLOSED_THREE_VALUE ];
			eval += blackOpenTwo * testValues[ OPEN_TWO_VALUE ];
			eval += blackClosedTwo * testValues[ CLOSED_TWO_VALUE ];

			//　勝ち以外の連結の評価（白）
			eval -= whiteClosedFour * testValues[ CLOSED_FOUR_VALUE ];
			eval -= whiteOpenThree * testValues[ OPEN_THREE_VALUE ];
			eval -= whiteClosedThree * testValues[ CLOSED_THREE_VALUE ];
			eval -= whiteOpenTwo * testValues[ OPEN_TWO_VALUE ];
			eval -= whiteClosedTwo * testValues[ CLOSED_TWO_VALUE ];

			// 手番にボーナス
			if( nextToMove == BLACK )
			{
				eval += testValues[ INITIATIVE ];
			}
			else
			{
				eval -= testValues[ INITIATIVE ];
			}

		}

	}

	// 手を変えるために乱数を追加
	eval += ( rand() % MAXRANDBONUS );
	
	// 白の評価だったら正負逆
	if( side == WHITE )
		return -eval;

	return eval;
}

// 最後の手の情報を利用し、五目並べになったかどうかをチェック
int fiveInRowCheck( int lastX, int lastY, int color ) 
{
	int x, y;
	int connectNo = 1;

	// 横の五目並べチェック:(lastx, lasty)座標からcolorと同じ色の石の連結を右と左にあわせて数える
	if( lastX > 1 ) 
	{
		for( x = lastX - 1; gomokuBoard[ x ][ lastY ] == color; x-- ) 
		{
			connectNo++;
			if( x == 1 )
				break;
		}
	}
	if( lastX < BOARDSIZE ) 
	{
		for( x = lastX + 1; gomokuBoard[ x ][ lastY ] == color; x++ ) 
		{
			connectNo++;
			if( x == BOARDSIZE )
				break;
		}
	}
	if( connectNo >= 5 ) 
		return 1;			// 横の五目並べ見つけた

	// 縦の五目並べチェック:(lastx, lasty)座標からcolorと同じ色の石の連結を上と下にあわせて数える
	connectNo = 1;
	if( lastY > 1 ) 
	{
		for( y = lastY - 1; gomokuBoard[ lastX ][ y ] == color; y-- ) 
		{
			connectNo++;
			if( y == 1 )
				break;
		}
	}
	if( lastY < BOARDSIZE ) 
	{
		for( y = lastY + 1; gomokuBoard[ lastX ][ y ] == color; y++ ) 
		{
			connectNo++;
			if( y == BOARDSIZE )
				break;
		}
	}
	if( connectNo >= 5 ) 
		return 1;			// 縦の五目並べ見つけた

	// 斜め（北西−南東）の五目並べチェック:(lastx, lasty)座標からcolorと同じ色の石の連結を北西と南東にあわせて数える
	connectNo = 1;
	if( lastX > 1 && lastY > 1 ) 
	{
		for( x = lastX - 1, y = lastY - 1; gomokuBoard[ x ][ y ] == color; x--, y-- ) 
		{
			connectNo++;
			if( x == 1 || y == 1 )
				break;
		}
	}
	if( lastX < BOARDSIZE && lastY < BOARDSIZE ) 
	{
		for( x = lastX + 1, y = lastY + 1; gomokuBoard[ x ][ y ] == color; x++, y++ ) 
		{
			connectNo++;
			if( x == BOARDSIZE || y == BOARDSIZE )
				break;
		}
	}
	if( connectNo >= 5 ) 
		return 1;			// 斜め（北西−南東）の五目並べ見つけた
	
	// 斜め（北東−南西）の五目並べチェック:(lastx, lasty)座標からcolorと同じ色の石の連結を北東と南西にあわせて数える
	connectNo = 1;
	if(	lastX < BOARDSIZE && lastY > 1 ) 
	{
		for( x = lastX + 1, y = lastY - 1; gomokuBoard[ x ][ y ] == color; x++, y-- ) 
		{
			connectNo++;
			if( x == BOARDSIZE || y == 1 )
				break;
		}
	}
	if( lastX > 1 && lastY < BOARDSIZE ) 
	{
		for( x = lastX - 1, y = lastY + 1; gomokuBoard[ x ][ y ] == color; x--, y++ ) 
		{
			connectNo++;
			if( x == 1 || y == BOARDSIZE )
				break;
		}
	}
	if( connectNo >= 5 ) 
		return 1;			// 斜め（北東−南西）の五目並べ見つけた

	return 0;
}

int flip( int toMove ) 
{
	if( toMove == BLACK )
		return WHITE;
	else if( toMove == WHITE )
		return BLACK;
	return 0;
}

// 石の連結の種類の数を更新（黒の場合）
void blackConnectionAdmin( int connectNo, int blocked, int openSquare ) 
{
	if( connectNo == 4 ) 
	{
		// 長さ４の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			blackOpenFour++;
		else if( blocked == 0 && openSquare == 1 )
			blackClosedFour++;
		else if( blocked == 1 && openSquare == 0 )
			blackClosedFour++;
		else if( blocked == 1 && openSquare == 1 )
			blackClosedFour++;
		else if( blocked == 2 && openSquare == 1 )
			blackClosedFour++;
	}
	else if( connectNo == 3 ) 
	{
		// 長さ３の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			blackOpenThree++;
		else if( blocked == 0 && openSquare == 1 )
			blackClosedThree++;
		else if( blocked == 1 && openSquare == 0 )
			blackClosedThree++;
		else if( blocked == 1 && openSquare == 1 )
			blackClosedThree++;
	}
	else if( connectNo == 2 ) 
	{
		// 長さ２の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			blackOpenTwo++;
		else if( blocked == 0 && openSquare == 1 )
			blackClosedTwo++;
		else if( blocked == 1 && openSquare == 0 )
			blackClosedTwo++;
		else if( blocked == 1 && openSquare == 1 )
			blackClosedTwo++;
	}
}

// 石の連結の種類の数を更新（白の場合）
void whiteConnectionAdmin( int connectNo, int blocked, int openSquare ) 
{
	if( connectNo == 4 ) 
	{
		// 長さ４の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			whiteOpenFour++;
		else if( blocked == 0 && openSquare == 1 )
			whiteClosedFour++;
		else if( blocked == 1 && openSquare == 0 )
			whiteClosedFour++;
		else if( blocked == 1 && openSquare == 1 )
			whiteClosedFour++;
		else if( blocked == 2 && openSquare == 1 )
			whiteClosedFour++;
	}
	else if( connectNo == 3 ) 
	{
		// 長さ３の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			whiteOpenThree++;
		else if( blocked == 0 && openSquare == 1 )
			whiteClosedThree++;
		else if( blocked == 1 && openSquare == 0 )
			whiteClosedThree++;
		else if( blocked == 1 && openSquare == 1 )
			whiteClosedThree++;
	}
	else if( connectNo == 2 ) 
	{
		// 長さ２の連結を発見した
		if( blocked == 0 && openSquare == 0 )
			whiteOpenTwo++;
		else if( blocked == 0 && openSquare == 1 )
			whiteClosedTwo++;
		else if( blocked == 1 && openSquare == 0 )
			whiteClosedTwo++;
		else if( blocked == 1 && openSquare == 1 )
			whiteClosedTwo++;
	}
}
