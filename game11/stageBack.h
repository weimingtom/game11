#ifndef INCLUDED_GAME9SEQ_STAGE_H
#define INCLUDED_GAME9SEQ_STAGE_H

#include "npdraw.h"
#include "interface.h"
#include "unit.h"
#include "script.h"

class Stage;

//�͈͌v�Z������Ă����N���X
class StageArea{
	int* mData;
	int mWidth,mHeight;
public:
	int mX,mY;			//���S�ɂȂ������W�����ڂ��Ă�����
	Stage* mStage;

	unsigned int mColor;

	StageArea();
	~StageArea();
	void create(Stage*);
	void release();
	void operator +=(const StageArea&);
	void operator -=(const StageArea&);
	void operator=(const StageArea&);

	int get(int i,int j) const;
	void set(int i,int j,int v);
	void set(int v);				//�S�}�X��v���Z�b�g
	void setEvent(int n);			//�w��C�x���g�|�C���g�}�X��I��
	int* data() const;

	//�͈͌v�Z
	void calc(RANGETYPE,int x,int y,int unitX,int unitY,int maxRange,int minRange,int up,int down,unsigned int interrupt);

	//�n�`�R�X�g�v�Z�^
	//interrupt�����@1���@2�ǁ@4���@8�v���C���[���j�b�g�@16�G�l�~�[���j�b�g
	void walk(int i,int j,int range,unsigned int interrupt,int up,int down);
	void walkC(int i,int j,int range,unsigned int interrupt,int up,int down);
	void walkX(int i,int j,int range,unsigned int interrupt,int up,int down);

	//���������^
	void teleport(int i,int j,int r);
	void circle(int i,int j,int r);						//���S�Ɣ��a���w�肵�ĉ~��͈̔͂�����
	void circle2(int i,int j,int r);						//���S�Ɣ��a���w�肵�ĉ~��͈̔͂�����
	void square(int i,int j,int r);						//�����`�͈̔�
	void line(int i,int j,int p,int q);					//����
	void fan(int i,int j,int p,int q,int size);			//���
	void xCross(int i,int j,int r,int top);						//X���^
	void cross(int i,int j,int r,int top);				//�\��

	void except(unsigned int);							//1���@2�ǁ@4���@8�v���C���[���j�b�g�@16�G�l�~�[���j�b�g�@�̂���}�X�����O
	void setTallRange(int x,int y,int top,int bottom);	//�N�_����̍����͈�
	void setMinRange(int x,int y,int range);
	void draw()const;
};


class SelectSquare{
public:

	int mX,mY;					//�J�[�\���ʒu
	StageArea mSelectArea;		//�I��͈�
	unsigned int mTarget;		//�I�ԑΏۃ}�X�@1�������Ȃ��}�X�@2���j�b�g������}�X

	bool mMenu;					//���j���[�o���邩�H
	Unit* mPrimaryUnit;			//�����ɕ\�����郆�j�b�g

	SelectSquare();
	~SelectSquare();
	void create(const StageArea&,bool menu);
	void create(int x,int y,const StageArea&,bool menu);
	void draw(Stage* stage);
	int update(Stage* parent);		 //1����@2�L�����Z��
};


class Square{
	static const int mHeight = 10;
public:
	static const int mSize = 40;

	int mX,mY,mZ;
	int mU,mV;
	int mEvent;

	//int mCost;		//�i���ɂ�����R�X�g(��̓[��)
	bool mPit;			//��
	bool mWall;			//��
	bool mWater;		//��
	int mSlant;			//�n�`�^�C�v�@�X��

	Batch* mBatch;
	int mTopPolygonIndex;
	int mTopPolygonNumber;

	Square();
	~Square();
	int vertexX(int)const;		//���_�̍��W�𓾂�
	int vertexY(int)const;		//���_�̍��W�𓾂�
	int vertexZ(int)const;		//���_�̍��W�𓾂�
	unsigned int type()const;				//�n�`�^�C�v�𓾂�@0�ӂ[�@1���@2�ǁ@4��
	GameLib::Math::Vector3 normal()const;	//
	void draw(const Stage* stage,bool water)const;
	void drawTop(const Stage*,MyTexture*,const BlendInfo&)const;
	void createBatch(Stage*);
	int getX() const;
	int getY() const;
	int getZ() const;

};

//////////////////////////////////////////////////////////////////////////////

//�_���[�W����
class Damage{
public:
	static const int mComboMax = 32;
	int mCombo;							//�A���U����
	int mResult;						//0�֌W�Ȃ��@1�_���[�W�@2�� 3������

	bool mHit[mComboMax];				//�q�b�g�������H
	int mDamage[mComboMax];				//�_���[�W��

	//�ǉ�����
	dState* mState;
	int mStateLevel;

	//���蕔��
	bool mCritical[mComboMax];			//�N���e�B�J���������H

	int mHitProbability;				//������
	int mCriticalProbability;			//�N���e�B�J����
	int mTotalDamage;					//�g�[�^���_���[�W�\�z

