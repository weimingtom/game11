

#include "stage.h"
#include "anime.h"
#include "npmath.h"
#include "npobject.h"
#include "modellist.h"
#include "config.h"

Stage::Stage(GameData* gamedata) {
	//������
	Sprite::instance()->removeObject();
	//�J����������
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


	//���
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

	//�n�`��p��
	loadFMF(battle->mMap.data());
	//�w�i��p��
	//battle->mBack.data();

	mTitle = battle->mTitle;
	mScript = battle->mScript;
	mWeather.change(battle->mWeather);

	//�{�[�i�X
	mBonus.mTool[0] = battle->mBonus[0];
	mBonus.mTool[1] = battle->mBonus[1];
	mBonus.mTool[2] = battle->mBonus[2];
	mBonus.mTool[3] = battle->mBonus[3];
	mBonus.mScore = battle->mScore;
	mBonus.mPower = battle->mPower;
}
void Stage::loadFMF(const char* file) {
	//�}�b�v�쐬
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
			int y=mHeight - j -1;	//������Y�����]

			square(i,j)->mX=i;
			square(i,j)->mY=j;
			square(i,j)->mPit=false;
			square(i,j)->mWall=false;
			square(i,j)->mWater=false;

			if (fmf.data(2,x,y) == 255) {
				square(i,j)->mZ=-1;
			}
			else {
				//square(i,j)->mZ=fmf.data(2,x,y);		//����
				square(i,j)->mZ=fmf.data(2,x,y) % 256 + fmf.data(2,x,y) / 256 * 16;
			}

			//�`�b�v
			if (fmf.data(0,x,y)==255) {
				square(i,j)->mU=0;
				square(i,j)->mV=0;
			}
			else {
				square(i,j)->mU=fmf.data(0,x,y) % 256;	//�`�b�v
				square(i,j)->mV=fmf.data(0,x,y) / 256;
			}

			//�n�`�^�C�v
			//256�ʏ�
			//0-3��
			//255�n�`
			//257��
			//258��
			//256��
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


			//�C�x���g�|�C���g
			int e = fmf.data(4,x,y) % 256 + fmf.data(4,x,y) / 256 * 8;
			square(i,j)->mEvent=e;
		}
	}
	createBatch();
};

//3D�}�b�v�^�C�v�̃o�b�`
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

	mUnit[i]->mGhost->mState = 1;	//�o�����ɂ��Ă���

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
			//�q���E��
			if (mUnit[w]->mParent == mUnit[i]) {
				mUnit[w]->mLPDamage = 256;
			}

			//�����̍U���ҏ�������
			if (mUnit[w]->mLastEnemy == mUnit[i] ) {
				mUnit[w]->mLastEnemy = 0;
			}
			//��Ԉُ��
			for (int k=0 ; k<State::mMax ; ++k) {
				if (mUnit[w]->mState.mMaster[k] == mUnit[i]) {
					mUnit[w]->mState.mMaster[k]=0;
				}
			}
		}
	}

	//�퓬��A�Q���������ǂ������f����̂�
	//�����ł͏o����Ԃ�������Ȃ�

	//�Q�X�g�S�[�X�g�Ȃ�S�[�X�g����
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

	//�V�X�e�����j���[
	if (mSystemMenu) {
		if (mSystemMenu->update(this)) {
			delete mSystemMenu;
			mSystemMenu = 0;
		}
		return 0;
	}

	//�����t�F�[�Y
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
	//���j�b�g�����t�F�[�Y
	if (mKillUnitPhase) {
		if (mKillUnitPhase->update(this)) {
			delete mKillUnitPhase;
			mKillUnitPhase = 0;
		}
		return 0;
	}
	//�i���Q�b�g
	if (mGameData->updateBag() == 0) {
		return 0;
	}

	//�p�����[�^�m�F
	if (mUnitStatusPhase) {
		if (mUnitStatusPhase->update()) {
			delete mUnitStatusPhase;
			mUnitStatusPhase = 0;
		}
		return 0;
	}
	//�p�[�e�B�W�J�t�F�[�Y
	if (mDeployPhase) {
		if (mDeployPhase->update(this)) {
			delete mDeployPhase;
			mDeployPhase = 0;
		}
		return 0;
	}
	//�X�N���v�g���s
	if (mScriptRunning) {
		if (mScript->update(mGameData)) {
			mScriptRunning = false;
		}
		return 0;
	}

	//�I���t�F�[�Y
	if (mResult) {
		if (mOverPhase == 0) {
			mOverPhase = new Over(mResult,mGameData);
		}
		if (mOverPhase->update(this)) {
			return mOverPhase->mResult;
		}
		return 0;
	}

	//�^�[���v�Z�t�F�[�Y
	//�R���g���[���^�[��
	//COM�^�[��
	//�Ȃɂ��Ȃ�������`���[�W�t�F�[�Y
	if (mPhase == 0) {
		mPhase = new TurnReady(this);
	}
	int r=mPhase->update(this);
	if (mNextPhase) {
		delete mPhase;
		mPhase = mNextPhase;
		mNextPhase = 0;
	}

	//�퓬�I�����f
	return 0;
}

