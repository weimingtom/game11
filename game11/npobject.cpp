#include "GameLib/Framework.h"

#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"

#include "npmath.h"
#include "npdraw.h"
#include "npsysc.h"
#include "npobject.h"


//�I�u�W�F�N�g�N���X++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int Object::mNumber=0;

Object::Object() {
	mAvailable=false;
	mNumber=0;
	mSprite = 0;
}
double Object::getX() const{
	return mPosition.mX.value();
}
double Object::getY() const{
	return mPosition.mY.value();
}
double Object::getZ() const{
	return mPosition.mZ.value();
}
void Object::getVelocity(double *vx,double *vy,double *vz) const{
	if (vx) {
		*vx=mPosition.mX.mVelocity;
	}
	if (vy) {
		*vy=mPosition.mY.mVelocity;
	}
	if (vz) {
		*vz=mPosition.mZ.mVelocity;
	}
}


void Object::setTexture(MyTexture* tex) {
	mTexture=tex;
}
void Object::setModel(Model* m,bool checkroot,bool checklock) {
	//ASSERT(m);
	if (m) {
		if (mOriginalModel!=m || checkroot==false) {			//�������f���Ȃ�A�ς��Ȃ�
			//if (checklock==false || mModel->mLock==false) {		//���b�N���������Ă�Ȃ�A�ς��Ȃ��i�A�j�����Ō�܂ōĐ��������Ƃ��Ɂj
				mModel=m;
				mOriginalModel=m;
				mAnimeCount=0;
			//}
		}
	}
}
void Object::setModel(const char *name,bool checkroot,bool checklock) {
	Model* m = mModel->mParent->model(name,false);
	if (m) {	//������Ȃ������疳��
		setModel(m,checkroot,checklock);
	}
}
////////////////////////////////////////////////////////////////////////

void Object::draw(){
	if (isEnable()) {

		bool v = true;
		if (mParent) {
			if (!mParent->isVisible()) {
				v = false;
			}
		}
		if (isVisible() && v) {
			if (mModel) {
				mModel->draw(mTexture,mWorld,mBlendElement,mAnimeCount);
			}

			//�f�o�b�O�p�����蔻��\��
			if(Main::mDebugPrint) {
				if (mCollision.mAvailable) {
					for (int i=0 ; i<mCollision.mPolygon.mNumber ; ++i){
						Math::Segment seg;
						mCollision.mPolygon.segment(&seg,i);
						Draw::instance()->drawLine(seg.mStart,seg.mEnd);
						//Draw::instance()->drawLine(
						//	seg.mStart.x,
						//	seg.mStart.y,
						//	seg.mEnd.x,
						//	seg.mEnd.y,
						//	1.0f,0xffffffff,0xff000000);
					}
				}
			}
		}
	}
}
BlendInfo Object::blendInfo() const{
	::BlendInfo result;

	mBlendInfo.write(&result);

	bool flash=false;
	if (mFlash) {
		int t=Main::instance()->mCount-mFlashFrame + mFlash;
		//flash= ((t / (mFlash+1) % 2) == 0);
		flash = (t % mFlash) < mFlashTime;
	}
	if (flash) {
		//�t���b�V�����̏���
		//�F��������΂���
		result.mBlendElement.mEmission = mFlashEmission;
	}

	result.mBlendElement.mTransparency = mAlpha.value();
	result.mBlendElement.mAmbient.set(
		mRed.value(),
		mGreen.value(),
		mBlue.value());

	result.mReverseX=mReverseX;
	result.mReverseY=mReverseY;
	result.mReverseZ=mReverseZ;

	result.mScale.set(mZoom.mX.value(),mZoom.mY.value(),mZoom.mZ.value());;
	result.mRotate.set(mRotate.mX.value(),mRotate.mY.value(),mRotate.mZ.value());

	return result;
};


void Object::BlendInfo::reset() {
	mBlendMode = -1;
	mEmission = 0.f;
	for (int i=0 ; i<4 ; ++i) {
		mLighting[i] = true;
	}
}
void Object::BlendInfo::write(::BlendInfo *result) const {
	if (mBlendMode < 0) {
		result->mBlendElement.mBlendMode = GameLib::Graphics::BLEND_MAX;
	}
	else {
		result->setBlendMode(mBlendMode);
	}

	result->mBlendElement.mEmission = mEmission;
	for (int i=0 ; i<4 ; ++i) {
		result->mBlendElement.mLighting[i] = mLighting[i];
	}
}

void Object::fadeAway(int t,float zoom) {
	mLimit = t;
	mAlpha.mVelocity = -mAlpha.value() / t;
	mZoom.mX.mVelocity = mZoom.mX.value() * zoom / t;
	mZoom.mY.mVelocity = mZoom.mY.value() * zoom / t;
	mZoom.mZ.mVelocity = mZoom.mZ.value() * zoom / t;
};
void Object::randomRotate(bool x,bool y,bool z) {
	if (x) {
		mRotate.mX.set(Random::instance()->getFloat(360.f));
	}
	if (y) {
		mRotate.mY.set(Random::instance()->getFloat(360.f));
	}
	if (z) {
		mRotate.mZ.set(Random::instance()->getFloat(360.f));
	}
}

////////////////////////////////////////////////////////////////////////
void Object::setPosition(double a,double b,double c) {
	mPosition.mX.set(a);
	mPosition.mY.set(b);
	mPosition.mZ.set(c);

	launch();
}

void Object::setPosition (const GameLib::Math::Vector3& vct) {
	setPosition(vct.x,vct.y,vct.z);
}
void Object::addPosition(const GameLib::Math::Vector3& vct) {
	setPosition(
		mPosition.mX.value() + vct.x,
		mPosition.mY.value() + vct.y,
		mPosition.mZ.value() + vct.z
	);
}

void Object::setVelocity(double a,double b,double c) {
	mPosition.mX.mVelocity = a;
	mPosition.mY.mVelocity = b;
	mPosition.mZ.mVelocity = c;
}
void Object::setVelocity(const GameLib::Math::Vector3& v) {
	setVelocity(v.x,v.y,v.z);
}
void Object::setVelocity2(double v,float hv,float vv) {
	float s = GameLib::Math::sin(hv);
	float c = GameLib::Math::cos(hv);

	setVelocity(
		v*GameLib::Math::cos(vv) * c,
		v*GameLib::Math::cos(vv) * s,
		v*GameLib::Math::sin(vv));
}
void Object::setAccel(double x,double y,double z) {
	mPosition.mX.mAccel = x;
	mPosition.mY.mAccel = y;
	mPosition.mZ.mAccel = z;
}
void Object::setAccel2(double v,double a) {
	mPosition.mX.mAccel=v*GameLib::Math::cos(a);
	mPosition.mX.mAccel=v*GameLib::Math::sin(a);
}

void Object::setResistance(double a,double b,double c) {
	mPosition.mX.mResistance = a;
	mPosition.mY.mResistance = b;
	mPosition.mZ.mResistance = c;
}
void Object::setResistance(double a) {
	setResistance(a,a,a);
}
///////////////////////////////////////////////////////////////////

