#ifndef INCLUDED_NPBASE_H
#define INCLUDED_NPBASE_H

#include "GameLib/Framework.h"

template <class SomeClass>
class List{
	class Member{
	public:
		SomeClass mValue;
		Member* mNext;
		Member* mPrev;

		Member();
		template<class T>Member(T);
		template<class T,class S>Member(T,S);
		template<class T,class S,class R>Member(T,S,R);
		~Member();	//mNext��delete����
	};

	Member* mHead;
	Member* mLast;

public:
	//�����q
	class Iterator{
		Member* mPosition;

	public:
		Iterator(const List&);
		bool end();
		void operator++();
		SomeClass* operator()() const;

	};

	List();
	~List();
	void release();
	SomeClass* add();					//���������Ȃ�
	SomeClass* insert(int i);
	template<class T>SomeClass* addInitializing(T);	//����������
	template<class T,class S>SomeClass* addInitializing(T,S);	//����������
	template<class T,class S,class R>SomeClass* addInitializing(T,S,R);	//����������

	void operator=(List<SomeClass>);	//�R�s�[�֎~

	void remove(int i);
	SomeClass* operator()(int i) const;
	SomeClass* last() const;
	int number() const;

private:
	Member* member(int i) const;
	void add(Member*,int i);
};


////////////////////////////////////////////////////////////////////////
//template�N���X�̒��g
template<class SomeClass>
List<SomeClass>::List() {
	mHead = 0;
	mLast = 0;
};

template<class SomeClass>
List<SomeClass>::~List() {
	release();
};
template<class SomeClass>
void List<SomeClass>::release() {
	if (mHead) {
		delete mHead;
		mHead = 0;
	}
	mLast = 0;
};


template<class SomeClass>
SomeClass* List<SomeClass>::add() {
	return insert(-1);
}

template<class SomeClass>
SomeClass* List<SomeClass>::insert(int i) {
	Member* n = new Member();
	add(n,i);
	return &n->mValue;
}

template<class SomeClass>
template<class T>
SomeClass* List<SomeClass>::addInitializing(T t) {
	Member* n = new Member(t);
	add(n,-1);
	return &n->mValue;
}

template<class SomeClass>
template<class T,class S>
SomeClass* List<SomeClass>::addInitializing(T t,S s) {
	Member* n = new Member(t,s);
	add(n,-1);
	return &n->mValue;
}


template<class SomeClass>
template<class T,class S,class R>
SomeClass* List<SomeClass>::addInitializing(T t,S s,R r) {
	Member* n = new Member(t,s,r);
	add(n,-1);
	return &n->mValue;
}


template<class SomeClass>
void List<SomeClass>::add(typename List<SomeClass>::Member* m,int i) {
	//�����ɂ���
	if (i<0) {
		i = number();
	}
	ASSERT(i<=number());

	Member* n = member(i);
	Member* p = member(i-1);

	m->mNext = n;
	m->mPrev = p;

	if (n) {
		n->mPrev = m;
	}
	else {
		mLast = m;
	}

	if (p) {
		p->mNext = m;
	}
	else {
		mHead = m;
	}

};

template<class SomeClass>
void List<SomeClass>::remove(int i) {
	Member* m = member(i);
	Member* prev = m->mPrev;
	Member* next = m->mNext;

	m->mNext = 0;	//�����O�ɐ؂藣���Ă���
	delete m;
	m = 0;

	if (prev) {
		prev->mNext = next;
	}
	else {
		mHead = next;	//�擪�̂��������Ƃ�
	}

	if (next) {
		next->mPrev = prev;
	}
	else {
		mLast = prev;	//�Ō���̂��������Ƃ�
	}
};


template<class SomeClass>
typename List<SomeClass>::Member* List<SomeClass>::member(int i) const {
	if (i<0) {
		return 0;
	}

	Member* result = mHead;
	for (int t=0 ; t<i ; ++t) {
		if (result==0) {
			break;
		}
		result = result->mNext;
	};

	return result;
}

template<class SomeClass>
SomeClass* List<SomeClass>::operator()(int i) const{
	Member* m = member(i);
	if (m) {
		return &m->mValue;
	}
	else {
		return 0;
	}
};

template<class SomeClass>
SomeClass* List<SomeClass>::last() const{
	return &mLast->mValue;
};

template<class SomeClass>
int List<SomeClass>::number()const {
	int result = 0;
	Member* m = mHead;
	while(m) {
		m = m->mNext;
		++result;
	};
	return result;
};
//�C�e���[�^
template<class SomeClass>
List<SomeClass>::Iterator::Iterator(const List& list) {
	mPosition = list.mHead;
}

template<class SomeClass>
bool List<SomeClass>::Iterator::end() {
	return mPosition==0;
}

template<class SomeClass>
void List<SomeClass>::Iterator::operator++() {
	if (mPosition) {
		mPosition = mPosition->mNext;
	}
}
template<class SomeClass>
SomeClass* List<SomeClass>::Iterator::operator()() const{
	return &mPosition->mValue;
}

//�����o


template <class SomeClass>
List<SomeClass>::Member::Member() {
	mNext = 0;
	mPrev = 0;
}

template <class SomeClass>
template <class T>
List<SomeClass>::Member::Member(T t) :
mValue(t){
	mNext = 0;
	mPrev = 0;
}

template <class SomeClass>
template <class T,class S>
List<SomeClass>::Member::Member(T t,S s) :
mValue(t,s){
	mNext = 0;
	mPrev = 0;
}
template <class SomeClass>
template <class T,class S,class R>
List<SomeClass>::Member::Member(T t,S s,R r) :
mValue(t,s,r){
	mNext = 0;
	mPrev = 0;
}


template <class SomeClass>
List<SomeClass>::Member::~Member() {
	if (mNext) {
		delete mNext;
		mNext = 0;
	}
};



#endif

