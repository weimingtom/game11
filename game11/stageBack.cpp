

#include "stage.h"
#include "anime.h"
#include "npmath.h"
#include "npobject.h"
#include "modellist.h"
#include "config.h"

Stage::Stage(GameData* gamedata) {
	//初期化
	Sprite::instance()->removeObject();
	//カメラ初期化
	Draw::instance()->mCamera.setAngle(45.f,45.f,0.f);
	Draw::instance()->mCamera.setTarget(0.f,0.f,0.f);

	mGameData = gamedata;
	mCount = 0;
	mLevel = gamedata->mFlagManager.getLevel();
	mSquare = 0;

	mScript = 0;
	for (int i=0 ; i<mUnitMax ; ++i) {
		mUnit[i] = 0;
	}
	for (int i=0 ; i<mGhostMax ; ++i) {
		mGhost[i] = 0;
	}
	mLastSpell = 0;


	//状態
	mResult = 0;
	mScriptRunning=true;
	mSystemMenu = 0;
	mPhase = 0;
	mNextPhase = 0;

	mCastPhase = 0;
	mUnitStatusPhase = 0;
	mKillUnitPhase = 0;
	mDeployPhase = 0;
	mOverPhase = 0;
}

Stage::~Stage() {
	if (mSystemMenu) {
		delete mSystemMenu;
		mSystemMenu = 0;
	}

	if (mPhase) {
		delete mPhase;
		mPhase = 0;
	}
	if (mCastPhase) {
		delete mCastPhase;
		mCastPhase = 0;
	}
	if (mUnitStatusPhase) {
		delete mUnitStatusPhase;
		mUnitStatusPhase = 0;
	}
	if (mKillUnitPhase) {
		delete mKillUnitPhase;
		mKillUnitPhase=0;
	}
	if (mDeployPhase) {
		delete mDeployPhase;
		mDeployPhase=0;
	}
	if (mOverPhase) {
		delete mOverPhase;
		mOverPhase = 0;
	}

	if (mSquare) {
		delete []mSquare;
		mSquare=0;
	}

	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			delete mUnit[i];
			mUnit[i] = 0;
		}
	}
	for (int i=0 ; i<mGhostMax ; ++i) {
		if (mGhost[i]) {
			delete mGhost[i];
			mGhost[i] = 0;
		}
	}
}

void Stage::set(Script::Battle* battle) {

	//地形を用意
	loadFMF(battle->mMap.data());
	//背景を用意
	//battle->mBack.data();

	mTitle = battle->mTitle;
	mScript = battle->mScript;
	mWeather.change(battle->mWeather);

	//ボーナス
	mBonus.mTool[0] = battle->mBonus[0];
	mBonus.mTool[1] = battle->mBonus[1];
	mBonus.mTool[2] = battle->mBonus[2];
	mBonus.mTool[3] = battle->mBonus[3];
	mBonus.mScore = battle->mScore;
	mBonus.mPower = battle->mPower;
}
void Stage::loadFMF(const char* file) {
	//マップ作成
	MyString tmp(64);
	tmp << "src/data/map/" << file;
	FMF fmf;
	fmf.load(tmp.data(),5);

	mWidth = fmf.mWidth;
	mHeight = fmf.mHeight;
	mTemp.create(this);
	mLuckZOC.create(this);
	mLuckZOC.mColor = 0x80808000;
	mDisturbZOC.create(this);
	mDisturbZOC.mColor = 0x80808000;

	ASSERT(mSquare==0);
	mSquare = new Square[mWidth * mHeight];

	for (int i=0 ; i<mWidth ;++i) {
		for (int j=0 ; j<mHeight ;++j) {
			int x=i;
			int y=mHeight - j -1;	//ここでY軸反転

			square(i,j)->mX=i;
			square(i,j)->mY=j;
			square(i,j)->mPit=false;
			square(i,j)->mWall=false;
			square(i,j)->mWater=false;

			if (fmf.data(2,x,y) == 255) {
				square(i,j)->mZ=-1;
			}
			else {
				//square(i,j)->mZ=fmf.data(2,x,y);		//高さ
				square(i,j)->mZ=fmf.data(2,x,y) % 256 + fmf.data(2,x,y) / 256 * 16;
			}

			//チップ
			if (fmf.data(0,x,y)==255) {
				square(i,j)->mU=0;
				square(i,j)->mV=0;
			}
			else {
				square(i,j)->mU=fmf.data(0,x,y) % 256;	//チップ
				square(i,j)->mV=fmf.data(0,x,y) / 256;
			}

			//地形タイプ
			//256通常
			//0-3坂
			//255地形
			//257壁
			//258穴
			//256水
			square(i,j)->mSlant = fmf.data(1,x,y);
			if (square(i,j)->mSlant == 257) {
				square(i,j)->mWall = true;
			}
			if (square(i,j)->mSlant == 258) {
				square(i,j)->mPit = true;
			}
			if (square(i,j)->mSlant == 259) {
				square(i,j)->mWater = true;
			}


			//イベントポイント
			int e = fmf.data(4,x,y) % 256 + fmf.data(4,x,y) / 256 * 8;
			square(i,j)->mEvent=e;
		}
	}
	createBatch();
};

//3Dマップタイプのバッチ
void Stage::createBatch() {
	mTest.load("src/picture/test",true);

	mMapChip.load("src/picture/map/chip",false);
	for (int i=0;i<mWidth ; ++i){
		for (int j=0 ; j<mHeight ; ++j) {
			square(i,j)->createBatch(this);
		}
	}
}



int Stage::getNewUnit() const{
	int result = -1;
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i] == 0 ) {
			result = i;
			break;
		}
	};
	ASSERT(result>=0);
	return result;
}
int Stage::getNewGhost() const{
	int result = -1;
	for (int i=0 ; i<mGhostMax ; ++i) {
		if (mGhost[i] == 0 ) {
			result = i;
			break;
		}
	};
	ASSERT(result>=0);
	return result;
}
Ghost* Stage::pick(dShell* shell) {
	Ghost* result = 0;
	for (int i=0 ; i<mGhostMax ; ++i) {
		if (mGhost[i]->mShell == shell) {
			result = mGhost[i];
			mGhost[i] = 0;
			break;
		}
	}
	return result;
};


Unit* Stage::launchUnit(Ghost* ghost, int pos,int party,bool control,int wait) {
	return launchUnit(ghost,
		square(pos)->mX,
		square(pos)->mY,
		party,control,wait);
}
Unit* Stage::launchUnit(Ghost* ghost,int x,int y,int party,bool control,int wait) {
	int i= getNewUnit();
	mUnit[i] = new Unit(*ghost,this);

	mUnit[i]->mGhost->mState = 1;	//出撃中にしておく

	mUnit[i]->mParty = party;
	mUnit[i]->mControl= control;
	mUnit[i]->mWait = wait;

	mUnit[i]->recalculation1();
	mUnit[i]->recalculation2();

	mUnit[i]->launch(x,y);

	return mUnit[i];
};

Unit* Stage::createUnit(dShell* shell,int level,int pos,int party,bool control,int wait,int abilityMode,int rank,int difficulty,dTool* drop,int dropProbability) {
	int i= getNewGhost();
	mGhost[i] = new Ghost;
	mGhost[i]->mShell =  shell;
	mGhost[i]->mExperience = mGhost[i]->totalExperience(level);
	mGhost[i]->mRank = rank;
	mGhost[i]->mDifficulty = difficulty;
	mGhost[i]->mDrop=drop;
	mGhost[i]->mDropProbability = dropProbability;

	Unit* result = launchUnit(mGhost[i],pos,party,control,wait);

	return result;
}

void Stage::removeUnit(int i) {
	for (int w=0 ; w<mUnitMax ; ++w) {
		if (mUnit[w]) {
			//子を殺す
			if (mUnit[w]->mParent == mUnit[i]) {
				mUnit[w]->mLPDamage = 256;
			}

			//自分の攻撃者情報を消す
			if (mUnit[w]->mLastEnemy == mUnit[i] ) {
				mUnit[w]->mLastEnemy = 0;
			}
			//状態異常も
			for (int k=0 ; k<State::mMax ; ++k) {
				if (mUnit[w]->mState.mMaster[k] == mUnit[i]) {
					mUnit[w]->mState.mMaster[k]=0;
				}
			}
		}
	}

	//戦闘後、参加したかどうか判断するので
	//ここでは出撃状態をいじらない

	//ゲストゴーストならゴースト消去
	for(int w=0 ; w<mGhostMax ; ++w ) {
		if (mUnit[i]->mGhost == mGhost[w]) {
			delete mGhost[w];
			mGhost[w] = 0;
		}
	}

	delete mUnit[i];
	mUnit[i] = 0;
}
void Stage::removeUnit(Unit* ut) {
	for (int w=0 ; w<mUnitMax ; ++w) {
		if (mUnit[w] == ut) {
			removeUnit(w);
			break;
		}
	}

};

void Stage::deploy(int area, int number) {
	ASSERT(mDeployPhase == 0);
	mDeployPhase = new Deploy(this,area,number);
};



Square* Stage::square(int i,int j) const{
	if (i>=0 && i<mWidth && j>=0 && j<mHeight) {
		return mSquare +i*mHeight + j;
	}
	else {
		return 0;
	}
};
Square* Stage::square(int n) const{
	Square* result = 0;
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			if (square(i,j)->mEvent == n) {
				result = square(i,j);
				break;
			}
		}
	}
	return result;
};

int Stage::update() {
	int result = 0;

	Sprite::instance()->updateObject();
	Draw::instance()->mCamera.update();

	drawArea();

	//システムメニュー
	if (mSystemMenu) {
		if (mSystemMenu->update(this)) {
			delete mSystemMenu;
			mSystemMenu = 0;
		}
		return 0;
	}

	//発動フェーズ
	if (mCastPhase) {
		if (mCastPhase->update(this)) {
			mCastPhase->end(this);
			checkUnitKilled();

			delete mCastPhase;
			mCastPhase = 0;

			mScriptRunning = true;
		}
		return 0;
	}
	//ユニット消去フェーズ
	if (mKillUnitPhase) {
		if (mKillUnitPhase->update(this)) {
			delete mKillUnitPhase;
			mKillUnitPhase = 0;
		}
		return 0;
	}
	//品物ゲット
	if (mGameData->updateBag() == 0) {
		return 0;
	}

	//パラメータ確認
	if (mUnitStatusPhase) {
		if (mUnitStatusPhase->update()) {
			delete mUnitStatusPhase;
			mUnitStatusPhase = 0;
		}
		return 0;
	}
	//パーティ展開フェーズ
	if (mDeployPhase) {
		if (mDeployPhase->update(this)) {
			delete mDeployPhase;
			mDeployPhase = 0;
		}
		return 0;
	}
	//スクリプト実行
	if (mScriptRunning) {
		if (mScript->update(mGameData)) {
			mScriptRunning = false;
		}
		return 0;
	}

	//終了フェーズ
	if (mResult) {
		if (mOverPhase == 0) {
			mOverPhase = new Over(mResult,mGameData);
		}
		if (mOverPhase->update(this)) {
			return mOverPhase->mResult;
		}
		return 0;
	}

	//ターン計算フェーズ
	//コントロールターン
	//COMターン
	//なにもなかったらチャージフェーズ
	if (mPhase == 0) {
		mPhase = new TurnReady(this);
	}
	int r=mPhase->update(this);
	if (mNextPhase) {
		delete mPhase;
		mPhase = mNextPhase;
		mNextPhase = 0;
	}

	//戦闘終了判断
	return 0;
}

