#include "GameLib/Graphics/Manager.h"
#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"
#include "GameLib/WindowCreator/WindowCreator.h"

#include "game.h"

#include "npsysc.h"
#include "npsound.h"
#include "npmath.h"
#include "npobject.h"
#include "npdraw.h"
#include "npinput.h"
#include "npstring.h"

#include "npfile.h"
#include "nml.h"


#include "modellist.h"


Config::Config() {
	initialize();
}
void Config::initialize() {
	//初期設定
	mWindowSize=1;
	mCurrentWindowSize=mWindowSize;
	mBGM = true;
	mBGMVolume=3;
	mSEVolume=4;

	mAntiAlias		=true;
	mTextureFilter	=true;
	mVSync			=true;

	mFullScreen		=false;
	mPad			=true;
	mFps			=false;
	mPixelShader	=true;
	mOmit			=false;

	mKeyAssign[0]=0;
	mKeyAssign[1]=1;
	mKeyAssign[2]=2;
	mKeyAssign[3]=3;
	mKeyAssign[4]=4;
	mKeyAssign[5]=5;
	mKeyAssign[6]=6;
	mKeyAssign[7]=7;
	mKeyAssign[8]=8;
	mKeyAssign[9]=9;
	mKeyAssign[10]=10;
	mKeyAssign[11]=11;

}
Config::~Config() {
};

//設定を適用
void Config::update() {
	//グラフィック関係
	if (mTextureFilter) {
		GameLib::Graphics::Manager::instance().setTextureFilter( GameLib::Graphics::TEXTURE_FILTER_LINEAR );
	}
	else {
		GameLib::Graphics::Manager::instance().setTextureFilter( GameLib::Graphics::TEXTURE_FILTER_POINT );
	}

	//音量
	{
		float v=-100;
		for (int i=0 ; i<mSEVolume ; ++i) {
			v/=2.0f;
		};
		Sound::mVolume =v;//(mSEVolume-10)*10;		//0～-100で指定する　-60以下はほとんど聞こえないので、補正をかけよう
	}
	{float v=-100;
		for (int i=0 ; i<mBGMVolume ; ++i) {
			v/=2.0f;
		};
 		BGM::instance()->changeVolume(v);
	 	//cSound::mBGM.mVolume=((mBGMVolume-10)*(mBGMVolume-10)*(mBGMVolume-10))/10;
	}

	//キー設定
	for (int i=0; i<10 ;++i) {
		cKey::mAssign[i]=mKeyAssign[i];
	}
};
void Config::load(){
	///設定ファイルをロード
	//nml.load("config.ini");

	initialize();
	//起動時に行うので、通常のload系関数は使えない
	if (!fileExist("config.ini")) {
		return;
	}

	MyString inifile;
	loadFile( &inifile , "config.ini" );

	NML nml;
	nml.read(inifile.data());

	if (nml.root()->attribute("windowsize")) {
		int arg = nml.root()->attribute("windowsize")->getIntValue();
		if (arg<4) {
			mWindowSize=arg;
			mCurrentWindowSize=mWindowSize;
		}
	};
	if (nml.root()->attribute("bgmvolume")) {
		int arg = nml.root()->attribute("bgmvolume")->getIntValue();
		if (arg<11) {
			mBGMVolume=arg;
		}
	};
	if (nml.root()->attribute("sevolume")) {
		int arg = nml.root()->attribute("sevolume")->getIntValue();
		if (arg<11) {
			mSEVolume=arg;
		}
	};
	mBGM		=nml.root()->get("bgm",false);


	mAntiAlias	=nml.root()->get("antialias",false);
	mTextureFilter	=nml.root()->get("texturefilter",false);
	mVSync		=nml.root()->get("vsync",false);
	mFullScreen	=nml.root()->get("fullscreen",false);
	mPad		=nml.root()->get("pad",false);
	mFps		=nml.root()->get("fps",false);
	mPixelShader=nml.root()->get("pixelshader",false);
	mOmit		=nml.root()->get("omit",false);
/*
	if (nml.root()->attribute("name")) {
		MyString tmp;
		tmp = nml.root()->attribute("name")->getValue();

		for (int i=0 ; i<3 ; ++i) {
			if (tmp.data(i)<32) {
				mPlayerName[i]=0;
				break;
			}
			if (tmp.data(i)>126) {
				mPlayerName[i]=0;
				break;
			}
			mPlayerName[i]=tmp.data(i);
			mPlayerName[i+1]=0;
		}
	};
	*/
	if (nml.root()->child("key")) {
		Tag* k = nml.root()->child("key");

		if (k->attribute("Z")) {
			int arg = k->get("Z",false);
			if (arg<10) {
				mKeyAssign[2]=arg;
			}
		};
		if (k->attribute("X")) {
			int arg = k->get("X",false);
			if (arg<10) {
				mKeyAssign[1]=arg;
			}
		};
		if (k->attribute("C")) {
			int arg = k->get("C",false);
			if (arg<10) {
				mKeyAssign[0]=arg;
			}
		};
		if (k->attribute("A")) {
			int arg = k->get("A",false);
			if (arg<10) {
				mKeyAssign[3]=arg;
				}
		};
		if (k->attribute("ctrl")) {
			int arg = k->get("ctrl",false);
			if (arg<10) {
				mKeyAssign[4]=arg;
			}
		};
		if (k->attribute("space")) {
			int arg = k->get("space",false);
			if (arg<10) {
				mKeyAssign[5]=arg;
			}
		};
		if (k->attribute("Q")) {
			int arg = k->get("Q",false);
			if (arg<10) {
				mKeyAssign[6]=arg;
			}
		};
		if (k->attribute("W")) {
			int arg = k->get("W",false);
			if (arg<10) {
				mKeyAssign[7]=arg;
			}
		};
		if (k->attribute("S")) {
			int arg = k->get("S",false);
			if (arg<10) {
				mKeyAssign[8]=arg;
			}
		};
	}
};

