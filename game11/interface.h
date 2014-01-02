//draw���ЂƂ����ȉ摜�@�\
#ifndef INCLUDED_NPINTERFACE_H
#define INCLUDED_NPINTERFACE_H

#include "npdraw.h"
#include "npinput.h"
#include "npstring.h"
#include "modellist.h"

class GameData;


//�t���[���N���X
class Frame {
public:
	static const int mWidth[3];
	static const int mHeight[3];

	static void draw(int x,int y,int w,int h);	
};
//���𔼓����`��B�t���[���̕ω��n�Ƃ������˂�
class Rect{
public:
	static void draw(int x,int y,int w,int h,unsigned int color = 0x80ffffff);
};

//�Q�[�W�N���X
class Gauge{
	static void draw(float value,unsigned int color1,unsigned int color2,const GameLib::Math::Vector3&,BlendInfo);
public:
	static void draw(float value,unsigned int color,unsigned int color2,int x,int y,int width,int height,float angle=0.f);
};

//�K���ɕ��������Ă����N���X
class Message{
	static int mX;
	static int mY;
	static int mSize;
	static unsigned int mColor;
	Message();
public:
	static void position(int x,int y);
	static void color(unsigned int );
	static void size(int );
	static int getHeight();		//��s�̃T�C�Y�𓾂�
	static void draw(const char*);
	static void draw(int );
};

class WallPaper{
public:
	static void draw();
	static void white();
	static void black();
};


//////////////////////////////////////////////////////////////////////////////

class Cursor{
public:
	int mX,mY,mZ;					//�\�����W
	int mAimX,mAimY;				//�ړI���W
	bool mReset;
	int mType;

	Cursor();
	void setPosition(int x,int y);
	void draw();
	void update();
};
//////////////////////////////////////////////////////////////////////////////

static const unsigned int DefaultTextColor = 0xffffffff;

class Widget;
//�E�B���h�E�N���X
class Window{
	static const int mPaddingX=0;
	static const int mPaddingY=0;
	static const int mCaptionHeight = 16;

	static const int mMax = 257;
	Widget* mWidget[mMax];
	int mCount;
	int mWidth,mHeight;					//�t���[���̃T�C�Y
	int mX,mY;							//�\���ʒu�@���S���W�ł����Ă�
	int mDatum;							//0�@���S��@1����
	int mScroll;						//�X�N���[����
	int mScrollHeight;					//�X�N���[���̏c��
	int mLine;							//��y�[�W������̍s��
	Cursor mCursor;
	int mIndex;

	MyString mCaption;
	bool mFocus;						//�J�[�\���\������H
public:

	Window();
	~Window();
	int update();									//1����@2�L�����Z��

	void release();
	void reset();			//���g���X�V����O�Ɏg���Brelease���Ə������������Ȃ��p�����[�^������̂ŁB
	void initialize();		//�J�[�\���A�X�N���[���ʒu��������
	void focus(int i);

	int add(const char*,unsigned int color,int size,int x,int y,bool resize,bool select=false);						//������ǉ�
	int add(int ,unsigned int color,int size,int x,int y,bool resize,bool select=false);							//������ǉ��i�����j
	int addChoice(const char*,int parameter,bool resize = true,unsigned int color = DefaultTextColor,int size = 1);	//������ǉ��B���񂽂񏈗��p�B���X�g�͂���ňꔭ�I
	int addLine(const char*,unsigned int color = DefaultTextColor,int size = 1,bool resize=true);					//������ǉ��B���񂽂񏈗��p�B���X�g�͂���ňꔭ�I

	void set(int i,const char*,bool resize=false);	//�������ύX
	void set(int i,int param,bool resize=false);	//�������ύX�i����

	int add(Batch*,BlendInfo&,int x,int y,bool select);
	void draw(bool cursor=true,int mark=-1);

	int add(Widget*,int x,int y,bool resize,bool select);