//パラメータ再計算
void Stage::refresh() {
	//呪力リセット
	for (int i=0 ; i<2 ; ++i) {
		mParty[i].mCapacity=5;
		mParty[i].mChargeSpeed=16;
	}
	//ZOCリセット
	mLuckZOC.set(0);
	mDisturbZOC.set(0);

	//一回目のループ。
	//ステータスは決まらないが、ZOCは展開してくれる。
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			mUnit[i]->recalculation1();
		}
	}
	//二回目のループ。ステータス確定。
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			mUnit[i]->recalculation2();

			int p=mUnit[i]->mParty;

			mParty[p  ].mCapacity += mUnit[i]->abilityTotalPower(204);	//呪力容量
			mParty[1-p].mCapacity += mUnit[i]->abilityTotalPower(205);	//呪力阻害

			mParty[p].mChargeSpeed += *mUnit[i]->mStatus.mAura;			//霊力
		}
	}


	//呪力容量・チャージ速度の範囲チェック
	for (int i=0 ; i<2 ; ++i) {
		if (mParty[i].mCapacity<0) {
			mParty[i].mCapacity=0;
		}
		if (mParty[i].mChargeSpeed < 16) {
			mParty[i].mChargeSpeed=16;
		}
	}

	//APチャージ速度を調べる
	//最大速度で64Fでフルチャージ。ゼロでその半分。

	//まず最大速度
	int maxspeed=1;
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			if (maxspeed < *mUnit[i]->mStatus.mSpeed) {
				maxspeed = *mUnit[i]->mStatus.mSpeed;
			}
		}
	}
	//APチャージ速度を調べる。
	//速度/最大速度*65536/64/2　+65536/64/2
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (mUnit[i]) {
			//mUnit[i]->mAPCharge = *mUnit[i]->mStatus.mSpeed *65536/ maxspeed /60 +65536/60/2;
			mUnit[i]->mAPCharge = *mUnit[i]->mStatus.mSpeed *(65536/64/2)/ maxspeed +65536/64/2;
		}
	}

};



void Stage::drawArea()const {
	drawArea1();
	drawChara();
	drawArea2();
}
void Stage::drawArea1() const{
	//マップ表示

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			square(i,j)->draw(this,false);
		}
	}

	BlendInfo bi;
	bi.mAmbient.set(0.f,0.f,0.f);
	mTest.batch("光源チェッカー")->draw(256,256,256,bi);

	mLuckZOC.draw();
	mDisturbZOC.draw();
};
void Stage::drawArea2()const {
	//水面
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; i<mHeight ; ++i) {
			square(i,j)->draw(this,true);
		}
	}

}

void Stage::drawChara() const{
	//キャラ表示
	Sprite::instance()->drawObject();
}

void Stage::drawFrontEnd() const{
	//ユニット影
	BlendInfo bi;
	bi.setBlendMode(1);
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			if (mUnit[i]->mParty == 0) {
				bi.mAmbient.set(-1,-1,1);
			}
			else {
				bi.mAmbient.set(1,-1,-1);
			}
			bi.mTransparency = 0.5f;

			drawSquareTop(
				&ModelList::instance()->mMap.tSpot,
				mUnit[i]->mX,mUnit[i]->mY,bi);
		}
	}

	drawQueue();
	drawEnergy();
}

void Stage::drawEnergy() const {
	BlendInfo bi;
	bi.mDepthTest=false;
	bi.mDepthWrite=false;

	int x =-160;
	int y =240;
	int w =80;
	int d= 32;
	for (int i=0 ; i<mParty[0].energy(); ++i) {
		ModelList::instance()->mMap.mEnergy->draw(i*d-w+x,y+32,0,bi);
	}
	for (int i=0 ; i<mParty[1].energy(); ++i) {
		ModelList::instance()->mMap.mEnergy->draw(-i*d+w+x,y-32,0,bi);
	}
};

void Stage::drawQueue()const{
	BlendInfo bi;
	//bi.mCamera = false;
	bi.mDepthTest =false;
	bi.mDepthWrite=false;
	bi.mFrontEnd = true;

	BlendInfo bi2;
	//bi2.mCamera = false;
	bi2.mDepthTest =false;
	bi2.mDepthWrite=false;
	bi2.mRotateZ = 90.f;


	for (int i=0 ; i<8; ++i) {
		Unit* ut = getQueue(i);

		float w = 1.f * ut->mAP /65536.f;
		bi2.setZoom(w,1.f,1.f);
		ModelList::instance()->mInterface.mGauge->draw(320-32 - i*64,-240,0,bi2);

		ut->mObject->mModel->draw(320-32-i*64,-240,0,bi);
	}
}
void Stage::drawSquareTop(MyTexture* tex,int i,int j,BlendInfo bi) const{
	square(i,j)->drawTop(this,tex,bi);
}

void Stage::drawSquareInfo(int x, int y, int i, int j) const {

	int line = -24;
	unsigned int color =0xffffffff;
	int size = 1;
	Frame::draw(x,y,160,-line*5);

	int c = 0;
	if (mWeather.get()==0) {
		Draw::instance()->text(x,y+line*c,"晴",color,size);
	}
	if (mWeather.get()==1) {
		Draw::instance()->text(x,y+line*c,"霧",color,size);
	}
	if (mWeather.get()==2) {
		Draw::instance()->text(x,y+line*c,"雪",color,size);
	}
	if (mWeather.get()==3) {
		Draw::instance()->text(x,y+line*c,"月",color,size);
	}
	if (mWeather.get()==4) {
		Draw::instance()->text(x,y+line*c,"花",color,size);
	}
	if (mWeather.get()==5) {
		Draw::instance()->text(x,y+line*c,"雨",color,size);
	}
	if (mWeather.get()==6) {
		Draw::instance()->text(x,y+line*c,"電磁嵐",color,size);
	}

	++c;
	//高さ
	Draw::instance()->text(x,y+line*c,square(i,j)->mZ,color,size);
	++c;
	//地形
	if (square(i,j)->mWater) {
		Draw::instance()->text(x,y+line*c,"水",color,size);
		++c;
	}
	if (square(i,j)->mPit) {
		Draw::instance()->text(x,y+line*c,"足場がない",color,size);
		++c;
	}
	if (square(i,j)->mWall) {
		Draw::instance()->text(x,y+line*c,"進入不可",color,size);
		++c;
	}

	//ＺＯＣ
	if (mLuckZOC.get(i,j)) {
		Draw::instance()->text(x,y+line*c,"必殺上昇ＺＯＣ",color,size);
		++c;
	}
	if (mDisturbZOC.get(i,j)) {
		Draw::instance()->text(x,y+line*c,"移動妨害ＺＯＣ",color,size);
		++c;
	}

};
void Stage::createNumber(float x, float y, float z,unsigned int number, unsigned int color) const {
	//桁数を調べる
	int od = 0;
	for (int i=1 ; number>=i ;i*=10) {
		++od;
	};
	//int width = 25;
	int width = 10;

	for (int i=0 ; i<od ; ++i) {
		int p = -width*i + ((od-1)*width/2);

		int n = number%10;
		number /= 10;

		Object* obj = Sprite::instance()->addObject(ModelList::instance()->mSystem.mNumber[n]);
		obj->setPosition(x,y,z+40);
		obj->mBlendInfo.mShift.set(p,0.f,i);
		obj->setColor(color);
		obj->setSize(0.75f,0.75f);

		obj->setVelocity(0,0,-20);
		obj->setResistance(0,0,0.4);

		obj->mLimit = 60;
	}

};
void Stage::getFocus(int i,int j) const {
	Draw::instance()->mCamera.aimTarget(
		square(i,j)->getX(),
		square(i,j)->getY(),
		square(i,j)->getZ());
}


void Stage::getSquare(int* i,int* j,int x,int y) const {
	*i = x / Square::mSize;
	if (*i < 0) {
		*i=0;
	}
	if (*i >= mWidth){
		*i = mWidth -1;
	}

	*j = y / Square::mSize;
	if (*j < 0) {
		*j=0;
	}
	if (*j >= mHeight) {
		*j= mHeight -1;
	}
};
void Stage::fixCamera() const {
	//ズーム
	Draw::instance()->mCamera.mAimZoom = 1.f;

	//垂直角度
	Draw::instance()->mCamera.mAimVerticalAngle = 30;

	//水平角度は、90n+45にする
	int h = Draw::instance()->mCamera.mHorizotalAngle;
	int i=h/90;
	if (h<0) {
		--i;
	}
	Draw::instance()->mCamera.mAimHorizotalAngle = i*90+30;
	//(i+1)*90
};



Unit* Stage::unit(int i,int j) const{
	Unit* result=0;
	for (int w=0 ; w<mUnitMax ; ++w) {
		if (mUnit[w]) {
			if (mUnit[w]->mX == i &&
				mUnit[w]->mY == j) {
				result = mUnit[w];
				break;
			}
		}
	}
	return result;
}

//行動順を得る
Unit* Stage::getQueue(int p) const{
	Unit* result=0;

	//APチャージ速度計算
	//最大のやつが1秒でフルチャージ
	//速度ゼロで、最大速度の半分

	//順番を調べる

	int ap[mUnitMax];
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			ap[i] = mUnit[i]->mAP;
		}
	}

	int c=0;
	for (int i=0 ; i<(p+1) ; ++i) {
		int t = -1;

		for (int j=0 ; j<mUnitMax ; ++j) {
			if (mUnit[j]) {

				if (t<0) {
					t=j;
				}
				else {
					if (((65536 - ap[j]) / mUnit[j]->mAPCharge) <
						((65536 - ap[t]) / mUnit[t]->mAPCharge)) {
						t=j;
					}
				}
			}
		}

		ap[t] -= 65536;
		result = mUnit[t];
	}
	return result;
};

