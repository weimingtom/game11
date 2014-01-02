#ifndef INCLUDED_NPSTRING_H
#define INCLUDED_NPSTRING_H

#include <string>


class MyString;
class nString {
public:
	//指定文字列ごとにぶったぎる　文字数を返す
	static int splitChar(char *result , const char* original,const char spliter,const int index);
	//指定文字列でぶったぎったときの要素数を得る
	static int getSplitNumber(const char* original,const char spliter=',');

	//行数を得る
	static int getLine(const char*);
	static void getLine(char* result,const char* original,const int r,bool remover=true);

	//CSVのセルを得る
	static void getCell(char* result,const char* original,const int r,const int c);
	static int getCell(const char* original,const int r,const int c);
	static void getCell(MyString* result,const char* original,const int r,const int c);

	//解析する
	static void atoiArray(int*,const char*,int number);	//文字列を整数型の配列にバラす。カンマで区切る。高速。
	static void splitChar(char[][256],const char* original,int number);	//文字列をカンマでくぎって返す。高速。

	static int calcChar(const char*);
	static float calcCharFloat(const char*);
private:
	static void calcString(int* out,float* outf,const char*);
public:
	static bool checkSingleByte(const char*,int n);		//n文字目が一バイト文字かどうかチェック shiftJis
	static bool equalSingleByte(const char*,int n,char);//n文字目が二バイト文字じゃないことをチェックして比較

	static void itoa(char* result,int i,int figure);	//整数を文字列に

	static int compare(const char* dst,const char* src);		//同じになってる文字数を得る
	static int inStr(const char* dst,const char* src);				//みつからなかったら-1を返す
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

	void add(const char*,int length);	//固定長さぶんコピー。足りない場合は半角スペースで埋める。全角でバグる。
	void space(int length);				//半角スペース要れ、長さlengthにする

	void release();
	char* data() const;
	char data(int) const;
	void resize(unsigned int length);
	int size() const;

	void convertToTalk(int line);			//TALK用の形式に変換する
	void shorten(int i);
};

//連結なんかがカンタンにできるようにするためのクラス。
class StringBuffer{

	MyString mBuffer;
public:
	static void create();
	static void destroy();
	//static StringBuffer* instance();	//インスタンスなんぞ意識しなくてもいい作りにしよう
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
