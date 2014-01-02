#include <string>
#include <sstream>
#include <stdlib.h>
#include "GameLib/Framework.h"

//#include "game.h"
//#include "stage.h"
#include "npstring.h"

int nString::splitChar(char *result , const char* original,const char spliter,const int index) {

	int c=0;	//�v�f�J�E���^
	int ws=0;	//�}���`�o�C�g��ws�����ڂł��邱�Ƃ�����
	int p=0;	//�R�s�[�ʒu

	for (int i=0 ; true ; ++i) {
		char t = *(original+i);
		//�}���`�o�C�g�̂����f
		if (ws) {
			++ws;
			if (ws>2) {
				ws=0;
			}
		}
		if (ws==0) {
			if (t<0) {
				ws=1;
			}
		}
		//�X�v���b�^�������͏I���ʒu�ɓ�������
		if (ws==0) {
			if (t==spliter) {
				++c;
				if (c > index) {
					break;
				}
				continue;
			}
			if (t==0) {
				break;
			}
		}

		//�������܃R�s�[��
		if (c==index) {
			*(result+p) = t;
			++p;
		}
	};
	*(result+p) = 0;
	return p;
};
//�v�f���𐔂��鏈��
int nString::getSplitNumber(const char* original,const char spliter){

	int c=0;	//�v�f�J�E���^
	int ws=0;	//�}���`�o�C�g��ws�����ڂł��邱�Ƃ�����

	for (int i=0 ; true ; ++i) {
		char t = *(original+i);
		//�}���`�o�C�g�̂����f
		if (ws) {
			++ws;
			if (ws>2) {
				ws=0;
			}
		}
		if (ws==0) {
			if (t<0) {
				ws=1;
			}
		}
		if (ws==0) {
			//�X�v���b�^�ɓ�������
			if (t==spliter) {
				++c;
			}
			//������I��
			if (t==0) {
				++c;
				break;
			}
		}
	};
	return c;
}

//�s���𓾂�
int nString::getLine(const char* original) {
	//return getSplitNumber(original,"\r\n");
	return getSplitNumber(original,'\n');
}
//�s�𓾂�@���s�R�[�h��\n�̂�
//\r�������ɂ��Ă�̂ŁA�����Ă������H�i�댯�ȏ����ł͂���̂����E�E�E�j
void nString::getLine(char* result,const char* original,const int i,bool remover) {
	int l=splitChar(result,original,'\n',i);
	if (l>0 && remover) {
		if (*(result+l-1) == '\r') {
			*(result+l-1)=0;
		}
	}
};

//CSV�̃Z���𓾂�i�s�A��Ŏw��j
void nString::getCell(char* result,const char* original,const int r,const int c) {
	char row[1024];

	getLine(row,original,r);
	splitChar(result,row,',',c);
};
int nString::getCell(const char* original,const int r,const int c){
	char cell[256];
	getCell(cell,original,r,c);
	return atoi(cell);
};
void nString::getCell(MyString* result,const char* original,const int r,const int c) {
	char col[256];

	getCell(col,original,r,c);
	*result = col;
};



//�����̔z���Ԃ�
void nString::atoiArray(int* result,const char* s,int n) {
	int ws=0;//��o�C�g����������
	char ss[256];

	int j=0;
	int a=0;
	for (int i=0; true ; ++i) {
		char t = *(s+i);

		if (ws) {
			++ws;
			if (ws>2) {
				ws=0;
			}
		}
		if (ws==0) {
			if (t<0) {
				ws=1;
			}
		}

		if (ws==0) {
			if (t==',') {
				ss[j] = 0;
				result[a] = calcChar(ss);
				++a;
				j=0;
				if (a>=n) {
					break;
				}
				continue;
			}
			if (t==0) {
				ss[j] = 0;
				result[a] = calcChar(ss);
				++a;
				j=0;
				break;
			}
		}

		ss[j]=t;
		++j;
	}
}
//������̔z���Ԃ�
void nString::splitChar(char result[][256],const char* original,int n) {
	int ws=0;//��o�C�g����������
	int c=0;
	int p=0;

	for (int i=0; true ; ++i) {
		char t = *(original+i);
		//�}���`�o�C�g�̂����f
		if (ws) {
			++ws;
			if (ws>2) {
				ws=0;
			}
		}
		if (ws==0) {
			if (t<0) {
				ws=1;
			}
		}
		if (ws==0) {
			//�X�v���b�^�ɓ�������
			if (t==',') {
				result[c][p] = 0;
				++c;
				p=0;
				if (c>=n) {
					break;
				}
				continue;
			}
			//������I��
			if (t==0) {
				result[c][p] = 0;
				break;
			}
		}
		//�R�s�[
		result[c][p] = t;
		++p;
	}
}
void nString::itoa(char* result,int i,int figure) {
	result[figure]=0;

	for (int w=0 ; w<figure ; ++w) {
		int f= i%10;
		i/=10;

		result[figure-w-1]= f+48;
	}
}