//やられたユニットがいたら、フェーズをつくる
void Stage::checkUnitKilled() {
	if (mKillUnitPhase == 0) {
		for (int i=0 ; i<mUnitMax ; ++i) {
			if (mUnit[i]) {
				if (mUnit[i]->isKilled() || mUnit[i]->mLPDamage) {
					mKillUnitPhase = new KillUnit(this);
					break;
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
Stage::Party::Party() {
	mEnergy = 0;
};

void Stage::Party::update() {
	//1Fで溜まる量は、
	//総霊力 / 16 * 2/64 *65536

	mEnergy += mChargeSpeed *2 *(65536/16/64);

	if (mEnergy > (65536 * mCapacity)) {
		mEnergy = 65536 * mCapacity;
	}
};

int Stage::Party::energy() const {
	return mEnergy / 65536;
};
void Stage::Party::increaseEnergy(int i) {
	mEnergy += i*65536;

	if (mEnergy > (65536 * mCapacity)) {
		mEnergy = 65536 * mCapacity;
	}
	if (mEnergy <0) {
		mEnergy = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
Stage::Weather::Weather() {
	mCurrent = -1;
	mBase = -1;
};
void Stage::Weather::update() {
	if (mCurrent!=mBase) {
		--mCount;
		if (mCount < 0) {
			mCurrent = mBase;
		}
	}
};
int Stage::Weather::get() const{
	return mCurrent;
}
void Stage::Weather::change(int i) {
	if (mBase <0) {
		mBase = i;
	}
	else {
		mCount = 256;
	}
	mCurrent = i;
};
///////////////////////////////////////////////////////////////////////////////
Stage::Bonus::Bonus() {
	mTool[0] = 0;
	mTool[1] = 0;
	mTool[2] = 0;
	mTool[3] = 0;

	mScore = 0;
	mPower = 0;
}
void Stage::Bonus::bag(GameData* gamedata) {
	gamedata->mBag.add(mTool[0],mScore,mPower);
	gamedata->mBag.add(mTool[1],0,0);
	gamedata->mBag.add(mTool[2],0,0);
	gamedata->mBag.add(mTool[3],0,0);
};

///////////////////////////////////////////////////////////////////////////////

SelectSquare::SelectSquare() {
	mPrimaryUnit = 0;
};

SelectSquare::~SelectSquare() {
}

void SelectSquare::create(const StageArea& sa, bool menu) {
	mMenu = menu;
	mSelectArea = sa;

	sa.mStage->fixCamera();
	sa.mStage->getSquare(
		&mX,&mY,
		Draw::instance()->mCamera.mAimTarget.x,
		Draw::instance()->mCamera.mAimTarget.y);
};

void SelectSquare::create(int x, int y, const StageArea& sa, bool menu) {
	mX = x;
	mY = y;
	mMenu = menu;
	mSelectArea = sa;

	sa.mStage->fixCamera();
};

void SelectSquare::draw(Stage* stage) {
	//表示
	mSelectArea.draw();

	//フォーカス表示
	BlendInfo bi;
	bi.setBlendMode(1);
	bi.mTransparency = 0.5f;
	stage->drawSquareTop(
		&ModelList::instance()->mMap.tSpot,
		mX,mY,bi);

	//高さ表示
	stage->drawSquareInfo(320-160,240,mX,mY);

	//ユニットウィンドウ
	if (mPrimaryUnit) {
		mPrimaryUnit->drawInMap(true);
	}
	else {
		if (stage->unit(mX,mY)) {
			stage->unit(mX,mY)->drawInMap(true);
		}
	}
	stage->drawFrontEnd();
}

int SelectSquare::update(Stage* parent) {
	int result = 0;

	//カメラ位置
	parent->getFocus(mX,mY);

	//カーソル移動
	//移動方向はカメラ向きに依存させなければならない
	int vx=0;
	int vy=0;
	if (cKey::get(0)) {
		--vx;
	}
	if (cKey::get(1)) {
		++vy;
	}
	if (cKey::get(2)) {
		++vx;
	}
	if (cKey::get(3)) {
		--vy;
	}

	int h = Draw::instance()->mCamera.mHorizotalAngle;
	if (h<0) {
		h-=90;	//45/90＝-45/90になるので、負の場合はひとつずらしてやる必要がある
	}
	h = (h/90 %4 +4) %4;

	int c=1;
	int s=0;
	if (h == 1) {
		c=0;
		s=-1;
	}
	if (h == 2) {
		c=-1;
		s=0;
	}
	if (h == 3) {
		c=0;
		s=1;
	}
	int x = mX + vx * c + vy * s;
	int y = mY + vy * c - vx * s;


	if (x>=0 &&
		(x < parent->mWidth) &&
		y>=0 &&
		(y < parent->mHeight)) {

		if (x>=0 && x<parent->mWidth) {
			if (!parent->square(x,y)->mWall ||
				(x>0 && x<parent->mWidth-1)) {
				mX=x;
			}
		}
		if (y>=0 && y<parent->mHeight) {
			if (!parent->square(x,y)->mWall ||
				(y>0 && y<parent->mHeight-1)) {
				mY=y;
			}
		}
	}

	//外壁に埋もれて操作できなくならないようにする
	if (parent->square(mX,mY)->mWall) {
		if (mX==0) {
			mX=1;
		}
		if (mX==parent->mWidth-1) {
			mX=parent->mWidth-2;
		}
		if (mY==0) {
			mY=1;
		}
		if (mY==parent->mHeight-1) {
			mY=parent->mHeight-2;
		}
	}

	//カメラ向き
	if (cKey::trigger(10)) {
		Draw::instance()->mCamera.mAimHorizotalAngle += 90.f;
	}
	if (cKey::trigger(11)) {
		Draw::instance()->mCamera.mAimHorizotalAngle -= 90.f;
	}
	//ズーム
	if (cKey::trigger(7)) {
		if (Draw::instance()->mCamera.mAimZoom > 1.f) {
			Draw::instance()->mCamera.mAimZoom = 1.f;
		}
		else {
			Draw::instance()->mCamera.mAimZoom = 2.f;
		}
	}
	//角度
	if (cKey::trigger(12)) {
		if (Draw::instance()->mCamera.mAimVerticalAngle < 25.f) {
			Draw::instance()->mCamera.mAimVerticalAngle = 30.f;
		}
		else {
			Draw::instance()->mCamera.mAimVerticalAngle = 15.f;
		}
	}


	//決定
	if (cKey::trigger(5)) {
		if (mSelectArea.get(mX,mY)) {
			result =1;
		};
	}

	//キャンセル / メニュー
	if (cKey::trigger(6)) {
		//ユニットステータス画面
		if (mMenu) {
			//ユニットがいたら、パラメータ見る
			if (parent->unit(mX,mY)) {
				if (parent->unit(mX,mY)->mDefined) {
					parent->mUnitStatusPhase = new UnitStatusPhase(parent->unit(mX,mY));
				}
			}
			else {
				//いなかったら、システムメニュー
				parent->mSystemMenu = new Stage::SystemMenu(parent);
			}
		}
		else {
			result = 2;
		}
	}

	//ユニットまわす
	if (cKey::trigger(4)) {
		if (mMenu) {
			int index = -1;

			Unit* unit = parent->unit(mX,mY);
			if (unit) {
				for (int i=0 ; i<Stage::mUnitMax ; ++i) {
					if (parent->mUnit[i] == unit) {
						index = i;
						break;
					}
				}
			}

			for (int i=index+1 ; true ; ++i) {
				i %= Stage::mUnitMax;
				if (parent->mUnit[i]) {
					if (parent->mUnit[i]->mTurn) {
						mX = parent->mUnit[i]->mX;
						mY = parent->mUnit[i]->mY;
						break;
					}
				}
			}
		}
	}

	return result;
};

///////////////////////////////////////////////////////////////////////////////
Square::Square() {
	mU=0;
	mV=0;
	mZ=0;
	//mCost=0;
	mPit=false;
	mWall=false;
	mWater=false;
	mSlant = 255;
	mEvent = 0;

	mBatch=0;
	//上面ポリゴンデータ
	mTopPolygonIndex=0;
	mTopPolygonNumber = 0;
}

Square::~Square() {
	if (mBatch) {
		delete mBatch;
		mBatch = 0;
	}
};

int Square::getX() const{
	return mSize*mX;
}
int Square::getY() const{
	return mSize*mY;
}
int Square::getZ() const{
	return mZ * mHeight;
}



int Square::vertexX(int i) const{
	int result = 0;
	i = (i%4+4)%4;
	if (i==0) {
		result -= mSize/2;
	}
	if (i==1) {
		result += mSize/2;
	}
	if (i==2) {
		result += mSize/2;
	}
	if (i==3) {
		result -= mSize/2;
	}
	return result;
}

int Square::vertexY(int i) const{
	int result = 0;
	i = (i%4+4)%4;
	if (i==0) {
		result += mSize/2;
	}
	if (i==1) {
		result += mSize/2;
	}
	if (i==2) {
		result -= mSize/2;
	}
	if (i==3) {
		result -= mSize/2;
	}
	return result;
}

int Square::vertexZ(int i) const{
	//壁
	if (mWall) {
		return 64 * mHeight;		//とにかく高く
	}
	if (mPit) {
		return -1;
	}
	if (mWater) {
		return -1;
	}

	if (mSlant == 256) {
		//円柱
		return -1;
	}

	i = (i%4+4)%4;
	int result = getZ();

	int tt = 0;
	if (mSlant!=255) {
		if (i==0) {
			int k[4]={1,-1,-1,1};
			tt = k[mSlant];
		}
		if (i==1) {
			int k[4]={1,1,-1,-1};
			tt = k[mSlant];
		}
		if (i==2) {
			int k[4]={-1,1,1,-1};
			tt = k[mSlant];
		}
		if (i==3) {
			int k[4]={-1,-1,1,1};
			tt = k[mSlant];
		}
		result += tt*mHeight;
	}
	return result;
};

unsigned int Square::type() const {
	if (mWall) {
		return 2;
	}
	if (mWater) {
		return 4;
	}
	if (mPit) {
		return 1;
	}
	return 0;
};

GameLib::Math::Vector3 Square::normal() const {
	GameLib::Math::Vector3 result;
	result.set(0,0,mSize);

	if (mSlant == 0) {
		result.y -= mHeight;
	}
	if (mSlant == 1) {
		result.x -= mHeight;
	}
	if (mSlant == 2) {
		result.y += mHeight;
	}
	if (mSlant == 3) {
		result.x += mHeight;
	}
	result.normalize();
	return result;
};


void Square::draw(const Stage* stage,bool water) const{
	if (mBatch) {
		BlendInfo bi;
		bi.mAmbient.set(0.f,0.f,0.f);
		mBatch->draw(
			getX(),getY(),0,
			bi);
	}
};
void Square::drawTop(const Stage* stage,MyTexture* tex,const BlendInfo& bi) const{
	if (mBatch) {
		BlendInfo b=bi;
		b.mPolygonIndex = mTopPolygonIndex;
		b.mPolygonNumber = mTopPolygonNumber;
		mBatch->draw(
			tex,
			getX(),getY(),0,
			b);
	}
}


void Square::createBatch(Stage * stage) {
	if (mBatch) {
		delete mBatch;
		mBatch = 0;
	}
	mTopPolygonIndex = 0;
	mTopPolygonNumber = 0;

	//穴
	if (mPit) {
		return;
	}
	//水
	if (mWater) {
		return;
	}

	//最大頂点はマス数*20
	//インデックス数はマス数*10*3
	int vtn = 120;
	int idn = 160;

	//マップバッチ作成
	float* u = new float[vtn];
	float* v = new float[vtn];
	float* x = new float[vtn];
	float* y = new float[vtn];
	float* z = new float[vtn];
	GameLib::Math::Vector3* NORMAL= new GameLib::Math::Vector3[vtn];
	unsigned int* COLOR= new unsigned int[vtn];
	int* index= new int[idn];

	//さあやるぞ
	int c=0;
	int d=0;

	if (mSlant == 256) {
		//円柱
		int m=8;
		mTopPolygonIndex=d;
		mTopPolygonNumber=m;

		//上面
		for (int k=0 ; k<m ; ++k) {
			float x1 = GameLib::Math::cos(360.f*k/m)		* mSize/2 ;
			float y1 = GameLib::Math::sin(360.f*k/m)		* mSize/2 ;
			float x2 = GameLib::Math::cos(360.f*(k+1)/m)	* mSize/2 ;
			float y2 = GameLib::Math::sin(360.f*(k+1)/m)	* mSize/2 ;

			u[c]	=mU	*		mSize + mSize/2;
			v[c]	=mV	*		mSize + mSize/2;
			u[c+1]	=mU	*		mSize+1;
			v[c+1]	=(mV+1)*	mSize+1;
			u[c+2]	=(mU+1)*	mSize-1;
			v[c+2]	=mV	*		mSize+1;

			x[c]	= 0.f;
			y[c]	= 0.f;
			x[c+1]	= x1;
			y[c+1]	= y1;
			x[c+2]	= x2;
			y[c+2]	= y2;

			for (int k = 0 ; k<3 ; ++k) {
				z[c+k]	= getZ();
				NORMAL[c+k].set(0.f,0.f,1.f);
				COLOR[c+k]=0xffffffff;
			}

			index[d]	=c+0;
			index[d+1]	=c+1;
			index[d+2]	=c+2;

			c+=3;
			d+=3;
		}
		for (int k=0 ; k<m ; ++k) {
			//側面
			float x1 = GameLib::Math::cos(360.f*k/m)		* mSize/2 ;
			float y1 = GameLib::Math::sin(360.f*k/m)		* mSize/2 ;
			float x2 = GameLib::Math::cos(360.f*(k+1)/m)	* mSize/2 ;
			float y2 = GameLib::Math::sin(360.f*(k+1)/m)	* mSize/2 ;

			u[c]	=mU	*		mSize+1;
			v[c]	=mV	*		mSize+1+mSize;
			u[c+1]	=(mU+1)*	mSize-1;
			v[c+1]	=mV	*		mSize+1+mSize;
			u[c+2]	=(mU+1)*	mSize-1;
			v[c+2]	=(mV+1)*	mSize-1+mSize;
			u[c+3]	=mU	*		mSize+1;
			v[c+3]	=(mV+1)*	mSize-1+mSize;

			x[c]	= x1;
			y[c]	= y1;
			z[c]	= getZ();
			x[c+1]	= x2;
			y[c+1]	= y2;
			z[c+1]	= getZ();
			x[c+2]	= x2;
			y[c+2]	= y2;
			z[c+2]	= -256;
			x[c+3]	= x1;
			y[c+3]	= y1;
			z[c+3]	= -256;

			float cs1=GameLib::Math::cos(360.f*k/m);
			float sn1=GameLib::Math::sin(360.f*k/m);
			float cs2=GameLib::Math::cos(360.f*(k+1)/m);
			float sn2=GameLib::Math::sin(360.f*(k+1)/m);
			NORMAL[c+0].set(cs1,sn1,0.f);
			NORMAL[c+1].set(cs2,sn2,0.f);
			NORMAL[c+2].set(cs2,sn2,0.f);
			NORMAL[c+3].set(cs1,sn1,0.f);

			for (int k = 0 ; k<4 ; ++k) {
				COLOR[c+k]=0xffffffff;
			}

			index[d]	=c+0;
			index[d+1]	=c+2;
			index[d+2]	=c+1;
			index[d+3]	=c+0;
			index[d+4]	=c+3;
			index[d+5]	=c+2;

			c+=4;
			d+=6;

		}
	}
	else {		
		if (!mWall) {
			//上面
			mTopPolygonIndex=d;
			mTopPolygonNumber=2;

			u[c]	=(mU)	*	mSize+1.f;
			v[c]	=(mV)	*	mSize+1.f;
			u[c+1]	=(mU+1)*	mSize-1.f;
			v[c+1]	=(mV)  *	mSize+1.f;
			u[c+2]	=(mU+1)*	mSize-1.f;
			v[c+2]	=(mV+1)*	mSize-1.f;
			u[c+3]	=(mU)	*	mSize+1.f;
			v[c+3]	=(mV+1)*	mSize-1.f;

			for (int k = 0 ; k<4 ; ++k) {
				x[c+k]	= vertexX(k);
				y[c+k]	= vertexY(k);
				z[c+k]	= vertexZ(k);
				NORMAL[c+k  ] = normal();
				COLOR[c+k]=0xffffffff;
			}

			index[d]	=c+0;
			index[d+1]	=c+2;
			index[d+2]	=c+1;
			index[d+3]	=c+0;
			index[d+4]	=c+3;
			index[d+5]	=c+2;

			c+=4;
			d+=6;
		}

		//側面つくる
		for (int k=0 ; k<4; ++k) {
			//隣のマスの頂点と比較する　めんどくさいにゃ
			Square* s = 0;
			if (k==0) {
				s=stage->square(mX,mY+1);
			}
			if (k==1) {
				s=stage->square(mX+1,mY);
			}
			if (k==2) {
				s=stage->square(mX,mY-1);
			}
			if (k==3) {
				s=stage->square(mX-1,mY);
			}
			if (mWall && s==0) {
				continue;
			}
			bool b=true;
			if (s) {
				if (mWall || !s->mWall) {
					if ((vertexZ(k)   <= s->vertexZ(k-1)) &&
						(vertexZ(k+1) <= s->vertexZ(k+1+1))) {
						b= false;
					}
				}
			}
			if (b) {
				u[c]	= mU		*mSize+1;
				v[c]	= mV		*mSize+1 +mSize;
				u[c+1]	=(mU+1)		*mSize-1;
				v[c+1]	= mV		*mSize+1 +mSize;
				u[c+2]	=(mU+1)		*mSize-1;
				v[c+2]	=(mV+1)		*mSize-1 +mSize;
				u[c+3]	= mU		*mSize+1;
				v[c+3]	=(mV+1)		*mSize-1 +mSize;

				x[c]	=vertexX(k);
				y[c]	=vertexY(k);
				z[c]	=vertexZ(k);

				x[c+1]	=vertexX(k+1);
				y[c+1]	=vertexY(k+1);
				z[c+1]	=vertexZ(k+1);

				x[c+2]	=vertexX(k+1);
				y[c+2]	=vertexY(k+1);
				z[c+2]	=-256;

				x[c+3]	=vertexX(k);
				y[c+3]	=vertexY(k);
				z[c+3]	=-256;

				for (int w=0 ; w<4 ; ++w) {
					NORMAL[c+w] = nRotateVector3(GameLib::Vector3(0,1.0f,0),0,0,90*k);
					COLOR[c+w]=0xffffff00;
				}

				index[d]	=c+0;
				index[d+1]	=c+1;
				index[d+2]	=c+2;
				index[d+3]	=c+0;
				index[d+4]	=c+2;
				index[d+5]	=c+3;

				c+=4;
				d+=6;
			}
		}
	}
	//穴があると飛ばすだけなので、別に合ってなくていい
	//ASSERT(c == vtn && d == idn);
	mBatch = new Batch;
	mBatch->create(&stage->mMapChip,"",u,v,x,y,z,COLOR,index,c,d,NORMAL);

	//mBatch->mMaterials.mLighting[0] = true;
	mBatch->mMaterials.mLighting[2] = true;
	mBatch->mMaterials.mAmbient.set(0.f,0.f,0.f);
	//mMap->mMaterials.mDeffuse.set(1.f,0,0);
	//mBatch->mMaterials.mSpecular.set(1.f,0,0);
	//mBatch->mMaterials.mSharpness = 0.f;
	mBatch->mMaterials.setCullMode(false,true);

/*
	if (c) {
		mWaterChip.load("src/picture/map/water",false);
		mWater = mWaterChip.createBatch("",u,v,x,y,z,COLOR,index,c,d,NORMAL);
		mWater->mMaterials.mLighting[0] = true;
		mWater->mMaterials.mAmbient.set(0.5f,0.5f,0.5f);
		mWater->mMaterials.setCullMode(false,true);
		mWater->mMaterials.mSharpness = 100.f;
	}
*/
	delete []u;
	delete []v;
	delete []x;
	delete []y;
	delete []z;
	delete []COLOR;
	delete []NORMAL;
	delete []index;
}
///////////////////////////////////////////////////////////////////////////////

StageArea::StageArea() {
	mStage = 0;
	mWidth = 0;
	mHeight = 0;
	mData = 0;
	mColor = 0x80808000;

};

StageArea::~StageArea() {
	release();
}
void StageArea::release() {
	if (mData) {
		delete []mData;
		mData = 0;
	}
};
void StageArea::operator +=(const StageArea &sa) {
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			set(i,j, 
				get(i,j) + sa.get(i,j));
		}
	}
};
void StageArea::operator -=(const StageArea &sa) {
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			set(i,j, 
				get(i,j) - sa.get(i,j));
		}
	}

};

//コピー
void StageArea::operator =(const StageArea& sa) {
	create(sa.mStage);
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			set(i,j,sa.get(i,j));
		}
	}
}

void StageArea::create(Stage* stage) {
	release();

	mStage = stage;
	mWidth = stage->mWidth;
	mHeight = stage->mHeight;
	mData = new int[mWidth * mHeight];
	set(0);

	mColor = 0x80808000;
};

int StageArea::get(int i,int j) const{
	ASSERT(i>=0 &&i<mWidth && j>=0 && j<mHeight);
	return  mData[i*mHeight + j];
}
void StageArea::set(int i, int j,int v) {
	//範囲外なら無視すりゃいいじょん
	if (i>=0 && i<mWidth && j>=0 && j<mHeight) {
		mData[i*mHeight + j] = v;
	}
}
void StageArea::set(int v) {
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			set(i,j,v);
		}
	}
}
void StageArea::setEvent(int e) {
	set(0);
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			if (mStage->square(i,j)->mEvent == e) {
				set(i,j,1);
			}
		}
	}
}