void Object::setRotate (double a,double b,double c) {
	mRotate.mX.set(a);
	mRotate.mY.set(b);
	mRotate.mZ.set(c);
	launch();
}
void Object::setRotateVelocity (float a,float b,float c) {
	mRotate.mX.mVelocity = a;
	mRotate.mY.mVelocity = b;
	mRotate.mZ.mVelocity = c;
}
void Object::setRotateVelocity (const GameLib::Math::Vector3& v) {
	setRotateVelocity(v.x,v.y,v.z);
}
void Object::setRotateAccel (double a,double b,double c) {
	mRotate.mX.mAccel = a;
	mRotate.mY.mAccel = b;
	mRotate.mZ.mAccel = c;
}
void Object::setRotateResistance (double a,double b,double c) {
	mRotate.mX.mResistance = a;
	mRotate.mY.mResistance = b;
	mRotate.mZ.mResistance = c;
}
void Object::setRotateResistance (double k) {
	setRotateResistance(k,k,k);
}
void Object::setRotateX (double a) {
	mRotate.mX.set(a);
	launch();
}
void Object::setRotateY (double a) {
	mRotate.mY.set(a);
	launch();
}
void Object::setRotateZ (double a) {
	mRotate.mZ.set(a);
	launch();
}

void Object::setAngle(const GameLib::Math::Vector3& dir) {
	float x;
	float y;
	Math::getAngle(&x,&y,dir);
	setRotateX(x);
	setRotateY(y);
}
void Object::aim(const Object* target) {
	aim(target->mWorldPosition);
}
void Object::aim(const GameLib::Math::Vector3& target) {
	GameLib::Math::Vector3 dir;
	getMyMatrix(&dir,target);
	setAngle(dir);
}

void Object::aimX(const GameLib::Math::Vector3& target) {
	GameLib::Math::Vector3 dir;
	getMyMatrix(&dir,target);

	//�p�x���Ƃ߂�
	float r = GameLib::Math::atan2(dir.y,dir.x);

	setRotate(0,0,r);	//�y���̂݁B
};


void Object::go(float v,float a,const GameLib::Math::Vector3& pos) {
	GameLib::Math::Vector3 target;
	getMyMatrix(&target,pos);
	target.normalize();
	setVelocity(
		v*target.x,
		v*target.y,
		v*target.z);
	setAccel(
		a*target.x,
		a*target.y,
		a*target.z);
};
void Object::getMyMatrix(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& pos) const {
	//�e�s��̋t�s��������Ă�邱�ƂŐ��m�ɂł��邪�A�d����
	GameLib::Math::Vector3 vct = pos;
	vct -= *worldPosition();

	GameLib::Math::Matrix34 tmp;
	tmp.setInverse(mParentWorld);	//�t�s�񂪍��Ȃ��΂���������

	tmp.mul(out,vct);
};
void Object::getWorldPositionFinal(GameLib::Math::Vector3* out) const {
	GameLib::Math::Vector3 pos(0.f);
	getWorldPositionFinal(out,pos);
}
void Object::getWorldPositionFinal(GameLib::Math::Vector3* out,GameLib::Math::Vector3 pos) const {
	pos *= 2.f / Draw::mWidth;	//window�n�ɒ���

	GameLib::Math::Matrix34 mtx;
	getMatrixFinal(&mtx);
	mtx.mul(out,pos);

	*out *= Draw::mWidth/2;		//pixel�ɂȂ���
}
void Object::getMatrixFinal(GameLib::Math::Matrix34* out) const{
	if (mModel) {
		mModel->getWorldMatrix(out,mWorld,mBlendElement,mAnimeCount);
	}
	else {
		*out = mWorld;
	}

}

///////////////////////////////////////////////////////////////////
void Object::setSize (double x,double y,double z) {
	mZoom.mX.set(x);
	mZoom.mY.set(y);
	mZoom.mZ.set(z);

	launch();
}
void Object::setSize (double s) {
	setSize(s,s,s);
}

void Object::setSizeX(double x) {
	mZoom.mX.set(x);
	launch();
}
void Object::setSizeY(double x) {
	mZoom.mY.set(x);
	launch();
}
void Object::setSizeZ(double x) {
	mZoom.mZ.set(x);
	launch();
}

void Object::setSizeVelocity (double x,double y,double z) {
	mZoom.mX.mVelocity = x;
	mZoom.mY.mVelocity = y;
	mZoom.mZ.mVelocity = z;
}
void Object::setSizeVelocity (double s) {
	setSizeVelocity(s,s,s);

}
void Object::setSizeAccel(double x,double y,double z) {
	mZoom.mX.mAccel = x;
	mZoom.mY.mAccel = y;
	mZoom.mZ.mAccel = z;
}

void Object::setSizeResistance(double x,double y,double z) {
	mZoom.mX.mResistance = x;
	mZoom.mY.mResistance = y;
	mZoom.mZ.mResistance = z;
}
void Object::setSizeResistance(double a) {
	setSizeResistance(a,a,a);
}
///////////////////////////////////////////////////////////////////

void Object::setColor (double alpha,double red,double green,double blue) {
	mAlpha.set(alpha);
	mRed.set(red);
	mGreen.set(green);
	mBlue.set(blue);
	launch();
}

void Object::setColor(unsigned int color) {
	float a,r,g,b;
	Math::unsignedToColor(&a,&r,&g,&b,color);
	setColor(a,r,g,b);
};
void Object::setColor(double a,const GameLib::Math::Vector3& color) {
	setColor(a,color.x,color.y,color.z);
}

void Object::setColorVelocity (double alpha,double red,double green,double blue) {
	mAlpha.mVelocity = alpha;

	mRed.mVelocity = red;
	mGreen.mVelocity = green;
	mBlue.mVelocity = blue;
}
void Object::setColorAccel (double alpha,double red,double green,double blue) {
	mAlpha.mAccel = alpha;
	mRed.mAccel = red;
	mGreen.mAccel = green;
	mBlue.mAccel = blue;
}
void Object::setColorResistance (double alpha,double red,double green,double blue) {
	mAlpha.mResistance = alpha;
	mRed.mResistance =red;
	mGreen.mResistance =green;
	mBlue.mResistance = blue;
}
void Object::setReverse(bool x,bool y,bool z) {
	mReverseX = x;
	mReverseY = y;
	mReverseZ = z;
	launch();
}
///////////////////////////////////////////////////////////////////
void Object::setCollision(unsigned int self,unsigned int target,unsigned int react,unsigned int priority) {
	mCollision.mFlag=self;				//�����蔻��t���O�B
	mCollision.mFlagTarget=target;		//�����蔻����s���Ώۂ̃t���O�B
	mCollision.mReactFlag=react;			//�Փˉ�������t���O
	mCollision.mPriority=priority;		//�D��x0-2�@0������

	ASSERT(target & react || react==0);	//�����Ώۂ�����ΏۂɂȂ��Ă��Ȃ�
	ASSERT(priority<3);
};

void Object::setParent (Object* o,bool stable,bool followonlyposition) {
	mParent=o;
	if (o==0) {
		mRoot=this;
	}
	else {
		mRoot=o;
	}
	mStable=stable;
	mFollowOnlyPosition = followonlyposition;
}
void Object::leaveParent() {
	mRoot=this;

	//�q���ɁA���[�g�������ɂȂ������Ƃ�`����
	for (int i=0 ; i <= mSprite->mMax ; ++i ) {
		if (mSprite->mObject[i].mParent== this ) {
			mSprite->mObject[i].mRoot=this;
		}
	}
	mParent=0;

}

void Object::setFlash(int f,int t) {
	if (mFlash!=f) {
		mFlash=f;
		mFlashFrame=Main::instance()->mCount;
	}
	if (t==0) {
		mFlashTime = mFlash / 2;
	}
	else {
		mFlashTime = t;
	}
};
void Object::setLimit(int i) {
	mLimit = i;
};
void Object::setAnimeSpeed(float a) {
	mAnimeSpeed = a;
}
void Object::setAnimeCount(float a) {
	mAnimeCount = a;
}
void Object::setLight(float intensity,const GameLib::Math::Vector3& color) {
	mLight=true;
	mLightIntensity = intensity;
	mLightColor = color;
}
void Object::setLight(float k) {
	if (k>0.f) {
		mLight = true;
		mLightIntensity = k;
		mLightColor = 1.f;
	}
	else {
		mLight =false;
	}
}
void Object::setFocus(bool b) {
	mFocus = b;
}

