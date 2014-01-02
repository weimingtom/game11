#ifndef INCLUDED_EXPRESSION_H
#define INCLUDED_EXPRESSION_H

#include "gamedata.h"

//��

class Expression {
	class Function {
	public:
		virtual ~Function();
		static Function* read(int* cont,GameData* const,const char*);
		static int readString(const char*,char end = ')');						//)������܂ł̕������𓾂�
		virtual int readParameter(GameData*,const char*) = 0;
		virtual int getValue(GameData*) = 0;
	};

	class Term{
	public:
		bool mAvailable;			//������ۂ����m��Ȃ��̂�
		char mOperator;				//���Z�q

		//�ǂꂩ�����p����
		Function* mFunction;
		Expression* mExpression;			//���ʂł������Ă�ꍇ�͂�����
		int mOperand;						//���l

		Term();
		~Term();
		int set(const char* exp,GameData*);
		int operand(GameData*) const;
	};


	static const int mMax = 16;		//���̍ő吔
	Term* mTerm[mMax];
public:
	Expression();
	~Expression();

	int set(const char*,char endmark = 0,GameData* gamedata = 0);
	int getValue(GameData* gamedata) const;
};



#endif
