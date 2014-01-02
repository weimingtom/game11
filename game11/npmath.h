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
//算数
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
		void getCuboid(GameLib::Math::Vector3* min,GameLib::Math::Vector3* max)const;	//収めることのできる長方体
	};

	void getAngle(float *x,float *y,const GameLib::Math::Vector3&);	//Z軸を向ける回転角度を得る


	float asymptote(float x,float alpha,float beta);	//alpha極限　x^2==betaで、alpha/2になる
	void getAsymptoteConstance(float* alpha,float* beta ,float finalX,float finalY,float K);	//成長の遅さ　最終Ｘ、Ｙ、k成長のおそさ0で一定値Ｙ最大1.f　でalphaとbetaが欲しい

	float asymptote2(float x,float alpha,float beta);	//alpha極限 x=betaのとき、alpha/2になる

	float floatMod(float i,float n);

	void rotateVector3(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& vct,float x,float y,float z) ;
	void angleToVector3(GameLib::Math::Vector3* out,float horizotalAngle,float verticalAngle);
	void vector3ToAngle(float *h,float *v,const GameLib::Math::Vector3&);
	void unsignedToColor(float* a,float* r,float* g,float* b,unsigned int color);
	void unsignedToColor(GameLib::Math::Vector3*,unsigned int color);
	unsigned int getUnsignedColor(float alpha,float red,float green,float blue);
	void nearSquare(int*x,int* y,int i,int j,int n);	//スクエアマスを近い順に得る
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

	bool implicate(const GameLib::Math::Vector3&,const Polygon&);	//点と矩形の包含
	bool collision(const Polygon&,const Polygon&);	//多角形同士の当たり判定
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


//数値をつっこんでおくと最大・最小を出してくれる
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

//補間クラス
class Interpolation{
	//区間
	class Interval{
		bool mReady;	//calcは一回だけ

		//mA2 *t*t + mA1*t +mA0
		float mA0,mA1,mA2,mA3;
	public:
		float mTime;	//区間の最大(幅ではない)

		//生成用の元データ
		float mStart;		//始点

		Interval();
		void calculate1(float final,float time);		//最終値と時間幅
		void calculate2(float final,float time,float t1,float velocity);				//二次補間
		void calculate3(float final,float time,float t1,float velocity,float accel);	//一点で速度と加速度指定
		void calculate3XV(float final,float time,float t1,float X,float V);				//一点で大きさと速度
		void calculate3(float final,float time,float t1,float v1,float t2,float v2);	//二点で速度指定
		float value(float t)const;
		float velocity(float t)const;
		float accel(float t)const;
	};
	//点
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
	//生成用パラメータ
	int mDegree;		//何次補間か？
	int mPointNumber;
	Point mPoint[2];	//制御点

	Interpolation();
	void operator=(Interpolation);		//コピー禁止
	void add(float value,float time);	//区間の初期値と、最終時間
	void calculate();					//データそろった、計算！
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
	void getVector3(GameLib::Math::Vector3*out,float r);					//半径rの範囲で乱数
	void getVector3(GameLib::Math::Vector3*out,float x,float y,float z);	//-x～xの範囲で乱数
	void getAngle(GameLib::Math::Vector3* out,float h=360.f,float v=180.f);
	void rotateVector3(GameLib::Math::Vector3*out,const GameLib::Math::Vector3&,float x=180.f,float y=180.f,float z=180.f);
	unsigned int getColor();
	static Random* instance();
	static void create();
	static void destroy();

	static Random* mInstance;	//シングルトンにはしない.　stageはメンバでrandomを持つように。
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