void Config::save() const{
	NML nml;
	nml.root()->addAttribute("windowsize",mWindowSize);
	nml.root()->addAttribute("bgm",mBGM);
	nml.root()->addAttribute("bgmvolume",mBGMVolume);
	nml.root()->addAttribute("sevolume",mSEVolume);
	nml.root()->addAttribute("antialias",mAntiAlias);
	nml.root()->addAttribute("texturefilter",mTextureFilter);
	nml.root()->addAttribute("vsync",mVSync);
	nml.root()->addAttribute("fullscreen",mFullScreen);
	nml.root()->addAttribute("pad",mPad);
	nml.root()->addAttribute("fps",mFps);
	nml.root()->addAttribute("pixelshader",mPixelShader);
	nml.root()->addAttribute("omit",mOmit);
//	nml.root()->addAttribute("name",mPlayerName);

	Tag* k = nml.root()->addChild("key");
	k->addAttribute("C",mKeyAssign[0]);
	k->addAttribute("X",mKeyAssign[1]);
	k->addAttribute("Z",mKeyAssign[2]);
	k->addAttribute("A",mKeyAssign[3]);
	k->addAttribute("ctrl",mKeyAssign[4]);
	k->addAttribute("space",mKeyAssign[5]);
	k->addAttribute("Q",mKeyAssign[6]);
	k->addAttribute("W",mKeyAssign[7]);
	k->addAttribute("S",mKeyAssign[8]);

	MyString data(1024);
	nml.write(&data);
	nSave("config.ini",data.data());
};

int Config::windowWidth() const{
	if (mCurrentWindowSize==0) {
		return 640;
	};
	if (mCurrentWindowSize==1) {
		return 800;
	};
	if (mCurrentWindowSize==2) {
		return 1024;
	};
	if (mCurrentWindowSize==3) {
		return 1280;
	};
};
int Config::windowHeight() const{
	if (mCurrentWindowSize==0) {
		return 480;
	};
	if (mCurrentWindowSize==1) {
		return 600;
	};
	if (mCurrentWindowSize==2) {
		return 768;
	};
	if (mCurrentWindowSize==3) {
		return 960;
	};
};
Config PrimaryConfig::mConfig;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
Main* Main::mInstance=0;

Main* Main::instance() {
	return mInstance;
}
void Main::create() {
	if (mInstance==0) {
		mInstance= new Main;
	}
}
void Main::destroy() {
	ASSERT(mInstance);

	delete mInstance;
	mInstance=0;
};
Main::Main() :
mSnapshot(mDebugRecord){

	mCount=0;
	mTarbo=false;
	mQuit=false;

	Draw::create();

	mNowLoading= new NowLoading;
	mLoadingCount = 0;

	cKey::reset();
	Random::create();

	BGM::create();
	Sprite::create();

	ModelList::create();
	StringBuffer::create();

	mRecordingNow = false;

}

