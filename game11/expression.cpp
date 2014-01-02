
#include "expression.h"
#include "gamedata.h"
#include "csv.h"

//���v�Z
Expression::Term::Term() {
	mAvailable = true;
	mOperator = '+';

	mExpression = 0;
	mOperand = 0;
	mFunction = 0;
};
Expression::Term::~Term() {
	if (mExpression){
		delete mExpression;
		mExpression = 0;
	}
	if (mFunction) {
		delete mFunction;
		mFunction = 0;
	}
};

int Expression::Term::set(const char * exp,GameData* gamedata) {
	int i=0;		//�����ʒu

	int mode = 0;	//0�I�y���[�^�[���������[�h�@1�I�y�����h�T�����[�h�@2�����ǉ����[�h�@3�֐��ǉ����[�h
	int value = 0;
	char end = 0;		//�I�[?

	while(true) {
		if (mode == 0) {
			//�I�y���[�^�[�݂���
			if (exp[i] == '+') { mOperator = '+';	mode = 1;	++i;	continue;}
			if (exp[i] == '-') { mOperator = '-';	mode = 1;	++i;	continue;}
			if (exp[i] == '*') { mOperator = '*';	mode = 1;	++i;	continue;}
			if (exp[i] == '/') { mOperator = '/';	mode = 1;	++i;	continue;}
			if (exp[i] == '>') { mOperator = '>';	mode = 1;	++i;	continue;}
			if (exp[i] == '<') { mOperator = '<';	mode = 1;	++i;	continue;}
			if (exp[i] == '=') { mOperator = '=';	mode = 1;	++i;	continue;}
			if (exp[i] == '!') { mOperator = '!';	mode = 1;	++i;	continue;}
			if (exp[i] == '&') { mOperator = '&';	mode = 1;	++i;	continue;}
			if (exp[i] == '|') { mOperator = '|';	mode = 1;	++i;	continue;}

			//�����������̓A���t�@�x�b�g�Ȃ�A���[�h��؂�ւ���
			if (exp[i] >='0' && exp[i]<='9') {
				mode = 2;
				continue;
			};

			//�A���t�@�x�b�g�������Ȃ�A�֐������Ă����܂�
			if (exp[i] >='A' && exp[i]<='z') {
				int r=0;
				mFunction = Function::read(&r,gamedata,exp+i);
				i += r;
				break;
			};
			//����������AExpression������Ă����܂�
			if (exp[i] == '(') {
				++i;
				mExpression = new Expression;
				i += mExpression->set(exp+i,')',gamedata);
				break;
			}

			//�I�[�������₪������I���B��̂܂܂����m��Ȃ����ǂ��܂�Ȃ�
			if (exp[i] == 0 || exp[i] == ')') {
				mAvailable = false;
				break;
			}
			
			//����ȊO�Ȃ�A���p�X�y�[�X�����F�߂Ȃ�
			ASSERT(exp[i] == ' ');
			++i;
		}
		//�I�y�����h�T�����[�h
		if (mode == 1) {
			//�����������̓A���t�@�x�b�g�Ȃ�A���[�h��؂�ւ���
			if (exp[i] >='0' && exp[i]<='9') {
				mode = 2;
				continue;
			};
			//�A���t�@�x�b�g�������Ȃ�A�֐������Ă����܂�
			if (exp[i] >='A' && exp[i]<='z') {
				int r=0;
				mFunction = Function::read(&r,gamedata,exp+i);
				i += r;
				break;
			};
			//����������AExpression������Ă����܂�
			if (exp[i] == '(') {
				++i;
				mExpression = new Expression;
				i += mExpression->set(exp+i,')',gamedata);
				break;
			}

			//����ȊO�Ȃ�A���p�X�y�[�X�����F�߂Ȃ�
			ASSERT(exp[i] == ' ');
			++i;
		}
		//�����ǉ����[�h
		if (mode == 2) {
			if (exp[i] >='0' && exp[i]<='9') {
				value = value*10 + (exp[i] - '0');
				++i;
				continue;
			};
			//�����ȊO��������A�I���@16�i���[�h�Ƃ��~�������ǁB
			mOperand = value;
			break;
		}
	};
	return i;
};
int Expression::Term::operand(GameData* gamedata) const{
	if (mFunction) {
		return mFunction->getValue(gamedata);
	}
	if (mExpression) {
		return mExpression->getValue(gamedata);
	}
	return mOperand;
}
//////////////////////////////////////////////////////////////////////////////

