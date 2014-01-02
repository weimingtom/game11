
#include "npsysc.h"
#include "npdraw.h"
#include "npinput.h"
#include "config.h"
#include "interface.h"
#include "CSV.h"

//#include "replaymenu.h"

sConfig::sConfig() {
	mSwitched=false;
	mCount=0;
	mCursorY=0;

	mConfig=PrimaryConfig::mConfig;
	mAssign=0;
	mInputPlayerName=0;


	//パラメータ登録
	mParameter[0].setName("ウィンドウモード");
	mParameter[0].set("ウィンドウ,フルスクリーン");
	mParameter[0].setReboot();
	mParameter[0].setValue(&mConfig.mFullScreen);

	mParameter[1].setName("ウェイト");
	mParameter[1].set("タイマー,垂直同期");
	mParameter[1].setReboot();
	mParameter[1].setValue(&mConfig.mVSync);

	mParameter[2].setName("FPS表示");
	mParameter[2].setBool();
	mParameter[2].setValue(&mConfig.mFps);

	
	mParameter[3].setName("画面サイズ");
	mParameter[3].set("640x480,800x600,1024x768,1280x960");
	mParameter[3].setReboot();
	mParameter[3].setValue(&mConfig.mWindowSize);

	mParameter[4].setName("テクスチャフィルター");
	mParameter[4].setBool();
	mParameter[4].setValue(&mConfig.mTextureFilter);

	mParameter[5].setName("アンチエイリアス");
	mParameter[5].setBool();
	mParameter[5].setReboot();
	mParameter[5].setValue(&mConfig.mAntiAlias);

	mParameter[6].setName("ピクセルシェーダ");
	mParameter[6].setBool();
	mParameter[6].setValue(&mConfig.mPixelShader);

	mParameter[7].setName("表示軽量化");
	mParameter[7].setBool();
	mParameter[7].setValue(&mConfig.mOmit);

	mParameter[8].setName("BGM");
	mParameter[8].setBool();
	mParameter[8].setReboot();
	mParameter[8].setValue(&mConfig.mBGM);

	mParameter[9].setName("BGMボリューム");
	mParameter[9].setInt(11);
	mParameter[9].setValue(&mConfig.mBGMVolume);

	mParameter[10].setName("SEボリューム");
	mParameter[10].setInt(11);
	mParameter[10].setValue(&mConfig.mSEVolume);

	MyString tmp(64);
	tmp << "パッド（接続数";
	tmp << cKey::joyStickNumber();
	tmp << "）";
	mParameter[11].setName(tmp.data());
	mParameter[11].setBool();
	mParameter[11].setValue(&mConfig.mPad);
}

