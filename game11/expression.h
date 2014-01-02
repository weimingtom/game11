#ifndef INCLUDED_EXPRESSION_H
#define INCLUDED_EXPRESSION_H

#include "gamedata.h"

//式

class Expression {
	class Function {
	public:
		virtual ~Function();
		static Function* read(int* cont,GameData* const,const char*);
		static int readString(const char*,char end = ')');						//)が来るまでの文字数を得る
		virtual int readParameter(GameData*,const char*) = 0;
		virtual int getValue(GameData*) = 0;
	};

	class Term{
	public:
		bool mAvailable;			//からっぽかも知れないので
		char mOperator;				//演算子

		//どれか一つが作用する
		Function* mFunction;
		Expression* mExpression;			//括弧でくくられてる場合はこいつで
		int mOperand;						//数値

		Term();
		~Term();
		int set(const char* exp,GameData*);
		int operand(GameData*) const;
	};


	static const int mMax = 16;		//項の最大数
	Term* mTerm[mMax];
public:
	Expression();
	~Expression();

	int set(const char*,char endmark = 0,GameData* gamedata = 0);
	int getValue(GameData* gamedata) const;
};



#endif
