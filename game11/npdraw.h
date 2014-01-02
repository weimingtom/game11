#ifndef INCLUDED_NPDRAW_H
#define INCLUDED_NPDRAW_H
#include <string>

#include "GameLib/Framework.h"
#include "GameLib/Graphics/Manager.h"
#include "GameLib/Graphics/Texture.h"
#include "GameLib/Graphics/VertexBuffer.h"
#include "GameLib/Graphics/IndexBuffer.h"
#include "GameLib/Graphics/Vertex.h"
#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Matrix34.h"

#include "GameLib/Scene/PrimitiveRenderer.h"

#include "GameLib/Scene/Font.h"
#include "GameLib/Scene/StringRenderer.h"

#include "npstring.h"
#include "npmath.h"

class Tag;
class MyTexture;
class Batch;
class Model;
class BlendInfo;


//画像制御系


//照明
class Light{
	float mIntensity;						//強度
	GameLib::Math::Vector3 mPosition;		//ライト位置
	GameLib::Math::Vector3 mColor;			//ライト色
	float mIntensityLevel;					//強度係数
	float mAimIntensityLevel;
public:

	Light();

	void set(float intensity,const GameLib::Math::Vector3& position,const GameLib::Math::Vector3& color);

	void setIntensity(float);
	void setPosition(const GameLib::Math::Vector3&);
	void setColor(const GameLib::Math::Vector3&);
	void setQuantity(float level);	//照明の強さ

	void update(int index);
	float intensity() const;


	void setPattern1();		//ななめ上から
	void setPattern2();		//下から
	void setPattern3();		//スポットライト
	void setPattern4();		//常に逆光

};

//ものの質感設定
class Materials{
public:
	GameLib::Graphics::BlendMode mBlendMode;//合成モード
	bool mDepthTest;						//Zテスト
	bool mDepthWrite;						//Z書き込み

	GameLib::Graphics::CullMode mCullMode;	//裏表モード
	//GameLib::Graphics::LightingMode mMode;	//照明　頂点/ピクセル　これは質感じゃねえよなあ
	bool mLighting[4];						//影響をうけるライト

	GameLib::Math::Vector3 mDeffuse;		//反射色
	GameLib::Math::Vector3 mSpecular;		//スペキュラ
	float mSharpness;						//ハイライトの鋭さ


	bool mFrontEnd;							//falseでフロントエンドモード
	int mSprite;							//スプライトモード　常に正面を向く
	bool mNegative;

//時間経過で変化するパラメータ
	GameLib::Math::Vector3 mAmbient;		//環境色
	GameLib::Math::Vector3 mEmission;		//発光
	float mTransparency;					//透明度

	GameLib::Math::Vector3 mAngle;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mPosition;


	Materials();
	void setBlendMode(int i);
	void setCullMode(bool front=false,bool back=false);		//削る面を指定
	void read(const Tag&);
	void merge(const Materials&,float blend);
	void getWorldMatrix(GameLib::Math::Matrix34* out)const;
};

//最終合成情報
class BlendElement{
	//materials由来
	GameLib::Math::Vector3 mDeffuse;		//反射色
	GameLib::Math::Vector3 mSpecular;		//スペキュラ
	float mSharpness;						//ハイライトの鋭さ
	GameLib::Graphics::CullMode mCullMode;
	bool mReverseCulling;

	void reverseCulling();

	//管理
	GameLib::Math::Vector3 mAngle;			//スプライト用に使う
public:
	//blendInfo由来
	int mPolygonIndex,mPolygonNumber;

	//両方由来
	GameLib::Graphics::BlendMode mBlendMode;	//合成モード
	bool mDepthTest;							//Zテスト
	bool mDepthWrite;							//Zバッファ書き込みモード　p370参照
	bool mNegative;

	GameLib::Math::Vector3 mAmbient;			//環境色
	GameLib::Math::Vector3 mEmission;			//発光
	float mTransparency;						//非透明度
	bool mFrontEnd;								//強制的にフロントエンド表示する
	bool mLighting[4];

	void initialize();
	void setBlendMode(int m);		//0.opaque 1.liner 2.add
	void setColor(unsigned int color);
	void set(const Materials&);
	void set(const BlendInfo&);
	void mulColor(const BlendElement&);	//transparency,emission,ambientを乗算

	void setGraphicsManager()const;

	void get(GameLib::Math::Matrix34* out,float x,float y,float z,const BlendInfo&,const Materials&);
	const GameLib::Math::Vector3* angle()const;
};

