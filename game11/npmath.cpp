#include  <time.h>

#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix34.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Functions.h"
#include "GameLib/Math/Random.h"


#include "npmath.h"
#include "npdraw.h"
#include "nml.h"

namespace Math{
	void getAngle(float *x,float *y,const GameLib::Math::Vector3& dir) {
		//Y軸角度はatan2( x, z )。
		float ty = GameLib::Math::atan2( dir.x, dir.z ); 
		//180度以上差があれば+-360度して逆回し
		//if ( ty - a.y > 180.f ){
		//	ty -= 2.f * 180.f;
		//}else if ( a.y - ty > 180.f ){
		//	ty += 2.f * 180.f;
		//}
		//X軸角度はY/(X,Z)。
		float zxLength = GameLib::Math::sqrt( dir.x * dir.x + dir.z * dir.z );
		float tx = GameLib::Math::atan2( dir.y, zxLength );

		*x = -tx;
		*y = ty;
	};

	//サガ式漸近線　αが極限、ベータが大きいほど変化がゆるやかになる
	float asymptote(float x,float alpha,float beta) {
		if (beta==0.f) {
			return alpha;
		}
		else {
			return (alpha * x*x) / (beta + x*x);
		}
	}
	//漸近線につかう定数を得る
	void getAsymptoteConstance(float* alpha,float* beta,float X,float Y,float K) {
		if (K == 0) {
			*alpha = Y;
			*beta = 0;
		}
		else {
			float y = Y;
			if (y<0.f) {
				y=-y;
			}
			//int k = K * X*X/y/101;
			//*alpha = X*X*y / (X*X-k*y);
			//*beta = k*X*X*y / (X*X-k*y);

			ASSERT(K<=1.f && K>=0.f);

			//*alpha = 1.f * y / (1- K/M);
			//*beta = 1.f * K*X*X/M / (1-K/M);
			*alpha = 1.f * y / (1.f-K);
			*beta  = 1.f * K*X*X/ (1.f-K);

			if (Y<0.f) {
				*alpha *= -1.f;
			}
		}
	};

	//目標X,Y
	//Y = (a*X*X) / (b+ X*X);
	//(b + X*X)Y = a *X*X;

	//成長のおそさをKとおく
	//b = Ka;

	//k=0のとき、
	//a = Y;
	//b = 0;

	//(Ka + X*X)Y = a*X*X;
	//(XX-KY)a = XXY;
	//a = XXY/(XX-KY);
	//b = KXXY / (XX - KY);

	//ただし XX > KY

	//Kの範囲は 0<K<XX/Y

	float asymptote2(float x,float a,float b) {

		//r = a*x/(x+b);

		//a/2  = a*x/(x+b);
		//1/2=x/(x+b);
		//(x+b)/2 = x;
		// b = x;
		//a*x/(x+b):

		float alpha = a;
		float beta = b;
		return alpha * x / (x+beta);
	};

	float floatMod(float a,float n) {
		ASSERT(n>0.f);
		for (int i=0 ; true ; ++i) {
			if (a>=0.f) {
				if ((a - n*i) < n) {
					a -= n*i;
					break;
				}
			}
			else {
				if ((a + n*i) >= 0.f) {
					a += n*i;
					break;
				}
			}
		}
		return a;
	};

	//ベクトルの回転。たぶんもうあるのがな。
	void rotateVector3(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& vct,float a,float b,float c) {
		float x,y,z,p,q,r;
		x=vct.x;
		y=vct.y;
		z=vct.z;

		p=x;
		q=y;
		r=z;
		if (c!=0.f) {
			//Z軸回転
			p=(x*GameLib::Math::cos(c))+(y*GameLib::Math::sin(c));
			q=(y*GameLib::Math::cos(c))-(x*GameLib::Math::sin(c));
			x=p;
			y=q;
		}
		if (a!=0.f) {
			//x軸回転
			q=(y*GameLib::Math::cos(a))+(z*GameLib::Math::sin(a));
			r=(z*GameLib::Math::cos(a))-(y*GameLib::Math::sin(a));
			y=q;
			z=r;
		}
		if (b!=0.f) {
			//y軸回転
			p=(x*GameLib::Math::cos(b))+(z*GameLib::Math::sin(b));
			r=(z*GameLib::Math::cos(b))-(x*GameLib::Math::sin(b));
			x=p;
			z=r;
		}

		out->x = p;
		out->y = q;
		out->z = r;
	}

	void angleToVector3(GameLib::Math::Vector3* out,float ha,float va) {
		out->set(
			GameLib::Math::cos(ha) * GameLib::Math::cos(va),
			GameLib::Math::sin(ha) * GameLib::Math::cos(va),
			GameLib::Math::sin(va));
	};
	void vector3ToAngle(float *h,float *v,const GameLib::Math::Vector3& vct) {
		if (h) {
			*h = GameLib::Math::atan2(vct.y,vct.x);
		}
		if (v) {
			*v = GameLib::Math::atan2(vct.z,GameLib::Math::sqrt((vct.x+vct.y)*(vct.x+vct.y)));
		}

	};

