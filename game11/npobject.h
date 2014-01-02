#ifndef INCLUDED_NPOBJECT_H
#define INCLUDED_NPOBJECT_H

#include "npsysc.h"

#include "npmath.h"
#include "npdraw.h"

//�I�u�W�F�N�g����



//�n�`�f�[�^
//�x�N�^�ƃu���b�N�ō\�������B
//�x�N�^�͂Ƃ������u���b�N�̓}�b�v�T�C�Y���������Ȃ����new����K�v�͂Ȃ��B
class Object;
class Area {
public:
	class Block {
	public:
		//�d�́E���E��C��R
		class Environment {
		public:
			float mGravity;
			float mAccelX;
			float mAccelY;
			float mResistance;
			bool mWater;

			Environment();
			void reset();
			void water();
		};

		bool mSide[4];
		unsigned int mX;
		unsigned int mY;
	public:
		unsigned int mType;			//����n�`�̃^�C�v
		unsigned int mPriority;		//�����蔻��D��x
		Environment mEnvironment;	//�d�͂Ȃ�
		bool mSeal;					//�S�[�X�g�o������

		int mParam;
		int mLimit;					//����n�`���ŃJ�E���g
		bool mVanish;				//����n�`���ŃI�[�_�[

		Block();
		void reset();
		void setLimit(int f);
		bool side(int);
	};

	class Vector {
	public:
		Math::Segment mSegment;
		Block* mParent;			//��������}�X;
		Vector();
	};

	class Container{
		class Object{
			static const int mMax = 64;
			::Object* mMember[mMax];
		public:
			Object();
			void reset();
			int number()const;
			::Object* member(int)const;
			void add(::Object*);
		};
	public:
		//���̃T�C�Y
		static const int mWidth=64;							//�����s�N�Z���@���̐����}�b�v�̉��̒���/�����s�N�Z��
		static const int mHeight=64;

		List<const Vector*> mVector;
		Object mObject;

		void collision() const;	//�����ő������菈��
	};

private:
	Block* mBlock;			//block�֐��ŃA�N�Z�X����
	Container* mContainer;	//container�֐��ŃA�N�Z�X����
public:

	List<Vector> mList;

	int mContainerX,mContainerY;
	unsigned int mContainerWidth,mContainerHeight;
	unsigned int mBlockX,mBlockY;
	unsigned int mBlockWidth,mBlockHeight;


	Area();
	~Area();
	void create(int offsetX,int offsetY,unsigned int chipwidth,unsigned int chipheight,unsigned int width,unsigned int height);
	void destroy();
	void reset();
	void removeVector();

	void addCollision(double x1,double y1,double x2,double y2,unsigned int i,unsigned int j) ;
	void split();																			//�n�`�̋ϓ�����

	Block* block(unsigned int i,unsigned int j)const ;
	Container* container(unsigned int i,unsigned int j)const;
	void removeBlock(unsigned int x,unsigned int y);		//���̌�split���邱��
	void removeBlock(Block*);								//���̌�split���邱��
	void createBlock();
	void createSide(unsigned int i,unsigned int j,int side);

	void flow(int x1,int y1,int x2,int y2,float ax,float ay);					//block��environment��͈͂Őݒ�
	void water(int x1,int y1,int x2,int y2);
	void gravity(int x1,int y1,int x2,int y2,float g);
	int getSplitAreaX(float x);	//�w���W����A�����n�`�̈ʒu�𓾂�
	int getSplitAreaY(float y);	//Y���W����A�����n�`�̈ʒu�𓾂�

};


//�I�u�W�F�N�g�f�[�^
class Sprite;
class Object {
public:
	//�����蔻�����������N���X
	class Collision{
	public:
		//�Փˉ������������邭�炷
		class React{
			static const int mMax = 16;
			Math::Segment mSegment[mMax];
			GameLib::Math::Vector3 mNormal;