int* StageArea::data() const{
	return mData;
}


//範囲計算

void StageArea::calc(RANGETYPE range, int x, int y, int unitX, int unitY, int maxRange, int minRange, int up, int down,unsigned int interrupt) {
	//移動
	if (range == RANGE_MOVE) {
		walk(x,y,
			maxRange,
			interrupt,
			up,down);
	}
	//移動十字
	//移動ななめ

	//点
	if (range == RANGE_POINT) {
		set(0);
		set(x,y,1);

		mX = x;
		mY = y;
	}
	//全部
	if (range == RANGE_ALL) {
		set(1);
		mX = x;
		mY = y;
	}

	//円
	if (range == RANGE_CIRCLE) {
		circle(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//円2
	if (range == RANGE_CIRCLE2) {
		circle2(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//矩形
	if (range == RANGE_RECT) {
		square(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//十字
	if (range == RANGE_C) {
		cross(x,y,maxRange,up);
	}
	//斜め
	if (range == RANGE_X) {
		xCross(x,y,maxRange,up);
		setTallRange(x,y,up,down);
	}
	//扇
	//線

	//最低射程チェック
	setMinRange(x,y,minRange);
};


//ルートを計算しての範囲
void StageArea::walk(int i,int j,int range,unsigned int interrupt,int up,int down) {
	mX=i;
	mY=j;

//障害物パラメータ
//1穴・2壁・4水・8プレイヤーユニット・16エネミーユニットを無視
//計算したあと穴・壁・水・ユニットマスを除外する
	set(0);						//初期化して
	set(i,j,range+1);		//始点を設定

	//まず移動コストを求めておく
	//敵や障害物などもここで調べておこう
	mStage->mTemp.set(1);

	//障害物を適用
	if (interrupt & 1+2+4) {
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {
				if (mStage->square(i,j)->type() & interrupt) {
					mStage->mTemp.set(i,j,255);
				}
			}
		}
	}
	//ユニット邪魔
	if (interrupt & 8+16) {
		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			if (mStage->mUnit[i]) {
				//どっち側のユニットか判断しよう
				int ai = mStage->mUnit[i]->AI();

				if ((ai==0 && interrupt&8) ||
					(ai==1 && interrupt&16)) {
					mStage->mTemp.set(
						mStage->mUnit[i]->mX,
						mStage->mUnit[i]->mY,255);
				}
			}
		}
		//移動妨害ZOC
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {
				if (interrupt & 8) {
					if (mStage->mDisturbZOC.get(i,j) < 0) {
						mStage->mTemp.set(2);
					}
				}
				if (interrupt & 16) {
					if (mStage->mDisturbZOC.get(i,j) > 0) {
						mStage->mTemp.set(2);
					}
				}
			}
		}

	}

	//履歴
	int recordI[256];
	int recordJ[256];
	int counter[256];

	int record=0;
	recordI[0] = i;
	recordJ[0] = j;
	counter[0] = 0;

	while (true) {

		int p=recordI[record];
		int q=recordJ[record];
		int u=p;
		int v=q;
		//既に四方向を調べてある場合、履歴をさかのぼって引き返す
		if (counter[record] > 3) {
			--record;

			//始点に戻ってた。終了。
			if(record<0) {
				break;
			}

			continue;
		}

		//移動
		if (counter[record] == 0) {
			--u;
		}
		if (counter[record] == 1) {
			++u;
		}
		if (counter[record] == 2) {
			--v;
		}
		if (counter[record] == 3) {
			++v;
		}

		//調べたカウンターを増やしておく
		++counter[record];

		//無効な範囲だったら戻ろう
		if (u<0 || u>=mWidth || v<0 || v>=mHeight) {
			continue;
		}

		//移動値が現状より多い状態になってたら、調べる必要はないので引き返す
		int r=get(p,q) - mStage->mTemp.get(p,q);
		if (get(u,v) >= r) {
			continue;
		};

		//高さ制限にひっかかった場合は引き返す
		if (!mStage->square(u,v)->mPit && !mStage->square(u,v)->mWall) { //穴・壁に高さはない。
			if (mStage->square(u,v)->mZ - mStage->square(p,q)->mZ > up ||
				mStage->square(u,v)->mZ - mStage->square(p,q)->mZ < -down) {
				continue;
			}
		}

		//有効な調査だったんで、移動値を更新して履歴に加える
		set(u,v,r);

		++record;
		ASSERT(record<256 && "calcArea record over");	
		recordI[record] = u;
		recordJ[record] = v;
		counter[record] = 0;
	}
};


void StageArea::circle(int x,int y,int r) {
	mX=x;
	mY=y;

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			int a=(i-x);
			int b=(j-y);
			if (a < 0) {
				a=-a;
			}
			if (b<0) {
				b=-b;
			}
	
			if (a+b <= r) {
				set(i,j,1);
			}
		}
	}
}
void StageArea::circle2(int x,int y,int r) {
	mX=x;
	mY=y;

	int R = r*r;

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			if ((i-x)*(i-x)+(j-y)*(j-y) < R) {
				set(i,j,1);
			}
		}
	}
}

