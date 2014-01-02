//drawよりひとつ高級な画像機能
#ifndef INCLUDED_NPINTERFACE_H
#define INCLUDED_NPINTERFACE_H

#include "npdraw.h"
#include "npinput.h"
#include "npstring.h"
#include "modellist.h"

class GameData;


//フレームクラス
class Frame {
public:
	static const int mWidth[3];
	static const int mHeight[3];

	static void draw(int x,int y,int w,int h);	
};
//■を半透明描画。フレームの変化系とおもいねえ
class Rect{
public:
	static void draw(int x,int y,int w,int h,unsigned int color = 0x80ffffff);
};

//ゲージクラス
class Gauge{
	static void draw(float value,unsigned int color1,unsigned int color2,const GameLib::Math::Vector3&,BlendInfo);
public:
	static void draw(float value,unsigned int color,unsigned int color2,int x,int y,int width,int height,float angle=0.f);
};

//適当に文字だしてくれるクラス
class Message{
	static int mX;
	static int mY;
	static int mSize;
	static unsigned int mColor;
	Message();
public:
	static void position(int x,int y);
	static void color(unsigned int );
	static void size(int );
	static int getHeight();		//一行のサイズを得る
	static void draw(const char*);
	static void draw(int );
};

class WallPaper{
public:
	static void draw();
	static void white();
	static void black();
};


//////////////////////////////////////////////////////////////////////////////

class Cursor{
public:
	int mX,mY,mZ;					//表示座標
	int mAimX,mAimY;				//目的座標
	bool mReset;
	int mType;

	Cursor();
	void setPosition(int x,int y);
	void draw();
	void update();
};
//////////////////////////////////////////////////////////////////////////////

static const unsigned int DefaultTextColor = 0xffffffff;

class Widget;
//ウィンドウクラス
class Window{
	static const int mPaddingX=0;
	static const int mPaddingY=0;
	static const int mCaptionHeight = 16;

	static const int mMax = 257;
	Widget* mWidget[mMax];
	int mCount;
	int mWidth,mHeight;					//フレームのサイズ
	int mX,mY;							//表示位置　中心座標でもってる
	int mDatum;							//0　中心基準　1左上基準
	int mScroll;						//スクロール量
	int mScrollHeight;					//スクロールの縦幅
	int mLine;							//一ページあたりの行数
	Cursor mCursor;
	int mIndex;

	MyString mCaption;
	bool mFocus;						//カーソル表示する？
public:

	Window();
	~Window();
	int update();									//1決定　2キャンセル

	void release();
	void reset();			//中身を更新する前に使う。releaseだと初期化したくないパラメータがあるので。
	void initialize();		//カーソル、スクロール位置を初期化
	void focus(int i);

	int add(const char*,unsigned int color,int size,int x,int y,bool resize,bool select=false);						//文字を追加
	int add(int ,unsigned int color,int size,int x,int y,bool resize,bool select=false);							//文字を追加（数字）
	int addChoice(const char*,int parameter,bool resize = true,unsigned int color = DefaultTextColor,int size = 1);	//文字を追加。かんたん処理用。リストはこれで一発！
	int addLine(const char*,unsigned int color = DefaultTextColor,int size = 1,bool resize=true);					//文字を追加。かんたん処理用。リストはこれで一発！

	void set(int i,const char*,bool resize=false);	//文字列を変更
	void set(int i,int param,bool resize=false);	//文字列を変更（数字

	int add(Batch*,BlendInfo&,int x,int y,bool select);
	void draw(bool cursor=true,int mark=-1);

	int add(Widget*,int x,int y,bool resize,bool select);

	void resize(int i);
	void resize(int width,int height);
	void resizeScroll(int i);

	void position(int x,int y);							//左上座標を指定
	void centerPosition(int x,int y);					//中心座標を指定
	void setParameter(int i,int value);
	int getParameter() const;									//フォーカスがいってるwidgetのパラメータを得る
	int getPositionX(float)const;
	int getPositionY(float)const;
	void setCaption(const char*);

private:
	int getNewWidget();

	int clientX() const;	//client
	int clientY() const;
	int clientWidth() const;
	int clientHeight() const;
};
//部品クラス
class Widget {
public:
	int mX,mY;
	int mWidth,mHeight;
	int mCursorX,mCursorY;	//カーソルがさす位置

	bool mFocus;	//フォーカスを得るか
	int mResult;

	int mParameter;	//汎用パラメータ。

	Widget();
	virtual ~Widget();
	virtual void draw(int x,int y,bool flash =false) = 0;
	virtual bool update() =0;
};


//テキストクラス
class Text : public Widget{
	//カーソルを使う時に必要なパラメータ
	int mLine;
	int mFontWidth,mFontHeight;

	MyString mString;
	int mFont;
	unsigned int mColor;
public:

	void set(const char* string,unsigned int color,int fontsize);
	void set(const char* string);

	void getSize();
	void draw(int x,int y,bool flash =false);
	bool update();
};

//バッチ
class Picture : public Widget{
	Batch* mBatch;
	BlendInfo mBlendInfo;
public:

	void set(Batch*,BlendInfo&);
	void draw(int x, int y ,bool flash =false);
	bool update();
};

//yesnoダイアログ
class Confirm{
	Window mWindow;
	bool mCancelButton;	//キャンセルキーによるキャンセル
public:
	void set(const char*,const char* yes = 0,const char* no = 0,bool cancelbutton = true);
	void position(int x,int y);
	void centerPosition(int x,int y);
	int update();				//はいなら1、キャンセルかいいえなら2
	void draw();
};


///////////////////////////////////////////////////////////////////
//複数のtextureを表示する背景用モジュール
class Site{
	class Object{
	public:
		MyTexture mTexture;
		GameLib::Math::Vector3 mPosition;
		GameLib::Math::Vector3 mScale;

		Object();
	};
	Object* mObject;
	int mObjectNumber;
public:
	Site();
	~Site();
	void initialize();
	void load(const char* path);
	void read(Tag*);
	void draw(const GameLib::Math::Vector3& pos,int count);
	void draw(const GameLib::Math::Vector3& pos);
};
//めんどくさいので放置
//class ListBox : public Widget {
//	Cursor mCursor;
//	int mIndex;
//	int mLine;		//表示する行数
//	int mScroll;	//スクロール量
//	int mMax;		//行数
//	Widget* mWidget[256];
//public:
//	void draw(int x, int y );
//	bool update();
//	bool modal();
//};


//テキスト内の行を選択する
/*
class ListBox : public Text{
	Cursor mCursor;

	bool mCancel;			//キャンセル可能か？
	int mCursorY;

	bool mReady;			//falseだと決定キーがきかない　上下キーを押すとtrueになる
	int mOffset;			//選択肢の範囲
	int mNumber;

	int mCount;				//つくってからの経過フレーム
public:
	int mResult;			//-1でキャンセル

	void create (const char*,int x,int y,int charsize,bool cancel=false,unsigned int cusror=0,bool ready=true,int min=0,int max=255);
	void position(int x,int y);
	bool update();										//決定したらtrueを返す
	void draw();
	void setString(const char*,bool resize=true);

};
*/

#endif
