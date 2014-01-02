#ifndef INCLUDED_NPSYSC_H
#define INCLUDED_NPSYSC_H

#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"
#include "GameLib/WindowCreator/WindowCreator.h"

#include "npdraw.h"
#include "npsound.h"


class Config {
public:
	unsigned int mKeyAssign[12];
	unsigned int mWindowSize;
	unsigned int mCurrentWindowSize;	//起動時のウィンドウサイズ
	bool mBGM;							//BGM使う？
	unsigned int mBGMVolume;
	unsigned int mSEVolume;
	int mSkip;				//こいつは無効

	bool mAntiAlias;		//ジャギ消し
	bool mTextureFilter;
	bool mVSync;
	bool mFullScreen;
	bool mPad;
	bool mFps;				//FPS表示
	bool mPixelShader;
	bool mOmit;
	//char mPlayerName[4];

	Config();
	~Config();

	void initialize();
	void update();		//設定を適用
	void load();
	void save() const;

	int windowWidth() const;
	int windowHeight() const;
};

class PrimaryConfig {
public:
	static Config mConfig;
};

//最初のロード画面
class NowLoading {
	MyTexture mTexture;
	int mCount;
public:
	NowLoading();
	bool update();
};

//スナップショット保存クラス
class Snapshot{
	const char* mPath;
	int mNumber;
public:
	Snapshot(bool record);
	void save();	//スナップショット番号の空きを探して保存
private:
	void save(int* number,const char* path,bool out,int seek);
};

//メインとなるクラス。
class Main {

public:
	static const bool mDebugMode=false;		//
	static const bool mDebugPrint=false;	//ポリゴン数表示など
	static const bool mDebugPlayer=false;	//プレイヤー無敵モード
	static const bool mDebugReport=false;	//デバッグ情報を書き出す
	static const bool mDebugBeta=false;		//動作確認
	static const bool mDebugDemo=false;		//会場でのリプレイリピート再生用
	static const bool mDebugRecord=false;	//動画録画用
	static const bool mTrialMode = false;	//体験版
	static const bool mImmortal = false;

private:
	Snapshot mSnapshot;
	NowLoading* mNowLoading;	//最初のロード画面
	int mLoadingCount;
	bool mRecordingNow;
	bool mTarbo;				//ターボスイッチ

public:
	int mCount;
	bool mQuit;


	//関数
	static Main* instance();
	static void create();
	static void destroy();
	bool update();
private:
	static Main* mInstance;
	Main();
	~Main();

};


#endif
