#ifndef INCLUDED_NML_H
#define INCLUDED_NML_H

#include "npstring.h"
#include "base.h"



class Attribute{
	MyString mName;
	List<MyString> mParameter;
public:

	void read(const char*);
	void readParameter(const char*);
	void write(MyString*) const;
	void set(const char* name,const char* value);
	void addParameter(int);

	MyString* name();
	char* getName()const;
	char* getValue(int i=-1,bool returnZero = true)const;
	int getIntValue(int i=-1,bool returnZero = true)const;	//指定番号の要素がない場合、ゼロを返すか最後の要素を返すか選べる
	float getFloatValue(int i=-1,bool returnZero = true) const;
	int getParameter()const;	//パラメータ数を得る
};

class Tag{
	MyString mName;
	Tag* mParent;

	List<Tag> mTag;
	List<Attribute> mAttribute;
public:
	Tag(Tag* parent);
	~Tag();
	void release();
	int read(const char*,bool errorbreak);	//-1でエラー

	int getAttribute(const char* name = 0) const;		//要素の数
	int getChild(const char* name = 0) const;
	//検索
	MyString* name();
	char* getName() const;

	Attribute* attribute(const char*,int i=0) const;
	Attribute* attribute(int) const;
	int get(const char*,bool checkError=true,int k=-1,bool returnZero=true) const;				//attributeがみつからなかったらゼロを返す

	Tag* child(const char*,int index = 0)const;
	Tag* child(int) const;

	//操作
	Tag* addChild(const char*);
	Attribute* addAttribute(const char*,const char*);
	Attribute* addAttribute(const char*,int);
	Attribute* addAttribute(const char*);

	void write(MyString* ,int indent) const;
};

class NML{
	Tag mRoot;

public:
	NML();
	void load(const char* path);
	bool read(const char*,bool errorbreak = true);

	//検索
	Tag* root();

	//操作
	void write(MyString*) const;		//とりあえず64KB
	void release();

};



#endif
