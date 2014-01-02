
#include "nml.h"
#include "npfile.h"
#include "GameLib/Framework.h"



////////////////////////////////////////////////////////////////////////
NML::NML() :
mRoot(0){
};

void NML::load(const char* path) {
	nFile file;
	file.load(path);
	read(file.data());
}

bool NML::read(const char* data,bool errorbreak) {
//���
	int r = mRoot.read(data,errorbreak);
	if (r<0) {
		return false;
	}
	else {
		return true;
	}
};

Tag* NML::root(){
	return &mRoot;
}

void NML::write(MyString* result) const{
	result->resize(65536*4);
	mRoot.write(result,0);
};

void NML::release() {
	mRoot.release();
};
///////////////////////////////////////////////////////////////////////////

Tag::Tag(Tag* parent) {
	//for (int i=0 ; i<mMax ; ++i) {
	//	mAttribute[i] = 0;
	//	mTag[i] = 0;
	//}
	mParent = parent;
	mName = "root";
};

Tag::~Tag() {
	release();
};
void Tag::release() {
	mParent = 0;
	mTag.release();
	mAttribute.release();

	//for (int i=0 ; i<mMax ; ++i) {
	//	if (mAttribute[i]) {
	//		delete mAttribute[i];
	//		mAttribute[i] = 0;
	//	}

	//	if (mTag[i]) {
	//		delete mTag[i];
	//		mTag[i] = 0;
	//	}
	//}
};

int Tag::read(const char* data,bool errorbreak) {
	MyString command(256);
	int i;
	bool commentOut = false;
	for (i=0 ; true ; ++i) {

		//��o�C�g����
		if (data[i] < 0) {
			if (!commentOut) {
				command << data[i];
				command << data[i+1];
			}

			++i;
			continue;
		}

		if (data[i] == 0) {
			//�I���B
			//�������e��������G���[�B�J�b�R��������Ȃ��B
			if (mParent) {
				i=-1;
				if (errorbreak) {
					HALT(0);
				}
			}
			break;
		};

		if (data[i] == '\t') {
			//����
			continue;
		};
		if (data[i] == '\n') {
			//����
			commentOut = false;
			continue;
		};
		if (data[i] == '\r') {
			//����
			continue;
		};
		if (command == "") {
			//�R�����g�A�E�g
			if (data[i] == '/' &&
				data[i+1] == '/') {
				commentOut = true;
				continue;
			};
			if (data[i] == ' ') {
				//����
				continue;
			};
		}
		if (commentOut) {
			continue;
		}


		if (data[i] == ';') {
			if ( command == "") {
				//ASSERT(0);
			}
			else {
				//attribute������
				mAttribute.add()->read(command.data());
				//for (int j=0 ; j<mMax ; ++j) {
				//	if (mAttribute[j] == 0) {
				//		mAttribute[j] = new Attribute;
				//		mAttribute[j]->read(command.data());
				//		break;
				//	}
				//}
				command="";
			}
			continue;
		};

		if (data[i] == '{') {
			//�^�O�������ēǂݍ��݊J�n
			//�����̔��p�X�y�[�X�͍��
			++i;

			Tag* t = mTag.addInitializing(this);
			t->mName = command.data();
			int r = t->read(data + i,errorbreak);
			if (r<0) {
				//�G���[
				i=-1;
			}
			else {
				i += r;
			}

			command="";
			if (i<0) {
				break;
			}
			continue;
		};

		if (data[i] == '}') {
			//�I���B�e�ɖ߂�
			//�e�����Ȃ�������G���[�B
			if (mParent==0) {
				i=-1;
				if (errorbreak) {
					ASSERT(mParent);
				}
			}
			break;
		};

		//�R�}���h�𑫂�
		command << data[i];
	}
	return i;

};

MyString* Tag::name() {
	return &mName;
}

char* Tag::getName() const{
	return mName.data();
}
int Tag::getAttribute(const char* name) const {
	int r=0;
	for (List<Attribute>::Iterator i(mAttribute) ; !i.end() ; i++) {

		if (name == 0) {
			++r;
		}
		else {
			if (i()->name()->equal(name)) {
				++r;
			}
		}
	}
	return r;
}


int Tag::getChild(const char* name) const {
	int r=0;
	for (List<Tag>::Iterator i(mTag) ; !i.end() ; i++) {
		if (name == 0) {
			++r;
		}
		else {
			if (i()->mName == name) {
				++r;
			}
		}
	}
	return r;
}

Attribute* Tag::attribute(const char *at,int n) const {
	Attribute* result =0;
	int c=0;
	for (List<Attribute>::Iterator i(mAttribute) ; !i.end(); i++) {
		if (*i()->name() == at) {
			if (c==n) {
				result = i();
				break;
			}
			++c;
		}
	}
	return result;
}
Attribute* Tag::attribute(int i) const {
	return mAttribute(i);
}
int Tag::get(const char* name,bool checkError,int k,bool returnZero) const {
	const Attribute* at = attribute(name);
	if (at == 0) {
		if (checkError) {
			ASSERT(0);
		}
		else {
			return 0;
		}
	}
	return at->getIntValue(k,returnZero);
};