	void unsignedToColor(float* a,float* r,float* g,float* b,unsigned int color) {
		if (a) {
			*a = 1.f*((color >> (4*6)) & 0xff)/255.f;
		}
		*r = 1.f*((color >> (4*4)) & 0xff)/255.f;
		*g = 1.f*((color >> (4*2)) & 0xff)/255.f;
		*b = 1.f*((color >> (4*0)) & 0xff)/255.f;
	}
	void unsignedToColor(GameLib::Math::Vector3* out,unsigned int color) {
		unsignedToColor(0,&out->x,&out->y,&out->z,color);
	}

	//マスを近い順に得る
	void nearSquare(int* x,int* y,int i,int j,int n) {
		int p=0;
		int q=0;
		for (int w=0 ; w<n ; ++w) {
			if (p>0 && q>=0) {
				--p;
				++q;
				continue;
			}
			if (p<=0 && q>0) {
				--p;
				--q;
				continue;
			}
			if (p<0 && q <=0) {
				++p;
				--q;
				continue;
			}
			if (p>=0 && q<0) {
				++p;
				++q;
			}
			if (q == 0) {
				++p;
			}
		}
		*x = i+p;
		*y = j+q;
	};

	//色をキャスト
	//ループさせるか、カンストさせるかは悩みどころではある
	unsigned int getUnsignedColor(float alpha,float red,float green,float blue){

		int a,r,g,b;
		a=static_cast<unsigned int>(alpha*255.f);
		if (a<0) {
			a=0;
		}
		if (a>255) {
			a=255;
		}

		r=static_cast<unsigned int>(red*255.f);
		if (r<0) {
			r=0;
		}
		if (r>255) {
			r=255;
		}
		g=static_cast<unsigned int>(green*255.f);
		if (g<0) {
			g=0;
			}
		if (g>255) {
			g=255;
		}
		b=static_cast<unsigned int>(blue*255.f);
		if (b<0) {
			b=0;
		}
		if (b>255) {
			b=255;
		}

		return (
			  a * 0x01000000
			+ r * 0x00010000
			+ g * 0x00000100
			+ b * 0x00000001
			);
	}

	float bezier(float t, float start,float control,float goal) {
		float p = lerp(t,start,control);
		float q = lerp(t,control,goal);
		return lerp(t,p,q);
	};

	void bezier(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& control,const GameLib::Math::Vector3& goal) {

		GameLib::Math::Vector3 tmp;

		lerp(out,t,start,control);
		lerp(&tmp,t,control,goal);

		*out *= 1.f -t;
		out->madd(tmp,t);
	}

	void differentialOfbezier(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& control,const GameLib::Math::Vector3& goal) {
		out->x = differentialOfbezier(t,start.x,control.x,goal.x);
		out->y = differentialOfbezier(t,start.y,control.y,goal.y);
		out->z = differentialOfbezier(t,start.z,control.z,goal.z);
	}

	float differentialOfbezier(float t,float p0,float p1,float p2) {

		//a = p0*(1-t)+p1*t
		//b = p1*(1-t)+p2*t
		//x = a*(1-t) +b*t;
		//x = (p0*(1-t)+p1*t) * (1-t) +  (p1*(1-t)+p2*t)*t
		 // = p0*(1-t)^2 + p1*t*(1-t) + p1*(1-t)*t + p2*t^2
		//  = p0*(1-t)^2 + 2p1*t*(1-t) + p2*t^2
		//  = t^2*( p0 - 2p1 + p2 )
		//   +t(-2p0 + 2p1)
		//   +p0
		//dx/dt = 2t(p0 - 2p1 + p2)
		  //     -2(p0-p1);
		return 2.f*t*(p0-2*p1+p2) - 2.f*(p0-p1);

	}

	
	void lerp(GameLib::Math::Vector3* out,float t,const GameLib::Math::Vector3& start,const GameLib::Math::Vector3& goal) {
		*out = start;
		*out *= 1.f - t;
		out->madd(goal,t);
	}
	float lerp(float t,float start,float goal) {
		return start*(1.f - t) + goal*t;
	};