//�v�Z�������߂���B�k����������[���ŕԂ��Ƃ��Â������Ȃ�
int nString::calcChar(const char* exp){
	int i;

	calcString(&i,0,exp);
	return i;
}
float nString::calcCharFloat(const char* exp) {

	float f;
	calcString(0,&f,exp);
	return f;
}

void nString::calcString(int* outi,float* outf,const char* exp) {
	int r=0;		//����
	float rf = 0.f;	//float��
	int a=0;		//�X�g�b�N���鐔�l
	int system = 10;//10�i��

	char ope='+';	//���Z�q

	//char chr[16];	//�L�^����Ă镶��
	//chr[0]=0;
	//int chrlen=0;

	for (int i=0 ; true ; ++i) {
		//���Z�q�������Ƃ��̏���
		//�ۑ�����Ă��������͂��Achr������������

		//�����@48�`57
		//f 102

		//���Z�q�������Ƃ�
		if (exp[i] == '+' ||
			exp[i] == '-' ||
			exp[i] == '*' ||
			exp[i] == '/' ||
			exp[i] == '+' ||
			exp[i] == '%' ||
			exp[i] == '>' ||
			exp[i] == '<' ||
			exp[i] == '=' ||
			exp[i] == '!' ||
			exp[i] == 0)
		{
			int b = a;

			if (ope=='+') {
				r +=b;
				rf+=b;
			}
			if (ope=='-') {
				r -=b;
				rf-=b;
			}
			if (ope=='*') {
				r *=b;
				rf*=b;
			}
			if (ope=='/') {
				r /=b;
				rf/=b;
			}
			if (ope=='%') {
				r%=b;
				//rf%=b;
				ASSERT(outi);
			}
			if (ope=='>') {
				r = (r>b) ? 1 : 0;
				rf = (rf>b) ? 1.f : 0.f;

			}
			if (ope=='<') {
				r= (r<b) ? 1 : 0;
				rf= (rf<b) ? 1.f : 0.f;
			}
			if (ope=='=') {
				r = (r==b) ? 1 : 0;
				rf = (rf==b) ? 1.f : 0.f;
			}
			if (ope=='!') {
				r= (r!=b) ? 1 : 0;
				rf= (rf!=b) ? 1.f : 0.f;
			}

			//�I��
			if (exp[i]==0) {
				break;
			}
			ope = exp[i];

			system = 10;
			a=0;
			continue;
		};
		//x�������Ƃ� 16�i���t���O�����Ă�
		if (exp[i] == 'x') {
			ASSERT(a==0);		//x�����ɂȂ��Ƃ��̓G���[.������0��ok
			system = 16;
			continue;
		}
		//b�������Ƃ� 2�i���t���O�����Ă�
		if (exp[i] == 'b') {
			ASSERT(a==0);		//x�����ɂȂ��Ƃ��̓G���[.������0��ok
			system = 2;
			continue;
		}


		//�����������Ƃ�(10�i���j
		if (system == 10) {
			if (exp[i]>=48 && exp[i]<58) {
				a = a*10 + exp[i]-48;
				continue;
			}
		}
		//16�i��
		if (system == 16) {
			if (exp[i]>=48 && exp[i]<58) {
				a = a*16 + exp[i]-48;
				continue;
			}
			if (exp[i]>=97 && exp[i]<103) {
				a = a*16 + exp[i]-97+10;
				continue;
			}
		}
		//��i��
		if (system == 2) {
			if (exp[i]>='0' && exp[i]<='1') {
				a = a*2 + exp[i]-'0';
				continue;
			}
		}

		//�����ȕ��� break���邾���ł��������B
		ASSERT(0);
	}
	if (outi){
		*outi = r;
	}
	if (outf) {
		*outf = rf;
	}
};