//合成情報
class BlendInfo {
public:
	BlendElement mBlendElement;

	bool mReverseX,mReverseY,mReverseZ;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mRotate;

	BlendInfo();
	void setBlendMode(int m);		//0.opaque 1.liner 2.add
	void setZoom(float x=1.0f,float y=1.0f,float z=1.0f);
	void setColor(unsigned int color);			//環境色と非透明度を設定

	void getWorldMatrix(GameLib::Math::Matrix34* out,float x,float y,float z,int sprite,bool frontend)const;
	bool cullReverse()const;	//反転ある？
	void getAngleMatrix(GameLib::Math::Matrix34* out,int sprite,bool frontend)const;
};


//モデルデータ
class Model {
public:
	//パタパタアニメ1ループぶんをあらわす
	class Anime{

		//素材情報の合成クラス
		class MaterialsQueue{
			//アニメーションクラス
			class Animate{
			public:
				Interpolation3 mAmbient;		//環境色
				Interpolation3 mEmission;		//発光
				Interpolation3 mSpecular;		//スペキュラ
				Interpolation mTransparency;	//透明度

				Interpolation3 mAngle;
				Interpolation3 mScale;
				Interpolation3 mPosition;

				void add(const Materials&,float t);
				void write(Materials* out,float t)const;
				void calculate();
				void read(const Tag&);
			private:
				void read(Interpolation* param,const Tag&,float devide);
				void read3(Interpolation3* param,const Tag&,const char* name,float devide);
			};

			Animate mAnimate;
			Materials mStart;	//初期値

		public:
			MaterialsQueue();
			void materials(Materials*,float count)const;

			void setStart(const Materials&);
			void add(const Materials&,float time);
			void read(const Tag&);		//animateタグ読み込み
			void refresh();				//計算する

			bool depthWrite() const;	//大まかに、Z描き込みするかどうかはモデルできまると考える
			int sprite() const;			//大まかに、Z描き込みするかどうかはモデルできまると考える
			bool frontEnd() const;
		};
		class Frame{
		public:
			Batch* mBatch;
			int mDelay;
			Math::Polygon mCollision;

			Frame();
		};

		static const int mMax = 16;
		Frame mFrame[mMax];
	public:
		MaterialsQueue mMaterials;

		Model* mModel;
		MyString mName;

		Anime();
		void add(Batch*,int delay,const Math::Polygon& collision);
		void draw(MyTexture* ,GameLib::Math::Matrix34 world,BlendElement be,float count,float scale) const;

		int getLoop(float x,float y, float z,float count) const;
		int getLoop(const GameLib::Math::Matrix34&,const BlendElement&,float count) const;
		Batch* batch(int i) const;
		Batch* batch(GameLib::Math::Matrix34* wm,BlendElement* be,int* loop,
			const GameLib::Math::Matrix34&,
			const BlendElement&,
			float count)const;
		const Frame* frame(GameLib::Math::Matrix34* wm,BlendElement* be,int* loop,
			const GameLib::Math::Matrix34&,
			const BlendElement&,
			float count)const;
	};

	const MyTexture* mParent;
	static const int mMax = 8;
	Anime* mAnime[mMax];	//四方向あれば充分だろうけど。
	MyString mName;
	Model* mNext;			//ループせずに別モデルに以降する
	int mAfterimage;		//残像数
	int mAfterimageInterval;
	float mAfterimageScale;

	Model(const MyTexture& parent);
	~Model();
	//void create(Batch* batch);	//バッチひとつだけなら、これで済ます
	void add(int animeindex,Batch*,int delay,const Math::Polygon& collision);

