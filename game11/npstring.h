#ifndef INCLUDED_NPSTRING_H
#define INCLUDED_NPSTRING_H

#include <string>


class MyString;
class nString {
public:
	//�w�蕶���񂲂ƂɂԂ�������@��������Ԃ�
	static int splitChar(char *result , const char* original,const char spliter,const int index);
	//�w�蕶����łԂ����������Ƃ��̗v�f���𓾂�
	static int getSplitNumber(const char* original,const char spliter=',');

	//�s���𓾂�
	static int getLine(const char*);
	static void getLine(char* result,const char* original,const int r,bool remover=true);

	//CSV�̃Z���𓾂�
	static void getCell(char* result,const char* original,const int r,const int c);
	static int getCell(const char* original,const int r,const int c);
	static void getCell(MyString* result,const char* original,const int r,const int c);

	//��͂���
	static void atoiArray(int*,const char*,int number);	//������𐮐��^�̔z��Ƀo�����B�J���}�ŋ�؂�B�����B
	static void splitChar(char[][256],const char* original,int number);	//��������J���}�ł������ĕԂ��B�����B

	static int calcChar(const char*);
	static float calcCharFloat(const char*);
private:
	static void calcString(int* out,float* outf,const char*);
public:
	static bool checkSingleByte(const char*,int n);		//n�����ڂ���o�C�g�������ǂ����`�F�b�N shiftJis
	static bool equalSingleByte(const char*,int n,char);//n�����ڂ���o�C�g��������Ȃ����Ƃ��`�F�b�N���Ĕ�r

	static void itoa(char* result,int i,int figure);	//�����𕶎����

	static int compare(const char* dst,const char* src);		//�����ɂȂ��Ă镶�����𓾂�
	static int inStr(const char* dst,const char* src);				//�݂���Ȃ�������-1��Ԃ�
};

class MyString {
	char* mData;
	int mBufferSize;
public:
	MyString();
	MyString(unsigned int length);
	~MyString();

	MyString& operator<<(int);
	MyString& operator<<(unsigned int);
	MyString& operator<<(bool);
	MyString& operator<<(const char*);
	MyString& operator<<(char);
	bool operator==(const char*) const;
	bool equal(const char*) const;
	void operator=(const char*);
	void operator=(int);
	void operator=(const MyString&);

	void add(const char*,int length);	//�Œ蒷���Ԃ�R�s�[�B����Ȃ��ꍇ�͔��p�X�y�[�X�Ŗ��߂�B�S�p�Ńo�O��B
	void space(int length);				//���p�X�y�[�X�v��A����length�ɂ���

	void release();
	char* data() const;
	char data(int) const;
	void resize(unsigned int length);
	int size() const;

	void convertToTalk(int line);			//TALK�p�̌`���ɕϊ�����
	void shorten(int i);
};

//�A���Ȃ񂩂��J���^���ɂł���悤�ɂ��邽�߂̃N���X�B
class StringBuffer{

	MyString mBuffer;
public:
	static void create();
	static void destroy();
	//static StringBuffer* instance();	//�C���X�^���X�Ȃ񂼈ӎ����Ȃ��Ă��������ɂ��悤
	static void initialize();
	static void add(const char*);
	static void add(int);
	static const char* data();
	static void shorten(int l);
private:
	StringBuffer();
	~StringBuffer();

	static StringBuffer* mInstance;
};


#endif