//�p�����[�^�Čv�Z
void Stage::refresh() {
	//���̓��Z�b�g
	for (int i=0 ; i<2 ; ++i) {
		mParty[i].mCapacity=5;
		mParty[i].mChargeSpeed=16;
	}
	//ZOC���Z�b�g
	mLuckZOC.set(0);
	mDisturbZOC.set(0);

	//���ڂ̃��[�v�B
	//�X�e�[�^�X�͌��܂�Ȃ����AZOC�͓W�J���Ă����B
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			mUnit[i]->recalculation1();
		}
	}
	//���ڂ̃��[�v�B�X�e�[�^�X�m��B
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			mUnit[i]->recalculation2();

			int p=mUnit[i]->mParty;

			mParty[p  ].mCapacity += mUnit[i]->abilityTotalPower(204);	//���͗e��
			mParty[1-p].mCapacity += mUnit[i]->abilityTotalPower(205);	//���͑j�Q

			mParty[p].mChargeSpeed += *mUnit[i]->mStatus.mAura;			//���
		}
	}


	//���͗e�ʁE�`���[�W���x�͈̔̓`�F�b�N
	for (int i=0 ; i<2 ; ++i) {
		if (mParty[i].mCapacity<0) {
			mParty[i].mCapacity=0;
		}
		if (mParty[i].mChargeSpeed < 16) {
			mParty[i].mChargeSpeed=16;
		}
	}

	//AP�`���[�W���x�𒲂ׂ�
	//�ő呬�x��64F�Ńt���`���[�W�B�[���ł��̔����B

	//�܂��ő呬�x
	int maxspeed=1;
	for (int i=0 ; i<mUnitMax ; ++i) {
		if (mUnit[i]) {
			if (maxspeed < *mUnit[i]->mStatus.mSpeed) {
				maxspeed = *mUnit[i]->mStatus.mSpeed;
			}
		}
	}
	//AP�`���[�W���x�𒲂ׂ�B
	//���x/�ő呬�x*65536/64/2�@+65536/64/2
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
	//�}�b�v�\��

	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; j<mHeight ; ++j) {
			square(i,j)->draw(this,false);
		}
	}

	BlendInfo bi;
	bi.mAmbient.set(0.f,0.f,0.f);
	mTest.batch("�����`�F�b�J�[")->draw(256,256,256,bi);

	mLuckZOC.draw();
	mDisturbZOC.draw();
};
void Stage::drawArea2()const {
	//����
	for (int i=0 ; i<mWidth ; ++i) {
		for (int j=0 ; i<mHeight ; ++i) {
			square(i,j)->draw(this,true);
		}
	}

}

void Stage::drawChara() const{
	//�L�����\��
	Sprite::instance()->drawObject();
}