Expression::Expression() {
	mTerm[0] = 0;
};
Expression::~Expression() {
	for (int i=0 ; i<mMax ; ++i) {
		if (mTerm[i]) {
			delete mTerm[i];
			mTerm[i] = 0;
		}
		else {
			break;
		}
	}
}

int Expression::set(const char* exp,char endmark,GameData* gamedata) {
	int result = 0;
	ASSERT(gamedata!=0);

	//term������čŌ�܂ōs����
	for (int i=0 ; ; ++i) {
		ASSERT(i<mMax);
		mTerm[i] = 0;

		//�I�[������������I��
		if (exp[result] == endmark) {
			break;
		};

		Term* term = new Term;
		result += term->set(exp + result,gamedata);
		if (term->mAvailable) {
			mTerm[i] = term;
		}
		else {
			//������ۂł����B�I���B
			delete term;
			term = 0;
			break;
		}
	}

	//�I�[�����͐��������H
	ASSERT(exp[result] == endmark);

	return result+1;
};


//�v�Z���悤
int Expression::getValue(GameData *gamedata) const {
	int result = 0;
	for (int i=0 ; i<mMax ; ++i) {
		if (mTerm[i] == 0) {
			break;
		}
		int p= mTerm[i]->operand(gamedata);

		//���Z�q����
		if (mTerm[i]->mOperator == '+') {
			result += p;
		}
		if (mTerm[i]->mOperator == '-') {
			result -= p;
		}
		if (mTerm[i]->mOperator == '*') {
			result *= p;
		}
		if (mTerm[i]->mOperator == '/') {
			result /= p;
		}
		if (mTerm[i]->mOperator == '<') {
			if (result < p) {
				result = 1;
			}
			else {
				result = 0;
			}
		}
		if (mTerm[i]->mOperator == '>') {
			if (result > p) {
				result = 1;
			}
			else {
				result = 0;
			}
		}
		if (mTerm[i]->mOperator == '=') {
			if (result == p) {
				result = 1;
			}
			else {
				result = 0;
			}
		}
		if (mTerm[i]->mOperator == '!') {
			if (result == p) {
				result = 0;
			}
			else {
				result = 1;
			}
		}
		if (mTerm[i]->mOperator == '&') {
			if (result && p) {
				result = 1;
			}
			else {
				result = 0;
			}
		}
		if (mTerm[i]->mOperator == '|') {
			if (result || p) {
				result = 1;
			}
			else {
				result = 0;
			}
		}

	}
	return result;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�֐��Q
//�K�v�Ȋ֐�
//�퓬��
//�����̐�
//�G�̐�
//�w�胆�j�b�g�������Ă邩�H�@�E�E��������󂯎��Ȃ��ƑʖڃW����
//

Expression::Function::~Function() {
};

Expression::Function* Expression::Function::read(int* const i,GameData* const gamedata,const char *func) {
	//�P��ǉ�
	//()�������ɂȂ邩������ɂȂ邩�ŏ����������̂Œ��ӁB

	Function* result = 0;

	//�ǉ����Ȃ��ƂȂ��E�E�E
	MyString name(64);
 	//�Ƃ肠�����J�b�R��T����

	for (*i=0;true; ++ *i) {
		if (func[*i] == '(') {
			name.add(func,*i);
			break;
		}
		ASSERT(func[*i]);	//�I�[������������G���[
	}



	++ *i;
	//������ǂށ@�����^�C�v�ƕ�����^�C�v�����邼�B
	int j = result->readParameter(gamedata,func + *i);

	*i += j;
	return result;
}

//����������T��
int Expression::Function::readString(const char* str,char end) {
	int i;
	for (i=0 ; true ; ++i){
		char t = str[i];
		ASSERT(t);			//�I�[������������G���[

		if (t==end) {
			break;
		}

		//��o�C�g�������
		if (t<0) {
			++i;
		}
	}
	return i;
};

