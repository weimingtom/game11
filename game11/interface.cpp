#include  "npstring.h"
#include  "npfile.h"

#include  "interface.h"
#include  "npsysc.h"
#include  "npsound.h"
#include  "modellist.h"
#include  "gamedata.h"

////////////////////////////////////////////////////////////////////////////////////////
const int Frame::mWidth[3] = {32,64,32};
const int Frame::mHeight[3] = {32,64,32};

void Frame::draw(int x,int y,int w,int h) {
	Draw::instance()->renderString();

	//枠サイズ調整
	float fw=0.5f;
	float fh=0.5f;

	//最小サイズチェック
///	if (w<(mWidth[0] + mWidth[2])*fw) {
//		w=(mWidth[0] + mWidth[2])*fw;
//	}
//	if (h<(mHeight[0] + mHeight[2])*fh) {
//		h=(mHeight[0] + mHeight[2])*fh;
//	}

	//中心の拡大率
	//float w1 = (1.0f*w-fw*(mWidth[0] +mWidth[2] )) / mWidth[1];
	//float h1 = (1.0f*h-fh*(mHeight[0]+mHeight[2])) / mHeight[1];
	float w1 = (1.0f*w) / mWidth[1];
	float h1 = (1.0f*h) / mHeight[1];

	//座標
	float x0=x;
	float y0=y;

	float x1=1.f*x +0.5f*w;
	float y1=1.f*y -0.5f*h;

	float x2=x +w;
	float y2=y -h;

	BlendInfo bi;
	//bi.mCamera=false;
	bi.setBlendMode(1);
	bi.mBlendElement.mDepthTest=false;
	int z=490;
	//左上
	bi.setZoom(fw,fh);
	ModelList::instance()->mSystem.mFrame.batch("左上")->draw(x0 ,y0 ,z,bi);

	//上
	bi.setZoom(w1,fh);
	ModelList::instance()->mSystem.mFrame.batch("上")->draw(x1 ,y0 ,z,bi);
	//右上
	bi.setZoom(fw,fh);
	ModelList::instance()->mSystem.mFrame.batch("右上")->draw(x2 ,y0 ,z,bi);
	//左
	bi.setZoom(fw,h1);
	ModelList::instance()->mSystem.mFrame.batch("左")->draw(x0 ,y1 ,z,bi);


	//まんなか
	bi.setZoom(w1,h1);
	ModelList::instance()->mSystem.mFrame.batch("まんなか")->draw(x1 ,y1 ,z,bi);	

	//右
	bi.setZoom(fw,h1);
	ModelList::instance()->mSystem.mFrame.batch("右")->draw(x2 ,y1 ,z,bi);
	//左下
	bi.setZoom(fw,fh);
	ModelList::instance()->mSystem.mFrame.batch("左下")->draw(x0 ,y2 ,z,bi);
	//下
	bi.setZoom(w1,fh);
	ModelList::instance()->mSystem.mFrame.batch("下")->draw(x1 ,y2 ,z,bi);
	//右下
	bi.setZoom(fw,fh);
	ModelList::instance()->mSystem.mFrame.batch("右下")->draw(x2 ,y2 ,z,bi);
};
void Rect::draw(int x,int y,int w,int h,unsigned int color) {
	float ww = (1.0f*w) / 32.f;
	float hh = (1.0f*h) / 32.f;

	BlendInfo bi;
	bi.setBlendMode(1);
	bi.mBlendElement.mDepthTest=false;
	int z=490;
	bi.setZoom(ww,hh);
	bi.setColor(color);
	ModelList::instance()->mSystem.mRect.batch("rect")->draw(x ,y ,z,bi);
}

void Gauge::draw(float value,unsigned int color1,unsigned int color2,int x,int y,int width,int height,float angle) {
	//32x32の画像を使う
	BlendInfo bi;
	bi.mScale.x = 1.f * width / 64;
	bi.mScale.z = 1.f * height / 64;
	bi.mBlendElement.mFrontEnd = true;
	bi.setBlendMode(1);
	bi.mBlendElement.mDepthTest=false;

	bi.mRotate.y=-angle;

	draw(value,color1,color2,
		GameLib::Math::Vector3(x,y,0),
		bi);
}

