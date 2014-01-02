#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include <string.h>
#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"
#include "GameLib/WindowCreator/WindowCreator.h"

#include "npsysc.h"
#include "npfile.h"

//�N�����ݒ�
	void GameLib::Framework::configure( Configuration* conf ){
		//�R���t�B�O�t�@�C����ǂ݂���
		//Config newconfig;
		//PrimaryConfig::mConfig = newconfig;
		PrimaryConfig::mConfig.load();

		conf->enableAntiAlias( PrimaryConfig::mConfig.mAntiAlias );		//�W���M�[���y�����鏈���̗L��
		conf->enableVSync( PrimaryConfig::mConfig.mVSync );				//��������

		conf->setWidth(PrimaryConfig::mConfig.windowWidth());			//�E�B���h�E��
		conf->setHeight(PrimaryConfig::mConfig.windowHeight());			//�E�B���h�E����

		if (Main::mDebugMode) {
			conf->setTitle("SuperMurasa debugmode");
		}
		else {
			if (Main::mDebugBeta) {
				conf->setTitle("SuperMurasa ����m�F��0.02");
			}
			else {
				conf->setTitle("SuperMurasa ver1.00");
			}
		}

		enableFullScreen( PrimaryConfig::mConfig.mFullScreen);			//�N�����̃t���X�N���[���t���O�ݒ�
		conf->forbidFullScreen(false);									//�t���X�N���[���֎~

		///�h���b�O�A���h�h���b�v�L����
		//void enableDragAndDrop( bool = true );

		//�A�[�J�C�u�ݒ�
		if (true) {
			conf->setArchiveNumber( 1 );
			//conf->setArchiveName( 0, "spmpatch.dat" );
			conf->setArchiveName( 0, "spm.dat" );
			conf->setLoadMode( LOAD_DIRECT_FIRST );	
			//LOAD_ARCHIVE_FIRST,
			//LOAD_DIRECT_FIRST,
		//	conf->setLoadMode( LOAD_ARCHIVE_ONLY );	
			//LOAD_ARCHIVE_ONLY,//���ꂾ�ƃZ�[�u�f�[�^��ǂݍ��߂Ȃ�
		}

		///�R�}���h���C���I�v�V�������擾
		//const char* commandLineString() const;
		///HALT,ASSERT�ő��������ɗ�O�𓊂���@�\�̗L�����B���蕨�ɑ����R�[�h�͓�����Ȃ��A�Ƃ����l�̂��߂̂��́B
		//void enableThrowWhenHalt( bool = true );
		///�ǉ��X���b�h���ݒ�
		conf->setExtraThreadNumber( 1 );			//BGM�p�ɂЂƂ�
		GameLib::Framework::setFrameRate(60);
	}
	void GameLib::Framework::update(){
		if (Main::instance()==0) {
			Main::create();
			PrimaryConfig::mConfig.update();
		}

		if (!Main::instance()->update()) {
			requestEnd();
		};

		//��Еt��
		if ( isEndRequested() ){
			Main::destroy();
		}
	};

#endif
