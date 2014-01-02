#ifndef INCLUDED_NPMATH_H
#define INCLUDED_NPMATH_H
#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix34.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Functions.h"
#include "GameLib/Math/Random.h"

#include "base.h"
class Tag;
//�Z��
namespace Math{
	class Segment{
	public:
		GameLib::Math::Vector3 mStart;
		GameLib::Math::Vector3 mEnd;
	};
	class Polygon{
	public:
		static const int mMax = 16;
		GameLib::Math::Vector3 mVertex[mMax];
		int mNumber;

		Polygon();
		void segment(Segment* out,int)const;
		void set(const Segment&);
		void mul(Polygon* out,const GameLib::Math::Matrix34&)const;
		void read(const Tag&);
		void add(const GameLib::Math::Vector3&);
		void operator *=(float);
		void getCuboid(GameLib::Math::Vector3* min,GameLib::Math::Vector3* max)const;	//���߂邱�Ƃ̂ł��钷����
	};

	void getAngle(float *x,float *y,const GameLib::Math::Vector3&);	//Z�����������]�p�x�𓾂�


	float asymptote(float x,float alpha,float beta);	//alpha�Ɍ��@x^2==beta�ŁAalpha/2�ɂȂ�
	void getAsymptoteConstance(float* alpha,float* beta ,float finalX,float finalY,float K);	//�����̒x���@�ŏI�w�A�x�Ak�����̂�����0�ň��l�x�ő�1.f�@��alpha��beta���~����

	float asymptote2(float x,float alpha,float beta);	//alpha�Ɍ� x=beta�̂Ƃ��Aalpha/2�ɂȂ�

	float floatMod(float i,float n);

	void rotateVector3(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& vct,float x,float y,float z) ;
	void angleToVector3(GameLib::Math::Vector3* out,float horizotalAngle,float verticalAngle);
	void vector3ToAngle(float *h,float *v,const GameLib::Math::Vector3&);
	void unsignedToColor(float* a,float* r,float* g,float* b,unsigned int color);
	void unsignedToColor(GameLib::Math::Vector3*,unsigned int color);
	unsigned int getUnsignedColor(float alpha,float red,float green,float blue);
	void nearSquare(int*x,int* y,int i,int j,int n);	//�X�N�G�A�}�X���߂����ɓ���
	void bezier(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& control,const GameLib::Math::Vector3& goal);
	float bezier(float t,float start,float control,float goal);

	void differentialOfbezier(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& control,const GameLib::Math::Vector3& goal);
	float differentialOfbezier(float t,float start,float control,float goal);

	void lerp(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& goal);
	float lerp(float t,float start,float goal);
	void addRandom(GameLib::Math::Vector3*,float x,float y,float z);
	void addRandom(float* out,float k);
	void colorCircle(GameLib::Math::Vector3*,float r);
	bool nearOne(const GameLib::Math::Vector3&);
	bool nearOne(float);

	float spacePartition(const GameLib::Math::Vector3&,const GameLib::Math::Vector3& p1,const GameLib::Math::Vector3& p2,const GameLib::Math::Vector3& p3);

	unsigned int bitFlag(unsigned int,int,bool);
	bool getFlag(unsigned int,int);

	bool implicate(const GameLib::Math::Vector3&,const Polygon&);	//�_�Ƌ�`�̕��
	bool collision(const Polygon&,const Polygon&);	//���p�`���m�̓����蔻��
	bool crossLineSegmentLineSegment(const Segment&,const Segment&);
	bool crossLineSegmentLine(const Segment&,const GameLib::Math::Vector3&,const GameLib::Math::Vector3&);



}

void normalize(double*,double*,double,double);
int normalize(double);
float getLength(float,float);


class DynamicValue {
	class Bezier {
		float mValue;
		float mStart;
		float mControl;
		float mEnd;
		float mFrame;
		int mCount;
	public:
		Bezier();
		void initialize();
		float value()const ;
		void update();
		void set(int frame,float P0,float P1,float P2);
		void stop();
	};

	float mValue;
public:
	float mVelocity;
	float mAccel;
	float mResistance;

	Bezier mBezier;

	DynamicValue();
	void initialize();
	float value()const;

	void set(float);
	void update();
	void stop();
};

