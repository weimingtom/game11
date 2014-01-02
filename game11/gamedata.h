#ifndef INCLUDED_GAMEDATA_H
#define INCLUDED_GAMEDATA_H

#include "interface.h"
class Stage;
class Ending;

class GameData{
	Ending* mEnding;
public:
	int mID;

	//フェーズ
	Stage* mStage;				//スクリプトからの操作を受け付けるため

	GameData();
	~GameData();
	void release();
	int update();
	void reset(int mode);			//0プレーン　1最初から　2デバッグ
	void draw(int x,int y);		//点・power情報表示 Flagを扱うのでconstにできない
	void drawHistory(int x,int y);//プレイ履歴表示
};

#endif

