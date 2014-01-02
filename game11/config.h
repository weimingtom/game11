#ifndef INCLUDED_GAME9SEQ_CONFIG_H
#define INCLUDED_GAME9SEQ_CONFIG_H

#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"

#include "interface.h"
#include "npsysc.h"



class sConfig {
	//キー設定
	class Assign {
		Window mWindow;
	public:
		Assign(const unsigned int* assign);
		bool update(unsigned int* assign);
		void refresh(const unsigned int* assign);
	};

	//プレイヤー名入力画面
	class InputPlayerName{
		Cursor mCursor;
		int mPosition;
		int mLength;
	
		Window mDialog;
		Window mDialog2;

	public:
		char mResult[4];

		InputPlayerName(const char* name = 0);
		bool update();
	};

	//各パラメータ
	class Parameter{
		MyString mName;		//表示パラメータ名
		MyString* mState;
		int mNumber;
		bool mReboot;		//再起動いる？

		unsigned int* mInt;		//intだけじゃないんだよなあ
		bool* mBool;
	public:
		Parameter();
		~Parameter();
		void setName(const char*);
		void setReboot();
		void setBool();			//無効/有効
		void setInt(int n);		//1-n
		void set(const char*);	//カンマで区切る
		void setValue(unsigned int*);	//
		void setValue(bool*);	//

		bool update();			//切り替わったらtrue
		void draw(int x,int y) const;
	};

public:
	bool mSwitched;	//変更した？
	int mCount;
	int mCursorY;
	Cursor mCursor;
	Config mConfig;

	Parameter mParameter[12];

	Assign *mAssign;
	InputPlayerName* mInputPlayerName;

	sConfig();
	~sConfig();
	bool update();
	
};



#endif