class DynamicVector3{
public:
	DynamicValue mX;
	DynamicValue mY;
	DynamicValue mZ;

	void initialize();
	void value(GameLib::Math::Vector3* out)const;
	void set(float x,float y, float z);
	void update();
	void stop();
};


//���l��������ł����ƍő�E�ŏ����o���Ă����
template <class T>
class RangeGenerator{
	T mMax;
	T mMin;
	int mMaxIndex;
	int mMinIndex;
	int mNumber;
public:
	RangeGenerator(T);
	void initialize(T n);
	void add(T n,int index = -1);
	T max() const;
	T min() const;
	int number() const;
};

//��ԃN���X
class Interpolation{
	//���
	class Interval{
		bool mReady;	//calc�͈�񂾂�

		//mA2 *t*t + mA1*t +mA0
		float mA0,mA1,mA2,mA3;
	public:
		float mTime;	//��Ԃ̍ő�(���ł͂Ȃ�)

		//�����p�̌��f�[�^
		float mStart;		//�n�_

		Interval();
		void calculate1(float final,float time);		//�ŏI�l�Ǝ��ԕ�
		void calculate2(float final,float time,float t1,float velocity);				//�񎟕��
		void calculate3(float final,float time,float t1,float velocity,float accel);	//��_�ő��x�Ɖ����x�w��
		void calculate3XV(float final,float time,float t1,float X,float V);				//��_�ő傫���Ƒ��x
		void calculate3(float final,float time,float t1,float v1,float t2,float v2);	//��_�ő��x�w��
		float value(float t)const;
		float velocity(float t)const;
		float accel(float t)const;
	};
	//�_
	class Point{
	public:
		float mTime;
		float mValue;
		float mVelocity;
		float mAccel;
		Point();
	};

	List<Interval> mInterval;

public:
	//�����p�p�����[�^
	int mDegree;		//������Ԃ��H
	int mPointNumber;
	Point mPoint[2];	//����_

	Interpolation();
	void operator=(Interpolation);		//�R�s�[�֎~
	void add(float value,float time);	//��Ԃ̏����l�ƁA�ŏI����
	void calculate();					//�f�[�^��������A�v�Z�I
	void calculate(int interval,float T,float V,float A);
	void calculate(int interval,const Point*,int number);
	float value(float t) const;
};
class Interpolation3{
public:
	Interpolation mX,mY,mZ;

	void add(const GameLib::Math::Vector3&,float time);
	void calculate();
	void value(GameLib::Math::Vector3* out,float t)const;
};



class Random {
	GameLib::Math::Random mRandom;
	int mSeed;
public:
	void release();
	void setSeed(int);
	int getInt();
	int getInt(int);
	bool getBool();
	float getFloat(float);
	void getVector3(GameLib::Math::Vector3*out,float r);					//���ar�͈̔͂ŗ���
	void getVector3(GameLib::Math::Vector3*out,float x,float y,float z);	//-x�`x�͈̔͂ŗ���
	void getAngle(GameLib::Math::Vector3* out,float h=360.f,float v=180.f);
	void rotateVector3(GameLib::Math::Vector3*out,const GameLib::Math::Vector3&,float x=180.f,float y=180.f,float z=180.f);
	unsigned int getColor();
	static Random* instance();
	static void create();
	static void destroy();

	static Random* mInstance;	//�V���O���g���ɂ͂��Ȃ�.�@stage�̓����o��random�����悤�ɁB
	Random();
	~Random();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
RangeGenerator<T>::RangeGenerator(T n) {
	initialize(n);
};

template<class T>
void RangeGenerator<T>::initialize(T n) {
	mNumber = 0;
	mMax = n;
	mMin = n;
	mMaxIndex = -1;
	mMinIndex = -1;
}


template<class T>
void RangeGenerator<T>::add(T n,int index) {
	if (mMax < n || mNumber==0) {
		mMax = n;
		mMaxIndex = index;
	}
	if (mMin > n || mNumber==0) {
		mMin = n;
		mMinIndex = index;
	}
	++mNumber;
};

template<class T>
T RangeGenerator<T>::max() const{
	return mMax;
};
template<class T>
T RangeGenerator<T>::min() const{
	return mMin;
};
template<class T>
int RangeGenerator<T>::number() const{
	return mNumber;
}


#endif