Tag* Tag::child(const char* t,int index) const{
	Tag* result =0;
	int c = 0;

	for (List<Tag>::Iterator i(mTag) ; !i.end() ; i++) {
		if (*(i()->name()) == t) {
			if (c == index) {
				result = i();
				break;
			}
			else {
				++c;
			}
		}
	}
	return result;
};
Tag* Tag::child(int i) const {
	return mTag(i);
}

Tag* Tag::addChild(const char* name) {
	Tag* result = mTag.addInitializing(this);
	result->mParent = this;
	result->mName = name;

	return result;
};
Attribute* Tag::addAttribute(const char * name, const char * param) {
	Attribute* result = mAttribute.add();
	result->set(name,param);
	return result;
};
Attribute* Tag::addAttribute(const char* name,int param) {
	MyString t;
	t= param;
	return addAttribute(name,t.data());
};
Attribute* Tag::addAttribute(const char * name) {
	Attribute* result = mAttribute.add();
	result->set(name,0);
	return result;
};

void Tag::write(MyString* str,int indent) const{
	//�����̖��O������
	if (indent) {
		//�C���f���g
		for (int i=0 ; i<indent-1 ; ++i) {
			*str << "\t";
		}

		*str << getName();
		*str << "{\r\n";
	}

	//attribute���܂�����
	for (List<Attribute>::Iterator i(mAttribute); !i.end() ; i++) {
		//�C���f���g
		for (int j=0 ; j<indent ; ++j) {
			*str << "\t";
		}

		i()->write(str);

		*str << "\r\n";
	}

	//child
	for (List<Tag>::Iterator i(mTag); !i.end() ; i++) {
		i()->write(str,indent + 1);
	}

	//����
	if (indent) {
		//�C���f���g
		for (int i=0 ; i<indent-1 ; ++i) {
			*str << "\t";
		}
		*str << "}\r\n";
	}
};

///////////////////////////////////////////////////////////////////////////
void Attribute::read(const char* data) {
	//���p�X�y�[�X�A�J���}�ŋ�؂�

	MyString tmp(256);
	int mode=0;

	for (int i=0 ; true ; ++i) {

		//���O
		if (mode==0) {
			if (data[i] == ' ') {
				mName = tmp.data();
				tmp = "";
				mode=1;
			}
			else {
				if (data[i] == 0) {
					mName = tmp.data();
					break;
				}
				else {
					tmp << data[i];
				}
			}
		}
		if (mode==1) {
			if (data[i] == 0) {
				break;
			}
			if (data[i] != ' ') {
				mode=2;
			}
		}

		//�p�����[�^
		if (mode==2) {
			if (data[i] == 0) {
				readParameter(tmp.data());
				break;
			}
			else {
				tmp << data[i];
			}
		};

		//2�o�C�g������������A�ЂƂi�߂Ă���
		if (data[i] < 0) {
			++i;

			tmp << data[i];
		}
	}
};
void Attribute::write(MyString *str) const {
	*str << getName();
	*str << " ";

	for (int j=0 ; true ; ++j) {
		if (getValue(j) == 0) {
			break;
		}
		if (j) {
			*str << ",";
		}
		*str << getValue(j);
	}
	*str << ";";
}

void Attribute::set(const char* name,const char* value) {
	mName = name;
	readParameter(value);
}
void Attribute::addParameter(int i) {
	//MyString *p = new MyString;
	MyString* r = mParameter.add();
	*r = i;
};

MyString* Attribute::name() {
	return &mName;
}
char* Attribute::getName() const {
	return mName.data();
}
char* Attribute::getValue(int i,bool returnZero) const {

	//��ȏ�p�����[�^��������ɔԍ��w��Ȃ��ŃA�N�Z�X����ƃG���[
	if (i<0) {
		ASSERT(mParameter.number()<2 && "must input paramater number");
		i=0;
	}

	if (mParameter(i) == 0) {
		if (returnZero) {
			return 0;
		}
		else {
			return mParameter.last()->data();
		}
	}
	return mParameter(i)->data();
};

int Attribute::getIntValue(int i,bool returnZero) const{
	const char* r = getValue(i,returnZero);
	if (r) {
		return nString::calcChar(r);
	}
	else {
		return 0;
	}
};
float Attribute::getFloatValue(int i,bool returnZero) const{
	const char* r = getValue(i,returnZero);
	if (r) {
		return nString::calcCharFloat(r);
	}
	else {
		return 0.f;
	}
};

int Attribute::getParameter() const {
	return mParameter.number();
};

void Attribute::readParameter(const char * data) {
	if (data) {
		MyString tmp(256);

		for (int i=0 ; true ; ++i) {

			//�p�����[�^
			if (data[i] == 0 || 
				data[i] == ',') {

				MyString*r = mParameter.add();
				*r = tmp.data();	

				tmp = "";

				if (data[i] == 0) {
					break;
				}
			}
			else {
				tmp << data[i];
			}

			//2�o�C�g������������A�ЂƂi�߂Ă���
			if (data[i] < 0) {
				++i;

				tmp << data[i];
			}
		}
	}
};