sConfig::~sConfig() {
	if (mAssign) {
		delete mAssign;
		mAssign=0;
	}
	if (mInputPlayerName) {
		delete mInputPlayerName;
		mInputPlayerName=0;
	}
};
bool sConfig::update() {
	if (mAssign) {
		bool r=mAssign->update( mConfig.mKeyAssign );
		if (r) {
			delete mAssign;
			mAssign=0;
		}
		return false;
	}

	Draw* Draw=Draw::instance();

	int x=-320;
	int y=240;
	int x2=x+240;
	int size=1;
	int i=0;
	int line=-24;
	Frame::draw(x,y,-x*2,y*2);

/*
	if (mInputPlayerName) {
		bool r=mInputPlayerName->update();
		if (r) {
			for (int i=0 ; i<4 ; ++i) {
				mConfig.mPlayerName[i]=mInputPlayerName->mResult[i];
			}

			delete mInputPlayerName;
			mInputPlayerName=0;
		}
		return false;
	}
*/
	if (mSwitched) {
		Draw->text(x,y+line*i,"保存せずもどる",0xffffffff,size);
	}
	else {
		Draw->text(x,y+line*i,"もどる",0xffffffff,size);
	}
	if (mCursorY==i) {
		if (cKey::frame(5)==1) {
			return true;
		}
	}
	++i;

	Draw->text(x,y+line*i,"保存",0xffffffff,size);
	if (mCursorY==i) {
		if (cKey::frame(5)==1) {
			PrimaryConfig::mConfig=mConfig;			//あれ、コピーできるの？
			PrimaryConfig::mConfig.update();
			PrimaryConfig::mConfig.save();
			mSwitched=false;
		}
	}
	++i;

	Draw->text(x,y+line*i,"デフォルト",0xffffffff,size);
	if (mCursorY==i) {
		if (cKey::frame(5)==1) {
			mConfig = Config();
			mSwitched=true;
		}
	}

	++i;

	Draw->text(x,y+line*i,"キー配置",0xffffffff,size);
	if (mCursorY==i) {
		if (cKey::frame(5)==1) {
			mAssign = new Assign(mConfig.mKeyAssign);
			mSwitched=true;
		}
	}
	++i;

////////////////////////////////////////////////////////////////
	//各パラメータ
	for (int w=0 ; w<12 ; ++w) {
		mParameter[w].draw(x,y+line*(i+w));
	}
	if (mCursorY >= i) {
		int w = mCursorY - i;
		bool s =  mParameter[w].update();
		if (s) {
			mSwitched = true;
		}
	};

	i += 12;

////////////////////////////////////////////////////////////////

	if (cKey::get(0)) {
		mCursorY=(mCursorY-1+i)%i;
	}
	if (cKey::get(2)) {
		mCursorY=(mCursorY+1+i)%i;
	}

	mCursor.setPosition(x,mCursorY*line+y);
	mCursor.update();
	mCursor.draw();
	++mCount;
	return 0;
};
////////////////////////////////////////////////////////////////

sConfig::Parameter::Parameter() {
	mNumber = 0;
	mReboot = false;
	mInt = 0;
	mBool = 0;
	mState = 0;
};
sConfig::Parameter::~Parameter() {
	if (mState) {
		delete []mState;
		mState = 0;
	}
};

void sConfig::Parameter::setName(const char* name) {
	mName = name;
}

void sConfig::Parameter::setReboot() {
	mReboot = true;
}
void sConfig::Parameter::setBool() {
	ASSERT(mState == 0);
	mState = new MyString[2];
	mState[0] = "無効";
	mState[1] = "有効";
	mNumber = 2;
}

void sConfig::Parameter::setInt(int n) {
	ASSERT(mState == 0);
	mState = new MyString[n];
	for (int i=0 ; i<n ; ++i) {
		mState[i] = i;
	}
	mNumber = n;
}
void sConfig::Parameter::set(const char* param) {
	ASSERT(mState == 0);

	CSV csv;
	csv.read(param);
	mNumber = csv.row(0)->line();
	mState = new MyString[mNumber];
	for (int i=0 ; i<mNumber ; ++i) {
		mState[i] = csv.cell(0,i)->mValue.data();
	}
}

void sConfig::Parameter::setValue(unsigned int* i) {
	mInt = i;
};
void sConfig::Parameter::setValue(bool* b) {
	mBool = b;
};


bool sConfig::Parameter::update() {
	int v=0;
	if (cKey::get(1)) {
		v = 1;


	}
	if (cKey::get(3)) {
		v = -1;
	}
	if (mInt) {
		*mInt += v;
		if (v>0) {
			if (*mInt >= mNumber) {
				*mInt = 0;
			}
		}
		if (v<0) {
			if (*mInt >= mNumber) {
				*mInt = mNumber-1;
			}
		}
	}
	if (mBool) {
		if (v) {
			*mBool = !(*mBool);
		}
	};
	if (v) {
		return true;
	}
	else {
		return false;
	}
};

void sConfig::Parameter::draw(int x,int y) const{
	int size=1;
	Draw* d = Draw::instance();

	d->text(x    ,y,mName.data(),0xffffffff,size);

	if (mInt) {
		d->text(x+240,y,mState[*mInt].data(),0xffffffff,size);
	}
	if (mBool) {
		if (*mBool) {
			d->text(x+240,y,mState[1].data(),0xffffffff,size);
		}
		else {
			d->text(x+240,y,mState[0].data(),0xffffffff,size);
		}
	}
	if (mReboot) {
		d->text(x+400,y,"再起動後に反映",0xffffffff,size);
	}
};


