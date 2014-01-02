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
	unsigned int mCurrentWindowSize;	//�N�����̃E�B���h�E�T�C�Y
	bool mBGM;							//BGM�g���H
	unsigned int mBGMVolume;
	unsigned int mSEVolume;
	int mSkip;				//�����͖���

	bool mAntiAlias;		//�W���M����
	bool mTextureFilter;
	bool mVSync;
	bool mFullScreen;
	bool mPad;
	bool mFps;				//FPS�\��
	bool mPixelShader;
	bool mOmit;
	//char mPlayerName[4];

	Config();
	~Config();

	void initialize();
	void update();		//�ݒ��K�p
	void load();
	void save() const;

	int windowWidth() const;
	int windowHeight() const;
};

class PrimaryConfig {
public:
	static Config mConfig;
};

//�ŏ��̃��[�h���
class NowLoading {
	MyTexture mTexture;
	int mCount;
public:
	NowLoading();
	bool update();
};

//�X�i�b�v�V���b�g�ۑ��N���X
class Snapshot{
	const char* mPath;
	int mNumber;
public:
	Snapshot(bool record);
	void save();	//�X�i�b�v�V���b�g�ԍ��̋󂫂�T���ĕۑ�
private:
	void save(int* number,const char* path,bool out,int seek);
};

//���C���ƂȂ�N���X�B
class Main {

public:
	static const bool mDebugMode=false;		//
	static const bool mDebugPrint=false;	//�|���S�����\���Ȃ�
	static const bool mDebugPlayer=false;	//�v���C���[���G���[�h
	static const bool mDebugReport=false;	//�f�o�b�O���������o��
	static const bool mDebugBeta=false;		//����m�F
	static const bool mDebugDemo=false;		//���ł̃��v���C���s�[�g�Đ��p
	static const bool mDebugRecord=false;	//����^��p
	static const bool mTrialMode = false;	//�̌���
	static const bool mImmortal = false;

private:
	Snapshot mSnapshot;
	NowLoading* mNowLoading;	//�ŏ��̃��[�h���
	int mLoadingCount;
	bool mRecordingNow;
	bool mTarbo;				//�^�[�{�X�C�b�`

public:
	int mCount;
	bool mQuit;


	//�֐�
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
