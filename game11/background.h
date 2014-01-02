#ifndef INCLUDED_BACKGROUND_H
#define INCLUDED_BACKGROUND_H

#include "npdraw.h"

class cStage;

//îwåiï\é¶
class BackGround {
public:
	MyTexture mTexture;
	Batch* mBatch;
	int mWidth;
	int mHeight;
	virtual BackGround::~BackGround();
	virtual void resize(int w,int h)=0;
	virtual void draw(float light)=0;
	virtual void effect(cStage*)=0;
};
/////////////////////////////////////////////////////


#endif