	void colorCircle(GameLib::Math::Vector3* out,float r) {
		r = floatMod(r,360.f);
		//0-120赤
		//120-240緑
		//240-360青
		float c[3];
		for (int i=0 ; i<3 ; ++i) {
			float a = r - (i*120);
			if (a>180.f) {
				a -= 360.f;
			}
			if (a<-180.f) {
				a += 360.f;
			}
			if (a<0.f) {
				a *= -1.f;
			}

			c[i] = (120.f - a) / 60.f;
			if (c[i]>1.f) {
				c [i]= 1.f;
			}
			if (c[i]<0.f) {
				c[i]=0.f;
			}

		}
		out->x = c[0];
		out->y = c[1];
		out->z = c[2];
	};
	bool nearOne(const GameLib::Math::Vector3& src) {
		if (!nearOne(src.x)) {
			return false;
		}
		if (!nearOne(src.y)) {
			return false;
		}
		if (!nearOne(src.z)) {
			return false;
		}
		return true;
	};

	bool nearOne(float src) {
		return ((src-1.f) * (src-1.f)) < (0.01f*0.01f);
	}

	float spacePartition(const GameLib::Math::Vector3& v,const GameLib::Math::Vector3& p1,const GameLib::Math::Vector3& p2,const GameLib::Math::Vector3& p3) {
		//まず面の垂線を出す
		GameLib::Math::Vector3 a;
		GameLib::Math::Vector3 b;

		a=p2;
		a-=p1;
		b =p3;
		b-=p1;
		GameLib::Math::Vector3 c;
		c.setCross(a,b);

		//内積を見れば終了
		GameLib::Math::Vector3 u=v;
		u -= p1;

		float d = u.dot(c);
		return d;
		//どのくらいの精度が出るかは不明なので、渡した先が判断すること

	//	if (d*d < 0.01f * 0.01f) {	//ゼロ付近はゼロ付近と返すこと
	//		return 0;
	//	}
	//	if (d>0.f) {
	//		return 1;
	//	}
	//	return -1;
	};

	unsigned int bitFlag(unsigned int v, int i,bool b) {
		ASSERT(i<32);
		unsigned int a = (1 << i);

		if (b) {
			v |= a;
		}
		else {
			v &= ~a;
		}

		return v;
	};
	bool getFlag(unsigned int v,int i) {
		ASSERT(i<32);
		unsigned int a = (1 << i);
		return (v & a);
	}


	//包含判定
	bool implicate(const GameLib::Math::Vector3& point,const Polygon& pol){

		ASSERT(pol.mNumber >2);

		bool result = true;
		float r;

		Segment seg;
		for (int k=0 ; k<pol.mNumber ; ++k) {
			pol.segment(&seg,k);

			GameLib::Math::Vector3 i;
			GameLib::Math::Vector3 j;

			i.x = point.x - seg.mStart.x;
			i.y = point.y - seg.mStart.y;
			i.z = 0.f;

			j.x = seg.mEnd.x - seg.mStart.x;
			j.y = seg.mEnd.y - seg.mStart.y;
			j.z = 0.f;

			GameLib::Math::Vector3 c;
			c.setCross(i,j);

			if (k) {
				if ((r * c.z) < 0.f) {
					result = false;
					break;
				};
			}
			else {
				r = c.z;
			}

		}

		return result;
	};

	bool crossLineSegmentLine(const Segment& s,const GameLib::Math::Vector3& p1,const GameLib::Math::Vector3& p2) {
		GameLib::Math::Vector3 v;
		v = p1;
		v -= p2;

		GameLib::Math::Vector3 p;
		GameLib::Math::Vector3 q;

		p = s.mStart;
		p -= p2;

		q = s.mEnd;
		q -= p2;

		GameLib::Math::Vector3 cp;
		GameLib::Math::Vector3 cq;
		cp.setCross(p,v);
		cq.setCross(q,v);

		return (cp.dot(cq) < 0.f);

	}

	//線分の交差判定
	bool crossLineSegmentLineSegment(const Segment& s1,const Segment& s2) {
		bool c1 = crossLineSegmentLine(s1,s2.mStart,s2.mEnd);
		bool c2 = crossLineSegmentLine(s2,s1.mStart,s1.mEnd);

		if ( c1 && c2){
			return true;
		}
		return false;
	}