void StageArea::square(int x,int y,int r) {
	mX=x;
	mY=y;

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			int a=(i-x);
			int b=(j-y);
			if (a < 0) {
				a=-a;
			}
			if (b<0) {
				b=-b;
			}
	
			if (a <= r &&
				b <= r) {
				set(i,j,1);
			}
		}
	}
}

void StageArea::xCross(int i,int j,int r,int top) {
	//高さ制限にひっかかったら終了
	mX=i;
	mY=j;

	int x,y;
	int p,q;
	for (int k=0 ; k<4 ; ++k) {
		if (k==0) {
			int p=1;
			int q=1;
		}
		if (k==1) {
			int p=-1;
			int q=1;
		}
		if (k==2) {
			int p=-1;
			int q=-1;
		}
		if (k==3) {
			int p=1;
			int q=-1;
		}

		for (int w=0 ; w<r+1 ; ++w) {
			x=i+p*w;
			y=j+q*w;
			if (x>=0 && x<mWidth && y>=0 && y<mHeight) {
				//高さ制限
				if (mStage->square(x,y)->mZ - mStage->square(i,j)->mZ  > top) {
					break;
				}
				set(x,y,1);
			}
			else {
				break;
			}
		}
	}
}

void StageArea::cross(int i,int j,int r,int top) {
	mX=i;
	mY=j;

	int x,y;
	int p,q;
	for (int k=0 ; k<4 ; ++k) {
		if (k==0) {
			int p=1;
			int q=0;
		}
		if (k==1) {
			int p=0;
			int q=1;
		}
		if (k==2) {
			int p=-1;
			int q=0;
		}
		if (k==3) {
			int p=0;
			int q=-1;
		}

		for (int w=0 ; w<r+1 ; ++w) {
			x=i+p*w;
			y=j+q*w;
			if (x>=0 && x<mWidth && y>=0 && y<mHeight) {
				//高さ制限
				if (mStage->square(x,y)->mZ - mStage->square(i,j)->mZ  > top) {
					break;
				}
				set(x,y,1);
			}
			else {
				break;
			}
		}
	}
}

void StageArea::except(unsigned int exc) {
	//障害物を除外
	if (exc & 1+2+4) {
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {

				if (mStage->square(i,j)->type() & exc) {
					set(i,j,0);
				}
			}
		}
	}
	//ユニットがいるマスを除外
	if (exc & 8+16) {
		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			if (mStage->mUnit[i]) {
				set(
					mStage->mUnit[i]->mX,
					mStage->mUnit[i]->mY,
					0);
			}
		}
	}
}
//高度射程を設定
void StageArea::setTallRange(int x,int y,int top, int bottom) {
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {

			int t = mStage->square(i,j)->mZ - mStage->square(x,y)->mZ;
			if (t>top || t<-bottom) {
				set(i,j,0);
			}
		}
	}
};
void StageArea::setMinRange(int x,int y,int range) {
	//最低射程チェック
	if (range > 0) {
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {
				if (get(i,j)) {
					int a=(i-x);
					int b=(j-y);
					if (a < 0) {
						a=-a;
					}
					if (b<0) {
						b=-b;
					}
	
					if (a+b < range) {
						set(i,j,0);
					}
				}
			}
		}	
	}
};


//有効なマスを表示
void StageArea::draw() const{
	unsigned int a= (mColor & 0xff000000) >> (4*6);
	unsigned int r= (mColor & 0x00ff0000) >> (4*4);
	unsigned int g= (mColor & 0x0000ff00) >> (4*2);
	unsigned int b= (mColor & 0x000000ff) >> (4*0);

	float A = 1.f* a /255.f;
	float R = 1.f* r /255.f;
	float G = 1.f* g /255.f;
	float B = 1.f* b /255.f;

	BlendInfo bi;
	bi.setBlendMode(1);
	bi.mAmbient.set(R,G,B);
	bi.mTransparency=A;

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			if (get(i,j) > 0) {
				mStage->drawSquareTop(&ModelList::instance()->mMap.tSpot,i,j,bi);
			}
		}
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Stage::SystemMenu::SystemMenu(const Stage* stage) {
	mWindow.position(-320,240);
	mWindow.add(stage->mTitle.data());

	MyString tmp(64);
	tmp << "スコア　";
	tmp << stage->mGameData->mFlagManager.getValue("score");
	mWindow.add(tmp.data());

	tmp = "";
	tmp << "パワー　";
	tmp << stage->mGameData->mFlagManager.getValue("power");
	mWindow.add(tmp.data());

	mWindow.add("　ユニット",1);
	mWindow.add("　設定",0);

	mUnitList.position(-320,240);
	int c=0;
	for (int i=0 ; i<Stage::mUnitMax; ++i) {
		if (stage->mUnit[i]) {
			unsigned int color = 0xff8080ff;
			if (stage->mUnit[i]->mParty ==1) {
				color = 0xffff8080;
			}
			mUnitList.add(stage->mUnit[i]->shell()->mChara.data(),i,c<16,color,2);
			++c;
		}
	}

	mConfig = 0;
	mPhase = 0;
};
Stage::SystemMenu::~SystemMenu() {
	if (mConfig) {
		delete mConfig;
		mConfig = 0;
	}
};

int Stage::SystemMenu::update(const Stage* stage) {
	if (mConfig) {
		if (mConfig->update()) {
			delete mConfig;
			mConfig = 0;
		}
		return 0;
	}
	if (mPhase) {
		mUnitList.draw();
		int r=mUnitList.update();
		int i=mUnitList.getParameter();
		stage->mUnit[i]->drawInMap(true);
		if (r==1) {
			stage->mUnit[i]->getFocus();
		}
		if (r==2) {
			mPhase = 0;
		}
		return 0;
	}
	//stage->drawArea();
	stage->drawFrontEnd();
	mWindow.draw();
	int r= mWindow.update();
	if (r==1) {
		int i= mWindow.getParameter();
		if (i==0) {
			mConfig = new sConfig;
		}
		if (i==1) {
			mPhase = 1;
		}
	}
	if (r==2) {
		return 2;
	}
	return 0;
};

Stage::Over::Over(int result,GameData* gamedata) {
	mResult = result;
	mCount = 0;

	//アビリティゲットと、ゴーストもどす
	for (int i=0; i<::Party::mGhostMax ; ++i) {
		if (gamedata->mParty.mGhost[i]) {
			if (gamedata->mParty.mGhost[i]->mState == 1) {
				//アビリティ経験値ゲット
				gamedata->mParty.mGhost[i]->learnAbility(&mLearningWindow);

				//出撃状態をもどす
				gamedata->mParty.mGhost[i]->mState=0;
			}
		}
	}
}
int Stage::Over::update(Stage * stage) {
	if (mCount==0) {
		stage->mBonus.bag(stage->mGameData);
	}
	if (mCount==1) {
		if (mLearningWindow.update()) {
			++mCount;
		}
		return 0;
	}

	++mCount;


	if (mCount>60) {
		return 1;
	}
	return 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Stage::Phase::~Phase() {

};

//APチャージフェーズ
Stage::TurnReady::TurnReady(Stage* stage) {
	mSide = -1;			//0プレイヤー操作　1味方　2敵

	//再計算
	stage->refresh();

}

int Stage::TurnReady::update(Stage* parent) {
	parent->drawFrontEnd();

	//ターンすすめる
	++parent->mCount;
	//呪力チャージ
	parent->mParty[0].update();
	parent->mParty[1].update();
	//天候カウント
	parent->mWeather.update();

	//APチャージ
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (parent->mUnit[i]) {
			parent->mUnit[i]->mAP += parent->mUnit[i]->mAPCharge;
		}
	};
	//たまったら、行動回数に追加
	//同時にたまった場合の処理を考えねばならない
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (parent->mUnit[i]) {

			if (parent->mUnit[i]->mAP >=65536) {
				if (mSide < 0 ||
					mSide == parent->mUnit[i]->turn()) {

					parent->mUnit[i]->mTurn += 1;
					parent->mUnit[i]->mAP -= 65536;

					//次ターンを判断
					if (mSide < 0) {
						mSide = parent->mUnit[i]->turn();
					}
				}
			};
		}
	}

	//別陣営にインタラプトされたら、ターン開始
	if (mSide >=0) {
		if (mSide != parent->getQueue(0)->turn()) {
			if (mSide == 0) {
				//味方ターン
				parent->mNextPhase = new Stage::Control(parent);
			}
			else {
				//NPCターン
				parent->mNextPhase = new Stage::Computer(parent);
			}
		}
	}

	return 0;
}
///////////////////////////////////////////////////////
Stage::Deploy::Deploy(Stage* stage,int area, int number) {
	mArea.create(stage);
	mArea.setEvent(area);
	mNumber = number;

	mSelectSquare.create(mArea,false);

	mPhase = 0;
	mConfirm.set("Ｏｋ？");
	mConfirm.position(0,0);
};

