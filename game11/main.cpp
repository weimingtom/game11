#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include <string.h>
#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"
#include "GameLib/WindowCreator/WindowCreator.h"

#include "npsysc.h"
#include "npfile.h"

//起動時設定
	void GameLib::Framework::configure( Configuration* conf ){
		//コンフィグファイルを読みこむ
		//Config newconfig;
		//PrimaryConfig::mConfig = newconfig;
		PrimaryConfig::mConfig.load();

		conf->enableAntiAlias( PrimaryConfig::mConfig.mAntiAlias );		//ジャギーを軽減する処理の有無
		conf->enableVSync( PrimaryConfig::mConfig.mVSync );				//垂直同期

		conf->setWidth(PrimaryConfig::mConfig.windowWidth());			//ウィンドウ幅
		conf->setHeight(PrimaryConfig::mConfig.windowHeight());			//ウィンドウ高さ

		if (Main::mDebugMode) {
			conf->setTitle("SuperMurasa debugmode");
		}
		else {
			if (Main::mDebugBeta) {
				conf->setTitle("SuperMurasa 動作確認版0.02");
			}
			else {
				conf->setTitle("SuperMurasa ver1.00");
			}
		}

		enableFullScreen( PrimaryConfig::mConfig.mFullScreen);			//起動時のフルスクリーンフラグ設定
		conf->forbidFullScreen(false);									//フルスクリーン禁止

		///ドラッグアンドドロップ有効化
		//void enableDragAndDrop( bool = true );

		//アーカイブ設定
		if (true) {
			conf->setArchiveNumber( 1 );
			//conf->setArchiveName( 0, "spmpatch.dat" );
			conf->setArchiveName( 0, "spm.dat" );
			conf->setLoadMode( LOAD_DIRECT_FIRST );	
			//LOAD_ARCHIVE_FIRST,
			//LOAD_DIRECT_FIRST,
		//	conf->setLoadMode( LOAD_ARCHIVE_ONLY );	
			//LOAD_ARCHIVE_ONLY,//これだとセーブデータを読み込めない
		}

		///コマンドラインオプションを取得
		//const char* commandLineString() const;
		///HALT,ASSERTで即死せずに例外を投げる機能の有効化。売り物に即死コードは入れられない、という人のためのもの。
		//void enableThrowWhenHalt( bool = true );
		///追加スレッド数設定
		conf->setExtraThreadNumber( 1 );			//BGM用にひとつ
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

		//後片付け
		if ( isEndRequested() ){
			Main::destroy();
		}
	};

#endif
