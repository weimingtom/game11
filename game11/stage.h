#ifndef INCLUDED_STAGE_H
#define INCLUDED_STAGE_H


#include "npobject.h"

class GameData;
class Stage;

class Chara{
	Object* mObject;
	int mObjectID;
protected:
	unsigned int mSilhouetteColor;
	int mSilhouetteWidth;
	int mSilhouetteHeight;
public:

	Chara();
	virtual ~Chara();
	virtual int update(Stage*) = 0;
	virtual void collision(Stage*) = 0;
	void drawRadar(Stage*);
	
	Object* object()const;
	Object* setObject(Stage*,Model*);
	void removeObject();
	bool lost() const;
	GameLib::Math::Vector3 position();
};

//当たり判定属性
//1 プレイヤー
//2 敵
//4 プレイヤー弾
//8 敵弾
//16　爆風
//32　アイテム

class Player : public Chara{
public:
	Player(Stage*);
	int update(Stage*);
	void collision(Stage*);
};

class Enemy : public Chara{
protected:
	int mScore;
public:
	Enemy();
	virtual ~Enemy();
	void collision(Stage*);
	virtual bool kill(Stage*)=0;
	virtual void drop(Stage*)=0;
};
class Zako : public Enemy {
	int mAttackPoint;
public:
	Zako();
	int update(Stage*);
	void attack(Stage*);
	virtual void shot(Stage*) = 0;
	bool kill(Stage*);
};

