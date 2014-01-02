

#include "game.h"
#include "npsysc.h"
#include "npstring.h"
#include "npdraw.h"
#include "npinput.h"
#include "npobject.h"
#include "npfile.h"

//#include "title.h"
//#include "chaptermenu.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
Game* Game::mInstance=0;
Game* Game::instance() {
	return mInstance;
}
Game::Game() {
	mCount=0;
	mReset = false;

	mGameData = new GameData;;
	mTitle = new Title;
}

Game::~Game() {
	release();
}
void Game::release() {
	mReset = false;
	if (mGameData) {
		delete mGameData;
		mGameData = 0;
	}
	if (mTitle) {
		delete mTitle;
		mTitle = 0;
	}
};

void Game::create() {
	if (mInstance==0) {
		mInstance= new Game;
	}
}
void Game::destroy() {
	if (mInstance) {
		delete mInstance;
		mInstance=0;
	}
}
void Game::reset() {
	mReset = true;
}

int Game::update() {
	//ƒŠƒZƒbƒg
	if (cKey::frame(23)==1) {
		reset();
	}
	if (mReset) {
		release();
		mGameData = new GameData;;
		mTitle = new Title;
	}

	if (mTitle) {
		int r=mTitle->update(mGameData);
		if (r == 1) {
			delete mTitle;
			mTitle = 0;

			Draw::instance()->setFade(0xff);
		}
		if (r == 2) {
			return 2;
		}

		return 0;
	}

	if (mGameData) {
		mGameData->update();
	}
	++mCount;

	return 0;
};


////////////////////////////////////////////////////////////////////////////////

