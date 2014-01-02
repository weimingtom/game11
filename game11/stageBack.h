#ifndef INCLUDED_GAME9SEQ_STAGE_H
#define INCLUDED_GAME9SEQ_STAGE_H

#include "npdraw.h"
#include "interface.h"
#include "unit.h"
#include "script.h"

class Stage;

//範囲計算をやってくれるクラス
class StageArea{
	int* mData;
	int mWidth,mHeight;
public:
	int mX,mY;			//中心になった座標をおぼえておこう
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
	void set(int v);				//全マスにvをセット
	void setEvent(int n);			//指定イベントポイントマスを選択
	int* data() const;

	//範囲計算
	void calc(RANGETYPE,int x,int y,int unitX,int unitY,int maxRange,int minRange,int up,int down,unsigned int interrupt);

	//地形コスト計算型
	//interrupt属性　1穴　2壁　4水　8プレイヤーユニット　16エネミーユニット
	void walk(int i,int j,int range,unsigned int interrupt,int up,int down);
	void walkC(int i,int j,int range,unsigned int interrupt,int up,int down);
	void walkX(int i,int j,int range,unsigned int interrupt,int up,int down);

	//純粋距離型
	void teleport(int i,int j,int r);
	void circle(int i,int j,int r);						//中心と半径を指定して円状の範囲をつくる
	void circle2(int i,int j,int r);						//中心と半径を指定して円状の範囲をつくる
	void square(int i,int j,int r);						//正方形の範囲
	void line(int i,int j,int p,int q);					//直線
	void fan(int i,int j,int p,int q,int size);			//扇状
	void xCross(int i,int j,int r,int top);						//X字型
	void cross(int i,int j,int r,int top);				//十字

	void except(unsigned int);							//1穴　2壁　4水　8プレイヤーユニット　16エネミーユニット　のあるマスを除外
	void setTallRange(int x,int y,int top,int bottom);	//起点からの高さ範囲
	void setMinRange(int x,int y,int range);
	void draw()const;
};


class SelectSquare{
public:

	int mX,mY;					//カーソル位置
	StageArea mSelectArea;		//選択範囲
	unsigned int mTarget;		//選ぶ対象マス　1何もいないマス　2ユニットがいるマス

	bool mMenu;					//メニュー出せるか？
	Unit* mPrimaryUnit;			//左下に表示するユニット

	SelectSquare();
	~SelectSquare();
	void create(const StageArea&,bool menu);
	void create(int x,int y,const StageArea&,bool menu);
	void draw(Stage* stage);
	int update(Stage* parent);		 //1決定　2キャンセル
};


class Square{
	static const int mHeight = 10;
public:
	static const int mSize = 40;

	int mX,mY,mZ;
	int mU,mV;
	int mEvent;

	//int mCost;		//進入にかかるコスト(基準はゼロ)
	bool mPit;			//穴
	bool mWall;			//壁
	bool mWater;		//水
	int mSlant;			//地形タイプ　傾き

	Batch* mBatch;
	int mTopPolygonIndex;
	int mTopPolygonNumber;

	Square();
	~Square();
	int vertexX(int)const;		//頂点の座標を得る
	int vertexY(int)const;		//頂点の座標を得る
	int vertexZ(int)const;		//頂点の座標を得る
	unsigned int type()const;				//地形タイプを得る　0ふつー　1穴　2壁　4水
	GameLib::Math::Vector3 normal()const;	//
	void draw(const Stage* stage,bool water)const;
	void drawTop(const Stage*,MyTexture*,const BlendInfo&)const;
	void createBatch(Stage*);
	int getX() const;
	int getY() const;
	int getZ() const;

};

//////////////////////////////////////////////////////////////////////////////

//ダメージ結果
class Damage{
public:
	static const int mComboMax = 32;
	int mCombo;							//連続攻撃回数
	int mResult;						//0関係なし　1ダメージ　2回復 3無効化

	bool mHit[mComboMax];				//ヒットしたか？
	int mDamage[mComboMax];				//ダメージ量

