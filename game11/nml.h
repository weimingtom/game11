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
	int getIntValue(int i=-1,bool returnZero = true)const;	//�w��ԍ��̗v�f���Ȃ��ꍇ�A�[����Ԃ����Ō�̗v�f��Ԃ����I�ׂ�
	float getFloatValue(int i=-1,bool returnZero = true) const;
	int getParameter()const;	//�p�����[�^���𓾂�
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
	int read(const char*,bool errorbreak);	//-1�ŃG���[

	int getAttribute(const char* name = 0) const;		//�v�f�̐�
	int getChild(const char* name = 0) const;
	//����
	MyString* name();
	char* getName() const;

	Attribute* attribute(const char*,int i=0) const;
	Attribute* attribute(int) const;
	int get(const char*,bool checkError=true,int k=-1,bool returnZero=true) const;				//attribute���݂���Ȃ�������[����Ԃ�

	Tag* child(const char*,int index = 0)const;
	Tag* child(int) const;

	//����
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

	//����
	Tag* root();

	//����
	void write(MyString*) const;		//�Ƃ肠����64KB
	void release();

};



#endif
