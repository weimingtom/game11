#include "title.h"




Title::Title() {
	mLogo.load("src/picture/logo",true);
	mWindow.centerPosition(0,-160);
	mWindow.addChoice("Start",0);
	mWindow.addChoice("Config",2);
	mWindow.addChoice("Quit",3);

	if (existSaveFile()) {
		mWindow.focus(1);
	}

	mConfig = 0;
	Draw::instance()->aimFade(0);
	//BGM::instance()->change("title");

	mCount = 0;

	Draw::instance()->mCamera.initialize();
	Draw::instance()->mCamera.aimTarget(0,-240,0);
	BGM::instance()->change(-1);

}

Title::~Title() {
	if (mConfig) {
		delete mConfig;
		mConfig = 0;
	}
};


int Title::update(GameData* result) {
	BlendInfo bi;
	ModelList::instance()->mBack.model()->draw(0.f,0.f,0.f,bi);

	mLogo.model()->draw(0.f,40.f,0.f,bi);

	Draw::instance()->mCamera.update();
	//なんとなく、randomを回してやる
	Random::instance()->getInt();

	if (mConfig ) {
		int r = mConfig->update();
		if (r) {
			delete mConfig;
			mConfig = 0;
		}
		return 0;
	}


	mWindow.draw();
	int r = mWindow.update();
	if (r == 1) {
		int p = mWindow.getParameter();
		if (p==0) {
			if (cKey::isOn(4) && Main::mDebugMode) {
				result->reset(-1);		//デバッグ
			}
			else {
				result->reset(1);
			}
			return 1;
		}

		if (p==2) {
			mConfig = new sConfig;
		}
			if (p==3) {	
			return 2;
		};
	};

	++mCount;
	return 0;
};


