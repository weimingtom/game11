
#ifndef INCLUDED_GAME9SEQ_CHAPTERMENU_H
#define INCLUDED_GAME9SEQ_CHAPTERMENU_H

#include "interface.h"
#include "savelist.h"
//#include "replaymenu.h"

class cGameData;


class ChapterList {
	MyString mString;
	int getLine(int i);
public:
	ChapterList();
	void getName(char* result,int chapter);
	void getFile(char* result,int chapter);
	int getStart(int chapter);
};


class ChapterMenu {
	char mChapterTitle[256];
public:
	SaveMenu* mSaveMenu;

	Dialog mDialog;
	Dialog mTitle;
	Dialog mLog;
	ChapterList mChapterList;
	SaveData* mSaveData;

	ChapterMenu(SaveData*);
	~ChapterMenu();

	bool update();
	void draw();
};


#endif


