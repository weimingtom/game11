#ifndef INCLUDED_GAME11_ENDING_H
#define INCLUDED_GAME11_ENDING_H
#include "gamedata.h"

class Ending{
	class Cast{
		MyString mName;
		Model* mModel;
		int mCount;
	public:
		Cast(const char* name,Model*);
		Cast(const char*);
		int update();
	};

	int mCount;
	List<Cast> mCast;
	MyTexture mCGA;
	MyTexture mCGB;
public:

	Ending();
	int update(GameData*);
};

#endif