void Object::savePrevPosition() {
	mWorldPositionprev=mWorldPosition;
}

void Object::stop() {
	mZoom.stop();
	mPosition.stop();
	mRotate.stop();
	mAlpha.stop();
	mRed.stop();
	mGreen.stop();
	mBlue.stop();

	mAnimeSpeed=0.0f;
	mLimit=0;

	mGravity=false;
	mResistance=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Object::id() const {
	return mID;
}
const GameLib::Math::Vector3* Object::worldPosition() const {
	return &mWorldPosition;
}
const GameLib::Math::Vector3* Object::prevWorldPosition() const {
	return &mWorldPositionprev;
}
void Object::getRotate(GameLib::Math::Vector3 *out) const{
	mRotate.value(out);
}
void Object::getVelocity(GameLib::Math::Vector3 *out) const{
	out->x = mRotate.mX.mVelocity;
	out->y = mRotate.mY.mVelocity;
	out->z = mRotate.mZ.mVelocity;
}

const Model* Object::model() const {
	return mModel;
}
int Object::animeCount() const {
	return mAnimeCount;
}
bool Object::isEnable() const {
	return mAvailable;
}
Object* Object::parent() const {
	return mParent;
}
float Object::collisionX(int i) const {
	return mCollision.mPolygon.mVertex[i].x;
}
float Object::collisionY(int i) const {
	return mCollision.mPolygon.mVertex[i].y;
}
bool Object::isVisible() const {
	return mVisible&&(mAnimeCount>=0.f);
};

void Object::Collision::addObject(Object* obj,bool react,const GameLib::Math::Vector3& vct) {
//�d�����Ȃ��悤�`�F�b�N
	for(int w=0 ; w<mMax ; ++w) {
		if (mObject[w] == obj) {
			break;
		}
		if (mObject[w] == 0) {
			mObject[w] = obj;

			if (react) {
				ASSERT(0);//�ۗ�
			}

			//�I�[�Ƀ`�F�b�N�k�������
			if (w<mMax-1) {
				mObject[w+1] = 0;
			}

			break;
		}
	}
}
void Object::Collision::addArea(const Area::Vector* area,bool react,const GameLib::Math::Vector3& vct){
	for(int w=0 ; w<mMax ; ++w){

		//�d�����ĂȂ����`�F�b�N
		if (mArea[w] == area) {
			break;
		}
		if (mArea[w] == 0) {
			mArea[w] = area;

			if ( mReactFlag & 1) {
				ASSERT(0);	//�Փˉ���
			}
	
			if (w<mMax-1) {
				mArea[w+1] = 0;
			}

			break;
		}
	}
}
int Object::Collision::objectNumber() const {
	int result = 0;
	for (int i=0 ; i<mMax; ++i) {
		if (mObject[i] == 0) {
			break;
		}
		++result;
	}
	return result;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Area::Vector::Vector() {
	mParent=0;
}

Area::Block::Block() {
	reset();
}
void Area::Block::reset() {
	mSide[0]=false;
	mSide[1]=false;
	mSide[2]=false;
	mSide[3]=false;
	mX=-1;
	mY=-1;

	mType=255;
	mPriority=0;
	mEnvironment.reset();
	mSeal = false;

	mParam=0;
	mLimit=0;
	mVanish=false;
}


void Area::Block::setLimit(int f) {
	if (mLimit==0) {
		mLimit=f;
	}
};
//�ӂ����݂��邩�m�F����
bool Area::Block::side(int s) {
	if (mSide[s]) {
		return mSide[s];
	}
	if (mType>255) {	//����u���b�N�́A�l�ӂ����݂���
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
//���N���X
Area::Block::Environment::Environment() {
	reset();
}
void Area::Block::Environment::reset() {
	mGravity=1.0f;
	mResistance=0.9f;
	mAccelX=0.f;
	mAccelY=0.f;
	mWater=false;
};
void Area::Block::Environment::water() {
	mGravity=0.7f;
	mResistance=0.7f;
	mWater=true;
};

///////////////////////////////////////////////////////////////////////////////////////////

//�n�`�N���X
Area::Area() {

	mContainer = 0;
	mBlock=0;

	mContainerX = 0;
	mContainerY = 0;
	mContainerWidth=0;
	mContainerHeight=0;

	mBlockWidth=0;
	mBlockHeight=0;
	mBlockX=0;
	mBlockY=0;
}

Area::~Area() {
	destroy();
};
void Area::create(int offsetX,int offsetY,unsigned int chipwidth,unsigned int chipheight,unsigned int width,unsigned int height) {

	mBlockWidth=chipwidth;
	mBlockHeight=chipheight;
	mBlockX=width;
	mBlockY=height;

	//������
	mContainerX = offsetX;
	mContainerY = offsetY;
	mContainerWidth = mBlockWidth*mBlockX / Container::mWidth +1;
	mContainerHeight = mBlockHeight*mBlockY / Container::mHeight +1;
	if (mContainer) {
		delete []mContainer;
		mContainer = 0;
	}
	mContainer = NEW Container[ mContainerWidth * mContainerHeight ];

	//�u���b�N�쐬
	if (mBlock) {
		delete []mBlock;
		mBlock = 0;
	}
	if (mBlockX * mBlockY) {
		mBlock=new Block[mBlockX * mBlockY];
	}

};


void Area::destroy() {
	removeVector();

	if (mContainer) {
		delete []mContainer;
		mContainer=0;
	}

	if (mBlock) {
		delete []mBlock;
		mBlock=0;
	}

	mContainerWidth=0;
	mContainerHeight=0;

	mBlockWidth=0;
	mBlockHeight=0;
	mBlockX=0;
	mBlockY=0;
};
void Area::reset() {
	for (int i=0 ; i<mBlockX ; ++i) {
		for (int j=0 ; j<mBlockY ; ++j) {
			if (block(i,j)) {
				block(i,j)->reset();
				block(i,j)->mX=i;
				block(i,j)->mY=j;
			}
		}
	}

	removeVector();
}
void Area::removeVector() {
	mList.release();

	if (mContainer) {
		for (int i=0 ; i<mContainerWidth ; ++i) {
			for (int j=0 ; j<mContainerHeight ; ++j) {
				container(i,j)->mVector.release();
			}
		}
	}
};


void Area::addCollision(double x1,double y1,double x2,double y2,unsigned int i,unsigned int j) {
	Vector* neo = mList.add();
	neo->mSegment.mStart.x=x1;
	neo->mSegment.mStart.y=y1;
	neo->mSegment.mStart.z=0.f;
	neo->mSegment.mEnd.x=x2;
	neo->mSegment.mEnd.y=y2;
	neo->mSegment.mEnd.z=0.f;

	neo->mParent=block(i,j);
}

//�n�`�̋ϓ�����
void Area::split() {
	for (int i=0; i<mContainerWidth ; ++i) {
		for (int j=0 ; j<mContainerHeight ; ++j) {
			container(i,j)->mVector.release();
		}
	}

	for (List<Vector>::Iterator i(mList) ; !i.end() ; i++) {
		//�܂�X��
		double m;
		double M;
		if (i()->mSegment.mStart.x < i()->mSegment.mEnd.x) {
			m = i()->mSegment.mStart.x;
			M = i()->mSegment.mEnd.x;
		}
		else {
			m = i()->mSegment.mEnd.x;
			M = i()->mSegment.mStart.x;
		}
		int a=getSplitAreaX(m);
		int b=getSplitAreaX(M);

		//����Y��
		if (i()->mSegment.mStart.y < i()->mSegment.mEnd.y) {
			m = i()->mSegment.mStart.y;
			M = i()->mSegment.mEnd.y;
		}
		else {
			m = i()->mSegment.mEnd.y;
			M = i()->mSegment.mStart.y;
		}
		int c=getSplitAreaY(m);
		int d=getSplitAreaY(M);

		//���ɓo�^
		for (int p=a ; p<(b+1) ; ++p){
			for (int q=c ; q<(d+1) ; ++q){
				container(p,q)->mVector.addInitializing(i());
			}
		}
	}
}
Area::Block* Area::block(unsigned int x,unsigned  int y) const{
	//ASSERT(x<mWidth);
	//ASSERT(y<mHeight);
	if ((x<mBlockX) && (y<mBlockY)) {
		return mBlock+ x*mBlockY + y;
	}
	else {
		return 0;
	}
}
Area::Container* Area::container(unsigned int x,unsigned  int y) const{
	//ASSERT(x<mWidth);
	//ASSERT(y<mHeight);
	if ((x<mContainerWidth) && (y<mContainerHeight)) {
		return mContainer+ x*mContainerHeight + y;
	}
	else {
		return 0;
	}
}

//����̃u���b�N������
void Area::removeBlock(unsigned int x,unsigned int y) {
	removeBlock(block(x,y));
}
void Area::removeBlock(Block* b) {

	//b->mSide[0]=false;
	//b->mSide[1]=false;
	//b->mSide[2]=false;
	//b->mSide[3]=false;

	//���̃u���b�N�ɏ�������x�N�^������
	for (int i=0 ; i<mList.number() ;) {
		if (mList(i)->mParent==b) {
			mList.remove(i);
		}
		else {
			++i;
		}
	}

	b->mType=255;

	//�܂��̕ӂ��C������
	createSide(b->mX  ,b->mY+1,0);
	createSide(b->mX-1,b->mY  ,1);
	createSide(b->mX  ,b->mY-1,2);
	createSide(b->mX+1,b->mY  ,3);

	//�\���������Ă���
	b->mVanish=false;
	b->mLimit=0;

	//�S�[�X�g�������
	b->mSeal=false;
	split();

};
//�u���b�N�̒��_����Ӄx�N�^������B
//�΂ߕ����̕ӂ͎蓮�ł��邱�Ɓ@������Ăяo���Ȃ����ƁB
void Area::createBlock() {
	for (int i=0; i<mBlockX; ++i) {
		for (int j=0 ; j<mBlockY ; ++j) {
			createSide(i,j,0);
			createSide(i,j,1);
			createSide(i,j,2);
			createSide(i,j,3);
		}
	}
};
//�u���b�N�̕ӂ�����B�͈͂����������Ƃ��͖������Ă����B
//�ӂ����݂���ꍇ��block�ɃS�[�X�g��������Ă�����
void Area::createSide(unsigned int i,unsigned int j,int side) {
	if (i>=mBlockX) {
		return;
	}
	if (j>=mBlockY) {
		return;
	}
	if (block(i,j)->side(side)) {
		block(i,j)->mSeal = true;			//�S�[�X�g����
		if (side==0) {
			//��
			if (j>0) {
				if (block(i,j-1)->side(2)) {
					return;
				}

				addCollision(
					i*mBlockWidth,    j*mBlockHeight,
					(i+1)*mBlockWidth,j*mBlockHeight,
					i,j);
			}
		}
		if (side==1) {
			//�E
			if (i<mBlockX-1) {
				if (block(i+1,j)->side(3)) {
					return;
				}

				addCollision(
					(i+1)*mBlockWidth,j*mBlockHeight,
					(i+1)*mBlockWidth,(j+1)*mBlockHeight,
					i,j);
			}
		}

		if (side==2) {
			//��
			if (j<mBlockY-1) {
				if (block(i,j+1)->side(0)) {
					return;
				}
	
				addCollision(
					(i+1)*mBlockWidth,(j+1)*mBlockHeight,
					i*mBlockWidth,(j+1)*mBlockHeight,
					i,j);
			}

		}

		if (side==3) {
		//��
			if (i>0) {
				if (block(i-1,j)->side(1)) {
					return;
				}
				addCollision(
					i*mBlockWidth,(j+1)*mBlockHeight,
					i*mBlockWidth,j*mBlockHeight,
					i,j);
			}
		}
	}
};

void Area::flow(int x1,int y1,int x2,int y2,float ax,float ay) {
	ASSERT(x1<=x2);
	ASSERT(y1<=y2);
	for (int i=x1 ; i<=x2 ; ++i) {
		for (int j=y1 ; j<=y2 ; ++j) {
			Block* b=block(i,j);
			if (b) {
				b->mEnvironment.mAccelX+=ax;
				b->mEnvironment.mAccelY+=ay;
			}
		}
	}
};
void Area::water(int x1,int y1,int x2,int y2) {
	ASSERT(x1<=x2);
	ASSERT(y1<=y2);
	for (int i=x1 ; i<=x2 ; ++i) {
		for (int j=y1 ; j<=y2 ; ++j) {
			Block* b=block(i,j);
			if (b) {
				b->mEnvironment.water();
			}
		}
	}
};
void Area::gravity(int x1,int y1,int x2,int y2,float g) {
	ASSERT(x1<=x2);
	ASSERT(y1<=y2);
	for (int i=x1 ; i<=x2 ; ++i) {
		for (int j=y1 ; j<=y2 ; ++j) {
			Block* b=block(i,j);
			if (b) {
				b->mEnvironment.mGravity=g;
			}
		}
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�X�v���C�g�N���X
Sprite* Sprite::mInstance[mInstanceMax];

void Sprite::create() {
	for (int i=0 ; i<mInstanceMax ; ++i) {
		mInstance[i] = 0;
	}
}
void Sprite::destroy() {
	for (int i=0 ; i<mInstanceMax ; ++i) {
		ASSERT (mInstance[i] == 0);			//�����Y�ꂪ��������ASSERT
	}
}
Sprite* Sprite::instance(int i) {
	ASSERT(i<mInstanceMax);
	return mInstance[i];
}

void Sprite::removeModel(Model* model) {
	for (int i=0 ; i<mInstanceMax ;++i) {
		if (mInstance[i]) {
			for (int j=0 ; j<mInstance[i]->mObjectMax ; ++j) {
				if (mInstance[i]->mObject[j].isEnable()) {
					if (mInstance[i]->mObject[j].model() == model) {
						mInstance[i]->mObject[j].remove();
					}
				}
			}
		}
	}

};

Sprite::Sprite(int objectmax) {
	bool r=false;
	for (int i=0 ; i<mInstanceMax ; ++i) {
		if (mInstance[i] ==0 ) {
			mInstance[i] = this;
			r = true;
			break;
		}
	}
	ASSERT(r);		//���肷��

	mObjectMax = objectmax;
	mObject = new Object[objectmax];

	for(int i=0 ; i<mObjectMax ; ++i) {
		mObject[i].mIndex=i;
		mObject[i].mSprite = this;
	}
	mMax=0;

	//�������I�u�W�F�N�g�͌�ŕ`�悷�邻�̏��
	mTransparentObject = new Object*[mObjectMax];
	mTransparentObjectNumber1=0;
	mTransparentObjectNumber2=0;

	mGroup = 0;

}
Sprite::~Sprite() {
	for (int i=0 ; i<mInstanceMax ; ++i) {
		if (mInstance[i] == this) {
			mInstance[i] = 0;
		}
	}

	if (mObject) {
		delete []mObject;
		mObject = 0;
	}


	if ( mTransparentObject) {
		delete []mTransparentObject;
		mTransparentObject=0;
	}
}

void Object::launch() {

	if (mParent) {
		if (mFollowOnlyPosition) {
			//�ʒu�̂ݒǏ]����
			mParentWorld.setIdentity();
			mParentWorld.setTranslation(mParent->mWorld.m03,mParent->mWorld.m13,mParent->mWorld.m23);
		}
		else {
			//mParentWorld = mParent->mWorld;
			mParent->getMatrixFinal(&mParentWorld);
		}
		mParentIsIdentityMatrix = false;
	}
	::BlendInfo bi = blendInfo();

	GameLib::Math::Matrix34 tmp;
	if (mModel) {
		bi.getWorldMatrix(&tmp,
			mPosition.mX.value(),
			mPosition.mY.value(),
			mPosition.mZ.value(),
			mModel->sprite(),mModel->frontEnd());
	}
	else {
		bi.getWorldMatrix(&tmp,
			mPosition.mX.value(),
			mPosition.mY.value(),
			mPosition.mZ.value(),
			0,false);
	}
	mBlendElement.set(bi);
	if (mParent) {
		mBlendElement.mulColor(mParent->mBlendElement);
	}

	if (mParentIsIdentityMatrix) {
		mWorld = tmp;
	}
	else {
		mWorld.setMul(mParentWorld,tmp);	//�d�����邩���B
	}

//	mWorld.mul(&mWorldPosition,GameLib::Math::Vector3(0.f,0.f,0.f));	//�����Ȃ񂾂��ǂˁB
	mWorldPosition.x=mWorld.m03;
	mWorldPosition.y=mWorld.m13;
	mWorldPosition.z=mWorld.m23;

	mWorldPosition *= Draw::mWidth/2;

};

//�Ȃ��ق��������I
void Sprite::launchObject() {
	ASSERT(0);

	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable()) {
			mObject[i].launch();
		}
	}
};

//�I�u�W�F�N�g�̓����蔻��ʒu��p�ӂ���
void Object::updateCollision() {
	//model���瓖���蔻��������炤
	if (model()) {
		model()->getCollision(&mCollision.mPolygon,
			mWorld,mBlendElement,mAnimeCount);
		mCollision.mPolygon *= Draw::mWidth / 2.f; 
	}
}

bool Object::Collision::seek(const Object& obj) const{
	bool r=false;
	for (int i=0 ; i<mMax ;++i) {
		if (mObject[i]==0) {
			break;
		}
		if (mObject[i] == &obj ){
			r=true;
			break;
		}
	};
	return r;
}

void Sprite::updateObject(unsigned int flag){
	//mMax�̃`�F�b�N�������ōs��
	int m=0;
	for(int i=0 ; i<mMax+1 ; ++i){
		mObject[i].mUpdated=false;

		if (mObject[i].isEnable()) {
			m=i;
		}
	}
	mMax=m;

	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable()) {
			mObject[i].update(flag);
		}
	}
}
void Object::update( unsigned int flag ) {
	if (!mUpdated) {
		mUpdated=true;

		if (isEnable()) {
			if (flag==0 || mCollision.mFlag & flag) {
				if (mParent) {
					if (!mParent->mUpdated) {
						mParent->update(flag);
					}
				}
				++mCount;

				if (mAnimeCount >= 0.f) {
					mPosition.update();
					mRotate.update();
					mZoom.update();
					mAlpha.update();
					mRed.update();
					mGreen.update();
					mBlue.update();
					//���Ԑ���
					if (mLimit>0) {
						--mLimit;

						if (mLimit <= 0 ) {
							remove();
							return;
						}
					}
				}

				//�A�j��
				mAnimeCount+=mAnimeSpeed;
				if (mAutoRemove) {
					if (mModel) {
						int r = mModel->anime(mWorld,mBlendElement)->
							getLoop(mWorld,mBlendElement,mAnimeCount);

						if (r >= mAutoRemove) {
							remove();
							return;
						}
					}
				}
			}
			launch();
			//������񑗂�
			if (mLight) {
				//�F��񂭂炢�͔��f���Ă��������낤
				float i = mLightIntensity * mBlendElement.mTransparency;
				GameLib::Math::Vector3 color = mLightColor;
				color *= mBlendElement.mAmbient;
				color += mBlendElement.mEmission;
				Draw::instance()->light(2)->setPosition(mWorldPosition);
				Draw::instance()->light(2)->setColor(color);
				Draw::instance()->light(2)->setQuantity(i);
			}
			//�����_���𑗂�
			if (mFocus) {
				Draw::instance()->mCamera.focus(mWorldPosition);
			}
		}
	}
}

int Sprite::getNewObject(int a) {
	int i;
	for (i=a ; i<(mObjectMax+1) ; ++i) {
		if (mObject[i].isEnable() == false ) {
			break;
		} 
	}
	ASSERT( i!=mObjectMax);		//�I�u�W�F�N�g�������ς��I

	if (mMax<i) {	//�ő�������X�V
		mMax=i;
	}
	return i;
}

Object* Sprite::addObject(Model* m) {
	int i=getNewObject();
	ASSERT( !mObject[i].isEnable() );		//�����Ƌ�̃i���o�[��U���Ă邩�`�F�b�N
	mObject[i].create(m);
	mObject[i].mGroup = mGroup;
	return mObject+i;
}
void Object::create(Model* m) {
	//mChara.reset();

	mBlendInfo.reset();

	mAvailable=true;
	mVisible=true;
	mReverseX=false;
	mReverseY=false;
	mReverseZ=false;
	mSortReverse=false;

	mFlashEmission.set(1.f,1.f,1.f);			//�f�t�H�͔��_��

	mParent=0;
	mRoot=this;
	mStable=false;
	mFollowOnlyPosition = false;

	mTexture=0;
	mModel=m;
	mOriginalModel=m;

	mAnimeCount=0.0f;
	mAnimeSpeed=1.0f;
	mAutoRemove = 0;
	mGroup = 0;

	m3D=false;
	mFlash=0;
	mLimit=-1;
	mCount=0;

	mZoom.initialize();
	mZoom.set(1.f,1.f,1.f);

	mPosition.initialize();
	mPosition.set(0.f,0.f,0.f);

	mRotate.initialize();
	mRotate.set(0.f,0.f,0.f);

	mAlpha.initialize();
	mAlpha.set(1.f);
	mRed.initialize();
	mRed.set(1.f);
	mGreen.initialize();
	mGreen.set(1.f);
	mBlue.initialize();
	mBlue.set(1.f);


	mLight = false;
	mLightColor = 1.f;

	mFocus = false;

	mCollision.initialize();

	mGravity=false;
	mResistance=false;



	mID=mNumber;

	mWorldPositionprev=0.f;		//�����t���[���ڂȂ̂Ŏd���Ȃ�
	mParentWorld.setIdentity();
	mParentIsIdentityMatrix = true;

	++mNumber;
}

void Object::Collision::initialize() {
	mAvailable = true;
	mFlag=0;
	mFlagTarget=0;
	mReactFlag = 0;
	mPriority=2;

	reset();

	mReact.initialize();
};
void Object::Collision::reset() {
	mObject[0]=0;
	mArea[0]=0;
}

void Object::Collision::React::initialize() {
	mOffset=0.f;

}


//�I�u�W�F�N�g�̃R�s�[�B���[�g�I�u�W�F�N�g������̂ł�����g������
Object* Sprite::copyObject(Object* obj) {
	int j=getNewObject();
	mObject[j].create(obj);

	return mObject+j;
}

void Object::create(const Object* src) {
	//�ۑ����ׂ��p�����[�^
	int index = mIndex;

	//�����ł��������ςɃR�s�[����
	*this = *src;

	//�߂��ׂ��p�����[�^�͖߂�
	mIndex=index;

	//�e�Ȃ��̏ꍇ�A���[�g�������ɂ���
	if ( mParent==0 ) {
		mRoot=this;
	}

	//ID�͕ʂɂȂ�
	mID=Object::mNumber;
	++Object::mNumber;
}


void Sprite::drawObject(bool zwriteon,bool zwriteoff){


	//Z�������݂���I�u�W�F�N�g�̕`��
	//������Z�`�����݂��Ȃ��I�u�W�F�N�g�̃��X�g�����
	if (zwriteon) {
		//off�̃f�[�^���c���Ă���A�O�t���[���ŕ`���킷��Ă���̂ŃG���[
		ASSERT(mTransparentObjectNumber1==0 &&
				mTransparentObjectNumber2==0);

		for(int i=0 ; i<mMax+1 ; ++i){
			if (mObject[i].isEnable()) {

			//	mObject[i].launch();
				if (mObject[i].isVisible() && mObject[i].model()) {

					//Z�`�����݂��邩���f�@�������ނƂ������`��
					bool b;
					if (mObject[i].mBlendInfo.mBlendMode < 0) {
						b=mObject[i].model()->depthWrite();
					}
					else {
						b=(mObject[i].mBlendInfo.mBlendMode == 0);
					}

					if (b) {
						mObject[i].draw();
					}
					else {
						if (mObject[i].mSortReverse) {
							mTransparentObject[mObjectMax - mTransparentObjectNumber2 -1]=mObject+i;
							++mTransparentObjectNumber2;
						}
						else {
							mTransparentObject[mTransparentObjectNumber1]=mObject+i;
							++mTransparentObjectNumber1;
						}
					};
				}
			}
		}
	}
	if (zwriteoff) {
		//Z�������ݖ����̃I�u�W�F�N�g�͌�ŕ`��
		//������Z�\�[�g�ł���΂����񂾂��ǁE�E�����_�i

		for (int i=0 ; i<mTransparentObjectNumber1 ; ++i) {
			mTransparentObject[i]->draw();
		};

		//sortreverse
		//�ԍ��̎Ⴂ�z�����ɕ`�悳���
		for (int i=0 ; i<mTransparentObjectNumber2 ; ++i) {
			//mTransparentObject[mObjectMax -mTransparentObjectNumber2 + i]->draw();
			mTransparentObject[mObjectMax -i-1]->draw();
		}

		//�`�����烊�Z�b�g�B�Ԉ����1�t���[���ɓ��Ăяo���Ă������悤�ɁB
		mTransparentObjectNumber1=0;
		mTransparentObjectNumber2=0;



		//�f�o�b�O�p�Ǖ\��
		if (Main::mDebugPrint) {
			Draw* Draw=Draw::instance();
			for (List<Area::Vector>::Iterator i(mArea.mList) ; !i.end() ; ++i) {
				Draw->drawLine(
					i()->mSegment.mStart.x,
					i()->mSegment.mStart.y,
					i()->mSegment.mEnd.x,
					i()->mSegment.mEnd.y,
					1.0f);
			}

			//�����ƕ�������Ă邩�`�F�b�N
			//BlendInfo bi;
			//bi.mDepthTest=false;
			//for (int i=0 ; i<mArea.mAvailableX ; ++i) {
			//	for (int j=0 ; j<mArea.mAvailableY ; ++j) {
			//		if (memberNumber(i,j)) {
			//			ModelList::instance()->mPlayer.mZero->mBatch->draw(
				//				i*64,j*64,
			//				0,bi);
			//			}
			//		}
			//}

			//�v�Z�ʃJ�E���g
			//Draw::instance()->text(16,240,mCalcCount,0xffffffff,2);
			//Draw::instance()->text(16,240+32,mMax,0xffffffff,2);
		}
	}
}

void Sprite::removeObject(unsigned int flag) {
	for (int i=0 ; i<mObjectMax ; ++i) {
		if ((mObject[i].mCollision.mFlag & flag) || (flag == 0)) {
			mObject[i].remove();
		}
	}
}

void Sprite::setGroup(unsigned int group) {
	mGroup = group;
}

//�q�̃I�u�W�F�N�g�����ׂď���
void  Object::remove() {
	if (mAvailable) {
		mAvailable=false;

		if (mSprite->mMax == mIndex) {	//�ő�������X�V
			--mSprite->mMax;
		}

		//���̃I�u�W�F�N�g����q��{��
		for (int i=0 ; i <= mSprite->mMax ; ++i ) {
			if (mSprite->mObject[i].mParent == this ) {
				//�e�����񂾂瓹�A��ɂ��邩�H�Ɨ������邩�H
				if (mSprite->mObject[i].mStable){
					mSprite->mObject[i].leaveParent();
				}
				else {
					mSprite->mObject[i].remove();
				}
			}
		}
	}
}

//�\��/��\���̈ꊇ�ݒ�
void Sprite::setVisible(bool v,unsigned int side) {
	//for(int i=0 ; i<mMax+1 ; ++i){
	for(int i=0 ; i<mObjectMax ; ++i){

		if (mObject[i].mCollision.mFlag & side || side==0) {
			mObject[i].mVisible=v;
		}
	}
}

int  Sprite::getObjectNumber() {
	return mMax;
}

void Sprite::spot(float intensity, int time, const GameLib::Math::Vector3 &pos,const GameLib::Math::Vector3& color) {
	Object* obj = addObject(0);
	obj->setPosition(pos);
	obj->setLimit(time);
	obj->setLight(intensity,color);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Area::Container::Object::Object() {
	reset();
};


void Area::Container::Object::reset() {
	mMember[0] = 0;
}

int Area::Container::Object::number() const {
	int result = 0;
	for (int i=0 ; i<mMax; ++i) {
		if (mMember[i] == 0) {
			break;
		}
		++result;
	}
	return result;
}

Object* Area::Container::Object::member(int i) const{
	ASSERT(i<mMax);
	return mMember[i];
}

void Area::Container::Object::add(::Object *obj){
	int i=number();
	if (i<mMax) {
		mMember[i] = obj;
		if (i+1 < mMax) {
			mMember[i+1] = 0;
		}
	};
};

//�L���ȃG���A��ݒ�
void Sprite::resizeArea(int offsetX,int offsetY,int w,int h,int chipw,int chiph) {
	//�ς��Ȃ��ꍇ�͏������Ȃ�����

	if (w != mArea.mBlockX ||
		h != mArea.mBlockY ||
		chipw != mArea.mBlockWidth ||
		chiph != mArea.mBlockHeight) {

		mArea.destroy();
		mArea.create(offsetX,offsetY,chipw,chiph,w,h);
	}

	//resize������reset�ł������낤
	mArea.reset();
}


//�����蔻�菈��
void Sprite::updateObjectCollision() {
	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable()) {
			mObject[i].updateCollision();
		}
	}
	getObjectCollision();

	//��������
	getObjectReact();

	//�ʒu��ۑ�
	saveObjectPosition();
};

void Sprite::getObjectCollision() {

	//����������
	for (int i=0 ; i<mArea.mContainerWidth ; ++i) {
		for (int j=0 ; j<mArea.mContainerHeight ; ++j) {
			mArea.container(i,j)->mObject.reset();
		}
	}

	//�I�u�W�F�N�g��������
	for (int i=0 ; i<mMax+1 ; ++i) {				//create���ł����Ə��������Ă�΂����őS������������K�v�͂Ȃ�
		mObject[i].mCollision.reset();
		mObject[i].mCollision.mReact.initialize();
	}

	//�܂��͕����@X,Y���ŕ�������B
	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable() && mObject[i].mCollision.mAvailable) {
			if (mObject[i].model()) {
				GameLib::Math::Vector3 m,M;
				mObject[i].mCollision.mPolygon.getCuboid(&m,&M);

				int a=mArea.getSplitAreaX(m.x);
				int b=mArea.getSplitAreaX(M.x);
				int c=mArea.getSplitAreaY(m.y);
				int d=mArea.getSplitAreaY(M.y);

				for (int p = a ; p<(b+1) ; ++p) {
					for (int q= c ; q<(d+1) ; ++q) {
						if (mArea.container(p,q)) {
							mArea.container(p,q)->mObject.add(mObject+i);
						}
					}
				}
			}
		}
	}

	//�����Ƃɏ�������
	for (int p=0 ; p<mArea.mContainerWidth ; ++p) {
		for (int q=0 ; q<mArea.mContainerHeight ; ++q) {
			mArea.container(p,q)->collision();
		}
	}

}
void Sprite::getObjectReact() {
	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable()) {
			mObject[i].mCollision.mReact.react(mObject[i].mCollision.mPolygon);
			mObject[i].addPosition(*mObject[i].mCollision.mReact.offset());	//���炵�Ă�鏈��
		}
	}
};

