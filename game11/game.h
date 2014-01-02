#ifndef INCLUDED_GAME9SEQ_GAME_H
#define INCLUDED_GAME9SEQ_GAME_H

#include "gamedata.h"
#include "title.h"

//ゲームの根元しーくえんす
//タイトル
//ステージパート

class Game {
public:
	GameData* mGameData;
	Title* mTitle;
	
	int mCount;
	bool mReset;

	int update();
	void reset();			//タイトルへもどる
	static Game* instance();
	static void create();
	static void destroy();
private:
	static Game* mInstance;
	Game();
	~Game();
	void release();

};




#endif