	//多角形の当たり判定
	bool collision(const Polygon& p1,const Polygon& p2) {

		//包含判定　線分に対しては行わない

		if (p2.mNumber>2) {
			if (implicate(p1.mVertex[0],p2)) {
				return true;
			}
		}
		if (p1.mNumber>2) {
			if (implicate(p2.mVertex[0],p1)) {
				return true;
			}
		}

		//交差判定
		Segment s1,s2;
		for (int i=0 ; i<p1.mNumber; ++i) {
			p1.segment(&s1,i);
			for (int j=0 ; j<p2.mNumber ; ++j) {
				p2.segment(&s2,j);

				if (crossLineSegmentLineSegment(s1,s2)) {
					return true;
				}
			}
		}
		return false;
	}
	Polygon::Polygon() {
		mNumber = 0;
	}
	void Polygon::segment(Math::Segment *out, int i) const {
		out->mStart = mVertex[i%mNumber];
		out->mEnd = mVertex[(i+1)%mNumber];
	};
	void Polygon::set(const Math::Segment & seg){
		mNumber = 2;
		mVertex[0] = seg.mStart;
		mVertex[1] = seg.mEnd;
	};
	void Polygon::mul(Polygon* out,const GameLib::Math::Matrix34& mtx)const {
		out->mNumber = this->mNumber;

		for (int i=0 ; i<mNumber ; ++i) {
			mtx.mul(&out->mVertex[i],this->mVertex[i]);
		};
	}
	void Polygon::read(const Tag & tag) {
		if (tag.child("rect")) {
			mNumber = 4;

			float w = tag.child("rect")->attribute("size")->getIntValue(0);
			float h = tag.child("rect")->attribute("size")->getIntValue(1);

			mVertex[0].x = -w/2.f;
			mVertex[0].y = +h/2.f;
			mVertex[0].z = 0.f;

			mVertex[1].x = +w/2.f;
			mVertex[1].y = +h/2.f;
			mVertex[1].z = 0.f;

			mVertex[2].x = +w/2.f;
			mVertex[2].y = -h/2.f;
			mVertex[2].z = 0.f;

			mVertex[3].x = -w/2.f;
			mVertex[3].y = -h/2.f;
			mVertex[3].z = 0.f;
		}
		if (tag.child("polygon")) {
			mNumber = tag.child("polygon")->attribute("number")->getIntValue();
			float w = tag.child("polygon")->attribute("size")->getIntValue(0);
			float h = tag.child("polygon")->attribute("size")->getIntValue(1);

			for (int i=0 ; i<mNumber ; ++i) {
				float r = 360.f * i / mNumber;
				mVertex[i].x = w/2.f * GameLib::Math::cos(r);
				mVertex[i].y = h/2.f * GameLib::Math::sin(r);
				mVertex[i].z = 0.f;
			}
		}
		if (tag.getAttribute("vertex")) {
			//頂点を数える
			mNumber = tag.getAttribute("vertex");
			for (int i=0 ; i<mNumber ; ++i) {
				const Attribute* a = tag.attribute("vertex",i);
				mVertex[i].x = a->getFloatValue(0);
				mVertex[i].y = a->getFloatValue(1);
				mVertex[i].z = a->getFloatValue(2);
			}
		}

		if (tag.attribute("position")) {
			GameLib::Math::Vector3 pos;
			pos.x = tag.attribute("position")->getIntValue(0);
			pos.y = tag.attribute("position")->getIntValue(1);
			pos.z = tag.attribute("position")->getIntValue(2);

			for (int i=0 ; i<mNumber ; ++i) {
				mVertex[i] += pos;
			}

		}
	}
	void Polygon::add(const GameLib::Math::Vector3& v) {
		ASSERT(mNumber<mMax);
		mVertex[mNumber] = v;
		++mNumber;
	};

	void Polygon::operator*=(float m) {
		for (int i=0 ; i<mNumber ; ++i) {
			mVertex[i] *= m;
		}
	}
	void Polygon::getCuboid(GameLib::Math::Vector3* mn,GameLib::Math::Vector3* mx) const {
		*mn = mVertex[0];
		*mx = mVertex[0];

		for (int i=1 ; i<mNumber ; ++i) {
			mn->min(mVertex[i]);
			mx->max(mVertex[i]);
		}
	};

};

///////////////////////////////////////////////////////////////////////////////////////////////////////


DynamicValue::DynamicValue() {
	initialize();
}
void DynamicValue::initialize() {
	mValue=0;
	mVelocity=0;
	mAccel=0;
	mResistance=1.f;

	mBezier.initialize();
};

float DynamicValue::value() const{
	return mValue+mBezier.value();
};
void DynamicValue::set(float v) {
	mValue = v - mBezier.value();
};

//変更を適用する。
void DynamicValue::update() {
	mValue += mVelocity;
	mVelocity += mAccel;

	mVelocity *= mResistance;

	mBezier.update();
};
void DynamicValue::stop() {
	mVelocity = 0.f;
	mAccel = 0.f;

	mBezier.stop();
}

DynamicValue::Bezier::Bezier() {
	initialize();
}
void DynamicValue::Bezier::initialize() {
	set(1,0.f,0.f,0.f);
};
void DynamicValue::Bezier::update() {
	mValue = Math::bezier(mCount,mStart,mControl,mEnd);
};
void DynamicValue::Bezier::set(int frame,float p0,float p1,float p2) {
	mCount=0;
	mValue=p0;

	mStart=p0;
	mControl=p1;
	mEnd=p2;
	mFrame=frame;	
	ASSERT(frame>0);
}
void DynamicValue::Bezier::stop() {
	mStart=mValue;
	mControl=mValue;
	mEnd=mValue;
};
float DynamicValue::Bezier::value() const{
	return mValue;
};