void Sprite::saveObjectPosition() {
	for(int i=0 ; i<mMax+1 ; ++i){
		if (mObject[i].isEnable()) {
			mObject[i].savePrevPosition();
		}
	}
}

//�����ő�������
void Area::Container::collision() const {
	for (int j=0 ; j < mObject.number() ; ++j) {
		::Object* s= mObject.member(j);

		//�΃I�u�W�F�N�g�̓����蔻��
		for(int k=(j+1) ; k < mObject.number() ; ++k) {
			::Object* t= mObject.member(k);

			if ((s->mCollision.mFlag & t->mCollision.mFlagTarget)||
				(s->mCollision.mFlagTarget & t->mCollision.mFlag))
			{
				bool r;
				GameLib::Math::Vector3 vct;
				bool vctcheck =
					(s->mCollision.mFlag & t->mCollision.mReactFlag)||
					(s->mCollision.mReactFlag & t->mCollision.mFlag);

				if (vctcheck) {
					r = Sprite::checkObjectCollision(&vct,*s,*t);
				}
				else {
					r = Sprite::checkObjectCollision(0,*s,*t);
				}

				//�Փ˂���
				if (r) {
					if (s->mCollision.mFlagTarget & t->mCollision.mFlag) {

						//��������
						if ( s->mCollision.mReactFlag & t->mCollision.mFlag) {
							s->mCollision.addObject(t,true,vct);
						}
						else {
							s->mCollision.addObject(t,false,vct);
						}
					}
					if (t->mCollision.mFlagTarget & s->mCollision.mFlag) {
						vct *= -1.f;
						if ( t->mCollision.mReactFlag & s->mCollision.mFlag) {
							t->mCollision.addObject(s,true,vct);
						}
						else {
							t->mCollision.addObject(s,false,vct);
						}
					}
				}
			}
		}

		//�n�`�Ƃ̓����蔻��
		if ( s->mCollision.mFlagTarget & 1) {

			for (List<const Area::Vector*>::Iterator i(mVector) ; !i.end() ; i++) {

				if (Sprite::checkObjectCollisionArea(*s,**i())) {
					GameLib::Math::Vector3 vct;
					//vct.x = a->mCollisionX2 - a->mCollisionX1;
					//vct.y = a->mCollisionY2 - a->mCollisionY1;
					s->mCollision.addArea(
						*i(),
						( s->mCollision.mReactFlag & 1),
						vct);
				}
			}
		}
	}
}