bool nString::checkSingleByte(const char* s,int n) {

	//�����������蓪�̈������@�ł��B�x���̂Œ��ӁB
	int wb=0;
	for (int i=0; i<n+1 ; ++i) {
		//char t=*(s+i);

		if (wb) {
			++wb;
			if (wb>2) {
				wb=0;
			}
		}
		if (wb==0) {
			unsigned char c = *(s+i); 
			if ( ((c >= 0x81) && (c <= 0x9f)) ||
				 ((c >= 0xe0) && (c <= 0xfc)) )
			{
				wb=1;
			}
		}
	}
	return (wb == 0);
};
bool nString::equalSingleByte(const char * s, int n, char c) {
	bool result=false;
	if (checkSingleByte(s,n)) {
		if (*(s+n)==c) {
			result=true;
		};
	};
	return result;
};

int nString::compare(const char*dest,const char* src){
	int result=0;
	for (int i=0 ; true ;++i) {
		if (dest[i] != src[i]) {
			break;
		}
		if (src[i] == 0) {
			break;
		}
		++result;
	}
	return result;
};

int nString::inStr(const char* dst,const char* src) {
	int result = -1;
	if (src[0] ==0) {
		;	//�ǂȂ����[���イ�񂶂�
	}
	else {
		bool wb=false;	//��o�C�g�����΍�B

		for (int i=0; true ; ++i) {
			//�Ō�܂ł��Ă��܂���
			if (dst[i] ==0) {
				break;
			}

			if (wb) {
				wb=false;
			}
			else {
				if (dst[i] == src[0]) {
					for (int j=1; true ; ++j) {
						if (src[j] == 0) {
							//�Ō�܂ň�v�B
							result = i;
							break;
						}
						if (dst[i + j] != src[j]) {
							//����Ȃ��B
							break;
						}
					}
					if (result> -1) {
						//���������̂Ō����I���B
						break;
					}
				}

				if (dst[i]<0 && src[0]>0) {
					wb=true;	//��o�C�g�����̓��������̂ŁA���̃��[�v�̓X�L�b�v����B
				}
			}
		}
	}
	return result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MyString::MyString() {
	mData= new char[1];
	mData[0]=0;
	mBufferSize = 1;
};

MyString::MyString(unsigned int l) {
	mData= new char[l];
	mData[0]=0;
	mBufferSize = l;
};

MyString::~MyString() {
	release();
}
void MyString::release() {
	if (mData) {
		delete []mData;
		mData=0;
	}
	mBufferSize = 0;
};

MyString& MyString::operator <<(bool b) {
	if (b) {
		*this << "1";
	}
	else {
		*this << "0";
	}
	return *this;
};
MyString& MyString::operator <<(int i) {
	char s[16];
	_itoa_s(i,s,16,10);

	*this << s;

	return *this;
}
MyString& MyString::operator <<(unsigned int i) {
	char s[16];
	_itoa_s(i,s,16,10);

	*this << s;

	return *this;
}

MyString& MyString::operator <<(const char *s) {
	if (s) {
		strcat(mData,s);
	}
	return *this;
}
MyString& MyString::operator <<(char s) {
	char c[2];
	c[0]=s;
	c[1]=0;

	*this << c;
	return *this;
}

bool MyString::operator ==(const char *s) const{
	return equal(s);
}
bool MyString::equal(const char*s) const {
	if (mData == 0) {
		return (s==0);
	}
	if (s == 0) {
		return false;
	}


	return (strcmp(mData,s)==0);
}
void MyString::operator =(const MyString& s){
	*this =s.data();
};

void MyString::operator =(const char *s) {
	if (s==0) {
		mData[0] = 0;
	}
	else {
		//�k���Ȃ�new���Ȃ�
		if (s[0]==0) {
			mData[0]=0;
		}
		else {
			int l=strlen(s);
			resize(l+1);
			strcpy(mData,s);
		}
	}
}
void MyString::operator =(int i) {
	char s[16];
	_itoa_s(i,s,16,10);

	*this = s;
}

void MyString::add(const char* str, int length) {
	char* tmp = new char[length+1];

	bool space=false;
	for (int i=0 ; i<length ; ++i) {
		if (space) {
			tmp[i] =' ';
		}
		else {
			if (str[i] == 0) {
				tmp[i] = ' ';
				space=true;
			}
			else {
				tmp[i] = str[i];
			}
		}
	};

	tmp[length] = 0;
	
	*this << tmp;
	delete []tmp;
};


void MyString::space(int length) {
	while(size() < length) {
		*this << ' ';
//	for (int i= strlen(mData) ; i<length ; ++i) {
//		mData[i] = ' ';
	};
//	mData[length] = 0;

};

char* MyString::data() const{
	return mData;
};
char MyString::data(int i) const{
	return *(mData + i);
};

void MyString::resize(unsigned int l) {
	if (mBufferSize != l) {
		if (mData) {
			delete []mData;
			mData=0;
		}

		mData= NEW char[l];
	}
	mData[0]=0;
	mBufferSize = l;
}

int MyString::size() const{
	return strlen(mData);
};

//char���R�s�[�B�i��b�X�N���v�g��p�j
//'\'+'n'���@'\n[sp][sp]'�ɕϊ�����@�\������
//�܂���s������̕�������40���z�����玩���ŉ��s

void MyString::convertToTalk(int linemax){

	MyString tmp(this->size()+1);			//�I�[�����Ԃ�1�v���X
	int linelength=0;						//��s������̕�����

	for (int i=0; true ; ++i) {
		char t = this->data(i);
		//�I�[����
		if (t==0) {
			break;
		}

		if (t=='\\') {
			char tt = this->data(i+1);
			if (tt =='n') {
				tmp << '\n';
				++i;
				linelength=0;
				continue;
			}
		}

		//��s������̕���������������ꍇ�A�������s
		//�����Ƃ��ɂ�ASSERT�ł������C������
		if (linelength>linemax) {
			tmp << '\n';
			linelength=0;

			//ASSERT(0 && "talkline too long")
		}

		//��o�C�g����
		if (t<0) {
			tmp << this->data(i);
			tmp << this->data(i+1);

			++i;

			linelength += 2;
			continue;
		}
		//���ʂɃR�s�[
		tmp << t;
		linelength += 1;
	}

	*this = tmp.data();
};
void MyString::shorten(int i){
	for (int w=0 ; w<(mBufferSize-4) ; ++w) {
		char t = mData[w];
		if (t==0) {
			break;
		}

		if (i<1) {
			//�ȉ��ȗ�
			//�ȗ���̂ق��������Ȃ�Ƃ�������...
			for (int k = 0 ; k<3 ; ++k) {
				mData[w+k] = '.';
			}
			mData[w+3] = 0;
			break;
		}

		if (t<0) {
			++w;
		}
		--i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StringBuffer* StringBuffer::mInstance = 0;

StringBuffer::StringBuffer() {
	mBuffer.resize(65536);
};
StringBuffer::~StringBuffer() {
	;
};
void StringBuffer::create() {
	if (mInstance == 0) {
		mInstance = new StringBuffer;
	}
}
void StringBuffer::destroy() {
	if (mInstance) {
		delete mInstance;
		mInstance = 0;
	}
};

void StringBuffer::initialize() {
	mInstance->mBuffer = "";
}

void StringBuffer::add(const char* data) {
	mInstance->mBuffer << data;
}
void StringBuffer::add(int i) {
	mInstance->mBuffer << i;
}

void StringBuffer::shorten(int l) {
	mInstance->mBuffer.shorten(l);
}

const char* StringBuffer::data() {
	return mInstance->mBuffer.data();
}