	void draw(float x,float y,float z,const BlendInfo&,float anime=0.f) const;
	void draw(const GameLib::Math::Vector3& pos,const BlendInfo&,float anime=0.f) const;
	void draw(MyTexture*,float x,float y,float z,const BlendInfo&,float anime=0.f) const;
	void draw(MyTexture*,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;

	Anime* anime(const GameLib::Math::Matrix34 wm,const BlendElement&) const; 
	Anime* anime(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& ang,bool frontend) const;
	
	void getWorldMatrix(GameLib::Math::Matrix34* out,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;
	void getCollision(Math::Polygon* out,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;
	int getAnimeNumber()const;

	//materialsで指定するが、model内では不変なのを前提にする値
	bool depthWrite() const;
	int sprite() const;
	bool frontEnd() const;
};

//バッチ作成時、最後に行う処理
//移動・拡大・回転
class VertexTransform{
public:
	GameLib::Math::Vector3 mPosition;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mAngle;
	bool mReverseNormal;

	VertexTransform();
	void position(float x,float y,float z);
	void scale(float x,float y,float z);
	void rotate(float x,float y,float z);
	void transform(int start,int number,float* x,float* y,float* z,GameLib::Math::Vector3* normal) const;
};

class BatchData;
//バッチ
class Batch {
	GameLib::Graphics::VertexBuffer vBuffer;	//頂点バッファ
	GameLib::Graphics::IndexBuffer iBuffer;		//インデックスバッファ
	int mPolygon;								//ポリゴン数
public:
	bool isEnable;
	MyTexture* mTexture;		//使用テクスチャ
	Materials mMaterials;		//質感
	MyString mName;

	//当たり判定.中心が原点。
	float mCollisionX[4];
	float mCollisionY[4];

	Batch();
	~Batch();
	void release();

	//バッチをつくるときはテクスチャにお願いすること。
	void create(MyTexture* tex,const char* name,float u,float v,float w,float h,unsigned int color,float sx=0,float sy=0,float sz=0,float zoomx=1.f,float zoomy=1.f,bool reversenormal=false,bool cut1pix=false);		//四角形を簡単登録
	void create(MyTexture* tex,const char* name,const float* u,const float* v,const float* x,const float* y,const float* z,const unsigned int* color,const int* index,int vtnumber,int indexnumber,const GameLib::Math::Vector3* normal = 0);		//複雑なバッチ
	void create(MyTexture* tex,const char* name,const BatchData&);

	void draw(float x,float y,float z,const BlendInfo& bi = BlendInfo(),const Materials* materials = 0)const;
	void draw(MyTexture* tex,float x,float y,float z,const BlendInfo& bi,const Materials* materials = 0) const;
	void draw(const GameLib::Math::Matrix34& wm,const BlendElement&)const;	//最後に通るdraw
	void draw(MyTexture* tex,const GameLib::Math::Matrix34& wm,const BlendElement&)const;	//最後に通るdraw
};

//テクスチャ
class MyTexture {
public:
	static const int mBatchMax = 128;
	static const int mModelMax = 64;
private:
	MyString mBatchPath;					//バッチファイルパス
	Batch* mBatch[mBatchMax];				//文字列で捜そう
	Model* mModel[mModelMax];
public:
	static int mLoadingNumber;				//ロード中の数
	static const int mLoadingTextureMax=128;
	static MyTexture* mLoadingTexture[mLoadingTextureMax];

	bool isEnable;
	bool isLoading;							//ロード中である
	bool isLoadingBatch;					//バッチロード待ち
	GameLib::Graphics::Texture mTexture;	//テクスチャ
	bool mExistTexture;						//テクスチャあるの？

	MyTexture();
	~MyTexture();
	void release();
	void load(const char* path, bool batch,bool wait=true,const char* batchpath=0);	//pathにヌルでテクスチャはらない
	void read(const Tag&,Materials mt = Materials());
	void loadUpdate();							//ロードアップデート
	static void loadUpdateAll();
	void draw( float x, float y, float z,const BlendInfo &bi);
	Batch* createBatch(const Tag&,Materials);	//NMLからバッチ作成
	Model* createModel(const Tag&,Materials);

	int getNewBatch() const;
	Batch* createBatch(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff,const VertexTransform& vt = VertexTransform());									//四角形を簡単登録
	Batch* createBatch(const char* name,float* u,float* v,float* x,float* y,float* z,unsigned int* color,int* index,int vtnumber,int indexnumber,GameLib::Math::Vector3* normal = 0);	//複雑なバッチ
	Batch* createBatch(const char* name,const BatchData&);	//複雑なバッチ

	Batch* createChip(const char* name,int chipwidth,int chipheight,int number,int* u,int* v,int* x,int* y,int* z);	//チップタイプのバッチ
	Batch* createMap(const char* name,int chipwidth,int chipheight,int height,int number,int* u,int* v,int* x,int* y,int* z,int* slant);	//3Dマップタイプのバッチ

	Batch* createTriangle(const char* name,float u,float v,float w,float h,unsigned int color1,unsigned int color2,unsigned int color3);	//正三角形
	Batch* createPyramid(const char* name,float u,float v,float w,float h,unsigned int color);		//正方形
	Batch* createHexWall(const char* name,float u,float v,float w,float h,unsigned int color);
	Batch* createGlass(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff);
	Batch* createLaputa(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff,int i=10,int j=8,int k=4);

	bool isReady();
	bool isError() const;
	int width() const;
	int height() const;
	int originalWidth() const;
	int originalHeight() const;

	Batch* batch(const char*)const;	//名前からバッチを捜す
	Batch* batch(int i=0) const;
	Model* model(const char*,bool assert=true)const;
	Model* model(int i=0) const;
	void drawModel(const GameLib::Math::Vector3& pos,const BlendInfo&,float  animecount = 0);
};

class TextureRect{
public:
	float mU,mV,mW,mH;	//テクスチャ座標
	unsigned int mColor;
	GameLib::Math::Vector3 mSize;
	TextureRect();
	void set(float u,float v,float w, float h,unsigned int color);
};

class BatchData{
public:
	float* mU;
	float* mV;
	float* mX;
	float* mY;
	float* mZ;
	GameLib::Math::Vector3* mNormal;
	unsigned int* mColor;
	int* mIndex;

	int mVertexMax;
	int mVertexNumber;
	int mIndexMax;
	int mIndexNumber;

	BatchData();
	~BatchData();
	void create(int vertexMax,int indexMax);
	void setColor(unsigned int color,int start=0,int number=-1);
	void set(const VertexTransform&, int start=0,int number=-1);
	void read(const Tag&);	//batchタグを解析して足す
	void checkError();
	void setGradation(unsigned int color1,unsigned int color2,const GameLib::Math::Vector3&,int start,int number=-1);

	void addTriangle(const GameLib::Math::Vector2* uv,const GameLib::Math::Vector3* ver,unsigned int color = 0xffffffff,const VertexTransform& vt = VertexTransform());	//三角形追加
	void addTriangle(float* u,float* v,float* x,float* y,float* z,unsigned int color = 0xffffffff,const VertexTransform& vt = VertexTransform());			//三角形追加
	void addSphere(const TextureRect&,const VertexTransform& vt = VertexTransform());

	void addSolidOfRevolution(const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addSolidOfRevolution(const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,int split,int min,int max,const TextureRect&,const VertexTransform& vt);

	void addRect			(const TextureRect&,const VertexTransform& vt = VertexTransform(),float normalMode=0.f);
	void addTree			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int number = 2);
	void addCube			(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addUFO				(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addBullet			(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool soko=true);
	void addCylinder		(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool top = true,bool bottom=true,float ratio = 0.f);	//円柱バッチ
	void addTorus			(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addHole			(const TextureRect&,const VertexTransform& vt = VertexTransform());

	void addOctahedron		(const TextureRect&,const VertexTransform& vt = VertexTransform());						//八面体
	void addRectangularPyramid(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool soko=true);	//四角錘

	void addPrism			(const float* x,const float * y,int number,const TextureRect&,const VertexTransform& vt = VertexTransform(),bool futa=true,bool soko=true);//正規化した座標を渡すこと
	void addPrism			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=3,bool futa = true ,bool soko = true);
	void addGear			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=3);
	void addStar			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=5);
	void addConvexPolytope	(const GameLib::Math::Vector3* ver,int n,const TextureRect&,const VertexTransform& vt = VertexTransform());	//正規化した座標を渡すこと
	void addRock			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=2);
};


//カメラクラス
//入れるべき情報
//注視点　角度　ズーム率
//注視点からカメラの距離・上方向ベクトルは固定にする

class Camera {
	bool mActive;							//動くかどうか。Aimが指定されると自動でon
	//再計算用
	GameLib::Math::Vector3 mUpVector;		//上方向
	GameLib::Math::Vector3 mEyeTarget;		//振動を含む注視点
	GameLib::Math::Vector3 mEyePosition;

	float mRange,mAimRange;

	GameLib::Math::Vector3 mFocus;		//フォーカス。毎フレーム送り続けなければならない
	float mFocusRate;					//フォーカスをどれだけ適用するか

	GameLib::Math::Vector3 mTarget;					//カメラ位置を決める注視点
	GameLib::Math::Vector3 mAimTarget;				//注視点

	float mVerticalAngle,mAimVerticalAngle;
	float mHorizotalAngle,mAimHorizotalAngle;
	float mUpAngle,mAimUpAngle;
	float mZoom,mAimZoom;							//ズーム率

	GameLib::Math::Vector3 mVibrate;				//振動

	bool mAngleResetSwitch;
	bool mTargetResetSwitch;
	bool mZoomResetSwitch;
	bool mRangeResetSwitch;
public:

	Camera();
	void reset();	//次に来た目標値をそのまま値にする
	void initialize();
	void update();

	//向きを設定
	void aimAngle(float horizotal,float vertical);				//上ベクトルは固定でいいので二回で済ます
	void autoAngle(float horizotal,float vertical);				//近い方の回転向きを使ってくれる
	void aimAngle(const GameLib::Math::Vector3&);				//対象座標-指定座標を通るようにカメラ向きを決める
	void setAngle(float horizotal,float vertical);				//普通に代入する
	void addAimAngle(float horizotal,float vertical=0.f);		//足す
	void aimVerticalAngle(float v);
	void aimHorizotalAngle(float h);
	void untie();	//一周以上の角度をなかったことにする

	//注視点指定
	void aimTarget(float x,float y,float z);
	void aimTarget(const GameLib::Math::Vector3&);
	void setTarget(float x,float y,float z);
	void setTarget(const GameLib::Math::Vector3&);
	void addAimTarget(float x,float y,float z);
	//ズーム
	void aimZoom(float z);
	void setZoom(float z);
	//視点距離
	void aimRange(float r);
	void vibrate(float x,float y,float z);
	//上方向ベクタの傾き
	void aimUpAngle(float );
	void setUpAngle(float );

	void focus(const GameLib::Math::Vector3&);			//フォーカス

	//情報
	float zoom()const;
	float horizotalAngle()const;
	float verticalAngle()const;
	const GameLib::Math::Vector3* aimTarget() const;
	const GameLib::Math::Vector3* target() const;
	const GameLib::Math::Vector3* position() const;
	const GameLib::Math::Vector3* upVector() const;
	const GameLib::Math::Vector3* eyePosition() const;
	const GameLib::Math::Vector3* eyeTarget() const;
	void getAngle(GameLib::Math::Vector3* out) const;//カメラから注視点へのベクタ
	float range() const;	//実際に使う距離
	float getAimRange()const;	//mAimRangeの値
	float getAimZoom()const;	//mAimRangeの値

};

//画像表示統合
class Draw {

public:
	static const int mWidth=800;				//内部で扱うときの座標
	static const int mHeight=600;
	static const int mDepth=512;
	static const int mFontMax = 3;
	static int mFontSize[mFontMax ];
private:
	GameLib::Scene::Font mFont[mFontMax ];
	GameLib::Scene::StringRenderer mStringRenderer;

	Light mLight[4];					//ライティング
	int mCount;

	//ほとんでデバッグ用の機能であることに注意。
	GameLib::Scene::PrimitiveRenderer mPrimitiveRenderer;

	//フェード ffでまっくら。
	int mFade;
	int mFadeAim;
public:
	int mPolygonPerFrame;	//デバッグ用カウンタ
	bool mSkip;				//描画しないモード。ターボの描画スキップに使えたらいいな。

	Camera mCamera;
	//ゼロバッチ
	Batch mZeroBatch;

	//ビュー変換	
	GameLib::Math::Matrix44 mPVM;
	GameLib::Math::Matrix44 mFrontEndPVM;
	GameLib::Math::Matrix34 mInvertWMforSprite1;
	GameLib::Math::Matrix34 mInvertWMforSprite2;

	//関数郡
	void update();

	//primitiveを使う。遅いので注意。
	void drawLine( int x1,int y1,int x2, int y2,float scroll=1.0f,unsigned int c1=0xffffffff,unsigned int c2=0xffffffff);
	void drawRect( int x1,int y1,int x2, int y2,unsigned int color=0xffffffff,int z=512);

	void drawLine(const GameLib::Math::Vector3&,const GameLib::Math::Vector3&);
	void draw();
	void setFade(int i);
	void aimFade(int i);
	void renderString();
	void nega() const;		//ネガティブにする

	//文字表示
	void text(int x, int y,const char* txt,unsigned int color,int size);
	void text(int x, int y,char           ,unsigned int color,int size);
	void text(int x, int y,int            ,unsigned int color,int size);
	void text(int x, int y,unsigned int   ,unsigned int color,int size);
	void text(int x, int y,bool           ,unsigned int color,int size);

	//照明操作
	Light* light(int i);

	void getScreenCoordinate(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& in) const;	//スクリーン座標を得る

	static Draw* instance();
	static void create();
	static void destroy();
private:
	static Draw* mInstance;
	Draw();
	~Draw();
};

class Scanimate{
public:
	MyTexture mTexture;
	Batch** mBatch;
	//Batch* ba;
	int mSplitNumber;
	int mCount;

	Scanimate();
	~Scanimate();
	void create(const char* path,int count,int i = -1);
	void draw(int x,int y);
};

#endif