int Stage::Deploy::update(Stage * stage) {
	if (mPhase == 0) {
		//座標選択
		int r = mSelectSquare.update(stage);
		mSelectSquare.draw(stage);

		Frame::draw(-320,240,80,20);
		Draw::instance()->text(-320   ,240,"あと",0xffffffff,1);
		Draw::instance()->text(-320+64,240,mNumber,0xffffffff,1);
		if (r == 1) {
			int x=mSelectSquare.mX;
			int y=mSelectSquare.mY;

			//既にいるなら、外す
			if (stage->unit(x,y)) {
				stage->unit(x,y)->mGhost->mState = 0;//出撃状態をもどす
				stage->removeUnit( stage->unit(x,y) );
				++mNumber;
			}
			else {
				//いないなら、ゴースト選択へ
				//既に出てる奴はリストに入れないようにしなくてはならない。
				if (mNumber>0) {
					mGhostListWindow.set(stage->mGameData);
					mGhostListWindow.position(-320,240);
					mPhase = 1;
				}
			}
		}
		if (r==2) {
			//キャンセルで確認
			//一人もキャラ配置してないとだめだ
			bool b=false;
			for (int i=0 ; i<::Party::mGhostMax ; ++i) {
				if (stage->mGameData->mParty.mGhost[i]) {
					if (stage->mGameData->mParty.mGhost[i]->mState == 1) {
						b=true;
						break;
					}
				}
			}
			if (b) {
				mPhase = 2;
			}
		}
		return 0;
	}
	if (mPhase == 1) {
		//パーティから選択
		mGhostListWindow.draw(true);
		int r=mGhostListWindow.update();
		if (r == 1) {
			int i = mGhostListWindow.getCurrentUnit();
			stage->launchUnit(
				stage->mGameData->mParty.mGhost[i],
				mSelectSquare.mX,
				mSelectSquare.mY,
				0,true,0);
			mPhase = 0;
			--mNumber;
		}
		if (r == 2) {
			mPhase = 0;
		}
		return 0;
	}
	if (mPhase == 2) {
		mConfirm.draw();
		int r = mConfirm.update();
		if (r==1) {
			return 1;
		}
		if (r == 2) {
			mPhase = 0;
		}
		return 0;
	}

	ASSERT(0);
};



///////////////////////////////////////////////////////
Stage::Control::Control(const Stage* stage) {
	mSelectUnit = 0;
	mCommand = 0;

	for (int i=0; i<Stage::mUnitMax; ++i) {
		if (stage->mUnit[i]) {
			if (stage->mUnit[i]->mTurn) {
				stage->mUnit[i]->getFocus();
				break;
			}
		}
	}
};
Stage::Control::~Control() {
	if (mSelectUnit) {
		delete mSelectUnit;
		mSelectUnit = 0;
	}
	if (mCommand) {
		delete mCommand;
		mCommand = 0;
	}
};
SelectSquare* Stage::Control::newSelectUnit(Stage* stage) {
	//ユニット選択
	StageArea area;
	area.create(stage);

	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (stage->mUnit[i]) {
			if (stage->mUnit[i]->mTurn) {
				area.set(
					stage->mUnit[i]->mX,
					stage->mUnit[i]->mY,
					1);
			}
		}
	}

	SelectSquare* result = new SelectSquare;
	result->create(area,true);

	return result;
}

int Stage::Control::update(Stage* parent) {
	if (mSelectUnit == 0 && mCommand ==0) {
		mSelectUnit = newSelectUnit(parent);
	}

	//ユニット選択
	if (mSelectUnit) {
		mSelectUnit->draw(parent);
		if (mSelectUnit->update(parent) == 1) {
			mCommand = new Command(
				parent->unit(mSelectUnit->mX,mSelectUnit->mY) );

			delete mSelectUnit;
			mSelectUnit = 0;
		}
	}
	else {
		//ユニットコマンド
		if (mCommand->update(parent)) {
			delete mCommand;
			mCommand = 0;
		}
	}

	//全員のターンがなくなったら、フェーズ終了
	int turn = 0;
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (parent->mUnit[i]) {
			turn += parent->mUnit[i]->mTurn;
		}
	}
	if (turn == 0) {
		parent->mNextPhase = new Stage::TurnReady(parent);
	}

	return 0;
};


///////////////////////////////////////////////////////

//ユニットコマンド
Stage::Control::Command::Command(Unit* ut){
	mUnit = ut;
	mUnitAbilityList.position(-320,240);
	mUnitAbilityList.set(ut,0);
	mSelectTarget = 0;

	ut->mCast.clear();
}
Stage::Control::Command::~Command(){
	if (mSelectTarget) {
		delete mSelectTarget;
		mSelectTarget = 0;
	}
};


int Stage::Control::Command::update(Stage *stage) {
	int result = 0;

	//行動回数ゼロなら抜ける
	if (mUnit->mTurn == 0) {
		result = 2;
	}
	else {
		if (mSelectTarget == 0) {
			//アビリティ選択
			Draw::instance()->mCamera.update();

			mUnit->getFocus();

			int r=mUnitAbilityList.update(true);
			if (r ==1) {
				//使用可能なら、座標選択へ
				if (mUnitAbilityList.ability()->isAvailable() == 1) {
					mSelectTarget = new SelectTarget(stage,mUnitAbilityList.ability());
				}
			}
			if (r==2) {
				//キャンセル
				if (mUnit->mCast.cancel()) {
					//ユニット選択に戻る
					result = 2;
				}
				mUnitAbilityList.refresh();
				stage->refresh();
			}
			stage->drawFrontEnd();
			mUnit->drawInMap(true);
			mUnitAbilityList.draw();
		}
		else {
			//座標選択
			int r=mSelectTarget->update(stage);

			mSelectTarget->draw(stage);
			if (r) {
				//決定。
				if (r == 1) {
					//発動フェーズつくる
					stage->mCastPhase = new Stage::CastAbility(stage);

					if (stage->mCastPhase->create(mUnit,
						mUnitAbilityList.ability(),
						mSelectTarget->mSelectSquare.mX,
						mSelectTarget->mSelectSquare.mY)) {

						//発動成功
						mUnit->mCast.add(mUnitAbilityList.ability());

						delete mSelectTarget;
						mSelectTarget = 0;
					}
					else {
						//発動不可なんですけど
						delete stage->mCastPhase;
						stage->mCastPhase = 0;
					}
				}
				//キャンセル
				if (r == 2) {
					delete mSelectTarget;
					mSelectTarget =0;
				}
				mUnitAbilityList.refresh();
			}
		}
	}

	return result;
}

//座標選択
Stage::Control::Command::SelectTarget::SelectTarget(Stage* stage,Unit::Ability* ab){
	//ダメージ先行計算
	mCastResult.calc(stage,*ab);

	mArea.create(stage);
	mAbility = ab;

	//射程範囲を出す
	StageArea sa;
	sa.create(stage);
	mAbility->setRange(&sa);
	mSelectSquare.create(mAbility->mUnit->mX,mAbility->mUnit->mY,sa,false);

	mSelectSquare.mPrimaryUnit=mAbility->mUnit;
	mSelectSquare.mSelectArea.mColor = 0xffff0000;
	mArea.mColor = 0xffff0000;
};