			int mNumber;
			GameLib::Math::Vector3 mPrimarySegment;			//���������x�N�^�̂�����\
			GameLib::Math::Vector3 mOffset;					//�Փˉ����␳�l

		public:
			void initialize();
			void add(const Math::Segment&);
			void react(const Math::Polygon&);			//�Փˉ�������
			const GameLib::Math::Vector3* offset()const;
		};


	private:
		//���������I�u�W�F�N�g�̃��X�g
		static const int mMax = 8;
		Object* mObject[mMax];						//AreaVector��Object������킯�����B
		const Area::Vector* mArea[mMax];
	public:

		//�����蔻��ʒu�B�v�Z����Ȃ����B
		Math::Polygon mPolygon;

		unsigned int mFlag;			//�����蔻��t���O�B
		unsigned int mFlagTarget;		//�����蔻����s���Ώۂ̃t���O�B
		unsigned int mReactFlag;		//�Փˉ�������t���O
		unsigned int mPriority;		//�D��x0-2�@0������

		React mReact;
		bool mAvailable;

		void reset();	//�����蔻�茋�ʂ�Y���
		void initialize();
		void addObject(Object*,bool react,const GameLib::Math::Vector3&);			//�Փ˃I�u�W�F�N�g���X�g�ɒǉ�����
		void addArea(const Area::Vector*,bool react,const GameLib::Math::Vector3&);	//�Փ˃I�u�W�F�N�g���X�g�ɒǉ�����
		bool seek(const Object&) const;	//�Փ˃��X�g�ɋ���H
		int objectNumber()const;		//�Փː�

	};


private:
	int mFlash;						//�t���b�V������
	int mFlashTime;					//�t���b�V�����鎞��(<����)
	int mFlashFrame;				//�t���b�V���J�n�t���[��
	GameLib::Math::Vector3 mFlashEmission;				//�t���b�V�����̃u�����h�@�ԓ_�ł��f�t�H�ɂ��Ă���

//public:

	//�I�u�W�F�N�g�p�̃u�����h���@�悤����ɗ]����̂�
	class BlendInfo{
	public:
		int mBlendMode;
		GameLib::Math::Vector3 mEmission;
		bool mLighting[4];

		void reset();
		void write(::BlendInfo*)const;
	};

	static int mNumber;					//ID�̐����Ɏg�p�B�����Y��
public:
	Sprite* mSprite;
	int mIndex;							//�����̓Y����
	bool mUpdated;
private:
	int mID;							//��������邽�тɂ�����ԍ�
	bool mAvailable;					//���݂���

	bool m3D;							//3D���[�h�B3D���f���̓����蔻��͉�]�Ɋ֌W�Ȃ��Ȃ�

	int mLimit;							//�������ł܂ł̎c�莞��
	unsigned int mCount;				//update������

	Object* mParent;					//�ЂƂ�̃I�u�W�F�N�g
	Object* mRoot;						//��ԏ�̐e�I�u�W�F�N�g

	bool mStable;						//�e�����񂾂�Ɨ�����
	bool mFollowOnlyPosition;			//�e������炤�͍̂��W�̂�

	bool mReverseX,mReverseY,mReverseZ;


	MyTexture* mTexture;
	Model* mModel;
	Model* mOriginalModel;

	float mAnimeCount;
	float mAnimeSpeed;

	DynamicVector3 mZoom;
	DynamicVector3 mPosition;
	DynamicVector3 mRotate;
	DynamicValue mAlpha,mRed,mGreen,mBlue;

	//����
	bool mLight;
	float mLightIntensity;
	GameLib::Math::Vector3 mLightColor;

	//�����_�ɂȂ邩�H
	bool mFocus;

public:
	bool mVisible;
	BlendInfo mBlendInfo;				//����킵�����Adraw�Ŏg�����̂Ƃ͕ʂ̃N���X