const GameLib::Math::Vector3* Object::Collision::React::offset() const {
	return &mOffset;
};

	void Object::Collision::React::react(const Math::Polygon& polygon){
/*
		//�Փˉ��� �ő�␳��32px
		for(int i=0 ; i<mMax+1 ; ++i){
			if (mObject[i].isEnable()) {
				//++mCalcCount;
				//�␳�l��������
				mObject[i].mCollision.mReact.mOffset=0.f;
				if (mObject[i].mCollision.mReact.mNumber) {
					//�����̈ʒu
					double px,py;
					px=mObject[i].worldPosition()->x;
					py=mObject[i].worldPosition()->y;
	
					double pdx,pdy;//�m�肳�ꂽ�␳�l
					pdx=0;
					pdy=0;

					//�������ׂ��I�u�W�F�N�g�̃��X�g 0-7�I�u�W�F�N�g�@8-15�n�`
					bool col[16]={false};
					for (int j=0; j<Object::Collision::mMax; ++j) {
						if (mObject[i].mCollision.mObject[j]) {
							//�I�u�W�F�N�g�Ƃ̓����蔻��
							if (mObject[i].mCollision.mReactFlag &
								mObject[i].mCollision.mObject[j]->mCollision.mFlag)
							{
								col[j]=true;
							}
						}
						else {
							break;
						}
					}
					for (int j=0; j<Object::Collision::mMax; ++j) {
						if (mObject[i].mCollision.mArea[j]) {
							//�n�`�Ƃ̓����蔻��
							if (mObject[i].mCollision.mReactFlag & 1) {
								col[j+Object::Collision::Max]=true;
							}
						}
						else {
							break;
						}
					}

					//�ڐG����I�u�W�F�N�g���ւ炷�@�[���ɂȂ�����悩������
					//���݈����Ă���D��x
					unsigned int priority=0;

					int l=0;	//�␳���钷��
					int w=-1;	//�Ώۂ̈���
					for (int count=0; count<1024 ; ++count)  {
						//���Ƀ`�F�b�N����I�u�W�F�N�g�����߂�
						bool found=false;
						for (int j=0 ; j<16 ; ++j){
							++w;
							w%=Object::Collision::mMax*2;
							if (w==0) {
								++l;
								//ASSERT(l<32);
								
							}

							if (!col[w]) {
								continue;
							}

							//�D�揇��
							if (w<Object::mCollisionMax) {
								if (mObject[i].mCollisionObjectResult[w]->mCollisionPriority >
									priority)
								{
									continue;
								}
							}
							else {
								if (mObject[i].mCollisionAreaResult[w-Object::mCollisionMax]->mParent->mPriority >
									priority)
								{
									continue;
								}
							}

							found=true;
							break;
						}
						if (found==false) {
							++priority;
							if (priority>2) {
								break;
							}
							continue;
						}

						//�����ɂ��킹�镪
						double mx=0;
						double my=0;

					//	if (w<8) {
							//�I�u�W�F�N�g�̓���
					//		Object* k=mObject[i].mCollisionObjectResult[w];
					//		mx= k->mWorldX - k->mWorldXprev;
					//		my= k->mWorldY - k->mWorldYprev;
					//	}
						//�ڐG�����Ӄx�N�^
						double vx,vy;
						vx = mObject[i].mReactVector[w].x;
						vy = mObject[i].mReactVector[w].y;

						//�Ӄx�N�^��␳�x�N�^�ɕϊ�
						double u,v;
						if (w<Object::mCollisionMax) {
							u=normalize(vy);
							v=normalize(-vx);
						}
						else {
							u=vy;
							v=-vx;

							//���܂�}�ȌX�΂͕ǈ����B
							if (v*v<(0.6*0.6)) {
								u=normalize(u);
								v=0;
							};

							//��邢�X�΂͏������B
							if (v*v>(0.8*0.8)) {
								u=0;
								v=normalize(v);
							};
						}

						//�������A�m��␳�ʂƋt�����ɕ␳�͂��Ȃ��i�����ς݂̃I�u�W�F�N�g�ɐڐG���Ȃ����Ă��܂�
						if (pdx*u<0) {
							u=0;
							mx=0;
						}
						if (pdy*v<0) {
							v=0;
							my=0;
						}
						//�����␳�l
						double dx=0;
						double dy=0;

						//�␳��̃I�u�W�F�N�g���痣�ꂽ���ǂ����`�F�b�N
						bool success=false;

						//����ŕ␳�x�N�^���Ȃ��Ȃ�����A�����������Ƃɂ���
						if ((u==0) && (v==0)) {
							col[w]=false;
							success=true;
						}
						else {
							//�I�u�W�F�N�g�̏ꍇ
							if (w<Object::mCollisionMax) {
								//X��Y�����݂Ƀ`�F�b�N
								if ((l%2) == 0 ||false) {
									u=0;
								}
								else {
									v=0;
								}
								int m=(l+1)/2;
								dx=u*0.5*m;
								dy=v*0.5*m;
							}
							else {
								dx=u*0.5*l;
								dy=v*0.5*l;
							}

							//l==0�ł��Ȃ��̂ɕ␳�x�N�^���Ȃ��Ȃ�����A�X�L�b�v���Ă��������
							if ((l!=0) && (u==0) && (v==0)) {
							}
							else {
								//�I�u�W�F�N�g���ړ�������
								mObject[i].setPosition(
									px + dx + mx + pdx,
									py + dy + my + pdy,
									mObject[i].getZ());

								//�X�V
								mObject[i].updateCollision();

							//	ASSERT(l);
								//�␳��̃I�u�W�F�N�g���痣�ꂽ���ǂ����`�F�b�N
								//�ڐG�`�F�b�N
								if (w<Object::mCollisionMax) {
									//�΃I�u�W�F�N�g
									if (!checkObjectCollision(
										0,
										*(mObject+i),
										*((mObject+i)->mCollisionObjectResult[w])))
									{
										ASSERT(l>0);		//�␳���ĂȂ��̂ɗ��ꂽ�H
									//	if (dy<0) {
									//		ASSERT(0);
									//	}
										col[w]=false;
										success=true;
									}
								}
									else {
									//�Βn�`
									if (!checkObjectCollisionArea(
										*(mObject+i),
										*mObject[i].mCollisionAreaResult[w-Object::mCollisionMax]))
									{
										col[w]=false;
										success=true;
									}
								}
							}
						}
						//�ڐG�I�u�W�F�N�g���ւ炷���Ƃɐ����B
						if (success) {
							//�I�u�W�F�N�g�ɏ���Ă���ꍇ�́A���E������������Ɉړ�
							if (w<Object::mCollisionMax) {
								if (dy) {
									Object* k=mObject[i].mCollisionObjectResult[w];
									mx= k->worldPosition()->x - k->prevWorldPosition()->x;
									//my= k->mWorldY - k->mWorldYprev;

									//�������m��␳�l�Ƌt�Ȃ�L�����Z��
									if (mx * pdx <0) {
										mx=0;
									}
								}
							}

							pdx+=dx+mx;
							pdy+=dy+my;

							if ((mObject[i].mPrimaryReactVector.x==0.f) &&
								(mObject[i].mPrimaryReactVector.y==0.f))
							{
								mObject[i].mPrimaryReactVector=mObject[i].mReactVector[w];
							};

							w=-1;
							l=-1;
						}
					}

					//�␳�l��ۑ����Ă���
					mObject[i].mReactOffset.x=pdx;
					mObject[i].mReactOffset.y=pdy;

					//�I�u�W�F�N�g���ړ�������
					mObject[i].setPosition(
						px + pdx , py + pdy ,mObject[i].getZ());
					mObject[i].updateCollision();
				}
			}
		}
		return resultTotal;
*/
	}

