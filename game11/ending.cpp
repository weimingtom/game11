
#include "ending.h"


Ending::Ending() {
	mCount = 0;

	mCGA.load("src/picture/�G���f�B���OA",false);
	mCGB.load("src/picture/�G���f�B���OB",false);

	Model* m;
	mCast.addInitializing("�@�@�@�@�@�@�@Cast");
	mCast.addInitializing("�@�@�@�@�@ �@Yousei",ModelList::instance()->mChara.mTexture.model("�d��"));
	mCast.addInitializing("�@�@�@ �@Hakurei Reimu",ModelList::instance()->mChara.mTexture.model("�얲"));
	mCast.addInitializing("�@�@�@  Kirisame Marisa",ModelList::instance()->mChara.mTexture.model("������"));
	mCast.addInitializing("�@�@�@ �@Kochiya Sanae",ModelList::instance()->mChara.mTexture.model("���c"));
	mCast.addInitializing("�@�@ �@�@Hinanai Tenshi",ModelList::instance()->mChara.mTexture.model("�V�q"));
	mCast.addInitializing("�@�@ �@�@�@Nagae Iku",ModelList::instance()->mChara.mTexture.model("�ߋ�"));
	mCast.addInitializing("�@  �@�@Watatuki Yorihime",ModelList::instance()->mChara.mTexture.model("�˕P"));
	mCast.addInitializing("  �@�@�@Watatuki Toyohime",ModelList::instance()->mChara.mTexture.model("�L�P"));
	mCast.addInitializing("               UFO",ModelList::instance()->mChara.mUFO.model(1));
	mCast.addInitializing("  �@�@�@�@�@�@Unzan",ModelList::instance()->mChara.mTexture.model("�_�R"));
	mCast.addInitializing("�@�@  �@�@Hisoutensoku",ModelList::instance()->mChara.mBoss.model());
	mCast.addInitializing("�@�@�@  �@�@Seirensen",ModelList::instance()->mChara.mTexture.model("���@"));
	mCast.addInitializing("�@�@�@�@�a�f�l�@�N���Q�m�{�[��");
	mCast.addInitializing("���ʉ��f�ށ@�U�E�}�b�`���C�J�@�Y");
	mCast.addInitializing("�@�@�@�@�@���̑��@�d����");
	mCast.addInitializing("�@�@�@����@2010���ꑕ�b��");
	mCast.addInitializing("�@�@�@�@�@�s�g�d�@�d�m�c");

	//BGM::instance()->change(-1);
	Draw::instance()->mCamera.setTarget(0,0,0);
}

int Ending::update(GameData* gamedata){
	Draw::instance()->mCamera.update();
	++mCount;

	if (mCount < 240) {
		BlendInfo bi;
		mCGB.draw(0.f,-64.f,0.f,bi);
//512-384
		if (mCount < 180) {
			mCGA.draw(0.f,-64.f,0.f,bi);
		}

		return 0;
	}
	else {
		if (mCast.number()) {
			if (mCast(0)->update()) {
				mCast.remove(0);
			}
			return 0;
		};
	}



	return 1;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ending::Cast::Cast(const char * name) {
	mModel = 0;
	mName = name;
	mCount = 0;
};
Ending::Cast::Cast(const char * name,Model* model) {
	ASSERT(model);
	mModel = model;
	mName = name;
	mCount = 0;
};
int Ending::Cast::update() {
	Draw::instance()->mCamera.update();

	float x;
	if (mCount < 15) {
		x = (15-mCount);
	}
	else {
		if (mCount < 45) {
			x = 0.f;
		}
		else {
			x = (45 - mCount);
		}
	}
	x *= 40.f;
	if (mModel == 0) {
		Draw::instance()->text(x-8*20,0.f,mName.data(),0xffffffff,1);
	}
	if (mModel) {
		Draw::instance()->text(x-8*20,-32.f,mName.data(),0xffffffff,1);

		BlendInfo bi;

		GameLib::Math::Matrix34 mtx;
		mtx.setIdentity();
		GameLib::Math::Matrix34 wm;
		mModel->getWorldMatrix(&wm,mtx,bi.mBlendElement,mCount);

		GameLib::Math::Vector3 pos;
		GameLib::Math::Vector3 center(0.f);
		wm.mul(&pos,center);

		pos *= -1.f * Draw::mWidth/2;
		pos.x += x;

		mModel->draw(pos,bi,mCount);
	}


	++mCount;
	if (mCount > 60) {
		return 1;
	}
	return 0;
};