void Gauge::draw(float value,unsigned int color1,unsigned int color2,const GameLib::Math::Vector3& vct,BlendInfo bi){
	if (color2) {
		bi.setColor(color2);
		ModelList::instance()->mSystem.mGauge.batch()->draw(vct.x,vct.y,vct.z,bi);
	}

	if (value > 0.f) {
		bi.setColor(color1);
		bi.mScale.x *= value;
		ModelList::instance()->mSystem.mGauge.batch()->draw(vct.x,vct.y,vct.z,bi);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Message::mX;
int Message::mY;
int Message::mSize;
unsigned int Message::mColor;

void Message::position(int x,int y) {
	mX = x;
	mY = y;
}
void Message::color(unsigned int color ) {
	mColor = color;
}
void Message::size(int size) {
	mSize = size;
}
void Message::draw(const char* str) {
	Draw::instance()->text(mX,mY-2,str,mColor,mSize);

	mY -= getHeight();		//そのままだと詰まりすぎるので、+4pxの余白を持たせよう
}

void Message::draw(int i) {
	Draw::instance()->text(mX,mY-2,i,mColor,mSize);

	mY -= getHeight();		//そのままだと詰まりすぎるので、+4pxの余白を持たせよう
}

int Message::getHeight() {
	return Draw::mFontSize[mSize]+4;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WallPaper::draw() {
//	ModelList::instance()->mSystem.mWall.batch()->draw(0,0,0);
};
void WallPaper::white() {
	BlendInfo bi;
	bi.mBlendElement.mEmission = 1.f;
//	ModelList::instance()->mSystem.mWall.batch()->draw(0,0,0,bi);
};
void WallPaper::black() {
	BlendInfo bi;
	bi.mBlendElement.mAmbient = -1.f;
//	ModelList::instance()->mSystem.mWall.batch()->draw(0,0,0,bi);
};






////////////////////////////////////////////////////////////////////////////////////////////////////

Widget::Widget() {
	mCursorX = 0;
	mCursorY = 0;

	mFocus = false;
	mResult = 0;
	mParameter = -1;
}

Widget::~Widget() {
};

void Text::set(const char* string,unsigned int color ,int fontsize) {
	mFont=fontsize;
	mString=string;
	mColor = color;

	getSize();
}
void Text::set(const char* string) {
	mString=string;
	getSize();
}

void Text::getSize() {
	//要素の最大数を求める
	mLine=nString::getLine(mString.data());

	int m=0;
	for (int i=0 ; i<mLine ; ++i) {
		char s[256];
		nString::getLine(s,mString.data(),i);
		int n=strlen(s);
		if (n>m) {
			m=n;
		}
	}

	//l行 m文字に必要なピクセル数を出す（適当でいいや）
	if (mFont==0) {
	}
	else {
	//フォントを使う場合、サイズは正確に分からない。
		mFontWidth  = Draw::mFontSize[mFont-1] /2;
		mFontHeight = Draw::mFontSize[mFont-1];
	}

	mWidth  = mFontWidth  *m;
	mHeight = mFontHeight *mLine+4;
}
void Text::draw(int x,int y,bool flash) {
	if(mFont==0) {
	}
	else {
		Draw::instance()->text(mX+x,mY+y-2,mString.data(),mColor,mFont-1);
	}
}
bool Text::update() {
	if (mFocus) {
		if (cKey::trigger(5)) {
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Picture::set(Batch* batch,BlendInfo& bi) {
	mBatch = batch;
	mBlendInfo = bi;
}
void Picture::draw(int x,int y,bool flash) {
	mBatch->draw(mX+x,mY+y,0,mBlendInfo);
};

bool Picture::update() {
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window() {
	for (int i=0 ; i<mMax ; ++i) {
		mWidget[i] = 0;
	}
	mDatum = 0;
	release();
}

Window::~Window() {
	release();	
}
int Window::update() {
	int result =0;
	if (mCount >7) {
		//何らかの理由で指してるwidgetがなかったら、処理しよう
		if (mWidget[mIndex] == 0 && mIndex > 0) {
			for (;;) {
				--mIndex;
				if (mIndex == 0) {
					break;
				}
				if (mWidget[mIndex]) {
					if (mWidget[mIndex]->mFocus) {
						break;
					}
				}

			};
		}
	
		bool t=false;
		bool b=false;
		if (mWidget[mIndex]) {
			b = mWidget[mIndex]->update();
		}

		if (!b) {
			int v=0;
			int c=1;
			if (cKey::get(0)) {
				v=-1;
			}
			if (cKey::get(1) || cKey::get(11)) {
				v=1;
				c=mLine;
			}
			if (cKey::get(2)) {
				v=1;
			}
			if (cKey::get(3) || cKey::get(10)) {
				v=-1;
				c=mLine;
			}
			if (v) {
				for (int j=0 ; j<c ; ++j) {
					int k=mIndex;
					for (int i=0 ; i<mMax; ++i) {
						k += v;
						if (k < 0) {
							if (c==1) {
								k = mMax-1;
							}
							else {
								c=0;
								break;
							}
						}
						if (k >= mMax) {
							if (c==1) {
								k = 0;
							}
							else {
								c=0;
								break;
							}
						}
						if (mWidget[k]) {
							if (mWidget[k]->mFocus) {
								mIndex = k;
								break;
							}
						}
					}

				}
			}
		}
		if (b) {
			//ModelList::instance()->mSoundSet.play("cursorOk");
			result = 1;
		}
		//キャンセル
		if (cKey::trigger(6)) {
			ModelList::instance()->mSoundSet.play("cursorCancel");
			result=2;
		}
	}

	return result;
}

void Window::release() {
	reset();
	resize(0,0);
	initialize();
};

void Window::reset() {
	for (int i=0 ; i<mMax ; ++i) {
		if (mWidget[i]) {
			delete mWidget[i];
			mWidget[i] = 0;
		}
	}
	mScrollHeight = 0;
	mLine = 0;
};
void Window::initialize() {
	//いちばん若いwidgetを探す
	mIndex = 0;
	mFocus = false;

	for (int i=0 ; i<mMax ; ++i) {
		if (mWidget[i]) {
			if (mWidget[i]->mFocus) {
				mFocus = true;
				mIndex = i;
				break;
			}
		}
	}

	mScroll= 0;
	mCount = 0;

	mCursor.mReset = true;

};
void Window::focus(int p) {
	for (int i=0 ; i<mMax ; ++i) {
		if (mWidget[i]) {
			if (mWidget[i]->mFocus) {
				if (mWidget[i]->mParameter == p) {
					mFocus = true;
					mIndex = i;
					break;
				}
			}
		}
	}

}

//汎用widget追加
int Window::add(Widget* widget,int x,int y,bool res,bool select) {
	ASSERT(x>=0 && y<=0);

	int c=getNewWidget();

	widget->mFocus = select;
	if (!mFocus && select) {
		mFocus = true;
		mIndex = c;
	}
	widget->mX = x;
	widget->mY = y;
	mWidget[c]= widget;

	resizeScroll(c);
	if (res){
		resize(c);
	}

	return c;
};

int Window::add(const char* str,unsigned int color,int size,int x,int y,bool res,bool select) {
	//上や左にはみでる場合は不許可にすべきだろう。
	ASSERT(x>=0 && y<=0);

	Text* t = new Text;
	t->set(str,color,size);

	return add(t,x,y,res,select);
};

int Window::add(int i,unsigned int color,int size,int x,int y,bool res,bool select) {
	MyString t;
	t = i;
	return add(t.data(),color,size,x,y,res,select);
}

int Window::addChoice(const char* str,int parameter,bool resize,unsigned int color,int size) {
	int r = add(str,color,size,0,-mScrollHeight,resize,true);
	mWidget[r]->mParameter = parameter;
	return r;
};

int Window::addLine(const char* str,unsigned int color,int size,bool resize) {
	int r = add(str,color,size,0,-mScrollHeight,resize,false);
	return r;
};

//バッチ追加
int Window::add(Batch* batch,BlendInfo& bi,int x,int y,bool select) {
	Picture* p = new Picture;
	p->set(batch,bi);

	return add(p,x,y,false,select);
}

int Window::getNewWidget() {
	int c=-1;
	for (int i=0 ; i<mMax ; ++i) {
		if (mWidget[i] == 0) {
			c=i;
			break;
		}
	}
	ASSERT(c>=0);
	return c;
}

void Window::set(int i,const char* string,bool res) {
	Text* t = dynamic_cast<Text*>(mWidget[i]);
	t->set(string);

	if (res){
		resize(i);
	}
}
void Window::set(int i,int param,bool res) {
	MyString t;
	t=param;
	set(i,t.data(),res);

}

//widgeのサイズに合わせる
void Window::resize(int i) {

	//widgetを収めるのに必要なwindowサイズ
	int w = mWidget[i]->mX + mWidget[i]->mWidth + mPaddingX*2;
	int h = -mWidget[i]->mY + mWidget[i]->mHeight+ mPaddingY*2;
	if(!(mCaption == "")) {
		h += mCaptionHeight;
	}

	if (w<mWidth) {
		w=mWidth;
	}
	if (h<mHeight) {
		h=mHeight;
	}
	if (w>mWidth || h>mHeight) {
		resize (w,h);
	}

}
void Window::resizeScroll(int i) {
	int h = -mWidget[i]->mY + mWidget[i]->mHeight+ mPaddingY*2;
	if (h>mScrollHeight) {
		mScrollHeight = h;
	}
};


void Window::resize(int w,int h) {
	ASSERT(w>=0 && h>=0);

	//左上基準なら、座標を覚えておかねばならない
	int x,y;
	if (mDatum == 1) {
		x = mX - mWidth/2;
		y = mY + mHeight/2;
	}

	mWidth = w;
	mHeight= h;

	if (mDatum == 1) {
		position(x,y);
	}
}
void Window::centerPosition(int x,int y) {
	mX=x;
	mY=y;
	mDatum = 0;
};

void Window::position(int x,int y) {
	mX=x + mWidth/2;
	mY=y - mHeight/2;
	mDatum = 1;
}

void Window::draw(bool cursor,int mark) {
	mLine = 0;
	if (mCount < 8) {
		Frame::draw(
			getPositionX(0.f),getPositionY(0.f),
			mWidth,mHeight * mCount / 8);

		++mCount;
	}
	else {
		//フレーム
		int x=mX-mWidth/2;
		int y=mY+mHeight/2;
		Frame::draw(x,y,mWidth,mHeight);

		//キャプション
		Draw::instance()->text(x,y,mCaption.data(),0xffffffff,0);


		//部品
		for (int i=0 ; i<mMax ; ++i) {
			if (mWidget[i]) {
				int q= mScroll + mWidget[i]->mY;
				if (q <= 0 &&
					q >= +mWidget[i]->mHeight - mHeight) {

					int x=clientX();
					int y=clientY()+mScroll;

					mWidget[i]->draw(x,y,mWidget[i]->mParameter==mark);

					//行数を数える
					if (mWidget[i]->mFocus) {
						++mLine;
					}
				}
			}
		}

		//カーソル表示するか
		if (mFocus) {
			if (mWidget[mIndex]) {
				//スクロール量
				if (mWidget[mIndex]->mY +mScroll > 0) {
					mScroll = -mWidget[mIndex]->mY;
				}
				if (mWidget[mIndex]->mY +mScroll < -clientHeight() + mWidget[mIndex]->mHeight) {
					mScroll = -clientHeight() + mWidget[mIndex]->mHeight - mWidget[mIndex]->mY;
				}

				if (cursor) {
					mCursor.setPosition(
						clientX() + mWidget[mIndex]->mX + mWidget[mIndex]->mCursorX,
						clientY() + mWidget[mIndex]->mY + mWidget[mIndex]->mCursorY + mScroll);
					mCursor.update();
					mCursor.draw();
				}
			}
		}
	}
}
int  Window::clientX() const {
	return mX-mWidth/2 + mPaddingX;
}
int  Window::clientY() const {
	int result = mY+mHeight/2 - mPaddingY;
	if (!(mCaption=="")) {
		result -= mCaptionHeight;
	}
	return result;
}
int Window::clientWidth() const {
	return mWidth - mPaddingX*2;
};
int Window::clientHeight() const {
	int result = mHeight - mPaddingY*2;
	if (!(mCaption=="")) {
		result -= mCaptionHeight;
	}
	return result;
};


void Window::setParameter(int i,int value) {
	mWidget[i]->mParameter = value;
};
int Window::getParameter() const{
	if (mWidget[mIndex]) {
		return mWidget[mIndex]->mParameter;
	}
	else {
		return -1;
	}
}
int Window::getPositionX(float a) const {
	return mX + (mWidth * (a-0.5f));
};
int Window::getPositionY(float a) const {
	return mY - (mHeight * (a-0.5f));
};
void Window::setCaption(const char* c) {
	mCaption = c;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------
void Confirm::set(const char * str,const char* yes,const char* no,bool cancelbutton) {
	mCancelButton = cancelbutton;

	mWindow.release();
	mWindow.add(str,DefaultTextColor,1,0,0,true,false);
	if (yes ==0) {
		mWindow.addChoice("はい",1,true);
	}
	else {
		mWindow.addChoice(yes,1,true);
	}

	if (no == 0) {
		mWindow.addChoice("いいえ",2,true);
	}
	else {
		mWindow.addChoice(no,2,true);
	}

	centerPosition(0,0);
}
void Confirm::position(int x,int y) {
	mWindow.position(x,y);
}
void Confirm::centerPosition(int x,int y) {
	mWindow.centerPosition(x,y);
}

void Confirm::draw() {
	mWindow.draw();
};
int Confirm::update() {
	int result = 0;

	int r =mWindow.update();
	if (r==1) {
		result = mWindow.getParameter();
	}
	if (mCancelButton) {
		if (r==2) {
			result = 2;
		}
	}

	if (result) {
		mWindow.initialize();
	}

	return result;
};


//int Window::letterX(int x, int t) {
//	int p=mX - mWidth/2  +mPaddingX;
//	return x * mText[t]->mFontWidth + p;
//}
//int Window::letterY(int y, int t) {
//	int q=mY + mHeight/2  -mPaddingY;
//	return -y * mText[t]->mFontHeight + q;
//}

//選択ダイアログ
/*
void ListBox::create(const char* string,int x,int y,int charsize,bool cancel,unsigned int cursor,bool ready,int offset,int number) {

	mWindow.create(x,y);
	mWindow.add(string,DefaultTextColor,charsize,0,0,true);

	mCancel=cancel;
	mCount=0;
	mCursorY=cursor;
	mReady=ready;
	mResult=0;

	//選択肢になる行の範囲
	mOffset=offset;
	mNumber=number;

	setString(string,true);
};

void Dialog::setString(const char* str,bool resize) {
	mWindow.set(0,str,resize);

	if (mOffset > mWindow.mText[0]->mLine) {
		mOffset = mWindow.mText[0]->mLine-1;
	}
	if (mOffset + mNumber>mWindow.mText[0]->mLine) {
		mNumber=mWindow.mText[0]->mLine - mOffset;
	}
}

void Dialog::position(int x,int y) {
	mWindow.position(x,y);
}

bool Dialog::update() {
	bool result=false;
	if (mReady) {
		//カーソル
		if (cKey::get(0)) {
			mCursorY=(mCursorY-1+mNumber)%mNumber;
		}
		if (cKey::get(2)) {
			mCursorY=(mCursorY+1+mNumber)%mNumber;
		}

		if (cKey::trigger(7)) {
			mResult=mCursorY;
			result=true;
				SoundSet::instance()->mCursorOk.play();
		}
		if (mCancel) {
			if (cKey::trigger(6)) {
				mResult=-1;
				result=true;
				SoundSet::instance()->mCursorCancel.play();
			}
		}
	}
	else {
		//寝ている状態を起こす
		if (cKey::trigger(0)) {
			mReady=true;
		}
		if (cKey::trigger(2)) {
			mReady=true;
		}
	}

	mCursor.setPosition(
		mWindow.letterX(0) ,
		mWindow.letterY(mOffset + mCursorY));		//行サイズも出しとかなきゃなあ
	mCursor.update();

	++mCount;
	return result;
}

void Dialog::draw() {
	mWindow.draw();
	if (mReady) {
		//カーソル
		mCursor.draw();
	}
}
//////////////////////////////
void Confirm::create(const char* str,int x,int y,int charsize){
	MyString temp;
	temp.resize(256);
	temp << str << "\n  うん\n  やっぱやめとく";
	mDialog.create(temp.data(),x,y,charsize,true,0,false,1,2);
};
bool Confirm::update() {
	bool r=mDialog.update();
	if (r) {
		if (mDialog.mResult<0) {
			mResult=false;
		}
		if (mDialog.mResult==0) {
			mResult=true;
		}
		if (mDialog.mResult==1) {
			mResult=false;
		}
	}

	return r;
}
void Confirm::draw() {
	mDialog.draw();
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//カーソルクラス
Cursor::Cursor() {
	mReset=true;
	mX=0;
	mY=0;
	mZ=510;
};

void Cursor::setPosition(int x,int y) {
	mAimX=x;
	mAimY=y;

	if (mReset) {
		mX=mAimX;
		mY=mAimY;
		mReset=false;
	}

};

void Cursor::update() {
	if (mReset) {
		mX=mAimX;
		mY=mAimY;
		mReset=false;
	}
	else {
		mX=(mX+mAimX)/2;
		mY=(mY+mAimY)/2;
	}

};
void Cursor::draw() {
	BlendInfo bi;
	//bi.mCamera=false;
	bi.mBlendElement.mDepthTest=false;
	bi.mBlendElement.mDepthWrite=false;
	ModelList::instance()->mSystem.mCursor.batch("カーソル")->draw(mX,mY,mZ,bi);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Site::Site() {
	mObject = 0;
	mObjectNumber = 0;
}
Site::~Site() {
	initialize();
}
void Site::initialize() {
	if (mObject) {
		delete []mObject;
		mObject = 0;
	}
	mObjectNumber = 0;
};
void Site::draw(const GameLib::Math::Vector3& pos) {
	draw(pos,Main::instance()->mCount);
}
void Site::draw(const GameLib::Math::Vector3& pos, int count) {
	BlendInfo bi;
	GameLib::Math::Vector3 p;
	for (int i= 0 ; i<mObjectNumber ; ++i) {
		p = pos;
		p += mObject[i].mPosition;

		bi.mScale = mObject[i].mScale;
		mObject[i].mTexture.drawModel(p,bi,count);
	}
}

void Site::load(const char* path) {
	NML nml;
	nml.load(path);
	read(nml.root());
}

void Site::read(Tag* t) {
	initialize();

	int n = t->getChild("object");
	mObjectNumber = n;
	if (n) {
		mObject = new Object[n];

		for (int i=0 ; i<n; ++i) {
			Tag* c = t->child("object",i);
			mObject[i].mTexture.load(c->attribute("path")->getValue(),true);
			if (c->attribute("position")) {
				mObject[i].mPosition.set(
					c->attribute("position")->getIntValue(0),
					c->attribute("position")->getIntValue(1),
					c->attribute("position")->getIntValue(2));
			}
			if (c->attribute("scale")) {
				mObject[i].mScale.set(
					c->attribute("scale")->getFloatValue(0)/100.f,
					c->attribute("scale")->getFloatValue(1)/100.f,
					c->attribute("scale")->getFloatValue(2)/100.f);
			}


		}

	}
}

Site::Object::Object() {
	mPosition = 0;
	mScale = 1.f;
};