//�I�u�W�F�N�g���m�̏Փ˔���B
bool Sprite::checkObjectCollision(GameLib::Math::Vector3* out,const Object& A,const Object& B) {
	//����
	bool result=false;

	//�����p�x�N�^�͌�񂵂ŁA
	if (out) {
		ASSERT(0);
	};

	return Math::collision(A.mCollision.mPolygon,B.mCollision.mPolygon);
}

//�I�u�W�F�N�g�ƒn�`�x�N�^�Ƃ̓����蔻��
bool Sprite::checkObjectCollisionArea(const Object& ob,const Area::Vector& area) {
	bool result=false;

	Math::Polygon a;
	a.set(area.mSegment);

	result = Math::collision(ob.mCollision.mPolygon,a);

	//���x�x�N�^�ƒn�`�x�N�^�̌�������.
	if (!result) {
		if ((ob.prevWorldPosition()->x !=0.f) &&			//�����t���[���߂��Ƒ��x�x�N�^������Ȃ�
			(ob.prevWorldPosition()->y !=0.f))
		{

			Math::Segment seg;
			seg.mEnd = *ob.worldPosition();

			seg.mStart.x = ob.prevWorldPosition()->x;
			seg.mStart.y = ob.prevWorldPosition()->y;
			seg.mStart.z = 0.f;
			Math::Polygon p;
			p.set(seg);

			result = Math::collision(p,a);
		}
	}

	return result;
};


