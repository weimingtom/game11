#ifndef INCLUDED_NPOBJECT_H
#define INCLUDED_NPOBJECT_H

#include "npsysc.h"

#include "npmath.h"
#include "npdraw.h"

//オブジェクト制御



//地形データ
//ベクタとブロックで構成される。
//ベクタはともかくブロックはマップサイズさえかわらなければnewする必要はない。
class Object;
class Area {
public:
	class Block {
	public:
		//重力・風・空気抵抗
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
		unsigned int mType;			//特殊地形のタイプ
		unsigned int mPriority;		//当たり判定優先度
		Environment mEnvironment;	//重力など
		bool mSeal;					//ゴースト出現制限

		int mParam;
		int mLimit;					//特殊地形消滅カウント
		bool mVanish;				//特殊地形消滅オーダー

		Block();
		void reset();
		void setLimit(int f);
		bool side(int);
	};

	class Vector {
	public:
		Math::Segment mSegment;
		Block* mParent;			//所属するマス;
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
		//箱のサイズ
		static const int mWidth=64;							//分割ピクセル　箱の数＝マップの横の長さ/分割ピクセル
		static const int mHeight=64;

		List<const Vector*> mVector;
		Object mObject;

		void collision() const;	//箱内で総当たり処理
	};

private:
	Block* mBlock;			//block関数でアクセスせよ
	Container* mContainer;	//container関数でアクセスせよ
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
	void split();																			//地形の均等分割

	Block* block(unsigned int i,unsigned int j)const ;
	Container* container(unsigned int i,unsigned int j)const;
	void removeBlock(unsigned int x,unsigned int y);		//その後splitすること
	void removeBlock(Block*);								//その後splitすること
	void createBlock();
	void createSide(unsigned int i,unsigned int j,int side);

	void flow(int x1,int y1,int x2,int y2,float ax,float ay);					//blockのenvironmentを範囲で設定
	void water(int x1,int y1,int x2,int y2);
	void gravity(int x1,int y1,int x2,int y2,float g);
	int getSplitAreaX(float x);	//Ｘ座標から、分割地形の位置を得る
	int getSplitAreaY(float y);	//Y座標から、分割地形の位置を得る

};


//オブジェクトデータ
class Sprite;
class Object {
public:
	//当たり判定を処理するクラス
	class Collision{
	public:
		//衝突応答を処理するくらす
		class React{
			static const int mMax = 16;
			Math::Segment mSegment[mMax];
			GameLib::Math::Vector3 mNormal;

			int mNumber;
			GameLib::Math::Vector3 mPrimarySegment;			//当たったベクタのうち代表
			GameLib::Math::Vector3 mOffset;					//衝突応答補正値

		public:
			void initialize();
			void add(const Math::Segment&);
			void react(const Math::Polygon&);			//衝突応答処理
			const GameLib::Math::Vector3* offset()const;
		};


	private:
		//当たったオブジェクトのリスト
		static const int mMax = 8;
		Object* mObject[mMax];						//AreaVectorかObjectが入るわけだが。
		const Area::Vector* mArea[mMax];
	public:

		//当たり判定位置。計算されなおす。
		Math::Polygon mPolygon;

		unsigned int mFlag;			//当たり判定フラグ。
		unsigned int mFlagTarget;		//当たり判定を行う対象のフラグ。
		unsigned int mReactFlag;		//衝突応答するフラグ
		unsigned int mPriority;		//優先度0-2　0が高い

		React mReact;
		bool mAvailable;

		void reset();	//当たり判定結果を忘れる
		void initialize();
		void addObject(Object*,bool react,const GameLib::Math::Vector3&);			//衝突オブジェクトリストに追加する
		void addArea(const Area::Vector*,bool react,const GameLib::Math::Vector3&);	//衝突オブジェクトリストに追加する
		bool seek(const Object&) const;	//衝突リストに居る？
		int objectNumber()const;		//衝突数

	};


private:
	int mFlash;						//フラッシュ周期
	int mFlashTime;					//フラッシュする時間(<周期)
	int mFlashFrame;				//フラッシュ開始フレーム
	GameLib::Math::Vector3 mFlashEmission;				//フラッシュ時のブレンド　赤点滅をデフォにしておく

//public:

	//オブジェクト用のブレンド情報　ようするに余りものだ
	class BlendInfo{
	public:
		int mBlendMode;
		GameLib::Math::Vector3 mEmission;
		bool mLighting[4];

		void reset();
		void write(::BlendInfo*)const;
	};

	static int mNumber;					//IDの生成に使用。総生産数
public:
	Sprite* mSprite;
	int mIndex;							//自分の添え字
	bool mUpdated;
private:
	int mID;							//生成されるたびにつけられる番号
	bool mAvailable;					//存在する