	//追加効果
	dState* mState;
	int mStateLevel;

	//飾り部分
	bool mCritical[mComboMax];			//クリティカルしたか？

	int mHitProbability;				//命中率
	int mCriticalProbability;			//クリティカル率
	int mTotalDamage;					//トータルダメージ予想

	Damage();
};
class CastResult{
public:
	Damage mDamage[256];//Stage::mMax;				//いいのかなあ
	float mPriority;
	Unit* mUnit;			//発動ユニット
	Unit::Ability* mAbility;//発動アビリティ
	bool mAvailable;		//発動許可するか？（対象がいないなどの場合不許可）

	CastResult();
	bool calc(Stage*,Unit::Ability&,const StageArea* sa = 0);		//誰も範囲にいなかったら、falseを返す sa=0で先行計算モード
	void draw(int x,int y,Unit*)const;		//結果予想表示
};
//////////////////////////////////////////////////////////////////////////////
//メイン
class Stage{

	//内部クラス
	//フェーズ
	class Phase {
	public:
		virtual ~Phase();
		virtual int update(Stage*) = 0;
	};
//////////////////////////////////////////////////////////////////////////////

	//ターン計算フェーズ
	class TurnReady : public Phase{
		int mSide;				//次ターンの陣営
	public:
		TurnReady(Stage*);
		int update(Stage*);
	};

//////////////////////////////////////////////////////////////////////////////
	//コントロールフェーズ
	class Control : public Phase{
		//キャラクター選択
		//コマンド選択
		//対象マス選択
		//アタックフェーズ

		//コマンド決定
		class Command{

			//座標決定
			class SelectTarget{
			public:
				SelectSquare mSelectSquare;
				StageArea mArea;				//効果範囲
				Unit::Ability* mAbility;
				CastResult mCastResult;

				SelectTarget(Stage*,Unit::Ability*);
				int update(Stage*);
				void draw(Stage*);
			};

			Unit* mUnit;
			UnitAbilityList mUnitAbilityList;

			SelectTarget* mSelectTarget;				//座標決定
		public:
			Command(Unit*);
			~Command();
			int update(Stage*);
		};

		SelectSquare* mSelectUnit;		//ユニット決定
		Command* mCommand;				//コマンド選択
	public:
		Control(const Stage*);
		~Control();
		int update(Stage*);

	private:
		SelectSquare* newSelectUnit(Stage*);
	};

//////////////////////////////////////////////////////////////////////////////

	//COMフェーズ
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
//ユニット消去フェーズ
	class KillUnit{
		int mCount;
	public:
		KillUnit(Stage*);
		int update(Stage*);
	};
//////////////////////////////////////////////////////////////////////////////
//パーティ展開フェーズ
	class Deploy {
		int mNumber;		//いるのか？

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
	//アビリティ発動フェーズ
public:
	class CastAbility{
		//戦闘アニメ・効果の基底クラス
	public:
		class Cast{
			Unit* mUnit;					//発動するユニット
			Unit::Ability* mAbility;		//使うアビリティ
			int mX,mY;						//対象マス
			CastResult mCastResult;			//その結果
		public:
			virtual int update(Stage*) = 0;					//1.連携重ねてもいいよ	2.おしまい。殺せ。
			bool create(Unit*,Unit::Ability*,int i,int j);	//発動不可だったらfalseを返す
			void apply(Stage*);								//実際に効果を適用する
		};

		static const int mCastMax = 8;
		Cast* mCast[mCastMax];				//最大8連携
		int mCurrentCast;
		Stage* mStage;
		Unit* mUnit;						//起点となるユニット

		CastAbility(Stage*);
		~CastAbility();
		bool create(Unit*,Unit::Ability*,int i,int j);	//発動不許可ならfalseが帰ってくるように
		int update(Stage*);
		void end(Stage*);						//終了処理
		Cast* newCastClass(int ID);
	};
//////////////////////////////////////////////////////////////////////////////

	//システムメニュー
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


	//戦闘終了
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
		int mEnergy;		//霊力
		int mCapacity;		//容量
		int mChargeSpeed;	//チャージ速度