class Reimu : public Zako{
public:
	Reimu(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Marisa : public Zako{
public:
	Marisa(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Sanae : public Zako{
	int mBullet;
public:
	Sanae(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Yousei : public Zako{
public:
	Yousei(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};

class Tenko : public Zako{
public:
	Tenko(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Iku : public Zako{
	int mBullet;
public:
	Iku(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Toyohime : public Zako{
public:
	Toyohime(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};
class Yorihime : public Zako{
	int mBullet;
public:
	Yorihime(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};


class UFO : public Zako{
public:
	UFO(Stage*);
	void shot(Stage*);
	void drop(Stage*);
};

class Hisoutensoku : public Enemy{
	class Move{
		GameLib::Math::Vector3 mStart;
		GameLib::Math::Vector3 mEnd;
		int mCount;
		Hisoutensoku* mParent;
	public:
		Move(Hisoutensoku*,const GameLib::Math::Vector3& end);
		int update(Stage* stage);
	};

	Move* mMove;
	static const int mMaxHP = 8;
	int mHP;
	int mCount;
	int mPhase;
public:
	Hisoutensoku(Stage*);
	~Hisoutensoku();
	int update(Stage*);
	bool kill(Stage*);
	void drop(Stage*);
};

class Shot : public Chara{
	bool mBomb;
public:

	Shot(Stage*,const GameLib::Math::Vector3& pos,bool bomb,Model* model = 0);
	int update(Stage*);
	void collision(Stage*);
};


class EnemyShot : public Chara{
public:
	EnemyShot(Stage*,const GameLib::Math::Vector3& pos,Model* model);
	int update(Stage*);
	void collision(Stage*);
};

class Onbasira : public Chara{
	int mCount;
	float mAngle;
public:
	Onbasira(Stage*,const GameLib::Math::Vector3& pos);
	int update(Stage*);
	void collision(Stage*);
};


class Item : public Chara{
	bool mEnd;
protected:
public:
	Item();
	void create(Stage*,const GameLib::Math::Vector3& pos,const char* name);
	int update(Stage*);
	void collision(Stage*);
	virtual void get(Stage*) = 0;
};
class OneUp : public Item{
public:
	OneUp(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};
class ShotNumber : public Item{
public:
	ShotNumber(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};
class SpeedUp : public Item{
public:
	SpeedUp(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};
class FullPower : public Item{
public:
	FullPower(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};
class Wave : public Item{
public:
	Wave(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};

class ShotSpeed : public Item{
public:
	ShotSpeed(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};

class Flash : public Item{
public:
	Flash(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};

class FourShot : public Item{
public:
	FourShot(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};

class Laser : public Item{
public:
	Laser(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};

class Bomb : public Item{
public:
	Bomb(Stage*,const GameLib::Math::Vector3& pos);
	void get(Stage*);
};


class Stage{
	class Phase{
	public:
		virtual ~Phase();
		virtual int update(Stage*) = 0;
	};
	class Ready : public Phase{
		int mCount;
	public:
		Ready();
		int update(Stage*);
	};
	class GameOver : public Phase{
		MyTexture mTexture;
		int mCount;
	public:
		GameOver();
		int update(Stage*);
	};
	class StageClear : public Phase{
		int mCount;
	public:
		StageClear();
		int update(Stage*);
	};
	class Destroy : public Phase{
		int mCount;
		Object* mObject;
	public:
		Destroy(Object*);
		int update(Stage*);
	};
	class CastSpell{
		int mCount;
		MyString mName;
		Model* mModel;
	public:
		CastSpell(Model*,const char*,int time);
		int update(Stage*);
	};


	class PlayerState{
	public:
		bool mWave;
		int mShot;
		int mSpeed;
		int mShotSpeed;

		int mSpecialLimit;
		int mSpecial;		//1四連　2スパーク　3ボム

		void initialize();
		float difficulty();	//0-10で帰ってくる
		float shotSpeed();
	};
	class Telop{
		int mCount;
		Model* mModel;
		MyTexture mTexture;
	public:
		Telop();
		void update();
		void set(const char*);
	};


	MyTexture mPanel;
	MyTexture mRadar;
	MyTexture mSilhouette;
	MyTexture mPichun;
	MyTexture mMasterSpark;
	MyTexture mBomber;
	MyTexture mAAG;
	MyTexture mSnipeShot;
	MyTexture m2WayShot;
	MyTexture mTensokuShot;

	GameData* mGameData;

	List<Chara*> mChara;

	Phase* mPhase;
	CastSpell* mCastSpell;

	bool mMiss;
	int mMissCount;
	int mScore;
	int mEnemyCount;	//出現カウント6
	bool mGameClear;
public:
	MyTexture mLaser;
	MyTexture mCutIn;
	MyTexture mCutInNue;
	SoundSet mSound;

	static const int mSpecialLimit = 600;
	int mLeft;
	Sprite mSprite;
	int mShotCount;
	int mRestShot;
	PlayerState mPlayer;
	bool mFlash;
	bool mFlashOrder;
	int mFlashEffect;
	GameLib::Math::Vector3 mPlayerPosition;

	//アイテムフラグ
	int mReimuCount;
	int mMarisaCount;
	int mSanaeCount;
	static const int mAttackTime = 128;
	int mAttackCount;	//敵攻撃カウント

	int mAppearCount;
	int mArea;	//0海底 1海上

	GameLib::Math::Vector3 mScroll;	//背景画像位置
	GameLib::Math::Vector3 mScrollOrder;

	Telop mTelop;

	Stage(GameData*);
	~Stage();
	void release();
	void ready();
	int update(GameData*);
	void draw();

	void enemyAppear();//敵を出現させるパターンが要る
	Player* createPlayer(const GameLib::Math::Vector3& pos);

	void createShot(const GameLib::Math::Vector3& pos);

	void createBomb(const GameLib::Math::Vector3& pos);
	void createFourShot(const GameLib::Math::Vector3& pos);
	void createMasterSpark(const GameLib::Math::Vector3& pos);
	void createBomber(const GameLib::Math::Vector3& pos);
	void createAAG(const GameLib::Math::Vector3& pos);
	void createCanon(const GameLib::Math::Vector3& pos);

	Enemy* createReimu(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createMarisa(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createSanae(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createYousei(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createUFO(const GameLib::Math::Vector3& pos,bool reverse);

	Enemy* createTenko(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createToyohime(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createYorihime(const GameLib::Math::Vector3& pos,bool reverse);
	Enemy* createIku(const GameLib::Math::Vector3& pos,bool reverse);

	Enemy* createHisoutensoku();

	void createEnemy(Enemy*,const GameLib::Math::Vector3& pos,bool reverse);

	void createEnemyShot(const GameLib::Math::Vector3& pos);
	void createEnemyLaser(const GameLib::Math::Vector3& pos);
	void createEnemyOnbasira(const GameLib::Math::Vector3& pos);
	void createEnemyKanameisi(const GameLib::Math::Vector3& pos);
	void createEnemyFan(const GameLib::Math::Vector3& pos);
	void createEnemySnipe(const GameLib::Math::Vector3& pos);
	void createEnemy2way(const GameLib::Math::Vector3& pos);
	void createEnemyTensokuA(const GameLib::Math::Vector3& pos);
	void createEnemyTensokuB(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& target);
	void createEnemyTensokuC(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& target);

	void createItemOneUp(const GameLib::Math::Vector3& pos);
	void createItemShot(const GameLib::Math::Vector3& pos);
	void createItemSpeed(const GameLib::Math::Vector3& pos);
	void createItemWave(const GameLib::Math::Vector3& pos);
	void createItemShotSpeed(const GameLib::Math::Vector3& pos);
	void createItemFullPower(const GameLib::Math::Vector3& pos);
	void createItemFlash(const GameLib::Math::Vector3& pos);
	void createItemFourShot(const GameLib::Math::Vector3& pos);
	void createItemLaser(const GameLib::Math::Vector3& pos);
	void createItemBomb(const GameLib::Math::Vector3& pos);

	void createExplode(const GameLib::Math::Vector3& pos);
	void createBossExplode(const GameLib::Math::Vector3& pos);
	void createPichun(const GameLib::Math::Vector3& pos);
	void drawRadar(const GameLib::Math::Vector3& pos,int w,int h,unsigned int color);
	void miss();
	void stageClear();
	void gameClear();
	void destroy(Object*);
	void castSpell(const char* name);
	void castNue();
	void getScore(int);
	void flush();	//プレイヤー以外を消す
	void flushBullet();	//アイテムと弾を消す
	void setCamera();
	void startStage();
	void randomDrop(const GameLib::Math::Vector3& pos);
};


#endif

