#include "npfile.h"

#include "npmath.h"
#include "npsysc.h"

#include  <time.h>
#include "savedata.h"

#include "interface.h"

//////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////

SaveData::SaveData() {
	mError=true;
}
SaveData::~SaveData() {
}

void SaveData::release() {
	mNML.release();
	mError=true;
};

bool SaveData::load(int n) {
	release();

	char path[256];
	saveFileName(path,n);
	nFile filedata;
	filedata.load(path,false,mCode);

	mError=true;
	if (filedata.size()>0) {
		//読み込めた。
		if (mNML.read(filedata.data(),false)) {
			mError = false;
		}
	}
	return !mError;
}

//保存
void SaveData::save(int n) {
	//文字列を作らんことには始まらない

	MyString str;
	mNML.write(&str);

	//セーブするよ
	char path[256];
	saveFileName(path,n);

	nSave(path,str.data(),str.size()+1,mCode && (n<255));
}

void SaveData::write(GameData* result){
	//GameDataに書き出す処理。大変である。
	result->reset(0);

	//バージョンチェック
	int ver = mNML.root()->child("head")->attribute("version")->getIntValue();

	//互換性のないバージョンなら、コンバートしてやる分岐が要る。
	if (ver == mVersion) {
	}
	else {
		ASSERT(0);
	}
};


void SaveData::read(const GameData* src) {
	mNML.release();

	//まずヘッダをつくる
	Tag* head = mNML.root()->addChild("head");
	//ヘッダには日付やらバージョン情報を入れるのだ
	time_t t;
	time(&t);
	struct tm  *local;
	local = localtime(&t);

	head->addAttribute("year",local->tm_year +1900);
	head->addAttribute("month",local->tm_mon  +1);
	head->addAttribute("day",local->tm_mday);
	head->addAttribute("hour",local->tm_hour);
	head->addAttribute("minute",local->tm_min);
	head->addAttribute("version",mVersion);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SaveList::SaveList() {

	mWindow.position(-320,240);
	refresh();
}
void SaveList::refresh() {
	mWindow.reset();

	int cur = 0;
	int curv = -1;

	SaveData sav;
	MyString tmp(256);
	for (int i=0 ; i<mMax ; ++i) {
		tmp = "";
		tmp << i;
		tmp.space(4);

		if (existSaveFile(i)) {
			if (sav.load(i)) {
				//日時・プレイ長さ・キャプション
				int year	= sav.mNML.root()->child("head")->attribute("year")->getIntValue();
				int month	= sav.mNML.root()->child("head")->attribute("month")->getIntValue();
				int day		= sav.mNML.root()->child("head")->attribute("day")->getIntValue();
				int hour	= sav.mNML.root()->child("head")->attribute("hour")->getIntValue();
				int minute	= sav.mNML.root()->child("head")->attribute("minute")->getIntValue();

				//新しいのを選ぶnoda
				int v = ((((year * 12) + month) * 31 + day) * 24 + hour) * 60 + minute;
				if (curv < 0 ||
					curv < v) {
					cur = i;
					curv = v;
				}

				tmp << year << "/";
				tmp << month << "/";
				tmp << day << " ";

				tmp.space(16);

				double t = sav.mNML.root()->child("system")->attribute("count")->getIntValue(1);
				t *= 0xffffff;
				t += sav.mNML.root()->child("system")->attribute("count")->getIntValue(0);
				int h = t/60/60/60;
				int m = static_cast<int>(t/60/60)%60;

				tmp << "play " << h << "時間 " << m << "分  ";
				tmp.space(34);

				if (sav.mNML.root()->child("system")->get("clearCount",false)) {
					tmp << "★";
				}
				tmp.space(38);
				tmp << sav.mNML.root()->child("system")->attribute("caption")->getValue();

			}
			else {
				tmp << "無効なファイル";
			}
		}
		else {
			tmp << "------";
		}
		mWindow.addChoice(tmp.data(),i,i<24);
	}
	mWindow.focus(cur);

};

int SaveList::update(const GameData* gamedata) {
	mWindow.draw();

	int r=mWindow.update();
	if (r == 1) {
		//セーブ
		if (gamedata) {
			SaveData sd;
			sd.read(gamedata);
			sd.save(mWindow.getParameter());
			if (Main::mDebugMode) {
				sd.save(256);
			}
			refresh();

			return 0;
		}
		else {
			//ロード。ファイルがあるか確認しよう
			if (existSaveFile(mWindow.getParameter())) {
				return 1;
			}
		}
	}
	if (r == 2) {
		return 2;
	}
	return 0;
};

bool SaveList::loadGameData(GameData* result)const {

	SaveData sd;
	if (sd.load(mWindow.getParameter())) {
		sd.write(result);
		return true;
	}
	else {
		return false;
	}

};


/////////////////////////////////////////////////////////////////////////////////////////
void saveFileName(char* result,int i) {
	MyString filename(256);
	filename << "savedata/" << i << ".sav";

	for (int w=0 ; w<filename.size() ; ++w) {
		result[w]=filename.data(w);
		result[w+1]=0;
	}
};

bool existSaveFile(int n) {
	char file[256];
	saveFileName (file,n);
	return fileExist(file);
}

bool existSaveFile() {
	bool result=false;
	for (int i=0 ; i<SaveList::mMax ; ++i) {
		if (existSaveFile(i)) {
			result=true;
			break;
		};
	}
	return result;
}