int Stage::Control::Command::SelectTarget::update(Stage* stage) {
	int r=mSelectSquare.update(stage);

	//効果範囲を更新
	mAbility->setArea(&mArea,mSelectSquare.mX,mSelectSquare.mY);
	return r;
};
void Stage::Control::Command::SelectTarget::draw(Stage* stage) {
	mSelectSquare.draw(stage);
	mArea.draw();

	//行動結果表示
	Unit* ut = stage->unit(mSelectSquare.mX,mSelectSquare.mY);
	if (ut) {
		mCastResult.draw(0,-100,ut);
		ut->drawInMap(false);
	};

};
///////////////////////////////////////////////////////
//敵フェーズ
Stage::Computer::Computer(Stage* stage) {
	mCurrentUnit = 0;
	mCast = 0;
	mRange.create(stage);
	mArea.create(stage);
	mCount = 0;
};
int Stage::Computer::update(Stage* stage) {
	if (mCast == 0) {
		if (mCurrentUnit==0) {
			//まず誰を動かすかきめる
			float max = -65536.f;
			for (int i=0 ; i<Stage::mUnitMax ; ++i) {
				if (stage->mUnit[i]) {
					if (stage->mUnit[i]->mTurn > 0) {
						float t = calc(stage->mUnit[i]);
						if (max < t) {
							max = t;
							mCurrentUnit = stage->mUnit[i];
						}
					}
				}
			}
		}
		//該当者なしだったらどうしよう？
		if (mCurrentUnit) {
			//計算
			int ab,tx,ty;
			calc(mCurrentUnit,&ab,&tx,&ty);

			//対象マス表示が要るなあ
			mCurrentUnit->mAbility[ab].setRange(&mRange);
			mCurrentUnit->mAbility[ab].setArea(&mArea,tx,ty);
			mCount = 0;

			//発動データつくる
			mCast = new Stage::CastAbility(stage);	//発動フェーズをつくる
			mCast->create(
				mCurrentUnit,
				&mCurrentUnit->mAbility[ab],
				tx,ty);

			mCurrentUnit->mCast.add(&mCurrentUnit->mAbility[ab]);

			//誰が行動するか考え直す
			if (mCurrentUnit->mCast.turnEnd()) {
				mCurrentUnit = 0;
			}
		}
	}

	//発動つくる
	if (mCast) {
		//対象マス表示
		++mCount;
		mRange.draw();
		if (mCount > 30) {
			mArea.draw();
			stage->getFocus(mArea.mX,mArea.mY);
		}
		if (mCount >60) {
			//発動フェーズを渡しす
			stage->mCastPhase = mCast;
			mCast=0;
		}
	}
	else {
		//発動フェーズがつくれなかったら、ターン終了
		stage->mNextPhase = new Stage::TurnReady(stage);
	}

	return 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//優先順位の高い行動を計算する

float Stage::Computer::calc(Unit* unit,int* ab,int* targetX,int* targetY){
	float result = 0.f;

	//とりあえず一番上のアビリティで初期化しとく。
	if (ab) {
		*ab=0;
	}
	if (targetX) {
		*targetX = 0;
	}
	if (targetY) {
		*targetY = 0;
	}

	//高速化のため、範囲変数は定義
	StageArea range;
	range.create(unit->mStage);
	StageArea area;
	area.create(unit->mStage);

	for (int k=0 ; k<Unit::mAbilityMax ; ++k) {
		if (unit->mAbility[k].isAvailable()==1) {
			unit->mCast.add(&unit->mAbility[k]);

			unit->mAbility[k].setRange(&range);

			for (int x=0 ; x<unit->mStage->mWidth ; ++x) {
				for (int y=0 ; y<unit->mStage->mHeight ; ++y) {
					if (range.get(x,y)) {
						float tmp = 0.f;

						unit->mAbility[k].setArea(&area,x,y);

						CastResult cr;
						if (cr.calc(unit->mStage,unit->mAbility[k],&area)) {
							tmp += cr.mPriority;

							//移動の場合、ユニット位置を変える
							if (unit->mAbility[k].mAbility->mAbility->mType == ABILITY_MOVE) {
								unit->launch(x,y);
							}
							//行動終了でないなら、再帰するぞ
							if (!unit->mCast.turnEnd()) {
								tmp += calc(unit);
							}
							if (tmp > result) {
								result = tmp;
								if (ab) {
									*ab = k;
								}
								if (targetX) {
									*targetX = x;
								}
								if (targetY) {
									*targetY = y;
								}
							}
						}
					}
				}
			}
			unit->mCast.cancel();
		}
	}
	return result;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////

//ユニット消去

Stage::KillUnit::KillUnit(Stage* stage) {
	mCount = 0;
}

int Stage::KillUnit::update(Stage* stage) {
	++mCount;

	//LPダメージフェーズ
	if (mCount >0) {
		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			if (stage->mUnit[i]) {
				if (stage->mUnit[i]->mLPDamage) {
					stage->mUnit[i]->createNumber(stage->mUnit[i]->mLPDamage,0xffff8080);
					stage->mUnit[i]->getFocus();

					stage->mUnit[i]->mLP -= stage->mUnit[i]->mLPDamage;
					stage->mUnit[i]->mLPDamage = 0;
					mCount -= 30;

					break;
				}
			}
		}
	}
	//ユニット消去フェーズ
	if (mCount>0){
		//フェードアウト
		float a = 1.f * (32-mCount) / 32.f;
		for (int i=0 ; i<Stage::mUnitMax; ++i) {
			if (stage->mUnit[i]) {
				if (stage->mUnit[i]->isKilled()) {
					stage->mUnit[i]->mObject->setColor(a,1.0,1.0,1.0);
					stage->mUnit[i]->mObject->mBlendInfo.setBlendMode(1);
				}
			}
		}

		//ユニット消す
		if (mCount == 32) {
			for (int i=0 ; i<Stage::mUnitMax; ++i) {
				if (stage->mUnit[i]) {
					if (stage->mUnit[i]->isKilled()) {

						//経験値・ドロップは手にはいるか？
						stage->mUnit[i]->getDrop(stage->mGameData);

						//子を殺す
						//攻撃者登録を消す（親、状態異常）
						//ゲストゴーストなら、消す

						stage->removeUnit(i);
					}
				}
			}
		}
	}

	return mCount > 32;
};

//////////////////////////////////////////////////////////////////////////////////////////

//アニメフェーズ
//長いので別ファイルにすること推奨

Stage::CastAbility::CastAbility(Stage* stage) {
	mStage = stage;
	for (int i=0 ; i<mCastMax ; ++i) {
		mCast[i] = 0;
	}
	mCurrentCast = 0;
	mUnit = 0;
};
Stage::CastAbility::~CastAbility() {
	for (int i=0 ; i<mCastMax ; ++i) {
		if (mCast[i]) {
			delete mCast[i];
			mCast[i] = 0;
		}
	}
}
bool Stage::CastAbility::create(Unit* ut,Unit::Ability* ab,int i,int j) {
	bool result;
	//キャスト情報登録
	//さらに連携データもつくること
	mUnit = ut;

	//派生クラスをもらう関数
	mCast[0] = newCastClass( ab->mAbility->mAbility->mID );
	result = mCast[0]->create(ut,ab,i,j);

	return result;
};
int Stage::CastAbility::update(Stage* stage) {
	int result = 0;

	//登録されてるcastを片付けていく
	bool b=true;						//ぜんぶのエフェクト終わってる？
	for (int i=0 ; i<mCastMax; ++i) {
		if (mCast[i]) {
			int j=mCast[i]->update(stage);
			if (j==0) {
				b=false;
				break;
			}
			if (j<2) {
				b=false;
			}
		}
	}

	//効果を適用して死ぬ
	if (b) {
		for (int i=0 ; i<mCastMax ; ++i) {
			if (mCast[i]) {
				mCast[i]->apply(stage);
			}
		}
		result = 1;
	}

	return result;
}
//終了処理
void Stage::CastAbility::end(Stage* stage) {
	//ホールドアクションか否か
	if (!mUnit->mCast.hold()) {

		//行動消費
		if (mUnit->mCast.turnEnd()) {
			-- mUnit->mTurn;
		}

		//ポイント・呪力コスト消費
		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			if (stage->mUnit[i]) {
				stage->mUnit[i]->mCast.apply();
			}
		}
	};
};

bool Stage::CastAbility::Cast::create(Unit* ut,Unit::Ability* ab,int i,int j) {
	mUnit = ut;
	mAbility = ab;
	mX=i;
	mY=j;

	//範囲を決めて、効果を再計算
	StageArea sa;
	sa.create(ut->mStage);
	ab->setArea(&sa,i,j);

	mCastResult.calc(ut->mStage,*ab,&sa);

	return mCastResult.mAvailable;

};
void Stage::CastAbility::Cast::apply(Stage* stage) {
	//スペカなら、想起用スペルをおぼえておく
	if (*mAbility->mAbility->mAbility->mElement.mSpell) {
		stage->mLastSpell = mAbility->mAbility;
	};

	//タイプによって場合わけ
	//移動
	if (mAbility->mAbility->mAbility->mType == ABILITY_MOVE) {
		mUnit->launch(mX,mY);
	}
	//召還
	if (mAbility->mAbility->mAbility->mType == ABILITY_CAST) {
		dShell* s = Database::instance()->mShellManager.shell(
			mAbility->mAbility->mAbility->mString.data());

		Unit* ut=stage->createUnit(
			s,
			mUnit->mLevel,0,mUnit->mParty,false,0,1,
			mUnit->mGhost->mRank,
			mUnit->mGhost->mDifficulty,
			0,0);
		ut->launch(mX,mY);
		ut->mDefined = true;
		if (!mAbility->mAbility->mAbility->mParameter) {
			ut->mParent = mUnit;
		}

		//ASSERT(0);	//とめとくよ
	}
	//攻撃・待機・サポート
	if (mAbility->mAbility->mAbility->mType == ABILITY_WAIT ||
		mAbility->mAbility->mAbility->mType == ABILITY_ATTACK ||
		mAbility->mAbility->mAbility->mType == ABILITY_SUPPORT) {

		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			bool hit=false;

			if (mCastResult.mDamage[i].mResult) {
				//アクションをうけたら、待機カウンターゼロに
				if (stage->mUnit[i]->mWait) {
					stage->mUnit[i]->mWait = 0;
				}

				for (int j=0 ; j<mCastResult.mDamage[i].mCombo ; ++j) {
					if (mCastResult.mDamage[i].mHit[j]) {
						stage->mUnit[i]->mLastEnemy = mCastResult.mUnit;

						//ダメージ
						if (mCastResult.mDamage[i].mResult == 1) {
							hit = true;
							stage->mUnit[i]->mHP -= mCastResult.mDamage[i].mDamage[j];

							//残機ダメージ
							if (stage->mUnit[i]->mHP < 1) {
								++stage->mUnit[i]->mLPDamage;
								stage->mUnit[i]->mHP = stage->mUnit[i]->mRecoverHP;
							}
						}
						if (mCastResult.mDamage[i].mResult == 2) {
							//回復
							hit = true;
							stage->mUnit[i]->mHP += mCastResult.mDamage[i].mDamage[j];
						}
					}
				}
				//ここに追加効果
				//いっぱつでも成功してる場合のみ
				if (hit) {
					stage->mUnit[i]->setState(
						mCastResult.mDamage[i].mState,
						mCastResult.mDamage[i].mStateLevel,
						true,
						mCastResult.mUnit);
				}
			}
		}
	}
	//ここに反動効果
	mUnit->setState(
		mCastResult.mAbility->mAbility->mAbility->mRecoilState,
		mCastResult.mAbility->mAbility->mAbility->mRecoilStateLevel,
		false,0);
}




//////////////////////////////////////////////////////////////////////////////
//ダメージ計算
Damage::Damage() {
	mCombo = 0;
	mResult = 0;
	for (int i=0 ; i<mComboMax ; ++i) {
		mHit[i] = false;
		mDamage[i] = 0;
		mCritical[i] = false;
	}
	mState = 0;
	mStateLevel = 0;
	mHitProbability = 0;
	mCriticalProbability = 0;
	mTotalDamage = 0;
};

