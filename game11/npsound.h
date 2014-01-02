#ifndef INCLUDED_NPSOUND_H
#define INCLUDED_NPSOUND_H

#include "GameLib/Sound/Manager.h"
#include "GameLib/Sound/Player.h"
#include "GameLib/Sound/Wave.h"
#include "GameLib/Threading/Thread.h"
#include "GameLib/Threading/Mutex.h"

#include "ogginmemory.h"
#include "vorbis/vorbisfile.h"

#include "npfile.h"
#include "npstring.h"
#include "nml.h"

class BGM {
public:
	static BGM* instance();
	static void create();
	static void destroy();

	virtual void change(int)=0;
	virtual void change(const char*)=0;

	virtual void changeVolume(int) = 0;

	virtual ~BGM();
private:
	static BGM* mInstance;

};

class DummyBGM : public BGM {
	void change(int);
	void change(const char*);
	void changeVolume(int );
};

class OggBGM :public BGM{
public:
	//スレッド部分
	class Thread : public ::GameLib::Threading::Thread {
		bool mQuit;				//終了してねサイン
		int mVolume;

		Tag* mOrder;		//変更要請曲
		Tag* mCurrent;	//現在の曲
		Tag* mNext;		//変更先の曲
		Tag* mLoad;		//mFileに入ってる曲
		Tag* mMemory;		//Oggに送られている局

		bool mFade;

		//一度に読む量
		static const int mBufferSize=4096;
		char mBuffer[mBufferSize];
		OggDecoderInMemory mOgg;

		GameLib::Sound::Player mPlayer;
		int mWritePos;
		static const int mPlayerBufferSize=128*1024;	//実際に鳴らす部分

		nFile mFile;									//生のogg

		GameLib::Threading::Mutex mMutex;
	public:
		Thread();
		~Thread();
		void changeVolume(int);
		void change(Tag*);
		void destroy();

		void operator()();	//スレッドループ

	private:
		void update();
		void write();
	};

	//曲リスト
	NML mList;
	Thread mThread;

	OggBGM();
	~OggBGM();
	void change(int);
	void change(const char*);
	void changeVolume(int);
private:
	void change(Tag*);	//止める場合どーすんのさ？

};


class Sound {
public:
	static int mVolume;

	MyString mName;
	GameLib::Sound::Wave mWave;
	GameLib::Sound::Player mPlayer;

	Sound();
	~Sound();

	void load(const char*);
	void play(bool loopflag=false,bool overwrite=true);
};

//効果音データリスト
class SoundSet {
	static const int mMax = 64;
	Sound* mSound[mMax];

public:
	SoundSet();
	~SoundSet();
	Sound* sound(const char*)const;
	void play(const char*,bool loop=false,bool overwrote = true)const;
	void play(int i = 0,bool loop=false,bool overwrote = true)const;
	void load(const char* path);
};


#endif