void DynamicVector3::initialize() {
	mX.initialize();
	mY.initialize();
	mZ.initialize();
};

void DynamicVector3::value(GameLib::Math::Vector3* out) const {
	out->x = mX.value();
	out->y = mY.value();
	out->z = mZ.value();
};

void DynamicVector3::set(float x,float y,float z) {
	mX.set(x);
	mY.set(y);
	mZ.set(z);
}
void DynamicVector3::stop() {
	mX.stop();
	mY.stop();
	mZ.stop();
};

void DynamicVector3::update() {
	mX.update();
	mY.update();
	mZ.update();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////
Interpolation::Interpolation() {
	mDegree = 1;
	mPointNumber = 1;
};
Interpolation::Point::Point() {
	mTime = 0.f;
	mVelocity = 0.f;
	mAccel = 0.f;
};


void Interpolation::add(float value ,float time) {
	if (mInterval.last()) {
		//初期値差し替え
		if (mInterval.last()->mTime == 0.f) {
			mInterval.last()->mStart = value;
			mInterval.last()->mTime = time;
			return;
		}
	};

	//入れるときにソート
	int k = mInterval.number();		//みつからなければ最後尾に
	for (int i=0 ; i<(mInterval.number()) ; ++i) {
		if (mInterval(i)->mTime > time) {
			k = i;
			break;
		}
	}
	Interval* itv = mInterval.insert(k);
	itv->mStart = value;
	itv->mTime = time;

}
void Interpolation::calculate() {
	//まずパラメータ指定されてる点を探して
	//その前後を計算　面倒だなあ
	int k=-1;
	for (int i=0 ; i<mInterval.number() ; ++i) {
		if (mInterval(i)->mTime >= mPoint[0].mTime) {
			calculate(i,mPoint,mPointNumber);
			k = i;
			break;
		}
	}
	ASSERT(k>=0);	//見つからないってことはないよなあ

	//前後を計算する
	for (int i=k+1 ; i<mInterval.number() ; ++i) {
		float T = mInterval(i-1)->mTime;
		float V = mInterval(i-1)->velocity(T);
		float A = mInterval(i-1)->accel(T);
		calculate(i,T,V,A);
	}
	for (int i=k-1 ; i>=0 ; --i) {
		float T = mInterval(i)->mTime;
		float V = mInterval(i+1)->velocity(T);
		float A = mInterval(i+1)->accel(T);
		calculate(i,T,V,A);
	}
};
void Interpolation::calculate(int i,float T,float V,float A) {
	Point point;
	point.mTime = T;
	point.mVelocity = V;
	point.mAccel = A;
	calculate(i,&point,1);
}
void Interpolation::calculate(int i,const Point* point,int number) {
	//時間幅
	float t = mInterval(i)->mTime;
	if (mInterval(i-1)) {
		t -= mInterval(i-1)->mTime;
	}

	if (mInterval(i+1)) {
		//終端値
		float final = mInterval(i+1)->mStart;

		if (mDegree == 1) {
			mInterval(i)->calculate1(final,t);
		}
		if (mDegree == 2) {
			mInterval(i)->calculate2(final,t,point[0].mTime,point[0].mVelocity);
		}
		if (mDegree == 3) {
			if (number==1) {
				mInterval(i)->calculate3(final,t,point[0].mTime,point[0].mVelocity,point[0].mAccel);
			}
			if (number == 2) {
				mInterval(i)->calculate3(final,t,point[0].mTime,point[0].mVelocity,point[1].mTime,point[1].mVelocity);
			}
		}
	}

};

float Interpolation::value(float t) const {
	ASSERT(mInterval.last());	//なんにも入ってない！
	float result = 0.f;

	//定数の場合
	if (mInterval.last()->mTime == 0) {
		result = mInterval.last()->value(t);
	}
	else {
		//まずはループを排除
		float tt = Math::floatMod(t,mInterval.last()->mTime);

		for (List<Interval>::Iterator i(mInterval) ; !i.end(); i++) {
			//ヒット
			if (i()->mTime > tt) {
				result = i()->value(tt);
				break;
			}
		}
	};
	
	return result;
};

Interpolation::Interval::Interval() {
	mReady = false;
	mA0 = 0.f;
	mA1 = 0.f;
	mA2 = 0.f;
	mA3 = 0.f;
	mTime = 0.f;

	mStart = 0.f;
};
//一次補間
void Interpolation::Interval::calculate1(float final,float time) {
	ASSERT(!mReady);

	if (time != 0.f) {
		mReady = true;
		float c = mStart;
		mA1 = (final - c) / time;
		mA0 = -mA1 * (mTime-time) + c;
	}
};
//二次補間
void Interpolation::Interval::calculate2(float final,float time,float t1,float velo) {
	ASSERT(!mReady);

	if (time != 0.f) {
		mReady = true;

		float t0 = mTime - time;

		//とくべき方程式
		//velocity = 2*mA2*t1 + mA1
		//mStart = mA2*t0*t0 + mA1*t0 + mA0
		//final = mA2*mTime*mTime + mA1*mTime + mA0

		//final - mStart = mA2*(mTime*mtime - t0*t0) + mA1(mTime - t0)
		//final - mStart = mA2*(mTime*mtime - t0*t0) + time*(velocity - 2*mA2*t1);
		//final - mStart = mA2*(mTime*mTime - t0*t0 - 2*time*t1) + time*velocity
		//final - mStart - time*velocity = mA2 * 
		mA3= 0.f;
		mA2 = (final - mStart - time*velo) / (mTime*mTime - t0*t0 - 2.f*time*t1);
		mA1 = velo - 2.f*mA2*t1;
		mA0 = mStart - (mA2*t0*t0 + mA1*t0);
	}
};
//三次補間
void Interpolation::Interval::calculate3(float final,float time,float t1,float velo,float ac){
	ASSERT(!mReady);

	if (time != 0.f) {
		mReady = true;

		float t0 = mTime - time;

		//一点で速度と加速度が指定されているばあい
		//velocity = 3*mA3*t1*t1 + 2*mA2*t1 + mA1
		//accel = 6*mA3*t1 + 2*mA2
		//mStart = mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0 + mA0
		//final = mA3*mTime*mTime*mTime + mA2*mTime*mTime + mA1*mTime + mA0

		//final-mStart = mA3(mTime^3 - t0^3) + mA2(mTime^2 - t0^2) +mA1(mTime-t0);
		//velocity*time = time*(3*mA3*t1^2 + 2*mA2*t1) + mA1*time
		//final-mStart - velocity*time = mA3(mTime^3 - t0^3 - 3*time*t1^2) + mA2(mTime^2-t0^2 - 2*time*t1)

		//K = final-mStart - velocity*time
		//T3 = (mTime^3 - t0^3 - 3*time*t1^2)
		//T2 = (mTime^2 - t0^2 - 2*time*t1)

		//K = mA3 * T3 + mA2 *T2

		//mA2 = accel/2 - 3*t1* mA3
		//K = mA3(T3 - 3*t1*T2) + T2*accel/2
		//mA3 = (K-T2*accel/2)/(T3-3*t1*T2)
		//mA2 = accel/2 - 3*t1 *mA3
		//mA1 = velocity - 3*mA3*t1^2 - 2*mA2*t1
		//mA0 = mStart - (mA3*t0^3 + mA2*t0^2 + mA1*t0);

		float K = final-mStart - velo*time;
		float T3 = mTime*mTime*mTime - t0*t0*t0 - 3.f*time*t1*t1;
		float T2 = mTime*mTime - t0*t0 - 2.f*time*t1;
		mA3 = (K - T2*ac/2.f)/(T3 - 3.f*t1*T2);
		mA2 = ac/2.f - 3.f*t1*mA3;
		mA1 = velo - 3.f*mA3*t1*t1 - 2.f*mA2*t1;
		mA0 = mStart - (mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0);

	}
}
//三次補間
void Interpolation::Interval::calculate3XV(float final,float time,float t1,float X,float V){
	ASSERT(!mReady);

	if (time != 0.f) {
		mReady = true;

		float t0 = mTime - time;
		//一点で大きさと速度が指定されているばあい

		//velocity = 3*mA3*t1*t1 + 2*mA2*t1 + mA1
		//X = = mA3*t1*t1*t1 + mA2*t1*t1 + mA1*t1 + mA0
		//mStart = mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0 + mA0
		//final = mA3*mTime*mTime*mTime + mA2*mTime*mTime + mA1*mTime + mA0

		//final-mStart	= mA3(mTime^3 - t0^3) + mA2(mTime^2 - t0^2) +mA1(mTime-t0);
		//final-X		= mA3(mTime^3 - t1^3) + mA2(mTime^2 - t1^2) +mA1(mTime-t1);

		//final-mStart - velocity*time = mA3(mTime^3 - t0^3 - 3*time*t1^2) + mA2(mTime^2-t0^2 - 2*time*t1)
		//final-X - velocity*(mTime-t1) = mA3(mTime^3 - t0^3 - 3*(mTime-t1)*t1^2) + mA2(mTime^2-t0^2 - 2*(mTime-t1)*t1)

		//F = final-mStart - velocity*time 
		//G = final-X - velocity*(mTime-t1)
		//P = (mTime^3 - t0^3 - 3*time*t1^2)
		//Q= (mTime^2-t0^2 - 2*time*t1)
		//R =(mTime^3 - t0^3 - 3*(mTime-t1)*t1^2)
		//S = (mTime^2-t0^2 - 2*(mTime-t1)*t1)

		//F = P *mA3 + Q*mA2
		//G = R *mA3 + S*mA2

		//mA3 = (F*S - G*Q) / (P*S - R*Q);
		//mA2 =-(F*R - G*P) / (P*S - R*Q);

		float F = final-mStart - V*time;
		float G = final-X - V*(mTime-t1);
		float P = mTime*mTime*mTime - t0*t0*t0 - 3.f*time*t1*t1;
		float Q = mTime*mTime - t0*t0 - 2.f*time*t1;
		float R = mTime*mTime*mTime - t0*t0*t0 - 3.f*(mTime-t1)*t1*t1;
		float S = mTime*mTime - t0*t0 - 2.f*(mTime-t1)*t1;

		mA3 = (F*S - G*Q) / (P*S - R*Q);
		mA2 =-(F*R - G*P) / (P*S - R*Q);
		mA1 = V - 3.f*mA3*t1*t1 - 2.f*mA2*t1;
		mA0 = mStart - (mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0);
	}
}
//三次補間
void Interpolation::Interval::calculate3(float final,float time,float t1,float v1,float t2,float v2){
	ASSERT(!mReady);

	if (time != 0.f) {
		mReady = true;

		float t0 = mTime - time;

		//二点で速度が指定されているばあい
		//V1 = 3*mA3*t1*t1 + 2*mA2*t1 + mA1
		//V2 = 3*mA3*t2*t2 + 2*mA2*t2 + mA1
		//mStart = mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0 + mA0
		//final = mA3*mTime*mTime*mTime + mA2*mTime*mTime + mA1*mTime + mA0

		//final-mStart - V1*time = mA3(mTime^3 - t0^3 - 3*time*t1^2) + mA2(mTime^2-t0^2 - 2*time*t1)
		//final-mStart - V2*time = mA3(mTime^3 - t0^3 - 3*time*t2^2) + mA2(mTime^2-t0^2 - 2*time*t2)

		//F = final-mStart - V1*time
		//G = final-mStart - V2*time
		//T11 = (mTime^3 - t0^3 - 3*time*t1^2)
		//T12 = (mTime^2-t0^2 - 2*time*t1)
		//T21 = (mTime^3 - t0^3 - 3*time*t2^2)
		//T22 = (mTime^2-t0^2 - 2*time*t2)

		//F = T11*mA3 + T12*mA2
		//G = T21*mA3 + T22*mA2

		//F*T22 = T11*T22*mA3 + T12*T22*mA2
		//G*T12 = T21*T12*mA3 + T12*T22*mA2
		//mA3 = (F*T22 - G*T12) / (T11*T22 - T21*T12);
		//mA2 =-(F*T21 - G*T11) / (T11*T22 - T21*T12);

		float F =  final-mStart - v1*time;
		float G =  final-mStart - v2*time;
		float T11 = (mTime*mTime*mTime - t0*t0*t0 - 3.f*time*t1*t1);
		float T21 = (mTime*mTime*mTime - t0*t0*t0 - 3.f*time*t2*t2);
		float T12 = (mTime*mTime - t0*t0 - 2.f*time*t1);
		float T22 = (mTime*mTime - t0*t0 - 2.f*time*t2);
		mA3 = (F*T22 - G*T12) / (T11*T22 - T21*T12);
		mA2 =-(F*T21 - G*T11) / (T11*T22 - T21*T12);
		mA1 = v1 - 3.f*mA3*t1*t1 - 2.f*mA2*t1;
		mA0 = mStart - (mA3*t0*t0*t0 + mA2*t0*t0 + mA1*t0);
	}
}
float Interpolation::Interval::value(float t) const{
//	ASSERT(mReady );
	if (mReady) {
		float r = 0.f;
		r += mA3;
		r *= t;
		r += mA2;
		r *= t;
		r += mA1;
		r *= t;
		r += mA0;
		return r;
		//return mA3*t*t*t+ mA2*t*t + mA1*t + mA0;
	}
	else {
		//定数モードで返す。本当は計算して欲しいのだけど。
		return mStart;
	}
}
float Interpolation::Interval::velocity(float t) const {
	return 3.f*mA3*t*t + 2.f*mA2*t + mA1;
};
float Interpolation::Interval::accel(float t) const {
	return 6.f*mA3*t + 2.f*mA2;
};


void Interpolation3::add(const GameLib::Math::Vector3& vct,float time) {
	mX.add(vct.x,time);
	mY.add(vct.y,time);
	mZ.add(vct.z,time);
}
void Interpolation3::calculate() {
	mX.calculate();
	mY.calculate();
	mZ.calculate();
}
void Interpolation3::value(GameLib::Math::Vector3* out,float time) const{
	out->x = mX.value(time);
	out->y = mY.value(time);
	out->z = mZ.value(time);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ノーマライ図
void normalize(double* p,double* q,double x,double y){
	//誤差をどう処理するか？
	if ((x==0)&&(y==0)) {
		*p=0;
		*q=0;
	}
	else {
		*p=x / GameLib::Math::sqrt((x*x) + (y*y));
		*q=y / GameLib::Math::sqrt((x*x) + (y*y));
	}
}
int normalize(double p){
	//誤差をどう処理するか？
	if (p<0) {
		return -1;
	}
	if (p>0) {
		return 1;
	}
	if (p==0) {
		return 0;
	}
}

//距離をもとめる
float getLength(float x,float y) {
	return GameLib::Math::sqrt(x*x + y*y);
}






///////////////////////////////////////////////////////////////////////////////////////////////////////
//乱数

Random* Random::mInstance=0;

Random* Random::instance() {
	return mInstance;
}
void Random::create() {
	if (mInstance==0) {
		mInstance= new Random;
	}
}
void Random::destroy() {
	ASSERT(mInstance);
	delete mInstance;
	mInstance=0;
}
Random::Random() {
	//ランダムっぽい値。windowsの起動時間とかから持ってくるか
	mSeed = time(0);
	mRandom=GameLib::Math::Random::create(mSeed);

};

Random::~Random() {
	mRandom.release();
};

void Random::setSeed(int i) {
	mSeed=i;
	mRandom.setSeed(i);
};

int Random::getInt() {
	return mRandom.getInt();
};

int Random::getInt(int m) {
	return mRandom.getInt(m);
};

bool Random::getBool() {

	int r;
	r=mRandom.getInt(2);
	if (r ==0 ) {
		return true;
	}
	else {
		return false;
	}
};

float Random::getFloat(float m) {
	return mRandom.getFloat(m);
};


void Random::getVector3(GameLib::Math::Vector3* out,float x,float y,float z) {
	out->set(0.f,0.f,0.f);
	if (x!=0.f) {
		out->x = getFloat(x*2.f) -x;
	}
	if (y!=0.f) {
		out->y = getFloat(y*2.f) -y;
	}
	if (z!=0.f) {
		out->z = getFloat(z*2.f) -z;
	}
};
void Random::getVector3(GameLib::Math::Vector3 *out, float r) {
	float d = getFloat(r);
	getAngle(out,360.f,180.f);
	*out *= r;
};
void Random::getAngle(GameLib::Math::Vector3* out,float h,float v) {
	float ha = 0.f;
	float va = 0.f;
	if (h>0.f) {
		ha = getFloat(h) - h/2.f;
	}
	if (v>0.f) {
		va = getFloat(v) - v/2.f;
	}

	float s = GameLib::Math::sin(va);
	float c = GameLib::Math::cos(va);

	out->x = c * GameLib::Math::cos(ha);
	out->y = c * GameLib::Math::sin(ha);
	out->z = s;
};

void Random::rotateVector3(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& src,float x,float y,float z) {
	out->x = src.x;
	out->y = src.y;
	out->z = src.z;

	Math::rotateVector3(out,*out,
		getFloat( x*2.f ) - x,
		getFloat( y*2.f ) - y,
		getFloat( z*2.f ) - z);
}

unsigned int Random::getColor() {
	float rnd = getFloat(360.f);
	GameLib::Math::Vector3 c;
	Math::colorCircle(&c,rnd);
	return Math::getUnsignedColor(0.f,c.x,c.y,c.z);
}



/*
	//シングルトンになってるかチェック
	//なってないらしい！すげえ！
	r1=mRandom=GameLib::Math::Random::create();
	r1.setSeed(1);
	for (int i=0 ; i<256; ++i) {
		exp1a[i]=r1.getInt();
	};
	r1.setSeed(6553);
	for (int i=0 ; i<256; ++i) {
		exp1b[i]=r1.getInt();
	};

	r2=mRandom=GameLib::Math::Random::create();
	r3=mRandom=GameLib::Math::Random::create();
	r2.setSeed(1);
	r3.setSeed(6553);
	for (int i=0 ; i<256; ++i) {
		exp2a[i]=r2.getInt();
		exp2b[i]=r3.getInt();
	};
	//チェック
	int eq=0;
	int noteq=0;
	for (int i=0 ; i<256 ; ++i) {
		if (exp1a[i] == exp2a[i]) {
			++eq;
		}
		if (exp1b[i] == exp2b[i]) {
			++eq;
		}
		if (exp1a[i] != exp1b[i]) {
			++noteq;
		}
		if (exp2a[i] != exp2b[i]) {
			++noteq;
		}

	}
*/