	bool mSortReverse;					//Z�o�b�t�@�������݂��ɂȂ����[�h�ŁA�Y�������ɂ��������]�����邩
	int mAutoRemove;					//�A�j����n������Ə���
	unsigned int mGroup;


private:

	bool mGravity;							//�d�͉e����������
	bool mResistance;						//��R�̉e����������


	//���[���h���W update���ƂɌv�Z����Ȃ����B
	GameLib::Math::Matrix34 mWorld;
	BlendElement mBlendElement;
	bool mParentIsIdentityMatrix;
	GameLib::Math::Matrix34 mParentWorld;
	GameLib::Math::Vector3 mWorldPosition;	//����͗ǂ��g������Ƃ��Ă�����

	//�O�t���[���̐�΍��W
	GameLib::Math::Vector3 mWorldPositionprev;

public:
	Collision mCollision;


	Object();

	void create(Model*);
	void create(const Object*);
	void draw();
	double getX() const;
	double getY() const;
	double getZ() const;

	void setTexture(MyTexture* tex=0);
	void setModel(Model*, bool checkroot=true,bool checklock=true) ;
	void setModel(const char*, bool checkroot=true,bool checklock=true);	//�e�e�N�X�`�����猟������

	void setPosition (double a,double b,double c) ;
	void setPosition(const GameLib::Math::Vector3&);
	void addPosition(const GameLib::Math::Vector3&);

	void setVelocity(double a,double b,double c) ;
	void setVelocity(const GameLib::Math::Vector3&);

	void setVelocity2(double v,float hv,float vv);				//�����E�p�x�Ŏw�肷�鑬�x

	void setAccel(double x,double y,double z);
	void setAccel2(double v,double a);					//�����E�p�x�Ŏw�肷������x
	void setResistance(double a,double b,double c);
	void setResistance(double a);

	void setRotate (double a,double b,double c);
	void setRotateVelocity (float a,float b,float c);
	void setRotateVelocity (const GameLib::Math::Vector3&);
	void setRotateAccel (double a,double b,double c) ;
	void setRotateResistance (double a,double b,double c) ;
	void setRotateResistance (double k) ;
	void setRotateX(double);
	void setRotateY(double);
	void setRotateZ(double);

	void setAngle(const GameLib::Math::Vector3&);	//�x�N�^�Ō����w��BZ����������
	void aim(const Object* target);
	void aim(const GameLib::Math::Vector3&);
	void aimX(const GameLib::Math::Vector3&);		//Z���݂̂���]�����AX����������

	void go(float v,float a,const GameLib::Math::Vector3&);
	//���������_�Ƃ������W�n�ɕϊ�����@�d��
	void getMyMatrix(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& pos) const;
	//materials�K�p��̍��W�����[���h���W�ɕϊ�����
	void getWorldPositionFinal(GameLib::Math::Vector3* out) const;
	void getWorldPositionFinal(GameLib::Math::Vector3* out,GameLib::Math::Vector3 pos) const;
	void getMatrixFinal(GameLib::Math::Matrix34* out) const;
	void setSize (double x,double y,double z);
	void setSize (double s);
	void setSizeX (double x);
	void setSizeY (double y);
	void setSizeZ (double z);

	void setSizeVelocity (double x,double y,double z);
	void setSizeVelocity (double s);
	void setSizeAccel(double x,double y,double z);
	void setSizeResistance(double x,double y,double z);
	void setSizeResistance(double a);

	void setColor(double alpha,double red,double green,double blue);
	void setColor(unsigned int color);
	void setColor(double a,const GameLib::Math::Vector3&);
	void setColorVelocity(double alpha,double red,double green,double blue);
	void setColorAccel(double alpha,double red,double green,double blue);
	void setColorResistance(double alpha,double red,double green,double blue);

	void setReverse(bool x,bool y,bool z);
	void setCollision(unsigned int self,unsigned int target,unsigned int react,unsigned int priority=2);
	void setParent (Object* o,bool stable=false,bool followonlyposition = false);
	void leaveParent();	//�e����Ɨ�����

