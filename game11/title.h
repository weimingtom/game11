#ifndef INCLUDED_TITLE_H
#define INCLUDED_TITLE_H


#include "interface.h"
#include "config.h"
#include "gamedata.h"
#include "savedata.h"


class Title{
	MyTexture mLogo;
	Window mWindow;
	sConfig* mConfig;

	int mCount;
public:
	Title();
	~Title();
	int update(GameData* result);
};


#endif
