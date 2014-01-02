
#include "expression.h"
#include "gamedata.h"
#include "csv.h"

//式計算
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
	int i=0;		//検索位置

	int mode = 0;	//0オペレーターさがすモード　1オペランド探すモード　2数字読解モード　3関数読解モード
	int value = 0;
	char end = 0;		//終端?

	while(true) {
		if (mode == 0) {
			//オペレーターみっけ
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

			//数字もしくはアルファベットなら、モードを切り替える
			if (exp[i] >='0' && exp[i]<='9') {
				mode = 2;
				continue;
			};

			//アルファベットがきたなら、関数つくっておしまい
			if (exp[i] >='A' && exp[i]<='z') {
				int r=0;
				mFunction = Function::read(&r,gamedata,exp+i);
				i += r;
				break;
			};
			//式が来たら、Expressionを作っておしまい
			if (exp[i] == '(') {
				++i;
				mExpression = new Expression;
				i += mExpression->set(exp+i,')',gamedata);
				break;
			}

			//終端文字列候補がきたら終了。空のままかも知れないけどかまわない
			if (exp[i] == 0 || exp[i] == ')') {
				mAvailable = false;
				break;
			}
			
			//それ以外なら、半角スペースしか認めない
			ASSERT(exp[i] == ' ');
			++i;
		}
		//オペランド探すモード
		if (mode == 1) {
			//数字もしくはアルファベットなら、モードを切り替える
			if (exp[i] >='0' && exp[i]<='9') {
				mode = 2;
				continue;
			};
			//アルファベットがきたなら、関数つくっておしまい
			if (exp[i] >='A' && exp[i]<='z') {
				int r=0;
				mFunction = Function::read(&r,gamedata,exp+i);
				i += r;
				break;
			};
			//式が来たら、Expressionを作っておしまい
			if (exp[i] == '(') {
				++i;
				mExpression = new Expression;
				i += mExpression->set(exp+i,')',gamedata);
				break;
			}

			//それ以外なら、半角スペースしか認めない
			ASSERT(exp[i] == ' ');
			++i;
		}
		//数字読解モード
		if (mode == 2) {
			if (exp[i] >='0' && exp[i]<='9') {
				value = value*10 + (exp[i] - '0');
				++i;
				continue;
			};
			//数字以外がきたら、終了　16進モードとか欲しいけど。
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

	//termを作って最後まで行こう
	for (int i=0 ; ; ++i) {
		ASSERT(i<mMax);
		mTerm[i] = 0;

		//終端文字がきたら終了
		if (exp[result] == endmark) {
			break;
		};

		Term* term = new Term;
		result += term->set(exp + result,gamedata);
		if (term->mAvailable) {
			mTerm[i] = term;
		}
		else {
			//からっぽでした。終了。
			delete term;
			term = 0;
			break;
		}
	}

	//終端文字は正しいか？
	ASSERT(exp[result] == endmark);

	return result+1;
};


//計算しよう
int Expression::getValue(GameData *gamedata) const {
	int result = 0;
	for (int i=0 ; i<mMax ; ++i) {
		if (mTerm[i] == 0) {
			break;
		}
		int p= mTerm[i]->operand(gamedata);

		//演算子処理
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
//関数群
//必要な関数
//戦闘編
//味方の数
//敵の数
//指定ユニットが生きてるか？　・・文字列を受け取れないと駄目ジャン
//

Expression::Function::~Function() {
};

Expression::Function* Expression::Function::read(int* const i,GameData* const gamedata,const char *func) {
	//単語読解
	//()内が式になるか文字列になるかで処理がかわるので注意。

	Function* result = 0;

	//読解しないとなあ・・・
	MyString name(64);
 	//とりあえずカッコを探すか

	for (*i=0;true; ++ *i) {
		if (func[*i] == '(') {
			name.add(func,*i);
			break;
		}
		ASSERT(func[*i]);	//終端文字がきたらエラー
	}



	++ *i;
	//引数を読む　数式タイプと文字列タイプがあるぞ。
	int j = result->readParameter(gamedata,func + *i);

	*i += j;
	return result;
}

//かっこ閉じを探す
int Expression::Function::readString(const char* str,char end) {
	int i;
	for (i=0 ; true ; ++i){
		char t = str[i];
		ASSERT(t);			//終端文字だったらエラー

		if (t==end) {
			break;
		}

		//二バイト文字回避
		if (t<0) {
			++i;
		}
	}
	return i;
};

