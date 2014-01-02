#ifndef INCLUDED_GAME9SEQ_CONFIG_H
#define INCLUDED_GAME9SEQ_CONFIG_H

#include "GameLib/Framework.h"
#include "GameLib/DebugScreen.h"

#include "interface.h"
#include "npsysc.h"



class sConfig {
	//�L�[�ݒ�
	class Assign {
		Window mWindow;
	public:
		Assign(const unsigned int* assign);
		bool update(unsigned int* assign);
		void refresh(const unsigned int* assign);
	};

	//�v���C���[�����͉��
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

	//�e�p�����[�^
	class Parameter{
		MyString mName;		//�\���p�����[�^��
		MyString* mState;
		int mNumber;
		bool mReboot;		//�ċN������H

		unsigned int* mInt;		//int��������Ȃ��񂾂�Ȃ�
		bool* mBool;
	public:
		Parameter();
		~Parameter();
		void setName(const char*);
		void setReboot();
		void setBool();			//����/�L��
		void setInt(int n);		//1-n
		void set(const char*);	//�J���}�ŋ�؂�
		void setValue(unsigned int*);	//
		void setValue(bool*);	//

		bool update();			//�؂�ւ������true
		void draw(int x,int y) const;
	};

public:
	bool mSwitched;	//�ύX�����H
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
