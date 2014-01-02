
#include "modellist.h"
#include "interface.h"
#include "nml.h"
#include "csv.h"
#include "npmath.h"


//モデルリスト
ModelList* ModelList::mInstance=0;
ModelList::ModelList() {
};

ModelList::~ModelList() {
}
void ModelList::create() {
	if (mInstance == 0) {
		mInstance = new ModelList;
	}
}
void ModelList::destroy() {
	if (mInstance) {
		delete mInstance;
		mInstance=0;
	}
}
ModelList* ModelList::instance() {
	return mInstance;
}
void ModelList::ready() {
	//メンバーをready
	mSystem.ready();
	mChara.ready();
	mSoundSet.load("src/system/systemsound.txt");
	mBack.load("src/picture/back",true);
};
/////////////////////////////////////////////////////////////////////////

void ModelList::Chara::ready() {
	mTexture.load("src/picture/chara",true,false);
	mUFO.load(0,true,false,"src/picture/UFO");
	mBoss.load("src/picture/非想天則",true);

}
/////////////////////////////////////////////////////////////////////////
ModelList::System::System() {
	mCursor.load("src/system/cursor",true,false);
	mFrame.load("src/system/frame",true,false);
	mRect.load("src/system/rect",true,false);
	mGauge.load("src/system/gauge",true,false);
};
void ModelList::System::ready() {
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelList::System::Number::Number() {
	mTexture.load("src/system/number",true,true);

	for (int i=0 ; i<10 ; ++i) {
		MyString tmp;
		tmp = i;
		mModel[i] = mTexture.model(tmp.data());
	}
	mMiss =  mTexture.model("miss");

	//mWidth = 46;
	mWidth = 36;
	//mHeight = 88;
}

void ModelList::System::Number::draw(int number,const GameLib::Math::Vector3& pos,unsigned int color) const{
	//桁数を調べる
	int od = 0;
	for (int i=1 ; number>=i ;i*=10) {
		++od;
	};
	BlendInfo bi;

	for (int i=0 ; i<od ; ++i) {
		int p = mWidth*(od-1-i);
		int n = number%10;
		number /= 10;

		mModel[n]->draw(
			pos.x+p+mWidth/2,
			pos.y-48,
			pos.z,bi,0);
	}
};

void ModelList::System::Number::create(Sprite* sprite,int number,const GameLib::Math::Vector3& POS, unsigned int color,bool big,int height) const{

	//スクリーン座標をもらう
	GameLib::Math::Vector3 pos;
	Draw::instance()->getScreenCoordinate(&pos,POS);

	float size = 0.5f;
	if (big) {
		size += 0.25f;
	}

	//ミス
	if (number<0)  {
		Object* obj = sprite->addObject(mMiss);
		obj->setPosition(pos);
		obj->setColor(color);
		obj->setSize(size);
		obj->setVelocity(0,height*20,0);
		obj->setResistance(0,0.4,0);
		//obj->mLimit = 60;
		obj->mAutoRemove = 1;
		obj->mSortReverse = true;
	}
	else {
		//桁数を調べる
		int od = 0;
		for (int i=1 ; number>=i ;i*=10) {
			++od;
		};
		//int width = 25;
		const int width = mWidth * size;

		for (int i=0 ; i<od ; ++i) {
			int p = -width*i + ((od-1)*width/2);

			int n = number%10;
			number /= 10;

			Object* obj = sprite->addObject(mModel[n]);

			obj->setPosition(pos.x + p,pos.y,pos.z);
			obj->setColor(color);
	
			if (i < od-1) {
				obj->setSize(size,size,1.f);
			}
			else {
				obj->setSize(size+0.1f,size+0.1f,1.f);
			}

			obj->setVelocity(0,height*20,0);
			obj->setResistance(0,0.4,0);

			obj->mAutoRemove = 1;
			obj->mSortReverse = true;
		}
	}
};

