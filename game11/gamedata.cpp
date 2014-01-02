
#include "gamedata.h"
#include "stage.h"
#include "ending.h"
#include "game.h"

GameData::GameData() {
	mStage = 0;
	mEnding = 0;
	mID = 0;

	//セーブリストなどで複数作る場合があるため、シーケンスはコンストラクタの時点では作成しない
	reset(0);
};

GameData::~GameData() {
	release();
}

void GameData::release() {
	if (mStage) {
		delete mStage;
		mStage = 0;
	}
	if (mEnding) {
		delete mEnding;
		mEnding = 0;
	}
};
void GameData::reset(int mode) {
	release();

	if (mode == 0){
		mID = 0;
	}
	if (mode == 1) {
		//ゲーム開始時データ
		mID = Random::instance()->getInt();
	}

	//デバッグ用データ
	if (mode < 0) {
		mID = Random::instance()->getInt();
	}
}

int GameData::update() {
	int result = 0;

	if (mStage) {
		if (mStage->update(this)) {
			delete mStage;
			mStage = 0;

			mEnding = new Ending;
		}
		return 0;
	}
	if (mEnding) {
		if (mEnding->update(this)) {
			delete mEnding;
			mEnding = 0;
			Game::instance()->mReset = true;
		}
		return 0;
	}
	//mEnding = new Ending();
	mStage = new Stage(this);
	return 0;
};



void GameData::draw(int x,int y){

	Message::position(x,y);
	Message::color(0xffffffff);
	Message::size(1);
	

	Message::draw("Score");
	Message::draw("Power");

	Message::position(x+64,y);
	Message::draw("");
};
