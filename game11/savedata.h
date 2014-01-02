#ifndef INCLUDED_SAVEDATA_H
#define INCLUDED_SAVEDATA_H

#include "nml.h"
#include "gamedata.h"

class SaveData{
	static const bool mCode = true;		//暗号化する？
	static const int mVersion = 0;		//現バージョン
public:
	NML mNML;							//こいつがメイン
	bool mError;

	SaveData();
	~SaveData();
	void release();
	bool load(int n);
	void save(int n);

	void read(const GameData* src);
	void write(GameData* result);

};

class SaveList{
	Window mWindow;
public:
	static const int mMax = 24;
	SaveList();
	void refresh();
	int update(const GameData*);	//ヌルをわたすとロードモード
	bool loadGameData(GameData*)const;


};
	void saveFileName(char* result,int i);
	bool existSaveFile(int n);
	bool existSaveFile();


#endif