	void resize(int i);
	void resize(int width,int height);
	void resizeScroll(int i);

	void position(int x,int y);							//������W���w��
	void centerPosition(int x,int y);					//���S���W���w��
	void setParameter(int i,int value);
	int getParameter() const;									//�t�H�[�J�X�������Ă�widget�̃p�����[�^�𓾂�
	int getPositionX(float)const;
	int getPositionY(float)const;
	void setCaption(const char*);

private:
	int getNewWidget();

	int clientX() const;	//client
	int clientY() const;
	int clientWidth() const;
	int clientHeight() const;
};
//���i�N���X
class Widget {
public:
	int mX,mY;
	int mWidth,mHeight;
	int mCursorX,mCursorY;	//�J�[�\���������ʒu

	bool mFocus;	//�t�H�[�J�X�𓾂邩
	int mResult;

	int mParameter;	//�ėp�p�����[�^�B

	Widget();
	virtual ~Widget();
	virtual void draw(int x,int y,bool flash =false) = 0;
	virtual bool update() =0;
};


//�e�L�X�g�N���X
class Text : public Widget{
	//�J�[�\�����g�����ɕK�v�ȃp�����[�^
	int mLine;
	int mFontWidth,mFontHeight;

	MyString mString;
	int mFont;
	unsigned int mColor;
public:

	void set(const char* string,unsigned int color,int fontsize);
	void set(const char* string);

	void getSize();
	void draw(int x,int y,bool flash =false);
	bool update();
};

//�o�b�`
class Picture : public Widget{
	Batch* mBatch;
	BlendInfo mBlendInfo;
public:

	void set(Batch*,BlendInfo&);
	void draw(int x, int y ,bool flash =false);
	bool update();
};

//yesno�_�C�A���O
class Confirm{
	Window mWindow;
	bool mCancelButton;	//�L�����Z���L�[�ɂ��L�����Z��
public:
	void set(const char*,const char* yes = 0,const char* no = 0,bool cancelbutton = true);
	void position(int x,int y);
	void centerPosition(int x,int y);
	int update();				//�͂��Ȃ�1�A�L�����Z�����������Ȃ�2
	void draw();
};


///////////////////////////////////////////////////////////////////
//������texture��\������w�i�p���W���[��
class Site{
	class Object{
	public:
		MyTexture mTexture;
		GameLib::Math::Vector3 mPosition;
		GameLib::Math::Vector3 mScale;

		Object();
	};
	Object* mObject;
	int mObjectNumber;
public:
	Site();
	~Site();
	void initialize();
	void load(const char* path);
	void read(Tag*);
	void draw(const GameLib::Math::Vector3& pos,int count);
	void draw(const GameLib::Math::Vector3& pos);
};
//�߂�ǂ������̂ŕ��u
//class ListBox : public Widget {
//	Cursor mCursor;
//	int mIndex;
//	int mLine;		//�\������s��
//	int mScroll;	//�X�N���[����
//	int mMax;		//�s��
//	Widget* mWidget[256];
//public:
//	void draw(int x, int y );
//	bool update();
//	bool modal();
//};


//�e�L�X�g���̍s��I������
/*
class ListBox : public Text{
	Cursor mCursor;

	bool mCancel;			//�L�����Z���\���H
	int mCursorY;

	bool mReady;			//false���ƌ���L�[�������Ȃ��@�㉺�L�[��������true�ɂȂ�
	int mOffset;			//�I�����͈̔�
	int mNumber;

	int mCount;				//�����Ă���̌o�߃t���[��
public:
	int mResult;			//-1�ŃL�����Z��

	void create (const char*,int x,int y,int charsize,bool cancel=false,unsigned int cusror=0,bool ready=true,int min=0,int max=255);
	void position(int x,int y);
	bool update();										//���肵����true��Ԃ�
	void draw();
	void setString(const char*,bool resize=true);

};
*/

#endif
