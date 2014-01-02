#include "npseq.h"
#include "npinput.h"
#include "npsound.h"
#include "npsysc.h"
#include "npfile.h"
#include "stage.h"
extern cMain gMain;

sTest::sTest(){
	mCount=0;
}
//char* skilldata;

//テスト用の処理。色々ためしてみよう。
void sTest::update(cMain* parent){

	if (mCount==0) {
	//	gMain.mSound.load(0,"charara.wav");
		mStage.mMap.load("test");
//		nSave("a.txt","てすとてすとてすと");
//		nFile file;
//		file.load("skill.csv");
//		char* skilldata = new char[64];
		gSound.mBGM.change(1);
	}
	if (mCount % 240 == 119) {
		gSound.mBGM.change(1);
//		gMain.mSound.play(0,false);
	}
	if (mCount % 240 == 239) {
		gSound.mBGM.change(2);
//		gMain.mSound.play(0,false);
	}
	mStage.mMap.draw();

	gDraw.setBlendInfo(blendInfo());
	gDraw.setFrontEnd(true);
	gDraw.setTexture(1);
	gDraw.copyRect(0,0,31,31,
		320,240,1,320,320,
		0,0,0,0xffffffff);
	gDraw.draw();


	if(gKey.isOn(0)) {
		gCursor.setPosition(320,16);
	}
	if(gKey.isOn(1)) {
		gCursor.setPosition(480,240);
	}
	if(gKey.isOn(2)) {
		gCursor.setPosition(320,400);
	}
	if(gKey.isOn(3)) {
		gCursor.setPosition(120,240);
	}
	gCursor.draw();

	//gMain.mWindow.text(16,16,skilldata,0xffffffff,0);
	gDraw.text(16,16,"てすてすてすてす",0xffffffff,0);
	++mCount;
};

