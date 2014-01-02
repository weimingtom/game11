
#ifndef INCLUDED_NPFILE_H
#define INCLUDED_NPFILE_H

#include "GameLib/FileIO/Manager.h"
#include "GameLib/FileIO/InFile.h"
#include "GameLib/FileIO/OutFile.h"

#include "npstring.h"
//ファイル読み込み・書き込みクラス

class nFile {
private:
	GameLib::FileIO::InFile mFile;	//エンコードされてないやつは、ここから
	char* mData;					//エンコードされたやつは、ここから持ってくる
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


//ファイルの暗号化とデコード
class FileSystem {
public:
	static int encode(char* result,const char* original,int length);		//帰り値は長さ。
	static int decode(char* result,const char* original,int length);
	static void setData(char* data,unsigned int i,int index,int length,bool little);	//文字列に数値を突っ込む
};

//platinum用マップファイル
//16bitモードでの値に変換する
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
	bool seek(int* x,int* y,unsigned int layer,unsigned int i,unsigned int j);			//検索する
};


//ライブラリのloadFileはリークするっぽいぞ
void loadFile( MyString* out, const char* filename );


//ファイルがあるか確認（アーカイブは知らん）
bool fileExist(const char *filename);

#endif