void Stage::drawFrontEnd() const{
	//���j�b�g�e
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
		Draw::instance()->text(x,y+line*c,"��",color,size);
	}
	if (mWeather.get()==1) {
		Draw::instance()->text(x,y+line*c,"��",color,size);
	}
	if (mWeather.get()==2) {
		Draw::instance()->text(x,y+line*c,"��",color,size);
	}
	if (mWeather.get()==3) {
		Draw::instance()->text(x,y+line*c,"��",color,size);
	}
	if (mWeather.get()==4) {
		Draw::instance()->text(x,y+line*c,"��",color,size);
	}
	if (mWeather.get()==5) {
		Draw::instance()->text(x,y+line*c,"�J",color,size);
	}
	if (mWeather.get()==6) {
		Draw::instance()->text(x,y+line*c,"�d����",color,size);
	}

	++c;
	//����
	Draw::instance()->text(x,y+line*c,square(i,j)->mZ,color,size);
	++c;
	//�n�`
	if (square(i,j)->mWater) {
		Draw::instance()->text(x,y+line*c,"��",color,size);
		++c;
	}
	if (square(i,j)->mPit) {
		Draw::instance()->text(x,y+line*c,"���ꂪ�Ȃ�",color,size);
		++c;
	}
	if (square(i,j)->mWall) {
		Draw::instance()->text(x,y+line*c,"�i���s��",color,size);
		++c;
	}

	//�y�n�b
	if (mLuckZOC.get(i,j)) {
		Draw::instance()->text(x,y+line*c,"�K�E�㏸�y�n�b",color,size);
		++c;
	}
	if (mDisturbZOC.get(i,j)) {
		Draw::instance()->text(x,y+line*c,"�ړ��W�Q�y�n�b",color,size);
		++c;
	}

};
void Stage::createNumber(float x, float y, float z,unsigned int number, unsigned int color) const {
	//�����𒲂ׂ�
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
	//�Y�[��
	Draw::instance()->mCamera.mAimZoom = 1.f;

	//�����p�x
	Draw::instance()->mCamera.mAimVerticalAngle = 30;

	//�����p�x�́A90n+45�ɂ���
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

//�s�����𓾂�
Unit* Stage::getQueue(int p) const{
	Unit* result=0;

	//AP�`���[�W���x�v�Z
	//�ő�̂��1�b�Ńt���`���[�W
	//���x�[���ŁA�ő呬�x�̔���

	//���Ԃ𒲂ׂ�

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

//���ꂽ���j�b�g��������A�t�F�[�Y������
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
	//1F�ŗ��܂�ʂ́A
	//����� / 16 * 2/64 *65536

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
	//�\��
	mSelectArea.draw();

	//�t�H�[�J�X�\��
	BlendInfo bi;
	bi.setBlendMode(1);
	bi.mTransparency = 0.5f;
	stage->drawSquareTop(
		&ModelList::instance()->mMap.tSpot,
		mX,mY,bi);

	//�����\��
	stage->drawSquareInfo(320-160,240,mX,mY);

	//���j�b�g�E�B���h�E
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

	//�J�����ʒu
	parent->getFocus(mX,mY);

	//�J�[�\���ړ�
	//�ړ������̓J���������Ɉˑ������Ȃ���΂Ȃ�Ȃ�
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
		h-=90;	//45/90��-45/90�ɂȂ�̂ŁA���̏ꍇ�͂ЂƂ��炵�Ă��K�v������
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

	//�O�ǂɖ�����đ���ł��Ȃ��Ȃ�Ȃ��悤�ɂ���
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

	//�J��������
	if (cKey::trigger(10)) {
		Draw::instance()->mCamera.mAimHorizotalAngle += 90.f;
	}
	if (cKey::trigger(11)) {
		Draw::instance()->mCamera.mAimHorizotalAngle -= 90.f;
	}
	//�Y�[��
	if (cKey::trigger(7)) {
		if (Draw::instance()->mCamera.mAimZoom > 1.f) {
			Draw::instance()->mCamera.mAimZoom = 1.f;
		}
		else {
			Draw::instance()->mCamera.mAimZoom = 2.f;
		}
	}
	//�p�x
	if (cKey::trigger(12)) {
		if (Draw::instance()->mCamera.mAimVerticalAngle < 25.f) {
			Draw::instance()->mCamera.mAimVerticalAngle = 30.f;
		}
		else {
			Draw::instance()->mCamera.mAimVerticalAngle = 15.f;
		}
	}


	//����
	if (cKey::trigger(5)) {
		if (mSelectArea.get(mX,mY)) {
			result =1;
		};
	}

	//�L�����Z�� / ���j���[
	if (cKey::trigger(6)) {
		//���j�b�g�X�e�[�^�X���
		if (mMenu) {
			//���j�b�g��������A�p�����[�^����
			if (parent->unit(mX,mY)) {
				if (parent->unit(mX,mY)->mDefined) {
					parent->mUnitStatusPhase = new UnitStatusPhase(parent->unit(mX,mY));
				}
			}
			else {
				//���Ȃ�������A�V�X�e�����j���[
				parent->mSystemMenu = new Stage::SystemMenu(parent);
			}
		}
		else {
			result = 2;
		}
	}

	//���j�b�g�܂킷
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
	//��ʃ|���S���f�[�^
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
	//��
	if (mWall) {
		return 64 * mHeight;		//�Ƃɂ�������
	}
	if (mPit) {
		return -1;
	}
	if (mWater) {
		return -1;
	}

	if (mSlant == 256) {
		//�~��
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

	//��
	if (mPit) {
		return;
	}
	//��
	if (mWater) {
		return;
	}

	//�ő咸�_�̓}�X��*20
	//�C���f�b�N�X���̓}�X��*10*3
	int vtn = 120;
	int idn = 160;

	//�}�b�v�o�b�`�쐬
	float* u = new float[vtn];
	float* v = new float[vtn];
	float* x = new float[vtn];
	float* y = new float[vtn];
	float* z = new float[vtn];
	GameLib::Math::Vector3* NORMAL= new GameLib::Math::Vector3[vtn];
	unsigned int* COLOR= new unsigned int[vtn];
	int* index= new int[idn];

	//������邼
	int c=0;
	int d=0;

	if (mSlant == 256) {
		//�~��
		int m=8;
		mTopPolygonIndex=d;
		mTopPolygonNumber=m;

		//���
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
			//����
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
			//���
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

		//���ʂ���
		for (int k=0 ; k<4; ++k) {
			//�ׂ̃}�X�̒��_�Ɣ�r����@�߂�ǂ������ɂ�
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
	//��������Ɣ�΂������Ȃ̂ŁA�ʂɍ����ĂȂ��Ă���
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

//�R�s�[
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
	//�͈͊O�Ȃ疳������Ⴂ�������
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


//�͈͌v�Z

void StageArea::calc(RANGETYPE range, int x, int y, int unitX, int unitY, int maxRange, int minRange, int up, int down,unsigned int interrupt) {
	//�ړ�
	if (range == RANGE_MOVE) {
		walk(x,y,
			maxRange,
			interrupt,
			up,down);
	}
	//�ړ��\��
	//�ړ��ȂȂ�

	//�_
	if (range == RANGE_POINT) {
		set(0);
		set(x,y,1);

		mX = x;
		mY = y;
	}
	//�S��
	if (range == RANGE_ALL) {
		set(1);
		mX = x;
		mY = y;
	}

	//�~
	if (range == RANGE_CIRCLE) {
		circle(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//�~2
	if (range == RANGE_CIRCLE2) {
		circle2(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//��`
	if (range == RANGE_RECT) {
		square(x,y,maxRange);
		setTallRange(x,y,up,down);
	}
	//�\��
	if (range == RANGE_C) {
		cross(x,y,maxRange,up);
	}
	//�΂�
	if (range == RANGE_X) {
		xCross(x,y,maxRange,up);
		setTallRange(x,y,up,down);
	}
	//��
	//��

	//�Œ�˒��`�F�b�N
	setMinRange(x,y,minRange);
};


//���[�g���v�Z���Ă͈̔�
void StageArea::walk(int i,int j,int range,unsigned int interrupt,int up,int down) {
	mX=i;
	mY=j;

//��Q���p�����[�^
//1���E2�ǁE4���E8�v���C���[���j�b�g�E16�G�l�~�[���j�b�g�𖳎�
//�v�Z�������ƌ��E�ǁE���E���j�b�g�}�X�����O����
	set(0);						//����������
	set(i,j,range+1);		//�n�_��ݒ�

	//�܂��ړ��R�X�g�����߂Ă���
	//�G���Q���Ȃǂ������Œ��ׂĂ�����
	mStage->mTemp.set(1);

	//��Q����K�p
	if (interrupt & 1+2+4) {
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {
				if (mStage->square(i,j)->type() & interrupt) {
					mStage->mTemp.set(i,j,255);
				}
			}
		}
	}
	//���j�b�g�ז�
	if (interrupt & 8+16) {
		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			if (mStage->mUnit[i]) {
				//�ǂ������̃��j�b�g�����f���悤
				int ai = mStage->mUnit[i]->AI();

				if ((ai==0 && interrupt&8) ||
					(ai==1 && interrupt&16)) {
					mStage->mTemp.set(
						mStage->mUnit[i]->mX,
						mStage->mUnit[i]->mY,255);
				}
			}
		}
		//�ړ��W�QZOC
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

	//����
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
		//���Ɏl�����𒲂ׂĂ���ꍇ�A�����������̂ڂ��Ĉ����Ԃ�
		if (counter[record] > 3) {
			--record;

			//�n�_�ɖ߂��Ă��B�I���B
			if(record<0) {
				break;
			}

			continue;
		}

		//�ړ�
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

		//���ׂ��J�E���^�[�𑝂₵�Ă���
		++counter[record];

		//�����Ȕ͈͂�������߂낤
		if (u<0 || u>=mWidth || v<0 || v>=mHeight) {
			continue;
		}

		//�ړ��l�������葽����ԂɂȂ��Ă���A���ׂ�K�v�͂Ȃ��̂ň����Ԃ�
		int r=get(p,q) - mStage->mTemp.get(p,q);
		if (get(u,v) >= r) {
			continue;
		};

		//���������ɂЂ����������ꍇ�͈����Ԃ�
		if (!mStage->square(u,v)->mPit && !mStage->square(u,v)->mWall) { //���E�ǂɍ����͂Ȃ��B
			if (mStage->square(u,v)->mZ - mStage->square(p,q)->mZ > up ||
				mStage->square(u,v)->mZ - mStage->square(p,q)->mZ < -down) {
				continue;
			}
		}

		//�L���Ȓ�����������ŁA�ړ��l���X�V���ė����ɉ�����
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
	//���������ɂЂ�����������I��
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
				//��������
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
				//��������
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
	//��Q�������O
	if (exc & 1+2+4) {
		for (int i=0 ; i<mWidth ; ++i) {
			for (int j=0 ; j<mHeight ; ++j) {

				if (mStage->square(i,j)->type() & exc) {
					set(i,j,0);
				}
			}
		}
	}
	//���j�b�g������}�X�����O
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
//���x�˒���ݒ�
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
	//�Œ�˒��`�F�b�N
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


//�L���ȃ}�X��\��
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
	tmp << "�X�R�A�@";
	tmp << stage->mGameData->mFlagManager.getValue("score");
	mWindow.add(tmp.data());

	tmp = "";
	tmp << "�p���[�@";
	tmp << stage->mGameData->mFlagManager.getValue("power");
	mWindow.add(tmp.data());

	mWindow.add("�@���j�b�g",1);
	mWindow.add("�@�ݒ�",0);

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

	//�A�r���e�B�Q�b�g�ƁA�S�[�X�g���ǂ�
	for (int i=0; i<::Party::mGhostMax ; ++i) {
		if (gamedata->mParty.mGhost[i]) {
			if (gamedata->mParty.mGhost[i]->mState == 1) {
				//�A�r���e�B�o���l�Q�b�g
				gamedata->mParty.mGhost[i]->learnAbility(&mLearningWindow);

				//�o����Ԃ����ǂ�
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

//AP�`���[�W�t�F�[�Y
Stage::TurnReady::TurnReady(Stage* stage) {
	mSide = -1;			//0�v���C���[����@1�����@2�G

	//�Čv�Z
	stage->refresh();

}

int Stage::TurnReady::update(Stage* parent) {
	parent->drawFrontEnd();

	//�^�[�������߂�
	++parent->mCount;
	//���̓`���[�W
	parent->mParty[0].update();
	parent->mParty[1].update();
	//�V��J�E���g
	parent->mWeather.update();

	//AP�`���[�W
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (parent->mUnit[i]) {
			parent->mUnit[i]->mAP += parent->mUnit[i]->mAPCharge;
		}
	};
	//���܂�����A�s���񐔂ɒǉ�
	//�����ɂ��܂����ꍇ�̏������l���˂΂Ȃ�Ȃ�
	for (int i=0 ; i<Stage::mUnitMax ; ++i) {
		if (parent->mUnit[i]) {

			if (parent->mUnit[i]->mAP >=65536) {
				if (mSide < 0 ||
					mSide == parent->mUnit[i]->turn()) {

					parent->mUnit[i]->mTurn += 1;
					parent->mUnit[i]->mAP -= 65536;

					//���^�[���𔻒f
					if (mSide < 0) {
						mSide = parent->mUnit[i]->turn();
					}
				}
			};
		}
	}

	//�ʐw�c�ɃC���^���v�g���ꂽ��A�^�[���J�n
	if (mSide >=0) {
		if (mSide != parent->getQueue(0)->turn()) {
			if (mSide == 0) {
				//�����^�[��
				parent->mNextPhase = new Stage::Control(parent);
			}
			else {
				//NPC�^�[��
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
	mConfirm.set("�n���H");
	mConfirm.position(0,0);
};

int Stage::Deploy::update(Stage * stage) {
	if (mPhase == 0) {
		//���W�I��
		int r = mSelectSquare.update(stage);
		mSelectSquare.draw(stage);

		Frame::draw(-320,240,80,20);
		Draw::instance()->text(-320   ,240,"����",0xffffffff,1);
		Draw::instance()->text(-320+64,240,mNumber,0xffffffff,1);
		if (r == 1) {
			int x=mSelectSquare.mX;
			int y=mSelectSquare.mY;

			//���ɂ���Ȃ�A�O��
			if (stage->unit(x,y)) {
				stage->unit(x,y)->mGhost->mState = 0;//�o����Ԃ����ǂ�
				stage->removeUnit( stage->unit(x,y) );
				++mNumber;
			}
			else {
				//���Ȃ��Ȃ�A�S�[�X�g�I����
				//���ɏo�Ă�z�̓��X�g�ɓ���Ȃ��悤�ɂ��Ȃ��Ă͂Ȃ�Ȃ��B
				if (mNumber>0) {
					mGhostListWindow.set(stage->mGameData);
					mGhostListWindow.position(-320,240);
					mPhase = 1;
				}
			}
		}
		if (r==2) {
			//�L�����Z���Ŋm�F
			//��l���L�����z�u���ĂȂ��Ƃ��߂�
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
		//�p�[�e�B����I��
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
	//���j�b�g�I��
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

	//���j�b�g�I��
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
		//���j�b�g�R�}���h
		if (mCommand->update(parent)) {
			delete mCommand;
			mCommand = 0;
		}
	}

	//�S���̃^�[�����Ȃ��Ȃ�����A�t�F�[�Y�I��
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

//���j�b�g�R�}���h
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

	//�s���񐔃[���Ȃ甲����
	if (mUnit->mTurn == 0) {
		result = 2;
	}
	else {
		if (mSelectTarget == 0) {
			//�A�r���e�B�I��
			Draw::instance()->mCamera.update();

			mUnit->getFocus();

			int r=mUnitAbilityList.update(true);
			if (r ==1) {
				//�g�p�\�Ȃ�A���W�I����
				if (mUnitAbilityList.ability()->isAvailable() == 1) {
					mSelectTarget = new SelectTarget(stage,mUnitAbilityList.ability());
				}
			}
			if (r==2) {
				//�L�����Z��
				if (mUnit->mCast.cancel()) {
					//���j�b�g�I���ɖ߂�
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
			//���W�I��
			int r=mSelectTarget->update(stage);

			mSelectTarget->draw(stage);
			if (r) {
				//����B
				if (r == 1) {
					//�����t�F�[�Y����
					stage->mCastPhase = new Stage::CastAbility(stage);

					if (stage->mCastPhase->create(mUnit,
						mUnitAbilityList.ability(),
						mSelectTarget->mSelectSquare.mX,
						mSelectTarget->mSelectSquare.mY)) {

						//��������
						mUnit->mCast.add(mUnitAbilityList.ability());

						delete mSelectTarget;
						mSelectTarget = 0;
					}
					else {
						//�����s�Ȃ�ł�����
						delete stage->mCastPhase;
						stage->mCastPhase = 0;
					}
				}
				//�L�����Z��
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

//���W�I��
Stage::Control::Command::SelectTarget::SelectTarget(Stage* stage,Unit::Ability* ab){
	//�_���[�W��s�v�Z
	mCastResult.calc(stage,*ab);

	mArea.create(stage);
	mAbility = ab;

	//�˒��͈͂��o��
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

	//���ʔ͈͂��X�V
	mAbility->setArea(&mArea,mSelectSquare.mX,mSelectSquare.mY);
	return r;
};
void Stage::Control::Command::SelectTarget::draw(Stage* stage) {
	mSelectSquare.draw(stage);
	mArea.draw();

	//�s�����ʕ\��
	Unit* ut = stage->unit(mSelectSquare.mX,mSelectSquare.mY);
	if (ut) {
		mCastResult.draw(0,-100,ut);
		ut->drawInMap(false);
	};

};
///////////////////////////////////////////////////////
//�G�t�F�[�Y
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
			//�܂��N�𓮂��������߂�
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
		//�Y���҂Ȃ���������ǂ����悤�H
		if (mCurrentUnit) {
			//�v�Z
			int ab,tx,ty;
			calc(mCurrentUnit,&ab,&tx,&ty);

			//�Ώۃ}�X�\�����v��Ȃ�
			mCurrentUnit->mAbility[ab].setRange(&mRange);
			mCurrentUnit->mAbility[ab].setArea(&mArea,tx,ty);
			mCount = 0;

			//�����f�[�^����
			mCast = new Stage::CastAbility(stage);	//�����t�F�[�Y������
			mCast->create(
				mCurrentUnit,
				&mCurrentUnit->mAbility[ab],
				tx,ty);

			mCurrentUnit->mCast.add(&mCurrentUnit->mAbility[ab]);

			//�N���s�����邩�l������
			if (mCurrentUnit->mCast.turnEnd()) {
				mCurrentUnit = 0;
			}
		}
	}

	//��������
	if (mCast) {
		//�Ώۃ}�X�\��
		++mCount;
		mRange.draw();
		if (mCount > 30) {
			mArea.draw();
			stage->getFocus(mArea.mX,mArea.mY);
		}
		if (mCount >60) {
			//�����t�F�[�Y��n����
			stage->mCastPhase = mCast;
			mCast=0;
		}
	}
	else {
		//�����t�F�[�Y������Ȃ�������A�^�[���I��
		stage->mNextPhase = new Stage::TurnReady(stage);
	}

	return 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�D�揇�ʂ̍����s�����v�Z����

float Stage::Computer::calc(Unit* unit,int* ab,int* targetX,int* targetY){
	float result = 0.f;

	//�Ƃ肠������ԏ�̃A�r���e�B�ŏ��������Ƃ��B
	if (ab) {
		*ab=0;
	}
	if (targetX) {
		*targetX = 0;
	}
	if (targetY) {
		*targetY = 0;
	}

	//�������̂��߁A�͈͕ϐ��͒�`
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

							//�ړ��̏ꍇ�A���j�b�g�ʒu��ς���
							if (unit->mAbility[k].mAbility->mAbility->mType == ABILITY_MOVE) {
								unit->launch(x,y);
							}
							//�s���I���łȂ��Ȃ�A�ċA���邼
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

//���j�b�g����

Stage::KillUnit::KillUnit(Stage* stage) {
	mCount = 0;
}

int Stage::KillUnit::update(Stage* stage) {
	++mCount;

	//LP�_���[�W�t�F�[�Y
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
	//���j�b�g�����t�F�[�Y
	if (mCount>0){
		//�t�F�[�h�A�E�g
		float a = 1.f * (32-mCount) / 32.f;
		for (int i=0 ; i<Stage::mUnitMax; ++i) {
			if (stage->mUnit[i]) {
				if (stage->mUnit[i]->isKilled()) {
					stage->mUnit[i]->mObject->setColor(a,1.0,1.0,1.0);
					stage->mUnit[i]->mObject->mBlendInfo.setBlendMode(1);
				}
			}
		}

		//���j�b�g����
		if (mCount == 32) {
			for (int i=0 ; i<Stage::mUnitMax; ++i) {
				if (stage->mUnit[i]) {
					if (stage->mUnit[i]->isKilled()) {

						//�o���l�E�h���b�v�͎�ɂ͂��邩�H
						stage->mUnit[i]->getDrop(stage->mGameData);

						//�q���E��
						//�U���ғo�^�������i�e�A��Ԉُ�j
						//�Q�X�g�S�[�X�g�Ȃ�A����

						stage->removeUnit(i);
					}
				}
			}
		}
	}

	return mCount > 32;
};

//////////////////////////////////////////////////////////////////////////////////////////

//�A�j���t�F�[�Y
//�����̂ŕʃt�@�C���ɂ��邱�Ɛ���

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
	//�L���X�g���o�^
	//����ɘA�g�f�[�^�����邱��
	mUnit = ut;

	//�h���N���X�����炤�֐�
	mCast[0] = newCastClass( ab->mAbility->mAbility->mID );
	result = mCast[0]->create(ut,ab,i,j);

	return result;
};
int Stage::CastAbility::update(Stage* stage) {
	int result = 0;

	//�o�^����Ă�cast��Еt���Ă���
	bool b=true;						//����Ԃ̃G�t�F�N�g�I����Ă�H
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

	//���ʂ�K�p���Ď���
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
//�I������
void Stage::CastAbility::end(Stage* stage) {
	//�z�[���h�A�N�V�������ۂ�
	if (!mUnit->mCast.hold()) {

		//�s������
		if (mUnit->mCast.turnEnd()) {
			-- mUnit->mTurn;
		}

		//�|�C���g�E���̓R�X�g����
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

	//�͈͂����߂āA���ʂ��Čv�Z
	StageArea sa;
	sa.create(ut->mStage);
	ab->setArea(&sa,i,j);

	mCastResult.calc(ut->mStage,*ab,&sa);

	return mCastResult.mAvailable;

};
void Stage::CastAbility::Cast::apply(Stage* stage) {
	//�X�y�J�Ȃ�A�z�N�p�X�y�������ڂ��Ă���
	if (*mAbility->mAbility->mAbility->mElement.mSpell) {
		stage->mLastSpell = mAbility->mAbility;
	};

	//�^�C�v�ɂ���ďꍇ�킯
	//�ړ�
	if (mAbility->mAbility->mAbility->mType == ABILITY_MOVE) {
		mUnit->launch(mX,mY);
	}
	//����
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

		//ASSERT(0);	//�Ƃ߂Ƃ���
	}
	//�U���E�ҋ@�E�T�|�[�g
	if (mAbility->mAbility->mAbility->mType == ABILITY_WAIT ||
		mAbility->mAbility->mAbility->mType == ABILITY_ATTACK ||
		mAbility->mAbility->mAbility->mType == ABILITY_SUPPORT) {

		for (int i=0 ; i<Stage::mUnitMax ; ++i) {
			bool hit=false;

			if (mCastResult.mDamage[i].mResult) {
				//�A�N�V��������������A�ҋ@�J�E���^�[�[����
				if (stage->mUnit[i]->mWait) {
					stage->mUnit[i]->mWait = 0;
				}

				for (int j=0 ; j<mCastResult.mDamage[i].mCombo ; ++j) {
					if (mCastResult.mDamage[i].mHit[j]) {
						stage->mUnit[i]->mLastEnemy = mCastResult.mUnit;

						//�_���[�W
						if (mCastResult.mDamage[i].mResult == 1) {
							hit = true;
							stage->mUnit[i]->mHP -= mCastResult.mDamage[i].mDamage[j];

							//�c�@�_���[�W
							if (stage->mUnit[i]->mHP < 1) {
								++stage->mUnit[i]->mLPDamage;
								stage->mUnit[i]->mHP = stage->mUnit[i]->mRecoverHP;
							}
						}
						if (mCastResult.mDamage[i].mResult == 2) {
							//��
							hit = true;
							stage->mUnit[i]->mHP += mCastResult.mDamage[i].mDamage[j];
						}
					}
				}
				//�����ɒǉ�����
				//�����ςł��������Ă�ꍇ�̂�
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
	//�����ɔ�������
	mUnit->setState(
		mCastResult.mAbility->mAbility->mAbility->mRecoilState,
		mCastResult.mAbility->mAbility->mAbility->mRecoilStateLevel,
		false,0);
}




//////////////////////////////////////////////////////////////////////////////
//�_���[�W�v�Z
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
		//�Ȃɂ���̂����H	
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
		Draw::instance()->text(x    ,y+line*1,"�_���[�W",color,size);
		Draw::instance()->text(x+128,y+line*1,mDamage[ut->getIndex()].mTotalDamage,color,size);
		Draw::instance()->text(x    ,y+line*2,"����",color,size);
		Draw::instance()->text(x+128,y+line*2,mDamage[ut->getIndex()].mHitProbability,color,size);
		Draw::instance()->text(x    ,y+line*3,"Cri",color,size);
		Draw::instance()->text(x+128,y+line*3,mDamage[ut->getIndex()].mCriticalProbability,color,size);
		Draw::instance()->text(x    ,y+line*4,"�ǉ�����",color,size);
	}
}
bool CastResult::calc(Stage* stage,Unit::Ability& ab,const StageArea* sa) {

//mode 0
//0�_���[�W�v�Z���[�h
//1�D��x�v�Z���[�h
//2�\�z�_���[�W�v�Z���[�h


	//�_���[�W�v�Z�������낤
	//�p�����[�^
	//�̗�				50�`200		���@500-1000
	//�U����			50-150		���@150-300
	//�A�r���e�B�З�	100-200		���@200-500
	//����h��@�@�@�@�@0			��	0-250

	//��{���j
	//�_���[�W���U��*�З�-�h��
	//50*100 = 5000
	//150*200 = 30000

	//150*200 = 30000
	//300*500 = 150000

	// /150
	// /150

	//�����Ōv�Z
	mUnit = ab.mUnit;

	mPriority = 0.f;
	//1.f�ň�̓|����D��x.

	mAbility = &ab;
	Ability* abl = ab.mAbility;
	//�܂�������
	for (int i=0; i<Stage::mUnitMax ; ++i) {
		mDamage[i].mResult = 0;
	}

	//���ʕ���
	//���̓R�X�g�ɂ��}�C�i�X�␳�@�ő�50��

	mPriority -= 0.1f * mAbility->mAbility->mAbility->mCost;
	//�g�p�񐔂ɂ��}�C�i�X�␳�@�ő�50��
	if (mAbility->mAbility->point()) {
		mPriority -= 0.5f / mAbility->mAbility->point();
	};
	//�������ʂɂ��␳
	mPriority -= mUnit->checkState(
		mAbility->mAbility->mAbility->mRecoilState,
		mAbility->mAbility->mAbility->mRecoilStateLevel,false);

	//�͈͓��Ƀ��j�b�g������Ǝ��s����^�C�v
	if (abl->mAbility->mType == ABILITY_MOVE ||
		abl->mAbility->mType == ABILITY_CAST) {
		mAvailable = true;

		//�_���[�W�\�����[�h�Ȃ�A�v�Z����Ȃ�
		if (sa) {
			for (int i=0; i<Stage::mUnitMax ; ++i) {
				if (stage->mUnit[i]) {
					//�͈͓��ɂ��邩�H
					if (sa->get(
						stage->mUnit[i]->mX,
						stage->mUnit[i]->mY)) {

						mAvailable = false;
						break;
					};
				}
			}
			if (abl->mAbility->mType == ABILITY_MOVE) {
				//�ړ����낤���������낤���A�G���j�b�g�ɋ߂������D�悷��
				//�⏕�n���j�b�g�͖����Ɍ�����ׂ������A����ᖳ����
				//�e���j�b�g������Ȃ�A�������Ɍ������B

				StageArea tmp;
				tmp.create(stage);
				tmp.set(0);
				tmp.walk(
					sa->mX,sa->mY,
					64,
					mUnit->interrupt(),	
					*mUnit->mStatus.mUp,
					*mUnit->mStatus.mDown);
				//�G���j�b�g����̋����𓾂�
				int t=0;	//�|�C���g
				int n=0;	//�G���j�b�g��
				if (mUnit->mParent) {
					//�e������Ȃ�A�e���S�Ă�
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

				//�ړ��̗D��x���v�Z����B�v����ɓG�ɋ߂Â��΂����킯��B
				//�ő�0.1
				if (abl->mAbility->mType == ABILITY_MOVE) {
					if (n) {
						mPriority += 0.1f*t/n/64.f;
					}
					if (mUnit->mWait) {
						mPriority -= 0.1f;
					}
				}
				//�����n�̗L���x
				//�ő�0.8�@���Ȃ荂���B�@�����o�Ă��邩�͖������B
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

	//�v�Z���悤
	mAvailable=false;
	for (int i=0; i<Stage::mUnitMax ; ++i) {
		if (stage->mUnit[i]) {
			bool b=true;
			if (sa) {
				//�͈͓��ɂ��邩�H
				if (!sa->get(
					stage->mUnit[i]->mX,
					stage->mUnit[i]->mY)) {
					b=false;
				}
			}
			if (b) {
				//�Ώێ����ōU�����Ɗ֌W�Ȃ��ɂ��Ă���
				if (mUnit == stage->mUnit[i]) {
					if (abl->mAbility->mType == ABILITY_ATTACK) {
						continue;
					}
				}
				//���ʑ����@�����ɍU���E�G�ɃT�|�[�g�𖳌�
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

				//����
				//0�֌W�Ȃ��@1�_���[�W�@2�񕜁@3������
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
				//�_���[�W���񕜂̂Ƃ��̂݌v�Z
				//��{���ʗ�
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

				//�����␳
				int ele=0;
				for (int j=0 ; j<Element::mMax ; ++j) {
					if (abl->mAbility->mElement.mElement[j]) {
						ele += stage->mUnit[i]->mResistElement.mElement[j];
					}
				}
				//�ϐ��l���v�@-���Q�@0���ʁ@+�ϐ��@100������
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

				//������
				//������K�p�����̂̓_���[�W���E��K������
				int ht = *mUnit->mStatus.mHit+100;
				if (damage == 1 && 	*abl->mAbility->mElement.mUnavoidable == 0) {
					ht -= *stage->mUnit[i]->mStatus.mAvoid;
				}
				ht = ht * (100+abl->mAbility->mHit) / 100;

				//�Ɍ�0�ɂȂ�Q�ߐ������낤
				if (ht<100) {
					int av = 100-ht;
					ht = 100 - asymptote(av,100,2500);	//���50�̂Ƃ��A�m��50%
				}
				else {
					ht = 100;
				}
				mDamage[i].mHitProbability = ht;
	
				//�N���e�B�J����
				mDamage[i].mCriticalProbability = abl->mAbility->mCritical + *mUnit->mStatus.mCritical;

				//�q�b�g��
				mDamage[i].mCombo = abl->mAbility->mCombo;

				//�\�z�_���[�W�ʂ����߂�
				int hp2 = stage->mUnit[i]->mHP;
				int lp2 = stage->mUnit[i]->mLP-1;
				int hp3 = stage->mUnit[i]->mHP;
				int lp3 = stage->mUnit[i]->mLP-1;

				//�e�q�b�g���Ƃ̏���
				for (int j=0 ; j < mDamage[i].mCombo ; ++j) {
					//�����ςӂ�̌��ʗ�
					int v = value / mDamage[i].mCombo;

					//�\�z���ʗ�
					int v2 = v;
					//�D��x�v�Z�p
					int v3 = v;

					//�����������H
					if (mDamage[i].mHitProbability > cRandom::instance()->getInt(100)) {
						mDamage[i].mHit[j] = true;
					}
					else {
						mDamage[i].mHit[j] = false;
					}

					//�D��x
					v3 = v3 * mDamage[i].mHitProbability / 100;

					//�N���e�B�J�����H
					if (damage == 1) {
						mDamage[i].mCritical[j] = mDamage[i].mCriticalProbability > cRandom::instance()->getInt(100);

						int critical = stage->mUnit[i]->critical();
						//�N���e�B�J������
						if (mDamage[i].mCritical[j]) {
							v = v * (100+critical) /100;
						}

						//�D��x
						v3 = v3*(100+ (critical*mDamage[i].mCriticalProbability/100))/100;
					}
					else {
						mDamage[i].mCritical[j] = false;
					}

					//�����K�p
					//�v�Z���ʏ�̂݁@�Œ�E�������ƌŒ�
					//������+-10���ł������낤
					//�_���[�W�l���ł�������ƃI�[�o�[�t���[����̂Œ��ӁB
					if (abl->mAbility->mPowerType == POWER_NORMAL) {
						int r=cRandom::instance()->getInt(201) -100;
						v = v * (1000 + r) / 1000;
					}

					//�h��l��K�p
					if (*abl->mAbility->mElement.mPierce == 0) {
						//�_���[�W�v�Z
						v2 -= *stage->mUnit[i]->mStatus.mDefense;
						if (v2<1) {
							if (mAbility->mPower) {
								v2=1;
							}
							else {
								v2=0;
							}
						}

						//�D��x�v�Z
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
							//�~�j�}���_���[�W�`�F�b�N
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

					//�\�z�_���[�W
					if (damage == 1) {
						hp2 -= v2;
						if (hp2 <= 0) {
							hp2 = stage->mUnit[i]->mRecoverHP;
							--lp2;
						}
						//�D��x�v�Z
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
				//�ǉ�����
				mDamage[i].mState= mAbility->mAbility->mAbility->mExtraState;
				mDamage[i].mStateLevel= mAbility->mAbility->mAbility->mExtraStateLevel;

				//���_���[�W�\�z
				int d = lp2 * *stage->mUnit[i]->mStatus.mHP + hp2;
				if (d < 0) {
					d = 0;
				}
				mDamage[i].mTotalDamage = stage->mUnit[i]->getTotalHP() - d;
				if (mDamage[i].mTotalDamage < 0) {
					mDamage[i].mTotalDamage = -mDamage[i].mTotalDamage;
				}

				//�D��x�v�Z
				int e = lp3 * *stage->mUnit[i]->mStatus.mHP + hp3;
				if (e < 0) {
					e = 0;
				}
				int f = stage->mUnit[i]->getTotalHP() - e;		//���҃_���[�W					
				float p = 1.f * f / stage->mUnit[i]->getTotalHP();

				//�ǉ����ʂɂ��D��x
				p += stage->mUnit[i]->checkState(
					mAbility->mAbility->mAbility->mExtraState,
					mAbility->mAbility->mAbility->mExtraStateLevel,
					true);

				//�Ō�ɐ��������߂�
				if (mUnit->AI() == stage->mUnit[i]->mParty){
					p=-p;
				}
				//�Ō�Ƀf�R�C�␳
				if (p>0.f) {
					p *= (10.f + stage->mUnit[i]->mDecoy) / 10.f;
				}

				mPriority += p;
			}
		}
	}
	//�U���Ȃ炻�̂܂�
	//�ҋ@�Ȃ�0.1f�Œ�
	//����ȊO�̓}�C�i�X
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