void Object::getAngle(float *h, float *v,const Object* target) const{
	GameLib::Math::Vector3 vct = target->mWorldPosition;
	vct -= mWorldPosition;
	Math::vector3ToAngle(h,v,vct);
}

//�����B���������g���̂ŏd�����B�C��t���悤
float Object::getDistance(const Object& obj) const{
	GameLib::Math::Vector3 vct = *worldPosition();
	vct -= *obj.worldPosition();
	return vct.length();
}

//�������̂ɁB���ŕԂ��̂ō���
float Object::getSquareDistance(const Object& obj) const{
	GameLib::Math::Vector3 vct = *worldPosition();
	vct -= *obj.worldPosition();
	return vct.squareLength();
}

int Area::getSplitAreaX(float x) {
	//int r=static_cast<unsigned int>( (x+4) / Sprite::mArea.mSplitWidth);
	int r= (x-mContainerX) / Container::mWidth;
	return r;
};
int Area::getSplitAreaY(float y) {
	//int r=static_cast<unsigned int>( (x+4) / Sprite::mArea.mSplitWidth);
	int r= (y-mContainerY) / Container::mHeight;
	return r;
};

void Object::getEnvironment(int gravity,bool water,float ax,float ay) {
	if (mGravity || mResistance) {
		Area::Block* block=getBlock();

		if (block) {
			if (mResistance) {
				setResistance(block->mEnvironment.mResistance,block->mEnvironment.mResistance,1.0);
			}
			if (mGravity) {
			///	setAccel(block->mEnvironment.mAccelX,block->mEnvironment.mAccelY+block->mEnvironment.mGravity);
			}
		}
	}
}
Area::Block* Object::getBlock() {

	int x=mWorldPosition.x / mSprite->mArea.mBlockWidth; 
	int y=mWorldPosition.y / mSprite->mArea.mBlockHeight; 
	return mSprite->mArea.block(x,y);
};
void Object::getFocus() const {
	Draw::instance()->mCamera.aimTarget(
		mWorldPosition.x,mWorldPosition.y,mWorldPosition.z);
}