	Damage();
};
class CastResult{
public:
	Damage mDamage[256];//Stage::mMax;				//�����̂��Ȃ�
	float mPriority;
	Unit* mUnit;			//�������j�b�g
	Unit::Ability* mAbility;//�����A�r���e�B
	bool mAvailable;		//���������邩�H�i�Ώۂ����Ȃ��Ȃǂ̏ꍇ�s���j

	CastResult();
	bool calc(Stage*,Unit::Ability&,const StageArea* sa = 0);		//�N���͈͂ɂ��Ȃ�������Afalse��Ԃ� sa=0�Ő�s�v�Z���[�h
	void draw(int x,int y,Unit*)const;		//���ʗ\�z�\��
};
//////////////////////////////////////////////////////////////////////////////
//���C��
class Stage{

	//�����N���X
	//�t�F�[�Y
	class Phase {
	public:
		virtual ~Phase();
		virtual int update(Stage*) = 0;
	};
//////////////////////////////////////////////////////////////////////////////

	//�^�[���v�Z�t�F�[�Y
	class TurnReady : public Phase{
		int mSide;				//���^�[���̐w�c
	public:
		TurnReady(Stage*);
		int update(Stage*);
	};

//////////////////////////////////////////////////////////////////////////////
	//�R���g���[���t�F�[�Y
	class Control : public Phase{
		//�L�����N�^�[�I��
		//�R�}���h�I��
		//�Ώۃ}�X�I��
		//�A�^�b�N�t�F�[�Y

		//�R�}���h����
		class Command{

			//���W����
			class SelectTarget{
			public:
				SelectSquare mSelectSquare;
				StageArea mArea;				//���ʔ͈�
				Unit::Ability* mAbility;
				CastResult mCastResult;

				SelectTarget(Stage*,Unit::Ability*);
				int update(Stage*);
				void draw(Stage*);
			};

			Unit* mUnit;
			UnitAbilityList mUnitAbilityList;

			SelectTarget* mSelectTarget;				//���W����
		public:
			Command(Unit*);
			~Command();
			int update(Stage*);
		};

		SelectSquare* mSelectUnit;		//���j�b�g����
		Command* mCommand;				//�R�}���h�I��
	public:
		Control(const Stage*);
		~Control();
		int update(Stage*);

	private:
		SelectSquare* newSelectUnit(Stage*);
	};

//////////////////////////////////////////////////////////////////////////////

	//COM�t�F�[�Y
	class CastAbility;
	class Computer : public Phase{
		Unit* mCurrentUnit;
		CastAbility* mCast;
		StageArea mRange;
		StageArea mArea;
		int mCount;
	public:
		Computer(Stage*);
		int update(Stage*);
		static float calc(Unit* unit,int*ab=0,int* x=0, int* y=0);
	};
//////////////////////////////////////////////////////////////////////////////
//���j�b�g�����t�F�[�Y
	class KillUnit{
		int mCount;
	public:
		KillUnit(Stage*);
		int update(Stage*);
	};
//////////////////////////////////////////////////////////////////////////////
//�p�[�e�B�W�J�t�F�[�Y
	class Deploy {
		int mNumber;		//����̂��H

		int mPhase;
		StageArea mArea;
		SelectSquare mSelectSquare;
		GhostListWindow mGhostListWindow;
		Confirm mConfirm;
	public:
		Deploy(Stage*,int area,int number);
		int update(Stage*);
	};
//////////////////////////////////////////////////////////////////////////////
	//�A�r���e�B�����t�F�[�Y
public:
	class CastAbility{
		//�퓬�A�j���E���ʂ̊��N���X
	public:
		class Cast{
			Unit* mUnit;					//�������郆�j�b�g
			Unit::Ability* mAbility;		//�g���A�r���e�B
			int mX,mY;						//�Ώۃ}�X
			CastResult mCastResult;			//���̌���
		public:
			virtual int update(Stage*) = 0;					//1.�A�g�d�˂Ă�������	2.�����܂��B�E���B
			bool create(Unit*,Unit::Ability*,int i,int j);	//�����s��������false��Ԃ�
			void apply(Stage*);								//���ۂɌ��ʂ�K�p����
		};

		static const int mCastMax = 8;
		Cast* mCast[mCastMax];				//�ő�8�A�g
		int mCurrentCast;
		Stage* mStage;
		Unit* mUnit;						//�N�_�ƂȂ郆�j�b�g

		CastAbility(Stage*);
		~CastAbility();
		bool create(Unit*,Unit::Ability*,int i,int j);	//�����s���Ȃ�false���A���Ă���悤��
		int update(Stage*);
		void end(Stage*);						//�I������
		Cast* newCastClass(int ID);
	};
//////////////////////////////////////////////////////////////////////////////

	//�V�X�e�����j���[
	class SystemMenu{
	public:
		sConfig* mConfig;
		Window mWindow;
		Window mUnitList;
		int mPhase;

		SystemMenu(const Stage*);
		~SystemMenu();
		int update(const Stage*);
	};
//////////////////////////////////////////////////////////////////////////////