Main::~Main() {
	Game::destroy();
	BGM::destroy();
	Draw::destroy();
	Random::destroy();
	Sprite::destroy();
	ModelList::destroy();
	StringBuffer::destroy();

	if (mNowLoading) {
		delete mNowLoading;
		mNowLoading=0;
	}
}


bool Main::update() {
	//ターボ機能
	int l=1;
	if (mTarbo) {
		++l;
	};
	int fps = 0;
	for (int t=0; t<l ;++t) {
		Draw* Draw=Draw::instance();
		Draw->mSkip=false;
		if (t<(l-1)) {
			Draw->mSkip=true;
		};
		//fps
		fps += GameLib::Framework::instance().frameRate();

		//共通処理
		Draw->update();
		cKey::update();
		//BGM::instance()->update();

		//シーケンスの更新　ゲーム本体
		if (mNowLoading) {
			if (mNowLoading->update()) {
				Game::create();
				delete mNowLoading;
				mNowLoading=0;
			}
			++mLoadingCount;
		}
		else {
			if (Game::instance()->update()) {
				mQuit = true;
				break;
			}
		}

		//スクショ
		bool cap=false;
		if (cKey::isOn(9) && (cKey::frame(9) == 1 )) {
			if (mDebugRecord) {
				//録画モード 30fpsでいっか
				mRecordingNow = !mRecordingNow;
			}
			else {
				cap = true;
			}
			ModelList::instance()->mSoundSet.sound("saved")->play();
		}
		if (mRecordingNow) {
			cap = mCount%2 == 0;
		}

		if (cap) {
			mSnapshot.save();
		};
		++mCount;
	}
	//FPS表示
	if (PrimaryConfig::mConfig.mFps) {
		Draw* Draw=Draw::instance();
		Draw->text(-400+16,-300+24,fps,0xffffffff,0);
	}
	//ポリゴン数表示
	if (Main::instance()->mDebugPrint) {
		Draw::instance()->text(-400+16,-300+24+24,Draw::instance()->mPolygonPerFrame,0xffffffff,0);	
	}


	Draw::instance()->draw();

	//ターボ
	mTarbo=false;
	if (cKey::isOn(8)) {
		mTarbo=true;
	};

	//ESCで終了する
	bool continueswitch;
	continueswitch=true;
	if (cKey::isOn(14)) {
		continueswitch=false;
	}
	//終了コマンド
	if (mQuit) {
		continueswitch=false;
	};

	return continueswitch;
};

Snapshot::Snapshot(bool record){
	mNumber = 0;

	if (!record) {
		mPath = "snapshot/ss";
		save(&mNumber,mPath,false,1);
	}
	else {
		mPath = "record/";
		save(&mNumber,mPath,false,10000);
	}

}
void Snapshot::save() {
	save(&mNumber,mPath,true,1);
};

void Snapshot::save(int* number,const char* path,bool out,int seek) {
	MyString ssfile(256);
	for (int i=*number ; true ; i+=seek) {
		ssfile = "";
		char f[8];
		nString::itoa(f,i,5);

		ssfile << path << f << ".bmp";
		bool b=fileExist(ssfile.data());

		if (!b) {
			*number=i;
			break;
		}
	}
	if (out) {
		GameLib::Graphics::Manager m = GameLib::Graphics::Manager::instance();
		m.captureScreen(ssfile.data());
	}
};

////////////////////////////////////////////////////////////////////////////////////////
//最初のロード画面

NowLoading::NowLoading() {
	mTexture.load("src/system/nowloading",false,false);
	mCount = 0;
};

bool NowLoading::update() {
	BlendInfo bi;
	bi.mBlendElement.mFrontEnd = true;
	bi.setBlendMode(1);
	bi.mBlendElement.mDepthTest=false;
	bi.setZoom(
		(Random::instance()->getFloat(1.f)+0.5f),
		(Random::instance()->getFloat(1.f)+0.5f)
	);
	mTexture.draw(0,0,0,bi);
	//Draw::instance()->text(16,16,MyTexture::mLoadingNumber,0xffffffff,1);

	//if (mCount > 120) {
		//ロード完了
		if (MyTexture::mLoadingNumber==0) {
			ModelList::instance()->ready();
			return true;
		}
	//}

	++mCount;
	return false;
}