CastResult::CastResult() {
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		//なにすんのここ？	
	}
	mPriority=0;
	mUnit = 0;
	mAbility = 0;
};
void CastResult::draw(int x,int y,Unit* ut) const{
	if (ut->mDefined) {
		int size = 1;
		unsigned int color = 0xffffffff;
		int line = -24;
		Frame::draw(x,y,160,160);
		Draw::instance()->text(x    ,y       ,mAbility->mAbility->mAbility->mName.data(),color,size);
		Draw::instance()->text(x    ,y+line*1,"ダメージ",color,size);
		Draw::instance()->text(x+128,y+line*1,mDamage[ut->getIndex()].mTotalDamage,color,size);
		Draw::instance()->text(x    ,y+line*2,"命中",color,size);
		Draw::instance()->text(x+128,y+line*2,mDamage[ut->getIndex()].mHitProbability,color,size);
		Draw::instance()->text(x    ,y+line*3,"Cri",color,size);
		Draw::instance()->text(x+128,y+line*3,mDamage[ut->getIndex()].mCriticalProbability,color,size);
		Draw::instance()->text(x    ,y+line*4,"追加効果",color,size);
	}
}
bool CastResult::calc(Stage* stage,Unit::Ability& ab,const StageArea* sa) {

//mode 0
//0ダメージ計算モード
//1優先度計算モード
//2予想ダメージ計算モード


	//ダメージ計算式をつくろう
	//パラメータ
	//体力				50～200		＞　500-1000
	//攻撃力			50-150		＞　150-300
	//アビリティ威力	100-200		＞　200-500
	//相手防御　　　　　0			＞	0-250

	//基本方針
	//ダメージ＝攻撃*威力-防御
	//50*100 = 5000
	//150*200 = 30000

	//150*200 = 30000
	//300*500 = 150000

	// /150
	// /150

	//ここで計算
	mUnit = ab.mUnit;

	mPriority = 0.f;
	//1.fで一体倒せる優先度.

	mAbility = &ab;
	Ability* abl = ab.mAbility;
	//まず初期化
	for (int i=0; i<Stage::mUnitMax ; ++i) {
		mDamage[i].mResult = 0;
	}

	//共通部分
	//呪力コストによるマイナス補正　最大50％

	mPriority -= 0.1f * mAbility->mAbility->mAbility->mCost;
	//使用回数によるマイナス補正　最大50％
	if (mAbility->mAbility->point()) {
		mPriority -= 0.5f / mAbility->mAbility->point();
	};
	//反動効果による補正
	mPriority -= mUnit->checkState(
		mAbility->mAbility->mAbility->mRecoilState,
		mAbility->mAbility->mAbility->mRecoilStateLevel,false);

	//範囲内にユニットがいると失敗するタイプ
	if (abl->mAbility->mType == ABILITY_MOVE ||
		abl->mAbility->mType == ABILITY_CAST) {
		mAvailable = true;

		//ダメージ予測モードなら、計算いらない
		if (sa) {
			for (int i=0; i<Stage::mUnitMax ; ++i) {
				if (stage->mUnit[i]) {
					//範囲内にいるか？
					if (sa->get(
						stage->mUnit[i]->mX,
						stage->mUnit[i]->mY)) {

						mAvailable = false;
						break;
					};
				}
			}
			if (abl->mAbility->mType == ABILITY_MOVE) {
				//移動だろうが召喚だろうが、敵ユニットに近い方が優先する
				//補助系ユニットは味方に向けるべきだが、そりゃ無理だ
				//親ユニットがいるなら、そっちに向かう。

				StageArea tmp;
				tmp.create(stage);
				tmp.set(0);
				tmp.walk(
					sa->mX,sa->mY,
					64,
					mUnit->interrupt(),	
					*mUnit->mStatus.mUp,
					*mUnit->mStatus.mDown);
				//敵ユニットからの距離を得る
				int t=0;	//ポイント
				int n=0;	//敵ユニット数
				if (mUnit->mParent) {
					//親がいるなら、親が全てだ
					t += tmp.get(
						mUnit->mParent->mX,
						mUnit->mParent->mY);
					++n;
				}
				else {
					for (int i=0 ; i<Stage::mUnitMax ; ++i) {
						if (stage->mUnit[i]) {
							if (mUnit->AI()!=stage->mUnit[i]->mParty) {
								t += tmp.get(
									stage->mUnit[i]->mX,
									stage->mUnit[i]->mY);
								++n;
							}
						}
					}
				}

				//移動の優先度を計算する。要するに敵に近づけばいいわけよ。
				//最大0.1
				if (abl->mAbility->mType == ABILITY_MOVE) {
					if (n) {
						mPriority += 0.1f*t/n/64.f;
					}
					if (mUnit->mWait) {
						mPriority -= 0.1f;
					}
				}
				//召喚系の有効度
				//最大0.8　かなり高い。　何が出てくるかは無視だ。
				if (abl->mAbility->mType == ABILITY_CAST) {
					if (n) {
						mPriority += 0.8f*t/n/64.f;
					}
					if (mUnit->mWait) {
						mPriority -= 0.8f;
					}
				}
			}
		}
		return mAvailable;
	}

	//計算しよう
	mAvailable=false;
	for (int i=0; i<Stage::mUnitMax ; ++i) {
		if (stage->mUnit[i]) {
			bool b=true;
			if (sa) {
				//範囲内にいるか？
				if (!sa->get(
					stage->mUnit[i]->mX,
					stage->mUnit[i]->mY)) {
					b=false;
				}
			}
			if (b) {
				//対象自分で攻撃だと関係なしにしておく
				if (mUnit == stage->mUnit[i]) {
					if (abl->mAbility->mType == ABILITY_ATTACK) {
						continue;
					}
				}
				//識別属性　味方に攻撃・敵にサポートを無効
				if (*abl->mAbility->mElement.mSmart) {
					if (abl->mAbility->mType == ABILITY_ATTACK) {
						if (mUnit->AI() == stage->mUnit[i]->mParty) {
							continue;
						}
					}
					if (abl->mAbility->mType == ABILITY_SUPPORT) {
						if (mUnit->AI() != stage->mUnit[i]->mParty) {
							continue;
						}
					}
				}

				//結果
				//0関係なし　1ダメージ　2回復　3無効化
				int damage;
				if (abl->mAbility->mType == ABILITY_WAIT) {
					damage = 2;
				}
				if (abl->mAbility->mType == ABILITY_ATTACK) {
					damage = 1;
				}
				if (abl->mAbility->mType == ABILITY_SUPPORT) {
					damage = 2;
				}

				mAvailable = true;
				//ダメージか回復のときのみ計算
				//基本効果量
				int value = 0;
				if (ab.mAbility->mAbility->mPowerType == POWER_CONST) {
					value = mAbility->mPower;
				}
				if (ab.mAbility->mAbility->mPowerType == POWER_RATE){
					value = *stage->mUnit[i]->mStatus.mHP * mAbility->mPower / 100;
				}
				if (ab.mAbility->mAbility->mPowerType == POWER_NORMAL){
					value = *mUnit->mStatus.mAttack * mAbility->mPower / 150;
				}

				//属性補正
				int ele=0;
				for (int j=0 ; j<Element::mMax ; ++j) {
					if (abl->mAbility->mElement.mElement[j]) {
						ele += stage->mUnit[i]->mResistElement.mElement[j];
					}
				}
				//耐性値合計　-抜群　0普通　+耐性　100無効化
				if (ele > 100) {
					value = value * (ele-100) /100;
					damage = 2;
				}
				if (ele == 100) {
					value = 0;
					damage = 3;
				}
				if (ele<100) {
					value = value * (100-ele) /100;
				}
				mDamage[i].mResult = damage;

				//命中率
				//回避が適用されるのはダメージ時・非必中属性
				int ht = *mUnit->mStatus.mHit+100;
				if (damage == 1 && 	*abl->mAbility->mElement.mUnavoidable == 0) {
					ht -= *stage->mUnit[i]->mStatus.mAvoid;
				}
				ht = ht * (100+abl->mAbility->mHit) / 100;

				//極限0になる漸近線をつくろう
				if (ht<100) {
					int av = 100-ht;
					ht = 100 - asymptote(av,100,2500);	//回避50のとき、確率50%
				}
				else {
					ht = 100;
				}
				mDamage[i].mHitProbability = ht;
	
				//クリティカル率
				mDamage[i].mCriticalProbability = abl->mAbility->mCritical + *mUnit->mStatus.mCritical;

				//ヒット数
				mDamage[i].mCombo = abl->mAbility->mCombo;

				//予想ダメージ量を求める
				int hp2 = stage->mUnit[i]->mHP;
				int lp2 = stage->mUnit[i]->mLP-1;
				int hp3 = stage->mUnit[i]->mHP;
				int lp3 = stage->mUnit[i]->mLP-1;

				//各ヒットごとの処理
				for (int j=0 ; j < mDamage[i].mCombo ; ++j) {
					//いっぱつ辺りの効果量
					int v = value / mDamage[i].mCombo;

					//予想効果量
					int v2 = v;
					//優先度計算用
					int v3 = v;

					//当たったか？
					if (mDamage[i].mHitProbability > cRandom::instance()->getInt(100)) {
						mDamage[i].mHit[j] = true;
					}
					else {
						mDamage[i].mHit[j] = false;
					}

					//優先度
					v3 = v3 * mDamage[i].mHitProbability / 100;

					//クリティカルか？
					if (damage == 1) {
						mDamage[i].mCritical[j] = mDamage[i].mCriticalProbability > cRandom::instance()->getInt(100);

						int critical = stage->mUnit[i]->critical();
						//クリティカル効果
						if (mDamage[i].mCritical[j]) {
							v = v * (100+critical) /100;
						}

						//優先度
						v3 = v3*(100+ (critical*mDamage[i].mCriticalProbability/100))/100;
					}
					else {
						mDamage[i].mCritical[j] = false;
					}

					//乱数適用
					//計算式通常のみ　固定・割合だと固定
					//乱数は+-10％でいいだろう
					//ダメージ値がでかすぎるとオーバーフローするので注意。
					if (abl->mAbility->mPowerType == POWER_NORMAL) {
						int r=cRandom::instance()->getInt(201) -100;
						v = v * (1000 + r) / 1000;
					}

					//防御値を適用
					if (*abl->mAbility->mElement.mPierce == 0) {
						//ダメージ計算
						v2 -= *stage->mUnit[i]->mStatus.mDefense;
						if (v2<1) {
							if (mAbility->mPower) {
								v2=1;
							}
							else {
								v2=0;
							}
						}

						//優先度計算
						v3 -= (100 - mDamage[i].mCriticalProbability) * *stage->mUnit[i]->mStatus.mDefense;
						if (v3<1) {
							if (mAbility->mPower) {
								v3=1;
							}
							else {
								v3=0;
							}
						}

						if (!mDamage[i].mCritical[j] && damage == 1) {
							v -= *stage->mUnit[i]->mStatus.mDefense;
							//ミニマムダメージチェック
							if (v<1) {
								if (mAbility->mPower) {
									v=1;
								}
								else {
									v=0;
								}
							}
						}
					}
					mDamage[i].mDamage[j] = v;

					//予想ダメージ
					if (damage == 1) {
						hp2 -= v2;
						if (hp2 <= 0) {
							hp2 = stage->mUnit[i]->mRecoverHP;
							--lp2;
						}
						//優先度計算
						hp3 -= v3;
						if (hp3 <= 0) {
							hp3 = stage->mUnit[i]->mRecoverHP;
							--lp3;
						}
					}
					else {
						hp2 += v2;
						if (hp2 > *stage->mUnit[i]->mStatus.mHP) {
							hp2 = *stage->mUnit[i]->mStatus.mHP;
						}
						hp3 += v3;
						if (hp3 > *stage->mUnit[i]->mStatus.mHP) {
							hp3 = *stage->mUnit[i]->mStatus.mHP;
						}
					}
				}
				//追加効果
				mDamage[i].mState= mAbility->mAbility->mAbility->mExtraState;
				mDamage[i].mStateLevel= mAbility->mAbility->mAbility->mExtraStateLevel;

				//総ダメージ予想
				int d = lp2 * *stage->mUnit[i]->mStatus.mHP + hp2;
				if (d < 0) {
					d = 0;
				}
				mDamage[i].mTotalDamage = stage->mUnit[i]->getTotalHP() - d;
				if (mDamage[i].mTotalDamage < 0) {
					mDamage[i].mTotalDamage = -mDamage[i].mTotalDamage;
				}

				//優先度計算
				int e = lp3 * *stage->mUnit[i]->mStatus.mHP + hp3;
				if (e < 0) {
					e = 0;
				}
				int f = stage->mUnit[i]->getTotalHP() - e;		//期待ダメージ					
				float p = 1.f * f / stage->mUnit[i]->getTotalHP();

				//追加効果による優先度
				p += stage->mUnit[i]->checkState(
					mAbility->mAbility->mAbility->mExtraState,
					mAbility->mAbility->mAbility->mExtraStateLevel,
					true);

				//最後に正負を決めて
				if (mUnit->AI() == stage->mUnit[i]->mParty){
					p=-p;
				}
				//最後にデコイ補正
				if (p>0.f) {
					p *= (10.f + stage->mUnit[i]->mDecoy) / 10.f;
				}

				mPriority += p;
			}
		}
	}
	//攻撃ならそのまま
	//待機なら0.1f固定
	//それ以外はマイナス
	if (mUnit->mWait) {
		if (!abl->mAbility->mType == ABILITY_ATTACK) {
			if (abl->mAbility->mType == ABILITY_WAIT) {
				mPriority = 0.1f;
			}
			else {
				mPriority = -256.f;
			}
		}
	}

	return mAvailable;

}


