
#include "GameLib/Input/Manager.h"
#include "GameLib/Input/Keyboard.h"
#include "GameLib/Input/Joystick.h"
#include "npinput.h"
#include "npsysc.h"


//キー入力系++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//キー配置
//0-3方向
//4-7基本 CXZA
//8　ターボ
//9　スクショ
//10
//11
//12
//13
//以降キーボードのみ
//14 ESC
//15-26 ファンクション
//ジョイスティックのキー配置...十字は固定らしい

int cKey::mFrame[27];		//押されているフレーム数
bool cKey::mOn[27];		//おされているかどうか
int cKey::mAssign[12];
int cKey::mCount=0;
bool cKey::mJoyStick=false;
int cKey::mJoyStickNumber=0;

//コンストラクタ(使われません)
cKey::cKey() {
	reset();
};
void cKey::reset() {

	mJoyStick=false;
	mJoyStickNumber=GameLib::Input::Manager::instance().joystickNumber();
	if (mJoyStickNumber) {
		mJoyStick = GameLib::Input::Manager::instance().joystick().isEnabled();
	}

	//デフォルトキー配置　十字キーは除く
	//mAssign[] = {0,1,2,3,4,5,6,7,8,9};
	mAssign[0] = 0;						//X
	mAssign[1] = 1;						//A
	mAssign[2] = 2;						//B
	mAssign[3] = 3;						//Y
	mAssign[4] = 4;						//ターボ
	mAssign[5] = 5;						//スクショ
	mAssign[6] = 6;
	mAssign[7] = 7;
	mAssign[8] = 8;
	mAssign[9] = 9;
	mAssign[10] = 10;					//L
	mAssign[11] = 11;					//R
};

//毎フレーム呼び出す　キー入力状況更新
void cKey::update() {
	for (int i=0 ; i<27 ; ++i) {
		mOn[i]=false ;
	}

	//リプレイ再生中の場合		リプレイが終わったらどうしよう？
	//if (mReplay==2) {
	//	if(cReplay::mIndex<cReplay::mLength) {
	//		for (int i=0 ; i<27 ; ++i) {
	//			 mOn[i]= cReplay::mOn[cReplay::mIndex][i];
	//		}
	//		++cReplay::mIndex;
	//	}
	//}
	//else {
	//ジョイスティックからの入力

	if (PrimaryConfig::mConfig.mPad) {
		if (mJoyStick) {
			if (GameLib::Input::Manager::instance().joystick().isEnabled()) {

				//上下左右
				mOn[0] = GameLib::Input::Manager::instance().joystick().isOn( GameLib::Input::Joystick::BUTTON_UP );
				mOn[1] = GameLib::Input::Manager::instance().joystick().isOn( GameLib::Input::Joystick::BUTTON_RIGHT );
				mOn[2] = GameLib::Input::Manager::instance().joystick().isOn( GameLib::Input::Joystick::BUTTON_DOWN );
				mOn[3] = GameLib::Input::Manager::instance().joystick().isOn( GameLib::Input::Joystick::BUTTON_LEFT );


				//アナログスティック非対応パッドの十字キーは、アナログで返ってくる
				int m=GameLib::Input::Manager::instance().joystick().analogNumber();
				if (m>1) {
					int x=GameLib::Input::Manager::instance().joystick().analog(0);
					int y=GameLib::Input::Manager::instance().joystick().analog(1);

					mOn[0] |= (y<16384);
					mOn[1] |= (x>(65536-16384));
					mOn[2] |= (y>(65536-16384));
					mOn[3] |= (x<16384);
				}

				//その他のキー
				for (int i=4 ; i<16 ;++i) {
					mOn[i] = GameLib::Input::Manager::instance().joystick().isOn( mAssign[i-4] );
				}
			}
		}
	}

	//キーボードからの入力
	mOn[0] |= GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_UP);
	mOn[1] |= GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_RIGHT);
	mOn[2] |= GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_DOWN);
	mOn[3] |= GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_LEFT);
	mOn[4] |= GameLib::Input::Manager::instance().keyboard().isOn('c');
	mOn[5] |= GameLib::Input::Manager::instance().keyboard().isOn('x');
	mOn[6] |= GameLib::Input::Manager::instance().keyboard().isOn('z');
	mOn[7] |= GameLib::Input::Manager::instance().keyboard().isOn('a');
	mOn[8] |= GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_CONTROL);
	mOn[9] |= GameLib::Input::Manager::instance().keyboard().isOn(' ');
	mOn[10]|= GameLib::Input::Manager::instance().keyboard().isOn('q');
	mOn[11]|= GameLib::Input::Manager::instance().keyboard().isOn('w');
	mOn[12]|= GameLib::Input::Manager::instance().keyboard().isOn('s');

	//パッドで押せないキー
	mOn[14] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_ESC);
	mOn[15] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F1);
	mOn[16] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F2);
	mOn[17] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F3);
	mOn[18] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F4);
	mOn[19] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F5);
	mOn[20] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F6);
	mOn[21] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F7);
	mOn[22] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F8);
	mOn[23] = GameLib::Input::Manager::instance().keyboard().isOn(GameLib::Input::Keyboard::KEY_F9);


	for(int i=0; i<27 ; ++i) {
		if (mOn[i]) {
			if (mFrame[i]<0) {
				mFrame[i]=0;
			}
			++mFrame[i];
		}
		else {
			if (mFrame[i]>0) {
				mFrame[i]=0;
			}
			--mFrame[i];
		}
	}

	//十字キーは片方だけしか押せないようにしておく
	if (mOn[0] && mOn[2]) {
		mOn[0] = mFrame[0] < mFrame[2];
		mOn[2] = mFrame[0] > mFrame[2];
	};
	if (mOn[1] && mOn[3]) {
		mOn[1] = mFrame[1] < mFrame[3];
		mOn[3] = mFrame[1] > mFrame[3];
	};

	//ためしにアナログスティック
	//実験結果
	//0 左スティックX方向 0-65535
	//1 左スティックY方向 0-65535
	//2 右スティック
	//5 右スティック
	//int m=GameLib::Input::Manager::instance().joystick().analogNumber();
	//Draw::instance()->text(90,-32+32,m,0xffffffff,2);
	//for (int i=0 ; i<m ;++i) {
	//	int r=GameLib::Input::Manager::instance().joystick().analog(i);
	//	Draw::instance()->text(90,i*32+32,r,0xffffffff,2);
	//}
	++mCount;
};

