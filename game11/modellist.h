#ifndef INCLUDED_MODELLIST_H
#define INCLUDED_MODELLIST_H

#include "npdraw.h"
#include "nml.h"
#include "npsound.h"
#include "npobject.h"

//class Sprite;
//class Object;
/////////////////////////////////////////////////////





class ModelList {

	class System {
		class Number{
			MyTexture mTexture;
			Model* mModel[10];
			Model* mMiss;
			int mWidth;
		public:
			Number();
			void create(Sprite*,int number,const GameLib::Math::Vector3& pos,unsigned int color,bool big,int height) const;
			void draw(int value,const GameLib::Math::Vector3& pos,unsigned int color) const;
		};

	public:
		MyTexture mCursor;
		MyTexture mFrame;
		MyTexture mRect;
		MyTexture mGauge;

		Number mNumber;

		System();
		void ready();
	};

/////////////////////////////////////////////////////
	class Chara{
	public:
		MyTexture mTexture;
		MyTexture mUFO;
		MyTexture mBoss;
		void ready();
	};


public:
	System mSystem;
	SoundSet mSoundSet;
	Chara mChara;
	MyTexture mBack;

	void ready();

	static void create();
	static void destroy();
	static ModelList* instance();
private:
	ModelList();
	~ModelList();
	static ModelList* mInstance;

};

/////////////////////////////////////////////////////



#endif