///////////////////////////////////////////////////////////////////////////////
sConfig::Assign::Assign(const unsigned int* assign) {
	mWindow.position(-320,240);
	refresh(assign);
};

void sConfig::Assign::refresh(const unsigned int* assign) {
	mWindow.reset();

	const int size = 2;
	const unsigned int color = DefaultTextColor;

	mWindow.addChoice("戻る",0,true,color,size);

	MyString tmp(64);

	tmp = "";
	tmp << *(assign+1) ;
	tmp << "  X       ショット・決定";
	mWindow.addChoice(tmp.data(),2,true,color,size);

	tmp = "";
	tmp << *(assign+2) ;
	tmp << "  Z       キャンセル";
	mWindow.addChoice(tmp.data(),3,true,color,size);

	tmp = "";
	tmp << *(assign+4) ;
	tmp << "  ctrl    倍速";
	mWindow.addChoice(tmp.data(),5,true,color,size);

	tmp = "";
	tmp << *(assign+5) ;
	tmp << "  space   スナップショット";
	mWindow.addChoice(tmp.data(),6,true,color,size);

};

bool sConfig::Assign::update(unsigned int *assign) {

	if (mWindow.getParameter()>0) {
		//倍速・スクショを無効
		cKey::assigning();
	}

	mWindow.draw();
	if (mWindow.update()) {
		if (mWindow.getParameter() == 0) {
			return true;
		}
	}

	//パッドから押されているキーをひとつ得る
	int k=cKey::getPadOne();
	if (k>=0) {
		if (mWindow.getParameter()>0) {
			*(assign+mWindow.getParameter()-1)=k;
			refresh(assign);
		};
	};

	return false;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
sConfig::InputPlayerName::InputPlayerName(const char* name) {
	if (name) {
		for (int i=0 ; i<4 ; ++i) {
			mResult[i]=name[i];//PrimaryConfig::mConfig.mPlayerName[i];
		}
	}
	else {
		for (int i=0 ; i<4 ; ++i) {
			mResult[i]=PrimaryConfig::mConfig.mPlayerName[i];
		}
	}
	for (int i=0 ; i<3 ; ++i) {
		mLength=i;
		if (mResult[i]==0) {
			break;
		}
	}

	mPosition=0;

	char s[128];
	int p=0;
	for (int i=0 ; i<96 ; ++i) {
		s[p]=i+32;
		s[p+1]=0;
		++p;
		if (i % 16 ==15) {
			s[p]='\r';
			s[p+1]='\n';
			s[p+2]=0;
			p+=2;
		}
	}

	mDialog.add(s,0xffffffff,0,0,0,true);
	mDialog.centerPosition(320,320);

	mDialog2.add("   ",0xffffffff,0,0,0,true);
	mDialog2.centerPosition(320,128);

	if (mLength) {
		mPosition=95;
		mDialog2.set(0,mResult,false);
	}
}

bool sConfig::InputPlayerName::update() {

	//96文字
	if (cKey::get(0)) {
		mPosition-=16;
	}
	if (cKey::get(1)) {
		++mPosition;
	}
	if (cKey::get(2)) {
		mPosition+=16;
	}
	if (cKey::get(3)) {
		--mPosition;
	}
	mPosition=(mPosition+96) % 96;

	if (cKey::get(7)) {
		if (mPosition==95) {
			return true;
		}
		else {
			mResult[mLength]=mPosition+32;
			mResult[mLength+1]=0;
			if (mLength<2) {
				++mLength;
			}
			mDialog2.set(0,mResult,0);
		}
	}
	if (cKey::get(6)) {
		if (mResult[mLength]==0) {
			if (mLength>0) {
				--mLength;
			}
		}
		mResult[mLength]=0;
		mDialog2.set(0,mResult,false);
	}

	WallPaper::draw();
	mDialog.draw();
	mDialog2.draw();
//	mCursor.setPosition( 
//		mDialog.letterX(mPosition % 16) +4,
//		mDialog.letterY(mPosition / 16) +12);
	mCursor.update();
	mCursor.draw();

	return false;
};
*/