	void setFlash(int f,int t=0);	//����>����
	void setLimit(int limit);
	void setAnimeSpeed(float);
	void setAnimeCount(float);
	void setLight(float intensity,const GameLib::Math::Vector3& color);
	void setLight(float);
	void setFocus(bool b = true);

	void savePrevPosition();
	void launch();					//���[���h���W�����Ƃ߂�i�ǉ��E�X�V�E�\���̍ۏ���ɂ�т������j
	void remove();
	void updateCollision();
	bool getCollision(const Object*) const;

	void update(unsigned int flag=0);
	void stop();

	void getEnvironment(int gravity,bool water,float ax=0.f,float ay=0.f);
	Area::Block* getBlock();

	void getFocus()const;
	void getAngle(float *h,float *v,const Object* target)const;

	::BlendInfo blendInfo() const;

	//�֗��֐�
	void fadeAway(int time,float zoom=0.f);	//limit��t���āA���X�ɏ����Ă����悤�ɂ���
	void randomRotate(bool x=true,bool y= true, bool z= true);
	float getDistance(const Object&) const;
	float getSquareDistance(const Object&) const;

	//�A�N�Z�b�T
	int id()const;
	const GameLib::Math::Vector3* worldPosition() const;
	const GameLib::Math::Vector3* prevWorldPosition() const;

	void getRotate(GameLib::Math::Vector3*)const;
	void getVelocity(GameLib::Math::Vector3*)const;
	void getVelocity(double *vx,double *vy,double *vz) const;

	const Model* model() const;
	int animeCount() const;
	bool isEnable() const;
	Object* parent() const;
	bool isVisible() const;	//animeCount�͕�����Ȃ����HmVisible=true���H
	float collisionX(int i) const;
	float collisionY(int i) const;
};


//�I�u�W�F�N�g����N���X
class Sprite {
	//�������I�u�W�F�N�g�̃��X�g
	Object** mTransparentObject;
	int mTransparentObjectNumber1;
	int mTransparentObjectNumber2;

	unsigned int mGroup;

public:
	int mObjectMax;
	int mMax;							//�z��̈�ԑ傫���z�B���m�ł͂Ȃ��B�ǂ����Ōv��Ȃ��ƃp�t�H�[�}���X�������Ă����B

	Object* mObject;
	Area mArea;

	//�֐�
	static void create();				//�C���X�^���X�̏�����
	static void destroy();
	static void removeModel(Model*);						//model�����I�u�W�F�N�g���������ď���

	void resizeArea(int offsetX,int offsetY,int w,int h,int chipw,int chiph);			//�d�l����G���A�����T�C�Y

	Object* addObject(Model* model);
	Object* copyObject(Object*);
	void launchObject();
	void updateObject(unsigned int flag=0);
	void drawObject(bool zwriteon = true,bool zwriteoff=true);
	void removeObject(unsigned int group = 0);
	void setGroup(unsigned int groupnumber);

	//�֗��֐�
	void spot(float intensity,int time,const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& color=GameLib::Math::Vector3(1.f));

	void setVisible(bool ,unsigned int side=0);		//���葮���̃I�u�W�F�N�g���\���ɂ���
	void updateObjectCollision();					//�����蔻��ʒu���v�Z����
private:
	void getObjectCollision();
	void getObjectReact();
	void saveObjectPosition();						//�O�t���[���̏���ۑ�
public:
	static bool checkObjectCollision(GameLib::Math::Vector3* vct,const Object& i,const Object& j);
	static bool checkObjectCollisionArea(const Object&,const Area::Vector&);
	int  getObjectNumber();	

	Sprite(int objectmax);
	~Sprite();
private:
	static const int mInstanceMax = 4;
	static Sprite* mInstance[mInstanceMax];
	static Sprite* instance(int i);


	int getNewObject(int a=0);
};



#endif