	bool m3D;							//3Dモード。3Dモデルの当たり判定は回転に関係なくなる

	int mLimit;							//自動消滅までの残り時間
	unsigned int mCount;				//updateした回数

	Object* mParent;					//ひとつ上のオブジェクト
	Object* mRoot;						//一番上の親オブジェクト

	bool mStable;						//親が死んだら独立する
	bool mFollowOnlyPosition;			//親からもらうのは座標のみ

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

	//光源
	bool mLight;
	float mLightIntensity;
	GameLib::Math::Vector3 mLightColor;

	//注視点になるか？
	bool mFocus;

public:
	bool mVisible;
	BlendInfo mBlendInfo;				//紛らわしいが、drawで使うものとは別のクラス

	bool mSortReverse;					//Zバッファ書き込みしにないモードで、添え字順にかくか反転させるか
	int mAutoRemove;					//アニメがn周すると消滅
	unsigned int mGroup;


private:

	bool mGravity;							//重力影響をうける
	bool mResistance;						//抵抗の影響をうける


	//ワールド座標 updateごとに計算されなおす。
	GameLib::Math::Matrix34 mWorld;
	BlendElement mBlendElement;
	bool mParentIsIdentityMatrix;
	GameLib::Math::Matrix34 mParentWorld;
	GameLib::Math::Vector3 mWorldPosition;	//これは良く使うからとっておこう

	//前フレームの絶対座標
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
	void setModel(const char*, bool checkroot=true,bool checklock=true);	//親テクスチャから検索する

	void setPosition (double a,double b,double c) ;
	void setPosition(const GameLib::Math::Vector3&);
	void addPosition(const GameLib::Math::Vector3&);

	void setVelocity(double a,double b,double c) ;
	void setVelocity(const GameLib::Math::Vector3&);

	void setVelocity2(double v,float hv,float vv);				//速さ・角度で指定する速度

	void setAccel(double x,double y,double z);
	void setAccel2(double v,double a);					//速さ・角度で指定する加速度
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

	void setAngle(const GameLib::Math::Vector3&);	//ベクタで向き指定。Z軸を向ける
	void aim(const Object* target);
	void aim(const GameLib::Math::Vector3&);
	void aimX(const GameLib::Math::Vector3&);		//Z軸のみを回転させ、X軸を向ける

	void go(float v,float a,const GameLib::Math::Vector3&);
	//自分を原点とした座標系に変換する　重い
	void getMyMatrix(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& pos) const;
	//materials適用後の座標をワールド座標に変換する
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
	void leaveParent();	//親から独立する

	void setFlash(int f,int t=0);	//周期>時間
	void setLimit(int limit);
	void setAnimeSpeed(float);
	void setAnimeCount(float);
	void setLight(float intensity,const GameLib::Math::Vector3& color);
	void setLight(float);
	void setFocus(bool b = true);

	void savePrevPosition();
	void launch();					//ワールド座標をもとめる（追加・更新・表示の際勝手によびだされる）
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

	//便利関数
	void fadeAway(int time,float zoom=0.f);	//limitを付けて、徐々に消えてくれるようにする
	void randomRotate(bool x=true,bool y= true, bool z= true);
	float getDistance(const Object&) const;
	float getSquareDistance(const Object&) const;

	//アクセッサ
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
	bool isVisible() const;	//animeCountは負じゃないか？mVisible=trueか？
	float collisionX(int i) const;
	float collisionY(int i) const;
};


//オブジェクト制御クラス
class Sprite {
	//半透明オブジェクトのリスト
	Object** mTransparentObject;
	int mTransparentObjectNumber1;
	int mTransparentObjectNumber2;

	unsigned int mGroup;

public:
	int mObjectMax;
	int mMax;							//配列の一番大きい奴。正確ではない。どこかで計らないとパフォーマンスが落ちていく。

	Object* mObject;
	Area mArea;

	//関数
	static void create();				//インスタンスの初期化
	static void destroy();
	static void removeModel(Model*);						//modelを持つオブジェクトを検索して消去

	void resizeArea(int offsetX,int offsetY,int w,int h,int chipw,int chiph);			//仕様するエリアをリサイズ

	Object* addObject(Model* model);
	Object* copyObject(Object*);
	void launchObject();
	void updateObject(unsigned int flag=0);
	void drawObject(bool zwriteon = true,bool zwriteoff=true);
	void removeObject(unsigned int group = 0);
	void setGroup(unsigned int groupnumber);

	//便利関数
	void spot(float intensity,int time,const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& color=GameLib::Math::Vector3(1.f));

	void setVisible(bool ,unsigned int side=0);		//特定属性のオブジェクトを非表示にする
	void updateObjectCollision();					//当たり判定位置を計算する
private:
	void getObjectCollision();
	void getObjectReact();
	void saveObjectPosition();						//前フレームの情報を保存
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