	//�퓬�I��
	class Over {
		int mCount;
		LearningWindow mLearningWindow;
	public:
		int mResult;
		Over(int result,GameData*);
		int update(Stage*);
	};

//////////////////////////////////////////////////////////////////////////////
	class Party{
	public:
		int mEnergy;		//���
		int mCapacity;		//�e��
		int mChargeSpeed;	//�`���[�W���x

		Party();
		void update();
		void draw()const;
		int energy()const;	//���͐�
		void increaseEnergy(int i);
	};
	class Weather {
		int mCurrent;
		int mBase;				//��{�V��B�J�E���g�[���ł��ׂĂ���ɖ߂�
		int mCount;				//�c�莞��
	public:
		Weather();
		void draw()const;
		void update();			//�J�E���g��i�߂�
		int get()const;			//���݂̓V��𓾂�
		void change(int i);			//�V��ω�
	};
	class Bonus {
	public:
		dTool* mTool[4];
		int mScore;
		int mPower;

		Bonus();
		void bag(GameData*);
	};
//////////////////////////////////////////////////////////////////////////////
public:
	MyTexture mTest;	//�����`�F�b�N

	GameData* mGameData;
	MyString mTitle;			//�^�C�g��
	bool mScriptRunning;
	Script* mScript;			//���s���̃X�N���v�g
	int mCount;					//�^�[���J�E���g
	int mResult;				//1�I���@-1�s�k�@�l�ɂ���ăG�t�F�N�g�����肵�悤�B
	Bonus mBonus;				//�{�[�i�X
	int mLevel;					//��Փx0-3

	//�͈͌v�Z���̍������p�B
	 StageArea mTemp;

	//�X�^�e�B�b�N�I�u�W�F�N�g
	MyTexture mMapChip;
	Batch* mMap;		//�n�`�o�b�`
	MyTexture mWaterChip;
	Batch* mWater;

	Square* mSquare;

	int mWidth,mHeight;

	//���
	SystemMenu* mSystemMenu;
	Phase* mPhase;
	Phase* mNextPhase;	
	CastAbility* mCastPhase;			//����
	UnitStatusPhase* mUnitStatusPhase;	//�X�e�[�^�X�m�F
	KillUnit* mKillUnitPhase;			//�|�������j�b�g�����ł���
	Deploy* mDeployPhase;				//�p�[�e�B�W�J�t�F�[�Y
	Over* mOverPhase;

	//�_�C�i�~�b�N�I�u�W�F�N�g
	static const int mUnitMax = 256;
	Unit* mUnit[mUnitMax];
	static const int mGhostMax = 64;
	Ghost* mGhost[mGhostMax];			//�G�Ȃǂ́A�ꎞ�S�[�X�g�p

	//�퓬�p�����[�^
	Party mParty[2];
	Weather mWeather;
	StageArea mLuckZOC;						//�K�^ZOC
	StageArea mDisturbZOC;					//�Ј�ZOC
	Ability* mLastSpell;					//�z�N�p�ɃX�y�����ڂ��Ă���

	//�ȉ��֐�
	Stage(GameData*);
	~Stage();
	void set(Script::Battle*);
	void loadFMF(const char* file);			//FMF�����[�h
	void createBatch();						//�o�b�`�쐬
	int update();					//1���� 2�s�k
	void refresh();							//�p�����[�^�Čv�Z
	void addDrop(int score,int power,dTool* drop);

	//���j�b�g�E�S�[�X�g����
	Unit* launchUnit(Ghost*,int pos,int party,bool control,int wait);
	Unit* launchUnit(Ghost*,int x,int y,int party,bool control,int wait);
	Unit* createUnit(dShell*,int level,int pos,int party,bool control,int wait,int abilityMode,int rank,int difficulty,dTool* drop,int dropProbability);
	int getNewUnit() const;
	int getNewGhost() const;
	Ghost* pick(dShell*);		//�Q�X�g�S�[�X�g���p�[�e�B�ɉ�����Ƃ��ɂ���
	void removeUnit(int i);		//���j�b�g������
	void removeUnit(Unit*);

	//�t�F�[�Y����
	void deploy(int area,int number);
	
	//�n�`����
	Square* square(int i,int j) const;
	Square* square(int n)const;

	//�O���t�B�b�N
	void drawArea() const;
	void drawArea1() const;
	void drawArea2() const;			//���ʓ��A�������I�u�W�F�N�g�`��
	void drawChara() const;
	void drawFrontEnd() const;
	void drawEnergy() const;
	void drawQueue() const;				//�^�[������\��
	void drawSquareTop(MyTexture*,int i,int j,BlendInfo) const;
	void drawSquareInfo(int x,int y,int i,int j) const;
	void createNumber(float x,float y,float z,unsigned int number,unsigned int color) const;
	void getFocus(int i,int j)const;

	//���
	Unit* unit(int i,int j) const;
	void getSquare(int* i,int* j,int x,int y)const;	//���W����}�X�𓾂�
	void fixCamera() const;					
	void checkUnitKilled();					//���񂾃��j�b�g������
	Unit* getQueue(int i)const;					//�s�����𓾂�
};

#endif
