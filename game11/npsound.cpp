#include  <time.h>

#include "GameLib/Threading/Functions.h"
#include "GameLib/Framework.h"
#include "GameLib/Sound/Manager.h"
#include "GameLib/Sound/Player.h"
#include "GameLib/Sound/Wave.h"
#include "vorbis/vorbisfile.h"

#include "npsound.h"
#include "npstring.h"
#include "npsysc.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BGM* BGM::mInstance=0;

BGM::~BGM() {
}
void BGM::create() {
	if (mInstance==0) {
		if (PrimaryConfig::mConfig.mBGM) {
			mInstance= new OggBGM;		//Dummy�ɂ���ƋȂ𗬂��Ȃ��@OggBGM�ŋȂ𗬂�
		}
		else {
			mInstance= new DummyBGM;	//Dummy�ɂ���ƋȂ𗬂��Ȃ��@OggBGM�ŋȂ𗬂�
		}
	}
}
void BGM::destroy(){
	ASSERT(mInstance);
	delete mInstance;
	mInstance=0;
};
BGM* BGM::instance() {
	return mInstance;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DummyBGM::change(int i) {
	;
}
void DummyBGM::change(const char*) {
	;
}
void DummyBGM::changeVolume(int i) {
	;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OggBGM::OggBGM() {
	//���X�g����
	mList.load("src/bgm/bgm.txt");
	mThread.start();
};
OggBGM::~OggBGM() {
	mThread.destroy();
	mThread.wait();	//����ŏI�������Ă�낤
};

void OggBGM::change(const char* name) {
	if (name == 0) {
		Tag* tag = 0;
		change(tag);
	}
	else {
		change(mList.root()->child(name));
	}
}
void OggBGM::change(int i) {
	change(mList.root()->child(i));
}

void OggBGM::change(Tag* bgm) {
	mThread.change(bgm);
}
void OggBGM::changeVolume(int i) {
	mThread.changeVolume(i);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OggBGM::Thread::Thread() {
	mQuit = false;
	mCurrent=0;
	mOrder = 0;
	mNext=0;
	mLoad=0;
	mMemory=0;

	mFade=false;

	mMutex = GameLib::Threading::Mutex::create();
	mVolume=-99;	//�ŏ��͖����ɂ��Ă�����
}

OggBGM::Thread::~Thread() {
	if (mMemory!=0) {
		mOgg.clear();
		mPlayer.release();
	}
};

void OggBGM::Thread::changeVolume(int i) {
	//Mutex���g���ĕύX
	mMutex.lock();
	mVolume = i;
	mMutex.unlock();
}

void OggBGM::Thread::destroy() {
	//Mutex���g���ĕύX
	mMutex.lock();
	mQuit = true;
	mMutex.unlock();
}

void OggBGM::Thread::change(Tag* bgm){
	//Mutex���g���ĕύX
	mMutex.lock();
	mOrder = bgm;
	mMutex.unlock();
}
void OggBGM::Thread::operator()() {
	while(!mQuit) {
		update();
		GameLib::Threading::sleep(1);
	}
}

void OggBGM::Thread::update() {
	//�Ȃ̕ύX�`�F�b�N
	//�J�ڏ��
	//mCurrent���Đ���
	//mNext���������mCurrent���t�F�[�h��
	//mCurrent���J���AmNext��ǂݍ���
	//mNext���Đ�


	//BGM�ύX�v������
	mMutex.lock();
	if (mNext != mOrder) {
		mNext = mOrder;

		if (mCurrent) {
			mFade=true;
		}
	}
	mMutex.unlock();

	//�t�@�C�����[�h���J�n
	if (mLoad != mNext) {
		if (mNext != 0) {
			mFile.release();
			
			MyString path(64);
			path << "src/bgm/";
			path << mNext->attribute("file")->getValue();
			path << ".ogg";
			mFile.load(path.data(),true);

			mLoad=mNext;
		}
	}

	//�t�F�[�h�A�E�g�@clock_t���g��
	if (mFade) {
		clock_t start = clock();

		while(1) {
			clock_t current = clock();
			float s = 1.f * (current - start) / CLOCKS_PER_SEC;

			int a=mVolume - s*100;
			if (a<-100) {
				a=-100;
				break;
			}
			mPlayer.setVolume(a);
			write();

			GameLib::Threading::sleep(1);
		}

		//�t�F�[�h�I��
		mFade = false;
		mCurrent=0;
		mPlayer.stop();
	}

	//�ǂݍ��񂾃f�[�^���������ɑ���
	if (mLoad) {
		if (mMemory != mLoad) {
			//�t�@�C�����[�h�����[�v�ő҂�
			while(!mFile.isFinished()){
				ASSERT(!mFile.isError());
			}

			mPlayer.release();
			mOgg.clear();
			mMemory = 0;

			mOgg.setSound(mFile);
			//�R�s�[��������܂ł����Ń��[�v�ő҂�
			while(true) {
				int result=mOgg.copy(mFile);
				ASSERT(result!=2);
				if (result == 1) {
					break;
				}
			}

			//vorbis_info* ovInfo=info(&mOVF,-1);
			vorbis_info* ovInfo=ov_info(&(mOgg.ovf_),-1);

			mPlayer=GameLib::Sound::Player::create(16,mPlayerBufferSize,
				ovInfo->rate,
				ovInfo->channels);
			mMemory = mLoad;
		}
	}

	//���̋Ȃ�
	if (mNext != mCurrent) {
		if (mNext) {
			//�������ɂ���Ȃ��Đ�����

			//���̕�����p��
			ov_time_seek(&(mOgg.ovf_),0.0);
			mWritePos=0;
			while (mWritePos<(mPlayerBufferSize/2)) {
				int pos=0;
				long readSize=ov_read(&(mOgg.ovf_),mBuffer,mBufferSize,0,2,1,&pos);
				bool suc=mPlayer.write(mWritePos,mBuffer,readSize);

				ASSERT (suc);
				mWritePos+=readSize;
			}

			mPlayer.play();
		}
		mCurrent=mNext;
	}

	//�Đ�����
	if (mCurrent!=0) {
		//���ʂ����������Đݒ肷��
		mPlayer.setVolume(mVolume);
		write();
	}
}
void OggBGM::Thread::write() {
	int playPos = mPlayer.position();
	while (((mWritePos>playPos) && (mWritePos-playPos < (mPlayerBufferSize/2)))
		|| ((mWritePos<playPos) && (playPos-mWritePos > (mPlayerBufferSize/2)))
		)
	{
		int maxBuffer;
		if (mPlayerBufferSize < (mWritePos+mBufferSize)) {
			maxBuffer=mPlayerBufferSize-mWritePos;
		}
		else {
			maxBuffer=mBufferSize;
		}

		int pos;
		long readSize=ov_read(&(mOgg.ovf_),mBuffer,maxBuffer,0,2,1,&pos);
		//long readSize=MyOgg::read(&mBuffer,mBufferSize,0,&pos);

		//���[�v
		if (readSize<1) {
			float lp = mMemory->attribute("loop")->getFloatValue();	//���[�v�ʒu�𓾂�
			ov_time_seek(&(mOgg.ovf_),lp);
			continue;
		}

		bool suc=mPlayer.write(mWritePos,mBuffer,readSize);

		ASSERT (suc);	//�������݃~�X���Ă����ȂȂ��B�E�B���h�E���h���b�O����Ɗ�Ȃ��悤���B���Ԃ�F10�ł��B
		if (suc) {
			mWritePos+=readSize;
			if (mWritePos>=mPlayerBufferSize) {
				mWritePos=0;
			}
		}
	}

};
/////////////////////////////////////////////////////////////////////////////
SoundSet::SoundSet() {
	for (int i=0 ; i<mMax ; ++i) {
		mSound [i] = 0;
	}
}
SoundSet::~SoundSet() {
	for (int i=0 ; i<mMax ; ++i) {
		if (mSound[i]) {
			delete mSound[i];
			mSound[i] = 0;
		}
	}
};

Sound* SoundSet::sound(const char* name)const {
	Sound* result = 0;
	for (int i=0 ; i<mMax ; ++i) {
		if (mSound[i]->mName == name) {
			result = mSound[i];
			break;
		}
	}
	return result;
};
void SoundSet::play(const char* name,bool loop,bool overwrite)const {
	sound(name)->play(loop,overwrite);
}
void SoundSet::play(int i,bool loop,bool overwrite) const {
	mSound[i]->play(loop,overwrite);
}


void SoundSet::load(const char* path) {
	NML nml;
	nml.load(path);

	int c = 0;

	for (int i=0 ; i<nml.root()->getChild() ; ++i) {
		Tag* tag = nml.root()->child(i);

		MyString folder;
		folder = tag->getName();

		MyString tmp(64);
		for (int j=0 ; j< tag->getAttribute() ; ++j) {
			mSound[c] = new Sound;

			mSound[c]->mName = tag->attribute(j)->getName();

			tmp = "";
			tmp << folder.data();
			tmp << "/";
			tmp << tag->attribute(j)->getValue();
			tmp << ".wav";
			mSound[c]->load(tmp.data());

			++c;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Sound::mVolume=0;

Sound::Sound() {

}
Sound::~Sound() {
	mWave.release();
	mPlayer.release();
};

//�������[�h
void Sound::load(const char* path ) {
	mWave=GameLib::Sound::Wave::create(path);
};

//����炷
void Sound::play(bool loopflag,bool overwrite) {
	if (mWave.isReady()) {
		if ( mWave.isError() ){
			ASSERT("wav error");//�G���[���`�F�b�N���悤�ˁB�t�@�C����ǂݑ��˂Ă��~�܂�Ȃ��̂ŁB
		}
		else {
			if (!mPlayer) {
				mPlayer = GameLib::Sound::Player::create(mWave);
			}
			if (overwrite || !mPlayer.isPlaying()) {
				mPlayer.setVolume(mVolume);
				mPlayer.play(loopflag);
			}
		}
	}
	
};
