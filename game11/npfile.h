
#ifndef INCLUDED_NPFILE_H
#define INCLUDED_NPFILE_H

#include "GameLib/FileIO/Manager.h"
#include "GameLib/FileIO/InFile.h"
#include "GameLib/FileIO/OutFile.h"

#include "npstring.h"
//�t�@�C���ǂݍ��݁E�������݃N���X

class nFile {
private:
	GameLib::FileIO::InFile mFile;	//�G���R�[�h����ĂȂ���́A��������
	char* mData;					//�G���R�[�h���ꂽ��́A�������玝���Ă���
	int mLength;
public:
	void load(const char* path,bool bgm=false,bool code=false);
	int size();

	nFile();
	~nFile();
	const char* data();
	const char data(int i);
	unsigned int data(int index,int length,bool little);	
	bool isFinished();
	bool isError();

	void release();	
};
void nSave(const char* path,const char* data,int length=-1,bool code=false);


//�t�@�C���̈Í����ƃf�R�[�h
class FileSystem {
public:
	static int encode(char* result,const char* original,int length);		//�A��l�͒����B
	static int decode(char* result,const char* original,int length);
	static void setData(char* data,unsigned int i,int index,int length,bool little);	//������ɐ��l��˂�����
};

//platinum�p�}�b�v�t�@�C��
//16bit���[�h�ł̒l�ɕϊ�����
class FMF {
private:
	nFile mFile;
	unsigned int mLayer;
	unsigned int mBitCount;

public:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mChipWidth;
	unsigned int mChipHeight;

	FMF();
	~FMF();
	void load(const char*,const unsigned int layer=6);
	unsigned int data(unsigned int layer,unsigned int x,unsigned int y);
	bool seek(int* x,int* y,unsigned int layer,unsigned int i,unsigned int j);			//��������
};


//���C�u������loadFile�̓��[�N������ۂ���
void loadFile( MyString* out, const char* filename );


//�t�@�C�������邩�m�F�i�A�[�J�C�u�͒m���j
bool fileExist(const char *filename);

#endif