		Party();
		void update();
		void draw()const;
		int energy()const;	//呪力数
		void increaseEnergy(int i);
	};
	class Weather {
		int mCurrent;
		int mBase;				//基本天候。カウントゼロですべてこれに戻る
		int mCount;				//残り時間
	public:
		Weather();
		void draw()const;
		void update();			//カウントを進める
		int get()const;			//現在の天候を得る
		void change(int i);			//天候変化
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
	MyTexture mTest;	//光源チェック

	GameData* mGameData;
	MyString mTitle;			//タイトル
	bool mScriptRunning;
	Script* mScript;			//実行中のスクリプト
	int mCount;					//ターンカウント
	int mResult;				//1終了　-1敗北　値によってエフェクトつけたりしよう。
	Bonus mBonus;				//ボーナス
	int mLevel;					//難易度0-3

	//範囲計算時の高速化用。
	 StageArea mTemp;

	//スタティックオブジェクト
	MyTexture mMapChip;
	Batch* mMap;		//地形バッチ
	MyTexture mWaterChip;
	Batch* mWater;

	Square* mSquare;

	int mWidth,mHeight;

	//状態
	SystemMenu* mSystemMenu;
	Phase* mPhase;
	Phase* mNextPhase;	
	CastAbility* mCastPhase;			//発動
	UnitStatusPhase* mUnitStatusPhase;	//ステータス確認
	KillUnit* mKillUnitPhase;			//倒したユニットが消滅する
	Deploy* mDeployPhase;				//パーティ展開フェーズ
	Over* mOverPhase;

	//ダイナミックオブジェクト
	static const int mUnitMax = 256;
	Unit* mUnit[mUnitMax];
	static const int mGhostMax = 64;
	Ghost* mGhost[mGhostMax];			//敵などの、一時ゴースト用

	//戦闘パラメータ
	Party mParty[2];
	Weather mWeather;
	StageArea mLuckZOC;						//幸運ZOC
	StageArea mDisturbZOC;					//威圧ZOC
	Ability* mLastSpell;					//想起用にスペルおぼえておく

	//以下関数
	Stage(GameData*);
	~Stage();
	void set(Script::Battle*);
	void loadFMF(const char* file);			//FMFをロード
	void createBatch();						//バッチ作成
	int update();					//1勝利 2敗北
	void refresh();							//パラメータ再計算
	void addDrop(int score,int power,dTool* drop);

	//ユニット・ゴースト制御
	Unit* launchUnit(Ghost*,int pos,int party,bool control,int wait);
	Unit* launchUnit(Ghost*,int x,int y,int party,bool control,int wait);
	Unit* createUnit(dShell*,int level,int pos,int party,bool control,int wait,int abilityMode,int rank,int difficulty,dTool* drop,int dropProbability);
	int getNewUnit() const;
	int getNewGhost() const;
	Ghost* pick(dShell*);		//ゲストゴーストをパーティに加えるときにつかう
	void removeUnit(int i);		//ユニットを消去
	void removeUnit(Unit*);

	//フェーズ制御
	void deploy(int area,int number);
	
	//地形制御
	Square* square(int i,int j) const;
	Square* square(int n)const;

	//グラフィック
	void drawArea() const;
	void drawArea1() const;
	void drawArea2() const;			//水面等、半透明オブジェクト描画
	void drawChara() const;
	void drawFrontEnd() const;
	void drawEnergy() const;
	void drawQueue() const;				//ターン順を表示
	void drawSquareTop(MyTexture*,int i,int j,BlendInfo) const;
	void drawSquareInfo(int x,int y,int i,int j) const;
	void createNumber(float x,float y,float z,unsigned int number,unsigned int color) const;
	void getFocus(int i,int j)const;

	//情報
	Unit* unit(int i,int j) const;
	void getSquare(int* i,int* j,int x,int y)const;	//座標からマスを得る
	void fixCamera() const;					
	void checkUnitKilled();					//死んだユニットを消す
	Unit* getQueue(int i)const;					//行動順を得る
};

#endif