bool cKey::isOn(int i) {
	return mOn[i];
};

int cKey::frame(int i) {
	if (mOn[i]) {
		return mFrame[i];
	}
	else {
		if (mFrame[i]<0) {
			return mFrame[i];
		}
		return 0;
	}
};
bool cKey::trigger(int i) {
	return mOn[i] && mFrame[i]==1;

}

bool cKey::get(int i,int pulse) {
	bool r=false;
	if (mOn[i]) {
		if (mFrame[i]>=32) {
			r=((mFrame[i]-32)%pulse==0);
		}
		else {
			r=(mFrame[i]==1);
		}
	}
	return r;
};

void cKey::assigning() {
	//パッドの十字キー以外のキーを無効にする。ESC・ファンクションキーはそのまま。
	for (int i=4 ; i<14 ; ++i) {
		mOn[i]=false;
		mFrame[i]=0;
	};

};
int cKey::getPadOne() {
	int result=-1;
	if (PrimaryConfig::mConfig.mPad) {
		if (mJoyStick) {
			if (GameLib::Input::Manager::instance().joystick().isEnabled()) {
				for (int i=0 ; i<10 ;++i) {
					if (GameLib::Input::Manager::instance().joystick().isOn( i )) {
						result=i;
						break;
					}
				}
			}
		}
	}

	return result;
};
int cKey::joyStickNumber() {
	return mJoyStickNumber;
}
//リプレイ開始
//void cKey::replay() {
//	mReplay=2;
//	cReplay::mIndex=0;
//}
//録画開始
//void cKey::record() {
//	mReplay=1;
//	cReplay::mLength=0;
//};

//int cReplay::mLength=0;
//int cReplay::mIndex=0;
//bool cReplay::mOn[mMax][27];
