#include <string>

#include "GameLib/Framework.h"
#include "GameLib/Graphics/Manager.h"
#include "GameLib/Graphics/Texture.h"
#include "GameLib/Graphics/VertexBuffer.h"
#include "GameLib/Graphics/IndexBuffer.h"
#include "GameLib/Graphics/Vertex.h"
#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Scene/Font.h"
#include "GameLib/Scene/StringRenderer.h"
#include "GameLib/Scene/PrimitiveRenderer.h"
#include "GameLib/Threading/Functions.h"

#include "npdraw.h"
#include "npsysc.h"
#include "npobject.h"
#include "npsound.h"

#include "npfile.h"
#include "nml.h"

#include "npstring.h"
#include "modellist.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//テクスチャクラス

int MyTexture::mLoadingNumber=0;
MyTexture* MyTexture::mLoadingTexture[mLoadingTextureMax];

MyTexture::MyTexture() {
	isEnable=false;
	mExistTexture = false;

	for (int i=0 ; i<mBatchMax; ++i) {
		mBatch[i]=0;
	}
	for (int i=0 ; i<mModelMax; ++i) {
		mModel[i]=0;
	}
}

MyTexture::~MyTexture() {
	release();
}
void MyTexture::release() {
	isEnable=false;
	mTexture.release();
	mExistTexture = false;

	for (int i=0 ; i<mBatchMax; ++i) {
		if (mBatch[i]) {
			delete mBatch[i];
			mBatch[i]=0;
		}
	}
	for (int i=0 ; i<mModelMax; ++i) {
		if (mModel[i]) {
			delete mModel[i];
			mModel[i]=0;
		}
	}

	//自分がロードリストに登録されてたら消しておこう
	for (int i=0 ; i<mLoadingTextureMax ; ++i) {
		if (mLoadingTexture[i] == this) {
			mLoadingTexture[i]=0;
			break;
		}
	}
}


bool MyTexture::isReady(){
	if (!mExistTexture) {
		return true;
	}
	return mTexture.isReady();
}

bool MyTexture::isError() const{
	if (!mExistTexture) {
		return false;
	}
	return mTexture.isError();
}

int MyTexture::width() const{
	if (!mExistTexture) {
		return 1;
	}
	return mTexture.width();
}

int MyTexture::height() const{
	if (!mExistTexture) {
		return 1;
	}
	return mTexture.height();
}

int MyTexture::originalWidth() const{
	if (!mExistTexture) {
		return 1;
	}
	return mTexture.originalWidth();
}

int MyTexture::originalHeight() const{
	if (!mExistTexture) {
		return 1;
	}
	return mTexture.originalHeight();
}


//テクスチャをロード
void MyTexture::load(const char* path,bool bat,bool wait,const char* batchpath) {
	//if (isEnable) {
	//	ASSERT(0);		//すでに読み込んである
	//}
	//初期化
	release();

	if (batchpath) {
		mBatchPath=batchpath;
	}
	else {
		mBatchPath=path;
	}

	if (path) {
		MyString pic(256);
		pic<<path<<".tga";
		mTexture= GameLib::Graphics::Texture::create( pic.data() );
		mExistTexture = true;
	}

	isEnable=false;
	isLoading=true;
	isLoadingBatch=bat;

	//ロード中テクスチャに登録
	//(バッチがないならほっとく
	if (bat) {
		//一枚ずつ読み込まれるのを待つ
		if (wait) {
			while(isLoading){
				loadUpdate();
			}
		}
		else {
		//待たずにストック
			for (int i=0 ; i<mLoadingTextureMax ; ++i) {
				if (mLoadingTexture[i] == 0) {
					mLoadingTexture[i]=this;
					break;
				}
				ASSERT(i < mLoadingTextureMax-1);
			}
			++mLoadingNumber;
		}
	}
}

void MyTexture::loadUpdate(){
	ASSERT(!isError());

	//読み込み完了！
	if (isLoading && isReady()) {
		isEnable=true;
		isLoading=false;

		//バッチを読み込む
		if (isLoadingBatch) {

			//nml記述に対応させよう
			MyString bat(256);
			bat<<mBatchPath.data()<<".txt";

			NML nml;
			nml.load(bat.data());
			read(*nml.root());

			isLoadingBatch=false;
		}
	}
	else {
		GameLib::Threading::sleep(1);
	}
}
void MyTexture::read(const Tag& tag,Materials mt) {
	//標準質感

	if (tag.child("materials")) {
		mt.read(*tag.child("materials"));
	}

	int m = tag.getChild();
	//モデルなしの単体バッチ作成
	for (int i=0  ; i < tag.getChild("batch") ; ++i) {
		Tag* t = tag.child("batch",i);
		createBatch(*t,mt);		//タグからバッチをつくる
	}
	//モデル作成
	for (int i=0  ; i < tag.getChild("model") ; ++i) {
		Tag* t = tag.child("model",i);
		createModel(*t,mt);							//タグからモデルをつくる
	}

	//include
	for (int i=0  ; i < tag.getAttribute("include") ; ++i) {
		NML nml;
		nml.load(tag.attribute("include",i)->getValue());
		read(*nml.root(),mt);
	}

};


Batch* MyTexture::createBatch(const Tag& t,Materials mt) {
	Batch* result = 0;

	//名前
	MyString name;
	if (t.attribute("name")) {
		name = t.attribute("name")->getValue();
	}

	//質感情報があるなら、読んでおこう
	if (t.child("materials")) {
		mt.read(*t.child("materials"));
	}

	//パーツ作成する
	//いくついるんだ頂点情報
	BatchData bd;
	bd.create(1024*4,1024*4);
	bd.read(t);
	result=createBatch(name.data(),bd);

	ASSERT(result);

	//質感
	if (t.child("materials")) {
		mt.read(*t.child("materials"));
	}
	result->mMaterials = mt;

	return result;
}

Model* MyTexture::createModel(const Tag& t,Materials mt) {
	//標準質感
	if (t.child("materials")) {
		mt.read(*t.child("materials"));
	}

	Model* result = 0;

	//モデルデータをnew
	for (int i=0 ; i<mModelMax ; ++i) {
		if (mModel[i] == 0) {
			mModel[i] = new Model(*this);
			result = mModel[i];
			break;
		}
	}

	//名前つける
	if (t.attribute("name")) {
		result->mName = t.attribute("name")->getValue();
	}
	//ループしたら別モデルに飛ぶか？
	if (t.attribute("next")) {
		result->mNext = model(t.attribute("next")->getValue());
		ASSERT(result->mNext);
	}
	if (t.attribute("afterimage")) {
		result->mAfterimage = t.attribute("afterimage")->getIntValue(0);
		result->mAfterimageInterval = t.attribute("afterimage")->getIntValue(1);
		result->mAfterimageScale = 1.f * t.attribute("afterimage")->getIntValue(2) / 100.f;
	}

	//アニメ
	for (int i=0 ; i < t.getChild("anime") ; ++i) {
		result->mAnime[i] = new Model::Anime;

		Tag* tt = t.child("anime",i);

		//materialsタグ
		{
			//まず質感情報から
			Materials mat = mt;
			result->mAnime[i]->mMaterials.setStart(mat);
			int count = 0;
			for (int j=0 ; j < tt->getChild("materials") ; ++j) {
				mat.read(*tt->child("materials",j));

				//初期状態セット
				if (j == 0) {
					result->mAnime[i]->mMaterials.setStart(mat);
				}

				count += tt->child("materials",j)->get("delay",false);
				result->mAnime[i]->mMaterials.add(mat,count);
			}
		}
		{
			//batch下のmaterials記述
			Materials mat = mt;
			int count = 0;
			for (int j=0 ; j < tt->getChild("batch") ; ++j) {
				if (tt->child("batch",j)->child("materials")) {
					mat.read(*tt->child("batch",j)->child("materials"));

					//初期状態セット
					if (j == 0) {
						result->mAnime[i]->mMaterials.setStart(mat);
					}

					count +=       tt->child("batch",j)->get("delay",false);
					result->mAnime[i]->mMaterials.add(mat,count);

					ASSERT(tt->child("batch",j)->child("materials")->attribute("delay") == 0);//delay指定しても意味ないよ
				};
			}
		}
		//animateタグ
		if (tt->child("animate")) {
			result->mAnime[i]->mMaterials.read(*tt->child("animate"));
		}

		result->mAnime[i]->mMaterials.refresh();

		//バッチつくる
		for (int j=0; j< tt->getChild("batch") ; ++j) {
			Batch *b = 0;
			//既に定義されてる奴を使う場合
			if (tt->child("batch")->attribute("name")) {
				b = batch(tt->child("batch")->attribute("name")->getValue());
				ASSERT(b);
			}
			else {
				//新規に宣言するばあい
				b = createBatch(*tt->child("batch",j),mt);
			}

			int d = tt->child("batch",j)->get("delay",false);

			//collisionタグ
			Math::Polygon p;
			p.add(GameLib::Math::Vector3(-16.f,16.f,0.f));
			p.add(GameLib::Math::Vector3( 16.f,16.f,0.f));
			p.add(GameLib::Math::Vector3( 16.f,-16.f,0.f));
			p.add(GameLib::Math::Vector3(-16.f,-16.f,0.f));
			if (tt->child("batch",j)->child("collision")) {
				p.read(*tt->child("batch",j)->child("collision"));
			};	
			p *= 2.f / Draw::mWidth;

			result->add(i,b,d,p);
		};

		if (tt->attribute("name")) {
			result->mAnime[i]->mName = tt->attribute("name")->getValue();
		}
	}

	return result;
};
void MyTexture::loadUpdateAll() {
	//テクスチャロード
	mLoadingNumber=0;
	for (int i=0 ; i<mLoadingTextureMax ; ++i) {
		if (mLoadingTexture[i]) {
			mLoadingTexture[i]->loadUpdate();
			if (mLoadingTexture[i]->isEnable) {
				mLoadingTexture[i]=0;
			}
			else {
				++mLoadingNumber;
			}
		}
	}
}

Batch* MyTexture::batch(const char * name) const{
	Batch* result=0;
	for (int i=0 ; i<mBatchMax;++i) {
		if (mBatch[i]) {
			if (mBatch[i]->mName == name) {
				result=mBatch[i];
				break;
			};
		}
	};
	if (result == 0) {
		ASSERT(result);
	}
	return result;
};
Batch* MyTexture::batch(int i) const{
	ASSERT(i<mBatchMax);
	return mBatch[i];
}

Model* MyTexture::model(const char * name,bool assert) const{
	Model* result=0;
	for (int i=0 ; i<mModelMax;++i) {
		if (mModel[i]) {
			if (mModel[i]->mName == name) {
				result=mModel[i];
				break;
			};
		}
	};
	if (assert) {
		if (result == 0) {
			ASSERT(result);
		}
	}
	return result;
};
Model* MyTexture::model(int i) const{
	ASSERT(i<mModelMax);
	return mModel[i];
}

void MyTexture::drawModel(const GameLib::Math::Vector3& pos,const BlendInfo& bi,float animecount) {
	for (int i=0 ; i<mModelMax ; ++i) {
		if (model(i)) {
			model(i)->draw(pos,bi,animecount);
		}
		else {
			break;
		}
	}
}

int MyTexture::getNewBatch() const{
	int r=-1;
	for(int i=0 ; i<mBatchMax ; ++i) {
		if (mBatch[i]==0) {
			r=i;
			break;
		}
	}
	ASSERT(r>=0);

	return r;
};

Batch* MyTexture::createBatch(const char* name,float u,float v,float w,float h,unsigned int color,const VertexTransform& Vt) {
	BatchData bd;
	bd.create(4,6);

	TextureRect tr;
	tr.mU = u;
	tr.mV = v;
	tr.mW = w;
	tr.mH = h;
	tr.mColor = color;
	tr.mSize.x = w;
	tr.mSize.y = h;
	tr.mSize.z = 1.f;	//なんでもいい。

	bd.addRect(tr,Vt);

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,bd);

	return mBatch[i];
};
Batch* MyTexture::createBatch(const char* name,float* u,float* v,float* x,float* y,float* z,unsigned int* color,int* index,int vtnumber,int indexnumber,GameLib::Math::Vector3* normal){
	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,u,v,x,y,z,color,index,vtnumber,indexnumber,normal);
	return mBatch[i];
}
Batch* MyTexture::createBatch(const char* name,const BatchData& bd){
	ASSERT(bd.mVertexMax > bd.mVertexNumber);

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,bd);
	return mBatch[i];
}


//チップタイプのバッチ
Batch* MyTexture::createChip(const char* name,int chipwidth,int chipheight,int number,int* u,int* v,int* x,int*y,int*z) {
	//チップ頂点を作る
	//頂点数の最適化はやっていない。
	//256x256マスx3レイヤーにチップを埋めると、インデックスは1179628必要になる。
	int idn=number * 6; 
	int vtn=number * 4;

	//必要な頂点・インデックス数を数える

	float* U= new float[vtn];
	float* V= new float[vtn];
	float* X= new float[vtn];
	float* Y= new float[vtn];
	float* Z= new float[vtn];

	GameLib::Math::Vector3* NORMAL= new GameLib::Math::Vector3[vtn];
	unsigned int* COLOR= new unsigned int[vtn];
	int* index= new int[idn];

	//テクスチャ座標をつくる
	for (int i=0;i<number ; ++i){
		U[i*4]		=u[i]		*chipwidth;
		V[i*4]		=v[i]		*chipheight;
		U[i*4+1]	=(u[i]+1)	*chipwidth;
		V[i*4+1]	=v[i]		*chipheight;
		U[i*4+2]	=(u[i]+1)	*chipwidth;
		V[i*4+2]	=(v[i]+1)	*chipheight;
		U[i*4+3]	=u[i]		*chipwidth;
		V[i*4+3]	=(v[i]+1)	*chipheight;

		X[i*4]		=x[i]		*chipwidth;
		Y[i*4]		=y[i]		*chipheight;
		Z[i*4]		=z[i];
		X[i*4+1]	=(x[i]+1)	*chipwidth;
		Y[i*4+1]	=y[i]		*chipheight;
		Z[i*4+1]	=z[i];
		X[i*4+2]	=(x[i]+1)	*chipwidth;
		Y[i*4+2]	=(y[i]+1)	*chipheight;
		Z[i*4+2]	=z[i];
		X[i*4+3]	=x[i]		*chipwidth;
		Y[i*4+3]	=(y[i]+1)	*chipheight;
		Z[i*4+3]	=z[i];

		NORMAL[i*4  ].set(0,0,1.f);
		NORMAL[i*4+1].set(0,0,1.f);
		NORMAL[i*4+2].set(0,0,1.f);
		NORMAL[i*4+2].set(0,0,1.f);

		COLOR[i*4]=0xffffffff;
		COLOR[i*4+1]=0xffffffff;
		COLOR[i*4+2]=0xffffffff;
		COLOR[i*4+3]=0xffffffff;

		index[i*6]		=i*4;
		index[i*6+1]	=i*4+2;
		index[i*6+2]	=i*4+1;
		index[i*6+3]	=i*4+0;
		index[i*6+4]	=i*4+3;
		index[i*6+5]	=i*4+2;
	}

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index,vtn,idn,NORMAL);

	delete []U;
	delete []V;
	delete []X;
	delete []Y;
	delete []Z;
	delete []COLOR;
	delete []NORMAL;
	delete []index;

	return mBatch[i];
}

//正三角形作成
Batch* MyTexture::createTriangle(const char* name,float u,float v,float w,float h,unsigned int color1,unsigned int color2,unsigned int color3){
	//厚みをもたせてやるバージョン。ようするに三角柱。
	float U[6];
	float V[6];
	float X[6];
	float Y[6];
	float Z[6];
	GameLib::Math::Vector3 NORMAL[6];
	unsigned int COLOR[6];

	int index[3*8];

	U[0]=u;
	V[0]=v;
	U[1]=u+w;
	V[1]=v;
	U[2]=u+w/2;
	V[2]=v+h/2;

	U[3]=u;
	V[3]=v+h/2;
	U[4]=u+w;
	V[4]=v+h/2;
	U[5]=u+w/2;
	V[5]=v+h;

	for (int i=0 ; i<2 ; ++i) {

		for (int j=0 ; j<3 ; ++j) {
			X[i*3+j]=GameLib::Math::cos(120.f*(j+1))*w/2 * (10.f-i)/10.f;
			Y[i*3+j]=GameLib::Math::sin(120.f*(j+1))*w/2 * (10.f-i)/10.f;
			Z[i*3+j]=w/2 * (1-i*2);
		}

		NORMAL[i*3+0] = GameLib::Math::Vector3(0,0,1.f-i*2);
		NORMAL[i*3+1] = GameLib::Math::Vector3(0,0,1.f-i*2);
		NORMAL[i*3+2] = GameLib::Math::Vector3(0,0,1.f-i*2);

		COLOR[i*3+0]=color1;	//0水色
		COLOR[i*3+1]=color2;	//1紫
		COLOR[i*3+2]=color3;	//2黄色
	}
	COLOR[3] &= 0x00ffffff;
	COLOR[4] &= 0x00ffffff;	//1紫
	COLOR[5] &= 0x00ffffff;	//2黄色


	//0120
	//3453

	//0-1 ok
	index[6]=0;
	index[7]=4;
	index[8]=3;
	index[9]=0;
	index[10]=1;
	index[11]=4;

	//1-2 ?
	index[12]=1;
	index[13]=5;
	index[14]=4;
	index[15]=1;
	index[16]=2;
	index[17]=5;

	//2-0 ok
	index[18]=2;
	index[19]=3;
	index[20]=5;
	index[21]=2;
	index[22]=0;
	index[23]=3;

	index[0]=0;
	index[1]=2;
	index[2]=1;

	index[3]=3;
	index[4]=4;
	index[5]=5;

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index+6,6,18,NORMAL);	//蓋なし
	return mBatch[i];

};



//pyramid
Batch* MyTexture::createPyramid(const char* name,float u,float v,float w,float h,unsigned int color) {
	const int vtn = 3*4;
	const int idn = 3*4;

	float X[vtn];
	float Y[vtn];
	float Z[vtn];
	float U[vtn];
	float V[vtn];
	GameLib::Math::Vector3 NORMAL[vtn];
	unsigned int COLOR[vtn];
	int index[idn];

	float s=w/2.f;

	for (int i=0 ; i<4 ;++i) {
		float sn1=GameLib::Math::cos(i*90+45);
		float cs1=GameLib::Math::sin(i*90+45);

		float sn2=GameLib::Math::cos(i*90-45);
		float cs2=GameLib::Math::sin(i*90-45);

		X[i*3+0] = 0;
		Y[i*3+0] = 0;
		Z[i*3+0] = s;
		X[i*3+1] = cs1*s;
		Y[i*3+1] = sn1*s;
		Z[i*3+1] = 0;
		X[i*3+2] = cs2*s;
		Y[i*3+2] = sn2*s;
		Z[i*3+2] = 0;

		U[i*3+0] = u+w/2;
		V[i*3+0] = v;
		U[i*3+1] = u;
		V[i*3+1] = v+h;
		U[i*3+2] = u+w;
		V[i*3+2] = v+h;

		NORMAL[i*3+0].set(
			GameLib::Math::cos(i*90),
			GameLib::Math::sin(i*90),
			1.f);
		NORMAL[i*3+0].normalize();
		NORMAL[i*3+1].set(
			GameLib::Math::cos(i*90),
			GameLib::Math::sin(i*90),
			1.f);
		NORMAL[i*3+1].normalize();
		NORMAL[i*3+2].set(
			GameLib::Math::cos(i*90),
			GameLib::Math::sin(i*90),
			1.f);
		NORMAL[i*3+2].normalize();

		COLOR[i*3+0]= 0xffffffff;
		COLOR[i*3+1]= 0xffffffff;
		COLOR[i*3+2]= 0xffffffff;

		index[i*3+0] = i*3;
		index[i*3+1] = i*3+2;
		index[i*3+2] = i*3+1;
	}

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index,vtn,idn,NORMAL);
	return mBatch[i];

}

Batch* MyTexture::createHexWall(const char* name,float u,float v,float w,float h,unsigned int color) {
	const int m=14;
	const int n=13;
	const int vtn = m*n*12;
	const int idn = m*n*18;
	float X[vtn];
	float Y[vtn];
	float Z[vtn];
	float U[vtn];
	float V[vtn];
	GameLib::Math::Vector3 NORMAL[vtn];
	unsigned int COLOR[vtn];
	int index[idn];

	int in=0;
	int vt=0;
	for (int i=0 ; i<m; ++i) {
		for (int j=0 ; j<n ; ++j) {

			int p=(i-(m/2))*64;
			int q=(j-(n/2))*64;
			int r=(i+j-((m+n)/2))*64;

			VertexTransform Vt;
			Vt.position(p,q,r+32);
	//		MyTexture::addRect(&vt,&in,U,V,X,Y,Z,NORMAL,index,
	//			u,v,w,h,Vt);

			Vt.position(p-32,q,r);
			Vt.rotate(0,270,0);
	//		MyTexture::addRect(&vt,&in,U,V,X,Y,Z,NORMAL,index,
	//			u,v,w,h,Vt);

			Vt.position(p,q-32,r);
			Vt.rotate(270,0,0);
	//		MyTexture::addRect(&vt,&in,U,V,X,Y,Z,NORMAL,index,
	//			u,v,w,h,Vt);

		}
	}

	for (int k=0 ; k<idn; ++k) {
		COLOR[k] = color;
	}

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index,vtn,idn,NORMAL);
	return mBatch[i];

}

//割れるガラスのバッチ
Batch* MyTexture::createGlass(const char* name,float u,float v,float w,float h,unsigned int color) {
	const int n=16;
	const int vtn = 1+n;
	const int idn = n*3;
	float X[vtn];
	float Y[vtn];
	float Z[vtn];
	float U[vtn];
	float V[vtn];
	GameLib::Math::Vector3 NORMAL[vtn];
	unsigned int COLOR[vtn];
	int index[idn];

	for (int i=0 ; i<vtn ; ++i) {
		Z[i] = 0.f;
		NORMAL[i].set(0,0,1.f);
		COLOR[i] = color;
	}
	X[0] = 0.f;
	Y[0] = 0.f;
	U[0] = u;
	V[0] = v;
	for (int i=0 ; i<n ; ++i) {
		float l=(Random::instance()->getFloat(1.0f)+1.f);
		X[i+1] = w*GameLib::Math::cos(i*360/n)*l;
		Y[i+1] = w*GameLib::Math::sin(i*360/n)*l;

		U[i+1] = u+w*i/(n-1);
		V[i+1] = v+h;
	}
	for (int i=0 ; i<n ; ++i) {
		index[i*3+0] = 0;
		index[i*3+2] = i+1;
		index[i*3+1] = (i+1)%n+1;
	}

	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index,vtn,idn,NORMAL);
	return mBatch[i];
}

//ラピュタパターン
Batch* MyTexture::createLaputa(const char* name,float u,float v,float w,float h,unsigned int color,int x,int y,int z) {
	//背景作成
	//背景+必要な立方体の数ぶん必要

	int vtn = x*y*24*4+4;
	int idn = x*y*36*4+6;

	float* X = new float[vtn];
	float* Y = new float[vtn];
	float* Z = new float[vtn];
	float* U = new float[vtn];
	float* V = new float[vtn];
	GameLib::Math::Vector3* NORMAL = new GameLib::Math::Vector3[vtn];
	unsigned int* COLOR = new unsigned int[vtn];
	int* index= new int[idn];

	//まずは背景つくる
	for (int i=0 ; i<4 ; ++i) {
		COLOR[i] = 0xff7f7f7f;
		Z[i] = -z*w;
		NORMAL[i].set(0,0,1.f);
	}
	X[0] = 0;
	Y[0] = 0;
	U[0] = u;
	V[0] = v;
	X[1] = w*x*2;
	Y[1] = 0;
	U[1] = u+w*x*2;
	V[1] = v;
	X[2] = w*x*2;
	Y[2] = h*y*2;
	U[2] = u+w*x*2;
	V[2] = v+h*y*2;
	X[3] = 0;
	Y[3] = h*y*2;
	U[3] = u;
	V[3] = v+h*y*2;
	index[0] = 0;
	index[1] = 2;
	index[2] = 1;
	index[3] = 0;
	index[4] = 3;
	index[5] = 2;

	int vt = 4;
	int id = 6;

	//さあ立方体つくるぞ！
	for (int i=0 ; i<x; ++i) {
		for (int j=0 ; j<y; ++j) {
			int p=i*y + j;

			int dd = Random::instance()->getInt(z);
			int d = -dd * w - w/2;

			//色設定
			for (int k=0 ; k<24*4 ; ++k) {
				COLOR[vt+k] = (z-dd) * 0x00202020 + 0xff7f7f7f;
			}

			VertexTransform Vt;
			Vt.position(i*w+w/2,j*h+h/2,0);
//			addCube(&vt,&id,
//				U,V,X,Y,Z,NORMAL,index,
//				u,v,w,h,Vt);

			Vt.position(i*w+w/2 + w*x,j*h+h/2,d);
//			addCube(&vt,&id,
///				U,V,X,Y,Z,NORMAL,index,
//				u,v,w,h,Vt);

			Vt.position(i*w+w/2,j*h+h/2 + h*y,d);
//			addCube(&vt,&id,
//				U,V,X,Y,Z,NORMAL,index,
//				u,v,w,h,Vt);

			Vt.position(i*w+w/2 + w*x,j*h+h/2 + h*y,d);
//			addCube(&vt,&id,
//				U,V,X,Y,Z,NORMAL,index,
//				u,v,w,h,Vt);

		}
	}


	int i=getNewBatch();
	mBatch[i]=new Batch;
	mBatch[i]->create(this,name,U,V,X,Y,Z,COLOR,index,vtn,idn,NORMAL);


	delete[] X;
	delete[] Y;
	delete[] Z;
	delete[] U;
	delete[] V;
	delete[] NORMAL;
	delete[] COLOR;
	delete[] index;

	return mBatch[i];
}







void MyTexture::draw(float x, float y, float z,const BlendInfo& bi){
	Draw* Draw=Draw::instance();

	float W=bi.mScale.x*width();
	float H=bi.mScale.y*height();
	BlendInfo bi2=bi;
	bi2.setZoom(W,H);

	Draw->mZeroBatch.draw(this,x,y,z,bi2);

//	float p=mTexture.width() -mTexture.originalWidth();
//	float q=mTexture.height()-mTexture.originalHeight();
//	float r=0;
//	float i,j,k;
	//回転させよう
//	i=p;
//	j=q*GameLib::Math::cos(bi.mRotate.x) + r*GameLib::Math::sin(bi.mRotate.x);
//	k=r*GameLib::Math::cos(bi.mRotate.x) - q*GameLib::Math::sin(bi.mRotate.x) ;

//	p=i*GameLib::Math::cos(bi.mRotate.y) + j*GameLib::Math::sin(bi.mRotate.y);
//	q=j;
//	r=k*GameLib::Math::cos(bi.mRotate.y) - i*GameLib::Math::sin(bi.mRotate.y) ;

//	i=p*GameLib::Math::cos(bi.mRotate.z) + q*GameLib::Math::sin(bi.mRotate.z);
//	j=q*GameLib::Math::cos(bi.mRotate.z) - p*GameLib::Math::sin(bi.mRotate.z);
//	k=r;

//	float X=x+i/2;
//	float Y=y+j/2;


	//Draw* Draw=Draw::instance();
//	Draw->mZeroBatch.draw(this,X,Y,z,bi2);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//バッチクラス
Batch::Batch() {
	isEnable=false;
};

Batch::~Batch() {
	release();
};
void Batch::release() {
	iBuffer.release();
	vBuffer.release();
};
//バッチ作成　かんたん矩形のみモード(直接呼び出すときはゼロバッチのみ。)
void Batch::create(MyTexture* tex,const char* name,float u,float v,float w,float h,unsigned int color,float sx,float sy,float sz,float zoomx,float zoomy,bool reversenormal,bool cut1pix) {
	float U[4];
	float V[4];
	float X[4];
	float Y[4];
	float Z[4];
	GameLib::Math::Vector3 NORMAL[4];

	unsigned int COLOR[4];
	int index[6];

	if (cut1pix) {
		U[0]=u+1;
		V[0]=v+1;
		U[1]=u-1+w;
		V[1]=v+1;
		U[2]=u-1+w;
		V[2]=v-1+h;
		U[3]=u+1;
		V[3]=v-1+h;
	}
	else {
		U[0]=u;
		V[0]=v;
		U[1]=u+w;
		V[1]=v;
		U[2]=u+w;
		V[2]=v+h;
		U[3]=u;
		V[3]=v+h;
	}

	X[0]=(-w /2)*zoomx + sx;
	Y[0]=( h /2)*zoomy - sy;
	X[1]=( w /2)*zoomx + sx;
	Y[1]=( h /2)*zoomy - sy;
	X[2]=( w /2)*zoomx + sx;
	Y[2]=(-h /2)*zoomy - sy;
	X[3]=(-w /2)*zoomx + sx;
	Y[3]=(-h /2)*zoomy - sy;

	if (reversenormal) {
		NORMAL[0] = GameLib::Math::Vector3(0,0,-1.f);
		NORMAL[1] = GameLib::Math::Vector3(0,0,-1.f);
		NORMAL[2] = GameLib::Math::Vector3(0,0,-1.f);
		NORMAL[3] = GameLib::Math::Vector3(0,0,-1.f);
	}
	else {
		NORMAL[0] = GameLib::Math::Vector3(0,0,1.f);
		NORMAL[1] = GameLib::Math::Vector3(0,0,1.f);
		NORMAL[2] = GameLib::Math::Vector3(0,0,1.f);
		NORMAL[3] = GameLib::Math::Vector3(0,0,1.f);
	}
	for (int i=0 ; i<4 ; ++i) {
		Z[i]=sz;
		COLOR[i]=color;
	}
	index[0]=0;
	index[1]=2;
	index[2]=1;
	index[3]=0;
	index[4]=3;
	index[5]=2;
	create(tex,name,U,V,X,Y,Z,COLOR,index,4,6,NORMAL);
};

void Batch::create(MyTexture* tex,const char* name,const BatchData& bd) {
	create(
		tex,name,
		bd.mU,bd.mV,
		bd.mX,bd.mY,bd.mZ,
		bd.mColor,bd.mIndex,
		bd.mVertexNumber,
		bd.mIndexNumber,
		bd.mNormal);
};

//バッチ作成　フルバージョン
//最終的にここに全部お願いするのだ
void Batch::create(MyTexture* tex,const char* name,
				   const float* u,const float* v,
				   const float* x,const float* y,const float* z,
				   const unsigned int* color,const int* index,
				   int vtnumber,int indexnumber,
				   const GameLib::Math::Vector3* normal) {
	isEnable=true;
	mPolygon=indexnumber/3;

	//頂点数0でも一応許可する
	if (vtnumber) {
		mTexture=tex;
		ASSERT(name);
		mName = name;

		iBuffer.release();
		vBuffer.release();

		//頂点バッファをつくる
		vBuffer = GameLib::Graphics::VertexBuffer::create( vtnumber);

		GameLib::Graphics::Vertex* vt = vBuffer.lock(); //書き込みポインタを得る

		if (tex) {
			while (! tex->isReady()) {
				ASSERT(!tex->isError());
	
		//		if (BGM::instance()) {
		//			BGM::instance()->update();
		//		}
			};
		}

		for (int i=0 ; i<vtnumber ; ++i) {
			float X=  2.0f * x[i] / Draw::mWidth;
			float Y=  2.0f * y[i] / Draw::mWidth;
			float Z=  2.0f * z[i] / Draw::mWidth;

			float U,V;
			if (tex) {
				U= u[i] / tex->width();
				V= v[i] / tex->height();
			}
			else {
				U=u[i];
				V=v[i];
			}

			vt[ i ].mPosition.set( X, Y, Z );				//頂点
			vt[ i ].mColor = color[i];						//色
			vt[ i ].mUv.set( U, V );						//テクスチャ座標
			if (normal) {
				vt[ i ].mNormal.set(normal[i].x,normal[i].y,normal[i].z);
			}
		};
		vBuffer.unlock( &vt );

		//-------------インデクスバッファを作る--------------------
		//つかう三角形ひとつにつ三頂点
		//頂点数は三の倍数
		ASSERT(indexnumber%3==0);

		iBuffer = GameLib::Graphics::IndexBuffer::create( indexnumber );
		unsigned short* iB = iBuffer.lock();
		for ( int i = 0; i < indexnumber ; ++i ){
			iB[ i ] = index[ i ];
		}
		iBuffer.unlock( &iB );
	}
};

void Batch::draw( float x, float y, float z, const BlendInfo& bi,const Materials* materials )const{
	draw(mTexture,x,y,z,bi,materials);
}

void BlendElement::set(const Materials& mtr) {
	//フロントエンド
	mFrontEnd |= mtr.mFrontEnd;

	//ブレンド設定
	if (mBlendMode == GameLib::Graphics::BLEND_MAX) {
		mBlendMode = mtr.mBlendMode;
		mDepthTest = mtr.mDepthTest;								//Zソートを行うかどうか
		mDepthWrite =  mtr.mDepthWrite;								//p370参照。
	}
	//ネガ
	mNegative ^= mtr.mNegative;

	//カリング
	mCullMode = mtr.mCullMode;
	//照明
	for (int i=0 ; i<4 ; ++i) {
		mLighting[i] &= mtr.mLighting[i];
	}

	///質感設定
	mDeffuse =  mtr.mDeffuse;						//反射色
	mSpecular =  mtr.mSpecular;						//すぺきゅら　ハイライトが小さく強くなる？
	mSharpness = mtr.mSharpness;					//ハイライトの鋭さ

	//環境色加算
	mAmbient += mtr.mAmbient;

	//発光色加算
	mEmission += mtr.mEmission;

	//透明色は乗三
	mTransparency *= mtr.mTransparency;
};
void BlendElement::reverseCulling() {
	mReverseCulling = !mReverseCulling;
}

void BlendElement::setGraphicsManager() const {
	//カリング
	if (mReverseCulling) {
		if (mCullMode == GameLib::Graphics::CULL_BACK) {
			GameLib::Graphics::Manager::instance().setCullMode( GameLib::Graphics::CULL_FRONT );
		}
		else {
			if (mCullMode == GameLib::Graphics::CULL_FRONT) {
				GameLib::Graphics::Manager::instance().setCullMode( GameLib::Graphics::CULL_BACK );
			}
			else {
				GameLib::Graphics::Manager::instance().setCullMode( mCullMode );
			}
		}
	}
	else {
		GameLib::Graphics::Manager::instance().setCullMode( mCullMode );
	}
	///質感設定
	GameLib::Graphics::Manager::instance().setDiffuseColor( mDeffuse );						//反射色
	GameLib::Graphics::Manager::instance().setSpecularColor( mSpecular );						//すぺきゅら　ハイライトが小さく強くなる？
	GameLib::Graphics::Manager::instance().setSpecularSharpness( mSharpness );					//ハイライトの鋭さ
}

//よく使うdraw
void Batch::draw(MyTexture* tex, float x,float y, float z, const BlendInfo& bi,const Materials* materials) const{
	Materials mtr;
	if (materials == 0) {
		mtr = this->mMaterials;
	}
	else {
		mtr = *materials;
	}

	BlendElement be;
	GameLib::Math::Matrix34 wm;
	be.get(&wm,x,y,z,bi,mtr);
	draw(tex,wm,be);
}
void BlendInfo::getWorldMatrix(GameLib::Math::Matrix34* wm,float x,float y,float z,int sprite,bool frontend)const {
	//blendinfoから、ワールド変換行列をつくる

	//移動
	wm->setTranslation(
		x*2.0/Draw::mWidth,
		y*2.0/Draw::mWidth,
		z*2.0/Draw::mWidth
	);


	//回転

	GameLib::Math::Matrix34 rwm;
	getAngleMatrix(&rwm,sprite,frontend);
	*wm *= rwm;

	//反転
	if (mReverseX) {
		wm->scale(-1.f,1.f,1.f);
	}
	if (mReverseY) {
		wm->scale(1.f,-1.f,1.f);
	}
	if (mReverseZ) {
		wm->scale(1.f,1.f,-1.f);
	}

	wm->scale(mScale);			//ズーム
};

void BlendInfo::getAngleMatrix(GameLib::Math::Matrix34* out,int sprite,bool frontend) const{
	out->setIdentity();

	//スプライト用の回転打消し
	if (!(mBlendElement.mFrontEnd || frontend)) {
		if (sprite == 1) {
			*out *= Draw::instance()->mInvertWMforSprite1;
		}
		if (sprite == 2) {
			*out *= Draw::instance()->mInvertWMforSprite2;
		}
	}

	//回転
	if (sprite) {
		out->rotateX( -90.f);
		out->rotateY(mRotate.y);
	}
	else {
		out->rotateY(mRotate.y);
		out->rotateX(mRotate.x);
		out->rotateZ(mRotate.z);
	}
}

bool BlendInfo::cullReverse() const {
	//反転ある？
	bool cullrev=false;
	if (mReverseX) {
		cullrev = !cullrev;
	}
	if (mReverseY) {
		cullrev = !cullrev;
	}
	if (mReverseZ) {
		cullrev = !cullrev;
	}
	return cullrev;

};

void Materials::getWorldMatrix(GameLib::Math::Matrix34* wm)const {
	wm->translate(
		 mPosition.x *2.0/Draw::mWidth,
		 mPosition.y *2.0/Draw::mWidth,
		 mPosition.z *2.0/Draw::mWidth);
	//wm->rotateY(mAngle.y);
	//wm->rotateX(mAngle.x);
	//wm->rotateZ(mAngle.z);

	wm->rotateY(mAngle.y);
	wm->rotateX(mAngle.x);
	wm->rotateZ(mAngle.z);

	wm->scale(mScale);
};


void BlendElement::set(const BlendInfo& bi) {
	//まずはまるまるコピー
	*this = bi.mBlendElement;
	//反転
	if (bi.cullReverse()) {
		reverseCulling();
	}

	//アングルを覚えておこう
	GameLib::Math::Matrix34 a;
	bi.getAngleMatrix(&a,0,false);
	a.mul(&mAngle,GameLib::Math::Vector3(1.f,0.f,0.f));
};

void BlendElement::get(GameLib::Math::Matrix34* wm,float x,float y,float z,const BlendInfo& bi,const Materials& mtr) {
	//biとmtrから、blendElementをつくる
	set(bi);
	set(mtr);

	//blendinfo由来のワールド変換
	//スプライトか？
	int sprite = 0;
	//if (!mFrontEnd) {
		sprite = mtr.mSprite;
	//}
	bi.getWorldMatrix(wm,x,y,z,sprite,mFrontEnd);

	//質感情報由来のワールド変換
	mtr.getWorldMatrix(wm);

}
void Batch::draw(const GameLib::Math::Matrix34& wm,const BlendElement& be)const{
	draw(mTexture,wm,be);
}

void Batch::draw(MyTexture* tex,const GameLib::Math::Matrix34& wm,const BlendElement& be)const{
	Draw* Draw=Draw::instance();

	if (Draw->mSkip) {
		return;
	}
	if (mPolygon==0) {
		return;
	}

	if (tex) {
		ASSERT(!tex->isError());

		if (!tex->isReady()) {
			return;
		}
	}

	//下地をネガティブにしておく
	if (be.mNegative) {
		Draw::instance()->nega();
	}

	GameLib::Graphics::Manager m = GameLib::Graphics::Manager::instance();

	//ビュー変換行列を選ぶ
	if (!be.mFrontEnd) {
		m.setProjectionViewMatrix( Draw->mPVM );
	}
	else {
		//カメラ使わない（フロントエンドモード）
		m.setProjectionViewMatrix( Draw->mFrontEndPVM );
	}

	//行列をハードウェアに渡す
	m.setWorldMatrix( wm );
	//m.setViewport(200,200,400,200);			//使い方わかんね

	//使うデータを指定する
	m.setVertexBuffer( vBuffer );
	m.setIndexBuffer( iBuffer );

	if (tex==0) {
		m.setTexture( 0 );
	}
	else {
		if (tex->mExistTexture) {
			m.setTexture( tex->mTexture );
		}
		else {
			m.setTexture( 0 );
		}
	}


	//視点位置　スペキュラに影響する
	m.setEyePosition(*Draw->mCamera.eyePosition());

	//ブレンド設定
	be.setGraphicsManager();
	m.setBlendMode( be.mBlendMode);
	m.enableDepthTest( be.mDepthTest );								//Zソートを行うかどうか
	m.enableDepthWrite( be.mDepthWrite );								//p370参照。

	//シェーダ使うか？
	bool shader = false;

	//環境色
	m.setAmbientColor( be.mAmbient);
	if (!Math::nearOne(be.mAmbient)) {
		shader = true;
	}

	//発光色
	m.setEmissionColor( be.mEmission);
	if (be.mEmission.squareLength() > 0.f) {
		shader = true;
	}

	//不透明度乗算
	m.setTransparency( be.mTransparency );
	if (be.mTransparency != 1.f) {
//		shader = true;
	}
	//ライティングスイッチ
	for (int i=0 ; i<4 ; ++i) {
		if (be.mLighting[i]) {
			m.setLightIntensity(i,Draw->light(i)->intensity());
			shader = true;
		}
		else {
			m.setLightIntensity(i,0.f);
		}
	}

	//ライティングモード　ambientやemissionを使うときにも必要。困ったものである。
	if (shader) {
		if (PrimaryConfig::mConfig.mPixelShader) {
			m.setLightingMode( GameLib::Graphics::LIGHTING_PER_PIXEL );
		}
		else {
			m.setLightingMode( GameLib::Graphics::LIGHTING_PER_VERTEX );
		}
	}
	else {
		m.setLightingMode( GameLib::Graphics::LIGHTING_NONE );
	}

	//描画
	int polygon=be.mPolygonNumber;
	if (polygon<0) {
		polygon=mPolygon;
	}
	if (polygon + be.mPolygonIndex/3 > mPolygon) {
		polygon = mPolygon - be.mPolygonIndex/3;
	}

	//m.drawIndexed( be.mPolygonIndex, polygon , GameLib::Graphics::PRIMITIVE_LINE );
	m.drawIndexed( be.mPolygonIndex, polygon , GameLib::Graphics::PRIMITIVE_TRIANGLE );
	Draw->mPolygonPerFrame += polygon;

	//ネガティブ戻す
	if (be.mNegative) {
		Draw::instance()->nega();
	}
};

void Draw::nega() const {
	BlendInfo nb;
	Materials mtr;

	nb.setBlendMode(4);
	nb.mBlendElement.mDepthTest = false;
	nb.mBlendElement.mDepthWrite = false;
	nb.setZoom(1024.f,1024.f,1024.f);

	mtr.mFrontEnd = true;

	mZeroBatch.draw(0,0,0,nb,&mtr);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//頂点変換

VertexTransform::VertexTransform() {
	mPosition = 0.f;
	mScale = 1.f;
	mAngle = 0.f;

	mReverseNormal = false;
}

void VertexTransform::position(float x,float y, float z) {
	mPosition.set(x,y,z);
}
void VertexTransform::scale(float x,float y, float z) {
	mScale.set(x,y,z);
}
void VertexTransform::rotate(float x,float y, float z) {
	mAngle.set(x,y,z);
}
void VertexTransform::transform(int start,int number,float* x,float* y,float* z,GameLib::Math::Vector3* normal) const{
	GameLib::Math::Matrix34 wm;
	wm.setScaling(1.f,1.f,1.f);
	wm.setIdentity();

	//移動
	wm.translate(mPosition);

	//回転
	wm.rotateX( mAngle.x );
	wm.rotateY( mAngle.y );
	wm.rotateZ( mAngle.z );

	//ズーム
	wm.scale(mScale);

	for (int i=0 ; i<number ; ++i) {
		GameLib::Math::Vector3 vct(
			x[start+i],
			y[start+i],
			z[start+i]);

		GameLib::Math::Vector3 res;
		wm.mul(&res,vct);
		x[start+i] = res.x;
		y[start+i] = res.y;
		z[start+i] = res.z;

		wm.mul33(&res,normal[start+i]);
		normal[start+i] = res;

		if (mReverseNormal) {
			normal[start+i] *= -1.f;
		}
		normal[start+i].normalize();
	};

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BatchData::BatchData(){
	mU = 0;
	mV = 0;
	mX = 0;
	mY = 0;
	mZ = 0;
	mNormal = 0;
	mColor = 0;
	mIndex = 0;

	mVertexMax = 0;
	mVertexNumber = 0;
	mIndexMax = 0;
	mIndexNumber = 0;
};
BatchData::~BatchData() {
	if (mU) {
		delete []mU;
		mU = 0;
	}
	if (mV) {
		delete []mV;
		mV = 0;
	}
	if (mX) {
		delete []mX;
		mX = 0;
	}
	if (mY) {
		delete []mY;
		mY = 0;
	}
	if (mZ) {
		delete []mZ;
		mZ = 0;
	}
	if (mNormal) {
		delete []mNormal;
		mNormal = 0;
	}
	if (mColor) {
		delete []mColor;
		mColor = 0;
	}
	if (mIndex) {
		delete []mIndex;
		mIndex = 0;
	}
	mVertexMax = 0;
	mVertexNumber = 0;
	mIndexMax = 0;
	mIndexNumber = 0;
};

void BatchData::create(int vm,int im) {
	ASSERT(mVertexMax == 0);

	mU = new float[vm];
	mV = new float[vm];
	mX = new float[vm];
	mY = new float[vm];
	mZ = new float[vm];
	mNormal = new GameLib::Math::Vector3[vm];
	mColor = new unsigned int[vm];
	mIndex = new int[im];	//最悪、頂点みっつの組み合わせ分必要になるのだが、ふつうそんなことにはならない

	//色だけは白で統一しておく
	for (int i=0 ; i<vm ; ++i) {
		mColor[i] = 0xffffffff;
	};

	mVertexMax = vm;
	mVertexNumber = 0;
	mIndexMax = im;
	mIndexNumber = 0;
};

void BatchData::setColor(unsigned int color ,int start,int number) {
	if (number<0) {
		number = mVertexNumber-start;
	}
	for (int i=0 ; i<number ; ++i) {
		mColor[i+start] = color;
	}
};
void BatchData::set(const VertexTransform& vt,int start,int number) {
	if (number<0) {
		number = mVertexNumber-start;
	}

	vt.transform(start,number,
		mX,mY,mZ,mNormal);
}

TextureRect::TextureRect() {
	mU=0.f;
	mV=0.f;
	mW = 1.f;
	mH=1.f;
	mSize = 1.f;
	mColor = 0xffffffff;
};
void BatchData::checkError() {
	ASSERT(mVertexNumber < mVertexMax);
	ASSERT(mIndexNumber < mIndexMax);
};

void BatchData::read(const Tag& t){

	int vtn = mVertexNumber;

	TextureRect tr;

	//座標
	if (t.attribute("rect")) {
		tr.mU = t.attribute("rect")->getIntValue(0);
		tr.mV = t.attribute("rect")->getIntValue(1);
		tr.mW = t.attribute("rect")->getIntValue(2);
		tr.mH = t.attribute("rect")->getIntValue(3);
	}

	//サイズ
	tr.mSize.x = tr.mW;
	tr.mSize.y = tr.mH;
	tr.mSize.z = (tr.mW + tr.mH) / 2.f;	//適当な値
	if (t.attribute("size")) {
		tr.mSize.x = t.attribute("size")->getIntValue(0);
		tr.mSize.y = t.attribute("size")->getIntValue(1);
		tr.mSize.z = t.attribute("size")->getIntValue(2);
	}


	//色
	if (t.attribute("color")) {
		tr.mColor = t.get("color");
	}

	//頂点変換
	VertexTransform vt;
	//中心
	if (t.attribute("position")) {
		vt.position(
			t.attribute("position")->getIntValue(0),
			t.attribute("position")->getIntValue(1),
			t.attribute("position")->getIntValue(2));
	}
	//拡大
	if (t.attribute("scale")) {
		vt.scale(
			1.f * t.attribute("scale")->getIntValue(0) / 100.f,
			1.f * t.attribute("scale")->getIntValue(1) / 100.f,
			1.f * t.attribute("scale")->getIntValue(2) / 100.f);
	}
	//回転
	if (t.attribute("angle")) {
		vt.rotate (
			t.attribute("angle")->getIntValue(0),
			t.attribute("angle")->getIntValue(1),
			t.attribute("angle")->getIntValue(2));
	}
	//法線反転
	if (t.attribute("reverseNormal")) {
		vt.mReverseNormal = true;
	}

	//形状パターン
	MyString command;
	if (t.attribute("type")) {
		command = t.attribute("type")->getValue(0);
	}
	else {
		//指定なければ矩形
		addRect(tr,vt);
	}

	//形状パターンごとのパラメータタグ
	const Tag* param = t.child("parameter");

	if (command=="none") {
	}

	if (command=="cylinder") {
		bool futa = true;
		bool soko = true;
		float ratio=0.f;

		//古い書式。引数が並ぶだけなので分かりにくい。
		if (t.attribute("type")->getValue(1)) {
			if (t.attribute("type")->getIntValue(1)) {
				futa = false;
				soko = false;
			}
		}
		if (t.attribute("type")->getValue(2)) {
			ratio = 1.f * t.attribute("type")->getIntValue(2) / 100;
		}

		//新しい書式。parameterタグで書いてある
		if (param) {
			if (param->attribute("top")) {
				futa = param->attribute("top")->getIntValue();
			}
			if (param->attribute("bottom")) {
				soko = param->attribute("bottom")->getIntValue();
			}
			if (param->attribute("proportion")) {
				ratio = param->attribute("proportion")->getFloatValue()/100.f;
			}
		};

		addCylinder(tr,vt,futa,soko,ratio);
	}
	if (command=="UFO") {
		addUFO(tr,vt);
	}
	if (command=="sphere") {
		addSphere(tr,vt);
	}
	if (command=="cube") {
		addCube(tr,vt);
	}
	if (command=="tree") {
		int number = 2;
		if (param) {
			if (param->attribute("number")) {
				number = param->attribute("number")->getIntValue();
			}
		}
		addTree(tr,vt,number);
	}
	if (command=="octahedron") {
		addOctahedron(tr,vt);
	}
	if (command=="rectPyramid") {
		addRectangularPyramid(tr,vt);
	}
	if (command=="torus") {
		addTorus(tr,vt);
	}
	if (command=="hole") {
		addHole(tr,vt);
	}

	if (command=="triangle") {
		float u[3];
		float v[3];
		float x[3];
		float y[3];
		float z[3] = {0.f,0.f,0.f};
		u[0] = tr.mU;
		v[0] = tr.mV;
		u[1] = tr.mU;
		v[1] = tr.mV+tr.mH;
		u[2] = tr.mU+tr.mW;
		v[2] = tr.mV;

		for (int i = 0 ; i<3 ; ++i) {
			x[i] = t.attribute("type")->getIntValue(1+i*2);
			y[i] = t.attribute("type")->getIntValue(1+i*2+1);
		}
		addTriangle(u,v,x,y,z,tr.mColor,vt);
	}
	if (command=="prism") {
		int n = t.attribute("type")->getIntValue(1);
		bool futa = true;
		bool soko = true;
		if (param) {
			if (param->attribute("number")) {
				n = param->attribute("number")->getIntValue();
			}
			if (param->attribute("top")) {
				futa = (param->attribute("top")->getIntValue() == 1);
			}
			if (param->attribute("bottom")) {
				soko = (param->attribute("bottom")->getIntValue() == 1);
			}
		}
		addPrism(tr,vt,n,futa,soko);
	}
	if (command=="gear") {
	
		int n = t.attribute("type")->getIntValue(1);
		addGear(tr,vt,n);
	}
	if (command=="bullet") {
		bool soko = true;
		if (param) {
			if (param->attribute("bottom")) {
				soko = (param->attribute("bottom")->getIntValue() == 1);
			}
		}
		else {
			if (t.attribute("type")->getValue(1)) {
				soko = t.attribute("type")->getIntValue(1);
			}
		}
		addBullet(tr,vt,soko);
	}
	if (command=="star") {
		int n = 5;
		if (t.attribute("type")->getValue(1)) {
			n = t.attribute("type")->getIntValue(1);
		}

		addStar(tr,vt,n);
	}

	if (command=="solidOfRevolution") {
		int n = t.attribute("vertex")->getParameter() / 2;
		float* x = new float [n];
		float* z = new float [n];


		for (int i=0 ; i<n ; ++i) {
			x[i] = 1.f * t.attribute("vertex")->getIntValue(i*2) / 100.f;
			z[i] = 1.f * t.attribute("vertex")->getIntValue(i*2+1) / 100.f;
		}
		addSolidOfRevolution(x,z,0,n,tr,vt);

		delete []x;
		delete []z;
	}
	if (command=="rock") {
		int n = 2;
		if (param) {
			if (param->attribute("cut")) {	//ダイヤモンドをカットするイメージ
				n = param->attribute("cut")->getIntValue();
			}
		}

		addRock(tr,vt,n);
	}
	if (command=="load") {
		int v = mVertexNumber;
		NML nml;
		nml.load(t.attribute("path")->getValue());
		read(*nml.root()->child("batch"));
		set(vt,v,mVertexNumber-v);
	}

	//この先工事中
	if (command=="pyramid") {
		//result=createPyramid(name.data(),U,V,W,H,0xffffffff);
		ASSERT(0);
	}
	if (command=="grass") {
		//result=createGlass(name.data(),U,V,W,H);
		ASSERT(0);
	}

	//頂点たりない
	ASSERT(mVertexMax > mVertexNumber);
	ASSERT(mIndexMax > mIndexNumber);


	//パーツ付け足し
	for (int i=0 ; i<t.getChild("parts"); ++i) {
		read(*t.child("parts",i));
	}

	//グラデーション
	if (t.child("gradation")) {
		unsigned int color1 = t.child("gradation")->attribute("color1")->getIntValue();
		unsigned int color2 = t.child("gradation")->attribute("color2")->getIntValue();
		GameLib::Math::Vector3 vct;
		vct.x = t.child("gradation")->attribute("vector")->getFloatValue(0);
		vct.y = t.child("gradation")->attribute("vector")->getFloatValue(1);
		vct.z = t.child("gradation")->attribute("vector")->getFloatValue(2);

		setGradation(color1,color2,vct,vtn);
	}

}

void BatchData::setGradation(unsigned int color1,unsigned int color2,const GameLib::Math::Vector3& vct,int start,int number){
	if (number < 0) {
		number = mVertexNumber - start;
	}

	//まず両端を調べる
	RangeGenerator<float> dotrange(0.f);
	for (int i=start ; i<start+number; ++i) {
		GameLib::Math::Vector3 v(mX[i],mY[i],mZ[i]);
		dotrange.add(v.dot(vct));
	}

	for (int i=start ; i<start+number; ++i) {
		GameLib::Math::Vector3 v(mX[i],mY[i],mZ[i]);
		float d = v.dot(vct);

		d -= dotrange.min();
		d /= dotrange.max()-dotrange.min();

		float a1,r1,g1,b1;
		float a2,r2,g2,b2;
		Math::unsignedToColor(&a1,&r1,&g1,&b1,color1);
		Math::unsignedToColor(&a2,&r2,&g2,&b2,color2);

		a1 = a1*(1.f-d) + a2*d;
		r1 = r1*(1.f-d) + r2*d;
		g1 = g1*(1.f-d) + g2*d;
		b1 = b1*(1.f-d) + b2*d;

		float a,r,g,b;
		Math::unsignedToColor(&a,&r,&g,&b,mColor[i]);

		mColor[i] = Math::getUnsignedColor(a1*a,r1*r,g1*g,b1*b);
	}
};


void BatchData::addRect(const TextureRect& tr,const VertexTransform& vt,float normalMode) {
	float*U = mU + mVertexNumber;
	float*V = mV + mVertexNumber;
	float*X = mX + mVertexNumber;
	float*Y = mY + mVertexNumber;
	float*Z = mZ + mVertexNumber;
	GameLib::Math::Vector3* normal = mNormal + mVertexNumber;
	int* index = mIndex + mIndexNumber;

	X[0]	= -tr.mSize.x/2;
	Y[0]	=  tr.mSize.y/2;
	X[1]	=  tr.mSize.x/2;
	Y[1]	=  tr.mSize.y/2;
	X[2]	=  tr.mSize.x/2;
	Y[2]	= -tr.mSize.y/2;
	X[3]	= -tr.mSize.x/2;
	Y[3]	= -tr.mSize.y/2;
	for (int i=0 ; i<4 ; ++i) {
		Z[i] = 0;
	}

	for (int i=0 ; i<4 ; ++i) {
		float z = GameLib::Math::cos(normalMode);
		float x = GameLib::Math::sin(normalMode) * GameLib::Math::cos(180.f -45.f - i*90.f);
		float y = GameLib::Math::sin(normalMode) * GameLib::Math::sin(180.f -45.f - i*90.f);
		normal[i].set(x,y,z);
	}

	U[0] = tr.mU;
	V[0] = tr.mV;
	U[1] = tr.mU+tr.mW;
	V[1] = tr.mV;
	U[2] = tr.mU+tr.mW;
	V[2] = tr.mV+tr.mH;
	U[3] = tr.mU;
	V[3] = tr.mV+tr.mH;

	index[0] = mVertexNumber+0;
	index[1] = mVertexNumber+2;
	index[2] = mVertexNumber+1;
	index[3] = mVertexNumber+0;
	index[4] = mVertexNumber+3;
	index[5] = mVertexNumber+2;

	//変形
	set(vt,mVertexNumber,4);
	setColor(tr.mColor,mVertexNumber,4);

	mVertexNumber += 4;
	mIndexNumber += 6;

}

void BatchData::addTriangle(const GameLib::Math::Vector2* uv,const GameLib::Math::Vector3* ver,unsigned int color,const VertexTransform& vt) {
	float u[3];
	float v[3];
	float x[3];
	float y[3];
	float z[3];

	for (int i=0 ; i<3 ;++i) {
		u[i] = uv[i].x;
		v[i] = uv[i].y;
		x[i] = ver[i].x;
		y[i] = ver[i].y;
		z[i] = ver[i].z;
	}
	addTriangle(u,v,x,y,z,color,vt);

}

void BatchData::addTriangle(float* u,float* v,float* x,float* y,float* z,unsigned int color,const VertexTransform& vt) {
	//ハイライト部分。法線計算。
	//外積でいっぱつだが。
	//a＝（a1,a2,a3）、 b＝（b1,b2,b3）としたとき、 
	//（a2b3-a3b2, a3b1-a1b3, a1b2-a2b1）

	GameLib::Math::Vector3 v1;
	GameLib::Math::Vector3 v2;
	v1.set(x[1]-x[0],y[1]-y[0],z[1]-z[0]);
	v2.set(x[2]-x[0],y[2]-y[0],z[2]-z[0]);
	GameLib::Math::Vector3 n;
	n.setCross(v1,v2);
	n.normalize();

	for (int i=0 ; i<3 ; ++i) {
		mU[ i + mVertexNumber ] = u[i];
		mV[ i + mVertexNumber ] = v[i];
		mX[ i + mVertexNumber ] = x[i];
		mY[ i + mVertexNumber ] = y[i];
		mZ[ i + mVertexNumber ] = z[i];
		mColor[ i + mVertexNumber ] = color;
		mNormal[ i+ mVertexNumber ] = n;
	}

	mIndex[ mIndexNumber+0 ] = mVertexNumber+0;
	mIndex[ mIndexNumber+1 ] = mVertexNumber+1;
	mIndex[ mIndexNumber+2 ] = mVertexNumber+2;

	set(vt,mVertexNumber,3);	//頂点単位で指定してるのに変換いるのか？

	mVertexNumber += 3;
	mIndexNumber += 3;
}

void BatchData::addRectangularPyramid(const TextureRect& tr ,const VertexTransform& vt,bool soko) {
	int vtn = mVertexNumber;

	if (soko) {
		//まず底をつくる
		VertexTransform vtt;
		vtt.mAngle.set(180.f,0.f,0.f);
		addRect(tr,vtt);
	}

	//三角形よっつ
	float U[3];
	float V[3];
	float X[3];
	float Y[3];
	float Z[3];

	//てっぺん
	U[0] = tr.mU+tr.mW/2;
	V[0] = tr.mV+tr.mH/2;


	//ここランダムでもいいようにする
	X[0] = 0.f;
	Y[0] = 0.f;

	//X[0] = Random::instance()->getFloat(tr.mSize.x) - tr.mSize.x/2;
	//Y[0] = Random::instance()->getFloat(tr.mSize.y) - tr.mSize.y/2;
//	Z[0] = GameLib::Math::sqrt(
//		(tr.mW+tr.mH)*(tr.mW+tr.mH)/4
//		-(tr.mW*tr.mW + tr.mH*tr.mH)/4);			//正三角形になるように
	Z[0] = tr.mSize.z;

	//Zは固定
	Z[1] = 0.f;
	Z[2] = 0.f;

	for ( int i =0 ; i<4 ; ++i) {
		for (int j= 0 ; j<2 ; ++j) {

			float r=(i+1-j)*90.f+45.f;
			if (GameLib::Math::cos(r) > 0) {
				X[j+1] = -tr.mSize.x/2;
				U[j+1] = tr.mU;
			}
			else {
				X[j+1] = tr.mSize.x/2;
				U[j+1] = tr.mU+tr.mW;
			}
			if (GameLib::Math::sin(r) > 0) {
				Y[j+1] = tr.mSize.y/2;
				V[j+1] = tr.mV;
			}
			else {
				Y[j+1] = -tr.mSize.y/2;
				V[j+1] = tr.mV+tr.mH;
			}
		}
		addTriangle(U,V,X,Y,Z,tr.mColor);
	}

	set(vt,vtn,mVertexNumber - vtn);

}

void BatchData::addCube(const TextureRect& tr,const VertexTransform& vt) {
	//側面
	for (int i=0 ; i<4 ; ++i) {
		TextureRect trr;
		trr = tr;
		trr.mV += trr.mH;
		if (i%2) {
			trr.mSize.x = tr.mSize.x;
		}
		else {
			trr.mSize.x = tr.mSize.y;
		}
		trr.mSize.y = tr.mSize.z;


		VertexTransform vtt;
		vtt.rotate(90.f,-90.f*i+90.f,0);

		GameLib::Math::Vector3 center(1.f,0.f,0.f);
		Math::rotateVector3(&center,center,0,0,90*i);
		center.x *= tr.mSize.x/2.f;
		center.y *= tr.mSize.y/2.f;
		center.z *= tr.mSize.z/2.f;

		vtt.position(center.x,center.y,center.z);

		addRect(trr,vtt);
	}

	//フタとそこ
	for (int i=0 ; i<2 ; ++i) {
		TextureRect trr;
		trr = tr;
		trr.mV += trr.mH*2*i;

		VertexTransform vtt;
		vtt.rotate(180.f*i,0.f,0.f);

		GameLib::Math::Vector3 center(0.f,0.f,1.f);
		Math::rotateVector3(&center,center,180*i,0,0);

		center.x *= trr.mSize.x/2.f;
		center.y *= trr.mSize.y/2.f;
		center.z *= trr.mSize.z/2.f;

		vtt.position(center.x,center.y,center.z);

		addRect(trr,vtt);
	}

	set(vt,mVertexNumber-24,24);
};

//矩形をふたつ組み合わせた木型
//cull0,0で使うこと　ライティング禁止
void BatchData::addTree(const TextureRect& tr,const VertexTransform& vt,int number) {

	for (int i=0 ; i<number ; ++i) {
		VertexTransform vtt;

		GameLib::Math::Vector3 ang(90.f,0.f,0.f);
		ang.y = 180.f / number * i;
		vtt.mAngle = ang;


		TextureRect trr = tr;
		if (i == 0) {
			trr.mSize.x = tr.mSize.x;
		}
		else {
			trr.mSize.x = tr.mSize.y;
		}

		trr.mSize.y = tr.mSize.z;
		addRect(trr,vtt);
	}

	set(vt,mVertexNumber-number*4,number*4);
}


void BatchData::addSphere(const TextureRect& tr,const VertexTransform& vt) {
	//サイズによってポリゴン数を決定しよう。
	//rがrevolutionに投げる頂点数。最低4
	//revolusuionに分割させる数はその倍。
	//あまり小さいのもだめだ。
	//8-12が普段
	//でかすぎてもやっぱりだめ。
	int r = tr.mSize.z / 16;						//頂点数をきめる
	int sp = (tr.mSize.x + tr.mSize.y) / 2.f / 8;	//分割数をきめる
	r += 4;
	sp += 8;
	if (PrimaryConfig::mConfig.mOmit) {
		r /= 2;
		sp /= 2; 
	}
	if (r<4) {
		r=4;
	}
	if (sp<8) {
		sp = 8;
	}
	if (r>16) {
		r= 16;
	}
	if (sp>32) {
		sp = 32;
	}

	float* x = new float[r];
	float* z = new float[r];
	GameLib::Math::Vector2* normal = new GameLib::Math::Vector2[r];

	for (int i=0 ; i<r ; ++i) {
		float rr = 180.f * i / (r-1);

		x[i] = GameLib::Math::sin(rr);	//完全な球になるように
		z[i] = GameLib::Math::cos(rr);

		normal[i].x = GameLib::Math::sin(rr);
		normal[i].y = GameLib::Math::cos(rr);
	}

	addSolidOfRevolution(x,z,normal,r,sp,0,-1,tr,vt);

	delete []x;
	delete []z;
	delete []normal;
}

//トーラス
void BatchData::addTorus(const TextureRect& tr,const VertexTransform& vt){
	//内径と外形の比が問題になる
	float r = tr.mSize.z / tr.mSize.x;				//筒の半径

	//テクスチャの上辺が外側になるように
	const int n=6;	//六角形

	float x[n];
	float z[n];
	GameLib::Math::Vector2 normal[n];
	for (int i=0 ; i<n ; ++i) {
		float a = 360.f *i /(n-1)+90.f;

		x[i] = GameLib::Math::sin(a) * r + 1.f - r;
		z[i] = GameLib::Math::cos(a);

		normal[i].x = GameLib::Math::cos(a);
		normal[i].y = GameLib::Math::sin(a);
	}

	addSolidOfRevolution(x,z,0,n,tr,vt);
}

//穴
void BatchData::addHole(const TextureRect& tr,const VertexTransform& vt) {
	int n = 4;
	float* x = new float [n];
	float* z = new float [n];

	//底を閉じておこう　やっぱりいらない
	//x[0] = 0.f;
	//z[0] = -1.f;
	for (int i=0 ; i<n ; ++i) {
		float m = 1.f*(n-1) /n - 1.f;

		float Z = (1.f*i/n) - 1.f;
		float X = (m*m)/(Z*Z);

		x[i] = X;
		z[i] = Z;
	}
	addSolidOfRevolution(x,z,0,n,tr,vt);

	delete []x;
	delete []z;
}

//角柱
void BatchData::addPrism(const TextureRect& tr,const VertexTransform& vt,int n,bool futa,bool soko) {
	float*x = new float [n];
	float*y = new float [n];
	for (int i=0 ; i<n ; ++i) {
		float r = 360.f*i/n;
		x[i] = GameLib::Math::cos(r);
		y[i] = GameLib::Math::sin(r);
	}
	addPrism(x,y,n,tr,vt,futa,soko);
	delete []x;
	delete []y;
}

//歯車
void BatchData::addGear(const TextureRect& tr,const VertexTransform& vt,int n) {
	float*x = new float [n*4];
	float*y = new float [n*4];

	for (int i=0 ; i<n ; ++i) {
		int j=i*4;
	
		float b = 0.8f;
		float d = 360.f /n;
		float dd = d/16.f;

		float r = d*i;
		x[j] = GameLib::Math::cos(r+dd);
		y[j] = GameLib::Math::sin(r+dd);

		x[j+1] = GameLib::Math::cos(r+d/2.f-dd);
		y[j+1] = GameLib::Math::sin(r+d/2.f-dd);
		x[j+2] = b * GameLib::Math::cos(r+d/2.f+dd);
		y[j+2] = b * GameLib::Math::sin(r+d/2.f+dd);
		x[j+3] = b * GameLib::Math::cos(r+d-dd);
		y[j+3] = b * GameLib::Math::sin(r+d-dd);
	}
	addPrism(x,y,n*4,tr,vt);
	delete []x;
	delete []y;

}
//星
void BatchData::addStar(const TextureRect& tr,const VertexTransform& vt,int n) {
	float* x = new float [n*2];
	float* y = new float [n*2];

	for (int i=0 ; i<n*2 ; ++i) {
		float r = 360.f*i/(n*2);
		float d = 1.f / (i%2 + 1);

		x[i] = d * GameLib::Math::cos(r);
		y[i] = d * GameLib::Math::sin(r);
	}
	addPrism(x,y,n*2,tr,vt);

	delete []x;
	delete []y;
}

void BatchData::addRock(const TextureRect& tr,const VertexTransform& vt,int n) {

	//まず四面体をつくる
	const int vmax = 256;
	const int imax = 512;
	GameLib::Math::Vector3 ver[vmax];
	unsigned int index[imax];
	ver[0].set(0.f,0.f,1.f);

	for (int i=0 ; i<3 ; ++i) {
		ver[i+1].z = -1.f / 3.f;

		float d = GameLib::Math::sqrt(8.f/ 9.f);
		ver[i+1].x = GameLib::Math::cos(i*120.f) * d;
		ver[i+1].y = GameLib::Math::sin(i*120.f) * d;
	}

	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
	index[3] = 0;
	index[4] = 1;
	index[5] = 3;
	index[6] = 0;
	index[7] = 2;
	index[8] = 3;
	index[9] = 1;
	index[10] = 2;
	index[11] = 3;

	int vtn = 4;
	int idn = 12;
	int ids = 0;

	//頂点をふやす作業
	//あんまり多くしても意味ないし、データ量が爆発する
	for (int i=0 ; i<2 ; ++i) {
		int s = ids/3;
		int m = idn/3;

		ids = idn;
		for (int j=s ; j<m ; ++j) {
			int k = j*3;

			GameLib::Math::Vector3 v;
			v = ver[index[k]];
			v += ver[index[k+1]];
			v += ver[index[k+2]];
			v.normalize();
			ver[vtn] = v;

			index[idn] = vtn;
			index[idn+1] = index[k+0];
			index[idn+2] = index[k+1];

			index[idn+3] = vtn;
			index[idn+4] = index[k+1];
			index[idn+5] = index[k+2];

			index[idn+6] = vtn;
			index[idn+7] = index[k+2];
			index[idn+8] = index[k+0];


			vtn += 1;
			idn += 9;
			ASSERT(idn < imax);
			ASSERT(vtn < vmax);
		}
	}

	addConvexPolytope(ver,vtn,tr,vt);

	//for (int i=0 ; i<mVertexMax ; ++i) {
	//	mNormal[i].set(mX[i],mY[i],mZ[i]);
	//	mNormal[i].normalize();
	//}

//	//まずは頂点リストを作ろう
//	GameLib::Math::Vector3* ver = new GameLib::Math::Vector3[n];

//	for (int i=0 ; i<n ; ++i) {
//		Random::instance()->getAngle(&ver[i]);	//これではいけない。近い頂点ができる可能性がある。
//	}

//	addConvexPolytope(ver,n,tr,vt);
//	delete []ver;
}

void BatchData::addConvexPolytope(const GameLib::Math::Vector3* ver,int n,const TextureRect& tr,const VertexTransform& vt) {
	int vtn =  mVertexNumber;

	//組み合わせを調べ、ポリゴンを登録
	for (int i=0 ; i<n ; ++i) {
		for (int j=i+1 ; j<n ; ++j) {
			for (int k=j+1 ; k<n ; ++k) {

				//頂点が面の裏側にあるか表側にあるかチェック
				bool r = true;	//有効か？
				bool a = false;	//向き
				bool s = false;
				for (int t = 0 ; t<n ; ++t) {
					if (t == i || t==j || t==k) {
						continue;
					}
					float b = Math::spacePartition(ver[t],ver[i],ver[j],ver[k]);
					if (!s) {
						a = (b<0.f);
					}
					else {
						if (a != (b<0.f)) {
							r = false;
							break;
						}
					}
					s = true;
				}
				if (r) {
					GameLib::Math::Vector3 v[3];

					v[0] = ver[i];
					if (a) {
						v[1] = ver[j];
						v[2] = ver[k];
					}
					else {
						v[1] = ver[k];
						v[2] = ver[j];
					}

					for (int t=0 ; t<3 ; ++t) {
						v[t].x *= tr.mSize.x / 2.f;
						v[t].y *= tr.mSize.y / 2.f;
						v[t].z *= tr.mSize.z / 2.f;
					}

					GameLib::Math::Vector2 uv[3];	//UV座標どうすっかなあ
					uv[0].set(0.f,0.f);
					uv[1].set(1.f,0.f);
					uv[2].set(0.f,1.f);
					addTriangle(uv,v);
				};
			}
		}
	}

	set(vt,vtn,mVertexNumber-vtn);

}

//角柱
void BatchData::addPrism(const float* X,const float * Y,int number,const TextureRect& tr,const VertexTransform& vt,bool futa,bool soko) {
	int vtn =  mVertexNumber;

	//UV座標をどう設定するか？
	//フタのUV
	float h = tr.mH;
	{
		int d = 1;
		if (futa) {
			++d;
		}
		if (soko) {
			++d;
		}
		h /= d;
	}
	float v = 0.f;

	float hu1[3],hv1[3];
	float hu2[3],hv2[3];

	//フタ
	hu1[0] = tr.mU + tr.mW/2.f;
	hv1[0] = tr.mV + v;
	hu1[1] = tr.mU;
	hv1[1] = tr.mV + v + h;
	hu1[2] = tr.mU + tr.mW;;
	hv1[2] = tr.mV + v + h;

	if (futa) {
		v += h;
	}


	//側面のUV
	float vu1[3],vv1[3];
	float vu2[3],vv2[3];
	vu1[0] = tr.mU;
	vv1[0] = tr.mV+v;
	vu1[1] = tr.mU;
	vv1[1] = tr.mV+v+h;
	vu1[2] = tr.mU+tr.mW;
	vv1[2] = tr.mV+v+h;

	vu2[0] = tr.mU;
	vv2[0] = tr.mV+v;
	vu2[1] = tr.mU+tr.mW;
	vv2[1] = tr.mV+v+h;
	vu2[2] = tr.mU+tr.mW;
	vv2[2] = tr.mV+v;

	v += h;

	//底
	hu2[0] = tr.mU + tr.mW/2.f;
	hv2[0] = tr.mV + v+h;

	hu2[1] = tr.mU;
	hv2[1] = tr.mV + v;
	hu2[2] = tr.mU + tr.mW;;
	hv2[2] = tr.mV + v;

	for (int i=0 ; i<number ; ++i) {
		int j= (i+1)%number;;

		//側面
		float xtop1 = X[i] * tr.mSize.x / 2.f;
		float ytop1 = Y[i] * tr.mSize.y / 2.f;
		float xtop2 = X[j] * tr.mSize.x / 2.f;
		float ytop2 = Y[j] * tr.mSize.y / 2.f;

		float ztop =  tr.mSize.z / 2.f;
		float zbottom = -tr.mSize.z / 2.f;

		float xbottom1 = xtop1;
		float ybottom1 = ytop1;
		float xbottom2 = xtop2;
		float ybottom2 = ytop2;

		float x[3];
		float y[3];
		float z[3];

		x[0] = xtop1;
		y[0] = ytop1;
		z[0] = ztop;
		x[1] = xbottom1;
		y[1] = ybottom1;
		z[1] = zbottom;
		x[2] = xbottom2;
		y[2] = ybottom2;
		z[2] = zbottom;

		addTriangle(vu1,vv1,x,y,z,tr.mColor);


		x[0] = xtop1;
		y[0] = ytop1;
		z[0] = ztop;
		x[1] = xbottom2;
		y[1] = ybottom2;
		z[1] = zbottom;
		x[2] = xtop2;
		y[2] = ytop2;
		z[2] = ztop;

		addTriangle(vu2,vv2,x,y,z,tr.mColor);

		//フタと底

		if (futa) {
			x[0] = 0;
			y[0] = 0;
			z[0] = ztop;
			x[1] = xtop1;
			y[1] = ytop1;
			z[1] = ztop;
			x[2] = xtop2;
			y[2] = ytop2;
			z[2] = ztop;
	
			addTriangle(hu1,hv1,x,y,z,tr.mColor);
		}
		if (soko) {
			x[0] = 0;
			y[0] = 0;
			z[0] = zbottom;
			x[1] = xbottom2;
			y[1] = ybottom2;
			z[1] = zbottom;
			x[2] = xbottom1;
			y[2] = ybottom1;
			z[2] = zbottom;

			addTriangle(hu2,hv2,x,y,z,tr.mColor);
		}
	}
	set(vt,vtn,mVertexNumber-vtn);

}

//分割数を勝手に判断するバージョン
void BatchData::addSolidOfRevolution(
									 const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,
									 const TextureRect& tr,const VertexTransform& vt){										 
	//分割数判断 最低6　最大・・・128
	int split = (tr.mSize.x*vt.mScale.x + tr.mSize.y*vt.mScale.y) / 2.f / 8 + 6;
	if (PrimaryConfig::mConfig.mOmit) {
		split /= 2;
	}
	if (split<6){
		split = 6;
	}
	if (split>128) {
		split=128;
	}

	addSolidOfRevolution(
		x,z,normal,number,split,0,-1,tr,vt);
}
//フルバージョン
void BatchData::addSolidOfRevolution(
									 const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,
									 int split,int min,int max,
									 const TextureRect& tr,const VertexTransform& vt){
	if (max<0) {
		max = split;
	}
	float* X = mX + mVertexNumber;
	float* Y = mY + mVertexNumber;
	float* Z = mZ + mVertexNumber;
	float* U = mU + mVertexNumber;
	float* V = mV + mVertexNumber;
	GameLib::Math::Vector3* NORMAL = mNormal + mVertexNumber;
	int* index = mIndex + mIndexNumber;

	int r=max - min;

	//一辺あたりに作るインデックス数を数えておこう
	//座標が同じなら作らない。
	//ただし座標が同じでも法線が別になるので、頂点は指定どおり作る

	int inumber = 0;
	for (int i= 0 ; i<number-1 ; ++i) {
		if (x[i] == x[i+1] &&
			z[i] == z[i+1]) {
			;	//座標が同じならとばす
		}
		else {
			++inumber;
		}
	};

	int vtn=(r+1)*number;
	int idn=r*inumber*6;
	const float vpp = tr.mH / inumber;

	int vcount = 0;
	int icount = 0;
	//Z軸を回転軸にして作成
	for (int i=0 ; i<r+1 ; ++i) {
		float sn=GameLib::Math::sin( (i+min) * 360/split);
		float cs=GameLib::Math::cos( (i+min) * 360/split);
		float vposition = 0.f;

		for (int j=0 ; j<number ; ++j) {
			X[ vcount ]  = cs * x[j]	* tr.mSize.x / 2.f;
			Y[ vcount ]  = sn * x[j]	* tr.mSize.y / 2.f;
			Z[ vcount ]  = z[j]			* tr.mSize.z / 2.f;

			U[ vcount ]=  (tr.mW*i/r )	+ tr.mU;
			V[ vcount ]=  vposition		+ tr.mV;

			//法線
			float m,n;
			if (normal) {
				//法線が指定されている場合
				m = normal[j].x;
				n = normal[j].y;
			}
			else {
				//法線自動生成
				//隣りの頂点座標からとろう
				float x1 = x[j];
				float x2 = x[j];
				float z1 = z[j];
				float z2 = z[j];

				if (j) {
					x1=x[j-1];
					z1=z[j-1];
				}
				if (j<number-1) {
					x2=x[j+1];
					z2=z[j+1];
				}

				//これを90度回転させればおｋ
				m= (z1-z2);
				n= (x2-x1);
			}

			//で、三次元空間に回転
			float a=cs * m;
			float b=sn * m;		//よくわからんが解決したのでいいや
			float c=n;

			NORMAL[vcount].set(a,b,c);
			NORMAL[vcount].normalize();

			//NORMAL[p].set(cs,-sn,0);

			//インデックスつくる
			if (j<(number-1)) {
				//座標が同じならつくらない
				if (x[j] != x[j+1] ||
					z[j] != z[j+1]) {
					if (i<r) {
						int q = (vcount+number) % vtn;
	
						index[icount  ] =  vcount	+mVertexNumber;
						index[icount +1] = q+1		+mVertexNumber;
						index[icount +2] = q		+mVertexNumber;
						index[icount +3] = vcount	+mVertexNumber;
						index[icount +4] = vcount+1	+mVertexNumber;
						index[icount +5] = q+1		+mVertexNumber;

						icount += 6;
					}
					vposition += vpp;
				}
			}
			++vcount;
		}
	}
	ASSERT(icount == idn);
	ASSERT(vcount == vtn);

	set(vt,mVertexNumber,vtn);
	setColor(tr.mColor,mVertexNumber,vtn);
	mVertexNumber += vtn;
	mIndexNumber += idn;
};

void BatchData::addOctahedron(const TextureRect& tr,const VertexTransform& vt){
	//簡単モードで済ませよう
	int v = mVertexNumber;

	TextureRect trr = tr;
	trr.mSize.z /= 2.f;

	VertexTransform vtt;
	addRectangularPyramid(trr,vtt,false);
	vtt.rotate(180.f,0.f,0.f);
	addRectangularPyramid(trr,vtt,false);

	set(vt,v,mVertexNumber-v);
}


void BatchData::addUFO(const TextureRect& tr,const VertexTransform& vt){
	const int n=10;

	//UFOつくるよ！
	int vtn = mVertexNumber;

	//傘部分

	//傘の先端はとがっているので同座標頂点が二つある
	float x[n] = {0.f,	0.5f	,0.8f	,1.0f	,1.1f	  ,1.2f	,1.4f  ,1.5f	,1.5f	,0.f};
	float z[n] = {1.f,	0.98f	,0.75f	,0.5f	,0.25f	  ,0.f	,-0.2f ,-0.4f	,-0.4f	,-0.4f};

	//正規化してやる
	for (int i=0 ; i<n ; ++i) {
		x[i] /= 1.5f;
		z[i] /= 1.5f;
	}
	//どのくらい頂点数が要るのか？　さー？
	addSolidOfRevolution(x,z,0,n,tr);

	//脚部分
	for (int i=0 ; i<3 ; ++i) {
		VertexTransform vt;
		vt.position(
			x[7] *tr.mSize.x / 2.f / 2.f * GameLib::Math::cos(120.f*i),
			x[7] *tr.mSize.y / 2.f / 2.f * GameLib::Math::sin(120.f*i),
			z[n-1]*tr.mSize.z / 2.f);

		vt.scale(
			0.6f/1.5f,
			0.6f/1.5f,
			0.6f/1.5f);

		TextureRect trr;
		trr = tr;
		trr.mV += trr.mH;

		addSphere(trr,vt);
	}

	//変形
	set(vt,vtn,mVertexNumber-vtn);
}

void BatchData::addBullet(const TextureRect& tr,const VertexTransform& vt,bool soko){
	int n=10;
	if (PrimaryConfig::mConfig.mOmit) {
		n = 5;
	}

	float* x = new float[n];
	float* z = new float[n];

	int k= n-2;	//曲線部分
	for (int i=0 ; i<k ; ++i) {
		float f = 1.f * i/(k-1);
		x[i] = f;
		z[i] = 1.f - f*f;
	}
	if (soko) {
		//底部分
		x[n-2] = 1.f;
		z[n-2] = 0.f;
		x[n-1] = 0.f;
		z[n-1] = 0.f;
	}
	else{
		n -= 2;
	}

	addSolidOfRevolution(x,z,0,n,tr,vt);

	delete []x;
	delete []z;
}

void BatchData::addCylinder(const TextureRect& tr,const VertexTransform& vt,bool futa,bool soko,float ratio){

	float top = 1.f;
	float bottom = 1.f;
	if (ratio < 0.f) {
		top = 1.f + ratio;
	}
	if (ratio > 0.f){
		bottom = 1.f - ratio;
	}

	float x[6];
	float z[6];
	x[0] = 0;
	x[1] = top;
	x[2] = top;
	x[3] = bottom;
	x[4] = bottom;
	x[5] = 0;

	z[0] = 1.f;
	z[1] = 1.f;
	z[2] = 1.f;
	z[3] = -1.f;
	z[4] = -1.f;
	z[5] = -1.f;


	int start =0;
	int number = 6;
	if (!futa) {
		start = 2;
		number -= 2;
	}
	if (!soko) {
		number -= 2;
	}

	addSolidOfRevolution(x+start,z+start,0,number,tr,vt);

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//モデルクラス

Model::Model(const MyTexture& parent) :
	mParent(&parent)
	{

	for (int i=0 ; i<mMax ; ++i) {
		mAnime[i] = 0;
	}
	mNext = 0;
	mAfterimage = 0;
	mAfterimageInterval = 0;
	mAfterimageScale = 0.f;

};
Model::~Model() {
	//オブジェクトを消しておきたいところだが。
	//終了時にすでに殺した後の可能性もある。チェックを入れよう
	Sprite::removeModel(this);

	for (int i=0 ; i<mMax ; ++i) {
		if (mAnime[i]) {
			delete mAnime[i];
			mAnime[i] = 0;
		}
	}
}
void Model::add(int index,Batch* batch,int delay,const Math::Polygon& collision) {
	if( mAnime[index] == 0) {
		mAnime[index] = new Anime;
	}
	mAnime[index]->mModel = this;
	mAnime[index]->add(batch,delay,collision);
};

void Model::draw(float x,float y,float z,const BlendInfo& bi,float animecount)const {
	draw(0,x,y,z,bi,animecount);
};
void Model::draw(const GameLib::Math::Vector3& pos,const BlendInfo& bi,float animecount)const {
	draw(pos.x,pos.y,pos.z,bi,animecount);
};

void Model::draw(MyTexture* tex,float x,float y,float z,const BlendInfo& bi,float animecount) const{
	GameLib::Math::Matrix34 wm;
	BlendElement be;
	be.set(bi);

	bi.getWorldMatrix(&wm,x,y,z,sprite(),frontEnd());

	draw(tex,wm,be,animecount);

};

void Model::draw(MyTexture* tex,GameLib::Math::Matrix34 wm,BlendElement be,float animecount) const{
	//残像処理
	for (int i=0 ; i<mAfterimage+1 ; ++i) {
		float c = animecount - (mAfterimage - i) * mAfterimageInterval;
		if (c>=0.f) {
			float zoom = 1.f + (mAfterimage-i) * mAfterimageScale;
			anime(wm,be)->draw(tex,wm,be,c,zoom);
		}
	}
}
void Model::getWorldMatrix(GameLib::Math::Matrix34* out,GameLib::Math::Matrix34 wm,BlendElement be,float animecount) const {
	anime(wm,be)->batch(out,0,0,wm,be,animecount);
};
void Model::getCollision(Math::Polygon* out,GameLib::Math::Matrix34 wm,BlendElement be,float animecount) const {
	GameLib::Math::Matrix34 world;
	anime(wm,be)->frame(&world,0,0,wm,be,animecount)->mCollision.mul(out,world);
}


Model::Anime* Model::anime(const GameLib::Math::Matrix34 wm,const BlendElement& be) const {
	GameLib::Math::Vector3 pos;
	pos.set(wm.m03,wm.m13,wm.m23);
	pos *= Draw::mWidth/2;
	return anime(pos,*be.angle(),be.mFrontEnd || frontEnd());
}

Model::Anime* Model::anime(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& ang,bool frontEnd) const{
	//アニメスレッドが複数登録されている場合
	//向きベクトルをビュー変換にかけてやって、方向を見るのがいいか
	//もっと厳密にするなら、向きはZ軸ではなくちゃんと変換かけてベクタをつくるほうがいい。面倒だけど。
	//なおスレッドの順は反時計回り。象限の順だ。

	//四パターン登録されているばあい、1,2,3,4で選べばいいが
	//二パターンの場合、画像はまず左右反転である
	//このとき1,2,2,1で選択される必要があるが。

	//四パターンの順を右下から開始にするべきか

	int a = getAnimeNumber();
	if (a>1) {
		float tt = 0.f;		//初期化しなければエラーチェックできるのだが、とりあえず見なかったことにする

		//ビュー変換にかけて
		GameLib::Math::Vector3 vct1;
		if (!frontEnd) {
			Draw::instance()->getScreenCoordinate(&vct1,pos);
		}
		else {
			vct1 = pos;
		}
		//そもそも表示圏外
		if (vct1.z > Draw::mDepth) {
			return mAnime[0];
		}

		for (int i=0 ; i<4; ++i) {
			//向きベクタをつくる
			//ニアクリップより近くなるとおかしくなるようだ。
			//たぶんXYの正負が逆転しているのだが、正確な挙動はわからないのでスキップしよう
			GameLib::Math::Vector3 vctB;
			Math::rotateVector3(&vctB,ang,0,0,90.f*i);
			vctB *= 128.f;
			if (frontEnd) {
				vctB.z *= -1.f;	//Z深度と、フロントエンドに指定するZ座標は別ものであることに注意。
			}
			vctB += pos;

			GameLib::Math::Vector3 vct2;
			if (!frontEnd) {
				Draw::instance()->getScreenCoordinate(&vct2,vctB);
			}else {
				vct2 = vctB;
			}

			if (vct2.z > Draw::mDepth) {
				continue;
			}

			float Z = vct2.z - vct1.z;
			float X = vct2.x - vct1.x;

			//Zがカメラからの距離ではないことに注意。このため向きベクタが画面に平行だとおかしくなる。
			//闢値をいくつにするかは微妙なところ。離れているとZは小さい値になる。Z!=0.fにするとちらつきが起こる
			//切り替わる角度付近でカメラを止めないというのが現実的なところだろう。
			if (Z*Z > 0.0001f) {	
				tt = GameLib::Math::atan2(Z,X) + 90.f*i;
				break;
			}
		}

		//角度を調べて
		float t = 360/a;
		tt+=90.f;

		//絵を象限ごとを想定してつくるか、XY軸方向を想定してつくるかで、どの角度で切り替わるかが代わる。
		int n = tt / t;
		//n += 1;

		if (tt<0) {
			n -= 1;
		}
		n = (n%a +a) %a;
		return mAnime[n];
	}

	return mAnime[0];
};

int Model::getAnimeNumber()const {
	int result = 0;
	for (int i=0 ; i<mMax ; ++i) {
		if (mAnime[i]==0) {
			break;
		}
		++result;
	}
	return result;
};
bool Model::depthWrite() const{
	return mAnime[0]->mMaterials.depthWrite();
}
int Model::sprite() const{
	return mAnime[0]->mMaterials.sprite();
}
bool Model::frontEnd() const{
	return mAnime[0]->mMaterials.frontEnd();
}

Model::Anime::Frame::Frame() {
	mBatch = 0;
	mDelay = 0;
	mCollision.mNumber = 0;
}
//アニメクラス
Model::Anime::Anime() {
	mModel = 0;
};
void Model::Anime::add(Batch* batch,int delay,const Math::Polygon& collision) {
	for (int i=0 ; i<mMax ; ++i) {
		if (mFrame[i].mBatch == 0) {
			mFrame[i].mBatch = batch;
			mFrame[i].mDelay = delay;
			mFrame[i].mCollision = collision;
			break;
		}
	}
}

void Model::Anime::draw(MyTexture* tex,GameLib::Math::Matrix34 wm,BlendElement be,float count,float scale) const {
	BlendElement blend;
	GameLib::Math::Matrix34 world;

	Batch* b = batch(&world,&blend,0,wm,be,count);
	world.scale(scale);
	if (tex) {
		b->draw(tex,world,blend);
	}
	else {
		b->draw(world,blend);
	}

}


int Model::Anime::getLoop(const GameLib::Math::Matrix34& wm,const BlendElement& be,float count) const {
	int result=0;
	batch(0,0,&result,wm,be,count);
	return result;
}

Batch* Model::Anime::batch(int i) const{
	ASSERT(i<mMax);
	return mFrame[i].mBatch;
}

Batch* Model::Anime::batch(GameLib::Math::Matrix34* outwm,BlendElement* outbe,int* loop,
	const GameLib::Math::Matrix34& wm,
	const BlendElement& be,
	float count)const{

	return frame(outwm,outbe,loop,wm,be,count)->mBatch;

}
const Model::Anime::Frame* Model::Anime::frame (GameLib::Math::Matrix34* outwm,BlendElement* outbe,int* loop,
	const GameLib::Math::Matrix34& wm,
	const BlendElement& be,
	float count)const
{
	const Frame* result = 0;
	int lp = 0;	//ループ数カウント
	const float mcount = count;

	//バッチさがす
	int i=0;
	bool next = false;
	while(true) {
		//ディレイが負　強制ストップ
		if (mFrame[i].mDelay < 0) {
			result = &mFrame[i];
			break;
		}

		//ディレイがゼロ　最初にもどる
		//添え字がゼロだったらストップ
		if (mFrame[i].mDelay == 0) {
			if (i==0) {
				result = &mFrame[i];
				break;
			}
			i=0;
			++lp;
			continue;
		}
		//カウントがディレイより減ったら、ヒット
		if (count < mFrame[i].mDelay) {
			result = &mFrame[i];
			break;
		}
		count -= mFrame[i].mDelay;
		++i;

		if (i>=mMax) {
			i=0;
			++lp;
		}

		if (mFrame[i].mBatch == 0) {
			i=0;
			++lp;
		}
		//次のアニメパターンへ以降
		if (lp && mModel->mNext) {
			result = mModel->mNext->anime(wm,be)->frame(outwm,outbe,loop,wm,be,count);
			next = true;
			break;
		}
	};

	if (next == false) {
		if (loop) {
			*loop = lp;
		}

	//materialss・world合成
		if (outwm || outbe) {
			Materials m;
			mMaterials.materials(&m,mcount);
			if (outwm) {
				*outwm = wm;
				m.getWorldMatrix(outwm);
			}
			if (outbe) {
				*outbe = be;
				outbe->set(m);
			}
		}
	}
	return result;
};

Model::Anime::MaterialsQueue::MaterialsQueue() {
};
void Model::Anime::MaterialsQueue::setStart(const Materials& mtr) {
	mStart = mtr;
	mAnimate.add(mtr,0.f);
}
void Model::Anime::MaterialsQueue::add(const Materials& mtr,float count) {
	mAnimate.add(mtr,count);
}

void Model::Anime::MaterialsQueue::read(const Tag& tag) {
	mAnimate.read(tag);
};

void Model::Anime::MaterialsQueue::refresh() {
	mAnimate.calculate();
}

void Model::Anime::MaterialsQueue::materials(Materials* result,float count) const {
	*result = mStart;
	mAnimate.write(result,count);
}
bool Model::Anime::MaterialsQueue::depthWrite() const{
	return mStart.mDepthWrite;
}
int Model::Anime::MaterialsQueue::sprite() const{
	return mStart.mSprite;
}
bool Model::Anime::MaterialsQueue::frontEnd() const{
	return mStart.mFrontEnd;
}

void Model::Anime::MaterialsQueue::Animate::add(const Materials& mtr,float t) {
	mAmbient.add(mtr.mAmbient,t);
	mEmission.add(mtr.mEmission,t);
	mSpecular.add(mtr.mSpecular,t);
	mTransparency.add(mtr.mTransparency,t);

	mAngle.add(mtr.mAngle,t);
	mScale.add(mtr.mScale,t);
	mPosition.add(mtr.mPosition,t);
}
void Model::Anime::MaterialsQueue::Animate::calculate() {
	mAmbient.calculate();
	mEmission.calculate();
	mSpecular.calculate();
	mTransparency.calculate();

	mAngle.calculate();
	mScale.calculate();
	mPosition.calculate();
};

void Model::Anime::MaterialsQueue::Animate::write(Materials* out,float t) const {
	mAmbient.value(&out->mAmbient,t);
	mEmission.value(&out->mEmission,t);
	mSpecular.value(&out->mSpecular,t);
	out->mTransparency = mTransparency.value(t);;

	mAngle.value(&out->mAngle,t);
	mScale.value(&out->mScale,t);
	mPosition.value(&out->mPosition,t);
};

void Model::Anime::MaterialsQueue::Animate::read(const Tag& tag) {
	if (tag.attribute("include")) {
		NML nml;
		nml.load(tag.attribute("include")->getValue());
		read(*nml.root());
	}

	read3(&mEmission,tag,"emission",100.f);
	read3(&mAmbient,tag,"ambient",100.f);
	read3(&mSpecular,tag,"specular",100.f);

	if (tag.child("transparency")) {
		read(&mTransparency,*tag.child("transparency"),100.f);
	}

	read3(&mAngle,tag,"angle",1.f);
	read3(&mScale,tag,"scale",100.f);
	read3(&mPosition,tag,"position",1.f);

};
void Model::Anime::MaterialsQueue::Animate::read3(Interpolation3* param,const Tag& tag,const char* name,float devide) {
	const Tag* t = tag.child(name);
	if (t) {
		if (t->child("all")) {
			read(&param->mX,*t->child("all"),devide);
			read(&param->mY,*t->child("all"),devide);
			read(&param->mZ,*t->child("all"),devide);
		}
		if (t->child("x")) {
			read(&param->mX,*t->child("x"),devide);
		}
		if (t->child("y")) {
			read(&param->mY,*t->child("y"),devide);
		}
		if (t->child("z")) {
			read(&param->mZ,*t->child("z"),devide);
		}
	}
}
void Model::Anime::MaterialsQueue::Animate::read(Interpolation* param,const Tag& tag,float devide) {
	if (tag.attribute("degree")) {
		param->mDegree = tag.attribute("degree")->getIntValue();
		param->mPointNumber = tag.getAttribute("time");
		for (int i=0 ; i<tag.getAttribute("time") ; ++i) {
			ASSERT(i<2);
			param->mPoint[i].mTime = tag.attribute("time",i)->getIntValue();
		}
		for (int i=0 ; i<tag.getAttribute("velocity") ; ++i) {
			ASSERT(i<2);
			param->mPoint[i].mVelocity = tag.attribute("velocity",i)->getFloatValue();
		}
		for (int i=0 ; i<tag.getAttribute("accel") ; ++i) {
			ASSERT(i<2);
			param->mPoint[i].mAccel = tag.attribute("accel",i)->getFloatValue();
		}
	};

	float count = 0.f;
	for (int i=0 ; i < tag.getChild() ; ++i) {
		const Tag* tt = tag.child(i);

		float v = tt->attribute("value")->getIntValue() / devide;
		if (tt->attribute("delay")) {
			count += tt->attribute("delay")->getIntValue();
		}

		param->add(v,count);
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Draw* Draw::mInstance=0;

int Draw::mFontSize[Draw::mFontMax] = {14,20,26};

Draw* Draw::instance() {
	return mInstance;
}
void Draw::create() {
	mInstance=new Draw;
};
void Draw::destroy() {
	if (mInstance) {
		delete mInstance;
		mInstance=0;
	}
	else {
		ASSERT(0);		//createより多くdestroyを読んだ
	}
}

Draw::Draw() {

	mCount=0;
	mSkip=false;

	//フェード ffでまっくら。
	mFade=255;
	mFadeAim=0;

	//フォント関係
	for (int i=0 ; i<mFontMax ; ++i) {
		int size = PrimaryConfig::mConfig.windowHeight() * mFontSize[i] / mHeight;
		//int size = PrimaryConfig::mConfig.windowHeight() * mFontSize[i] / 480;
		if (i<3) {
			mFont[i] = GameLib::Scene::Font::create("ＭＳ ゴシック",size,false,false,false,size>16,512,512);
		}
		//else {
		//	mFont[i] = GameLib::Scene::Font::create("HGP行書体",size,false,false,false,size>16,512,512);
		//}
	}

	mStringRenderer=GameLib::Scene::StringRenderer::create(8192,1024);	//文字数

	//デバッグ機能
	mPrimitiveRenderer=GameLib::Scene::PrimitiveRenderer::create(8192,1024);

	//ゼロバッチ
	mZeroBatch.create(0,"zero",
		0.0f,0.0f,
		1.0f,1.0f,
		0xffffffff);
	mZeroBatch.mMaterials.setCullMode(false,false);

	//テクスチャ管理
	for (int i=0 ; i<MyTexture::mLoadingTextureMax ; ++i) {
		MyTexture::mLoadingTexture[i] = 0;
	}

	//ライト
	mLight[0].setPattern1();
	for (int i=0 ; i<4 ; ++i) {
		//mLight[i].mIndex=i;
		mLight[i].update(i);
	}

	//フロントエンドモードのビュー変換は固定だ
	//フロントエンドでもそうでない描画も最終的にZバッファ0-1で共通なので
	//普通の描画でニアクリップ付近を描画すると
	//必ずしもフロントエンドが手前に来るとは限らない
	//かといってZチェックをオフにすると3Dモデルは使えない・・・
	//なるべくフロントエンドはニアクリップぎりぎりを使って描こう
	mFrontEndPVM.setOrthogonalTransform(
		-1.f,1.f,
		-0.75f,0.75f,
		-1.f,2.f);				//Zの範囲をどうとるか？

};

//終了処理
Draw::~Draw(){
	mFont[0].release();
	mFont[1].release();
	mFont[2].release();
	mStringRenderer.release();
	mPrimitiveRenderer.release();
	mZeroBatch.release();
};

void Draw::update(){
	//ライト
	for (int i=0 ; i<4 ; ++i) {
		mLight[i].update(i);
	}
	mLight[0].setPattern1();	//上からのライト
	mLight[1].setPattern2();	//下からのライト
	mLight[2].setPattern3();	//任意
	mLight[3].setPattern4();	//逆光



	//フェード
	if (mFade!=mFadeAim) {
		if (mFade-8>mFadeAim) {
			mFade-=8;
		}
		if (mFade>mFadeAim) {
			--mFade;
		}
		if (mFade+8<mFadeAim) {
			mFade+=8;
		}
		if (mFade<mFadeAim) {
			++mFade;
		}
	}

	++mCount;
	if (!mSkip) {
		mPolygonPerFrame=0;
	}

	//ロード中のテクスチャがいたら処理してやろう
	if (MyTexture::mLoadingNumber) {
		MyTexture::loadUpdateAll();
	}

	//ビュー変換行列
	float range = mCamera.range();
	float zoom  = mCamera.zoom();

	//見える幅を一定にする　歪ませたくないならカメラを近づけるのではなくズーム値でやろう
	//画角はタンジェントでとる。正直、レンジはあんまりいじりたくない
	//基本距離30000fのとき、画角1fを想定
	const float tn = GameLib::Math::tan(1.f);
	float gkk = GameLib::Math::atan2(30000.f / 2.f * tn,range) * 2.f;

	mPVM.setPerspectiveTransform(
		gkk / zoom,
		PrimaryConfig::mConfig.windowWidth(),PrimaryConfig::mConfig.windowHeight(),
		range/10 / mWidth*2,				//near	変に消えるよりは、消えないほうが自然か？
		(range+30000.f*9.f) / mWidth*2);	//far	遠くはいくら遠くても問題ない たぶん
	//ニアクリップとファークリップの比は1000が限界
	//いまファークリップは300000くらいだから、レンジは300が限界か　入力する数値としては100。

	//ビュー行列

	GameLib::Vector3 vp = *mCamera.eyePosition();
	GameLib::Vector3 vt = *mCamera.eyeTarget();

	mPVM.multiplyViewTransform(
		vp,vt,
		*mCamera.upVector());

	mPVM.setOrthogonalTransform(
		mCamera.eyeTarget()->x - 1.f,
		mCamera.eyeTarget()->x + 1.f,
		mCamera.eyeTarget()->y -0.75f,
		mCamera.eyeTarget()->y +0.75f,
		-1.f,2.f);				//Zの範囲をどうとるか？

	//スプライト用の回転うちけし行列
	//行列を使った回転うちけし
	//カメラがいかなるパラメータでも使える
	//VP,VTの高さをあわせればZ軸はそのままにすることはできる
	GameLib::Math::Matrix34 tmp;

	vt -= vp;
	vp = 0.f;
	tmp.setViewTransform(
		vp,vt,
		GameLib::Math::Vector3(0.f,1.f,0.f));
	tmp.invert();
	mInvertWMforSprite2.setIdentity();
	mInvertWMforSprite2 *= tmp;

	//Z軸はそのまま
	//vt.z = 0.f;
	mInvertWMforSprite1.setViewTransform(
		vp,vt,
		GameLib::Math::Vector3(0.f,1.f,0.f));
	mInvertWMforSprite1.invert();

}

void Draw::setFade(int i) {
	mFade=i;
};
void Draw::aimFade(int i) {
	mFadeAim=i;
};


void Draw::getScreenCoordinate(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& in) const {
	//実験部分・・・違う
	GameLib::Math::Vector4 vct;

	GameLib::Math::Vector3 pos;
	pos.x = in.x *2.f / mWidth;
	pos.y = in.y *2.f / mWidth;
	pos.z = in.z *2.f / mWidth;

	mPVM.mul(&vct,pos);

	out->x = vct.x / vct.w * mWidth/2;
	out->y = vct.y / vct.w * mHeight/2;
	out->z = vct.z / vct.w * mDepth;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//文字表示
void Draw::text(int x,int y,const char* txt,unsigned int color ,int size) {
	ASSERT(size < mFontMax);
	if (mSkip) {
		return;
	}
	if (txt == 0) {
		return;
	}
	if (mFade<128) {
		mStringRenderer.setFont( mFont[size] );

		x+=mWidth/2;
		y*=-1;
		y+=mHeight/2;
		int p=x * PrimaryConfig::mConfig.windowWidth() / mWidth; 
		int q=y * PrimaryConfig::mConfig.windowHeight() / mHeight; 

		float z=500.f / mDepth;
		//影　黒字なので今回はパス
	//	if (color&0xffffff) {
	//		mStringRenderer.add(p+1,q+1,txt ,0xff000000,false, z+0.1f );
	//	}

		mStringRenderer.add(p  ,q,  txt ,color ,false, z );
	}

	//一回ごとにやるとさすがに重そうだ
	//renderString();
};
void Draw::text(int x,int y,char c ,unsigned int color,int size) {
	char s[2] = {c,0};
	text(x,y,s,color,size);
}
void Draw::text(int x,int y,int i,unsigned int color ,int size) {
	char s[64];
	itoa(i,s,10);
	text(x,y,s,color,size);
}
void Draw::text(int x,int y,unsigned int i,unsigned int color ,int size) {
	char s[64];
	itoa(i,s,10);
	text(x,y,s,color,size);
}
void Draw::text(int x,int y,bool b,unsigned int color ,int size) {
	if (b) {
		text(x,y,"有効",color,size);
	}
	else {
		text(x,y,"無効",color,size);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//一フレームにいっかいだけ呼び出す
void Draw::draw() {
	if (mSkip) {
		return;
	}

	renderString();

	if (mFade>0) {
		drawRect(0,0,mWidth,mHeight,
			0x01000000*mFade,
			0.0f);
	}
	mPrimitiveRenderer.draw();
};

void Draw::renderString() {
	//バッチで使ったのが残ってることがあるみたいなので、初期化してやらねばならない
	//いまのところαブレンドが残るのを確認
	GameLib::Graphics::Manager m = GameLib::Graphics::Manager::instance();
	m.setTransparency( 1.f);
	mStringRenderer.draw();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera() {
	initialize();
	mActive=false;
}

void Camera::reset() {
	mAngleResetSwitch = true;
	mTargetResetSwitch = true;
	mZoomResetSwitch = true;
	mRangeResetSwitch = true;
};
void Camera::initialize() {
	reset();

	mEyePosition.set(0.f,0.f,500.f);
	mEyePosition *= 2.f / Draw::mWidth;

	mUpVector.set(0.f,1.f,0.f);
	mEyeTarget=0.f;

	mZoom=1.0f;
	mAimZoom = 1.f;

	mTarget.set(0.f,0.f,0.f);
	mAimTarget.set(0.f,0.f,0.f);

	mHorizotalAngle = 0.f;
	mVerticalAngle = 90.f;
	mUpAngle = 0.f;

	mAimHorizotalAngle = 0.f;
	mAimVerticalAngle = 90.f;
	mAimUpAngle = 0.f;

	mFocus = 0.f;
	mFocusRate = 0.f;

	mRange=300000.f;
	mAimRange = 3000001.f;

	mVibrate = 0.f;

};


void Camera::update() {
	//目的地に近づける
	mTarget.set(
		mTarget.x*0.95f + mAimTarget.x*0.05f,
		mTarget.y*0.95f + mAimTarget.y*0.05f,
		mTarget.z*0.95f + mAimTarget.z*0.05f);

	//ズームを更新
	mZoom=mZoom*0.95f + mAimZoom*0.05f;

	//アングルを更新
	mVerticalAngle	= mVerticalAngle*0.95f	+ mAimVerticalAngle*0.05f;
	mHorizotalAngle	= mHorizotalAngle*0.95f	+ mAimHorizotalAngle*0.05f;
	mUpAngle		= mUpAngle*0.95f		+ mAimUpAngle*0.05f;

	//カメラ位置を計算
	mRange=mRange*0.95f + mAimRange*0.05f;

	Math::angleToVector3(
		&mEyePosition,
		mHorizotalAngle,
		mVerticalAngle);

	mEyePosition *= range();
	mEyePosition += mTarget;
	mEyePosition *= 2.f / Draw::mWidth;


	//最終注視点
	Math::lerp(&mEyeTarget,mFocusRate,mTarget,mFocus);
	if (mVibrate.squareLength() > 1.f) {
		mEyeTarget += mVibrate;
	}

	mEyeTarget *= 2.f / Draw::mWidth;

	if (Main::instance()->mCount%3 == 0) {
			mVibrate *= -0.8f;
	}

	//上方向
	GameLib::Math::Vector3 dir;	//カメラ向きベクタ
	dir = mEyeTarget;
	dir -= mEyePosition;
	GameLib::Math::Vector3 hv;	//横方向ベクタ
	mUpVector.set(0.f,1.f,0.f);

	hv.setCross(dir,mUpVector);
	hv.normalize();
	mUpVector.setCross(hv,dir);
	mUpVector.normalize();

	mUpVector *= GameLib::Math::cos(mUpAngle);
	mUpVector.madd(hv,GameLib::Math::sin(mUpAngle));


	//フォーカス減衰
	mFocusRate *= 0.95f;
}


//目標角度は全部ここを通る
void Camera::aimAngle(float h,float v) {
	mAimHorizotalAngle = h;
	mAimVerticalAngle = v;

	if (mAngleResetSwitch) {
		mAngleResetSwitch = false;
		setAngle(h,v);
	}
};

void Camera::autoAngle(float h,float v) {
	untie();
	if (mHorizotalAngle > h) {
		while (true) {
			if (mHorizotalAngle - h <180.f) {
				break;
			}
			h += 360.f;
		}
	}
	else {
		while (true) {
			if (h - mHorizotalAngle <180.f) {
				break;
			}
			h -= 360.f;
		}
	}

	aimAngle(h,v);

}

void Camera::aimAngle(const GameLib::Math::Vector3& pos) {

	float h = GameLib::Math::atan2(
		pos.y - mAimTarget.y,
		pos.x - mAimTarget.x);

	float v = GameLib::Math::atan2(
		pos.z - mAimTarget.z,
		GameLib::Math::Vector2(pos.x - mAimTarget.x , pos.y - mAimTarget.y).length());

	autoAngle(h,v);
};


void Camera::addAimAngle(float h,float v) {
	mAimHorizotalAngle	+= h;
	mAimVerticalAngle	+= v;
};
void Camera::aimVerticalAngle(float v) {
	aimAngle(mAimHorizotalAngle,v);
};

void Camera::untie() {

	mVerticalAngle		= Math::floatMod(mVerticalAngle   ,360.f);
	mAimVerticalAngle	= Math::floatMod(mAimVerticalAngle,360.f);
	mHorizotalAngle		= Math::floatMod(mHorizotalAngle  ,360.f);
	mAimHorizotalAngle	= Math::floatMod(mAimHorizotalAngle,360.f);
};

void Camera::setAngle(float h,float v) {
	mHorizotalAngle = h;
	mVerticalAngle = v;

	aimAngle(h,v);
};

//目標中心点変更は全部ここを通る
void Camera::aimTarget(float x,float y,float z) {
	mAimTarget.set(x,y,z);

	if (mTargetResetSwitch) {
		mTargetResetSwitch = false;
		setTarget(x,y,z);
	}
};

void Camera::aimTarget(const GameLib::Math::Vector3& vct) {
	aimTarget(vct.x,vct.y,vct.z);
}

void Camera::setTarget(float x,float y,float z) {
	mTarget.set(x,y,z);
	aimTarget(x,y,z);
};
void Camera::setTarget(const GameLib::Math::Vector3& vct) {
	mTarget = vct;
	aimTarget(vct);
}

void Camera::addAimTarget(float x,float y,float z) {
	aimTarget(
		mAimTarget.x + x,
		mAimTarget.y + y,
		mAimTarget.z + z);
};
//目標拡大率変更はすべてここを通る
void Camera::aimZoom(float z) {
	mAimZoom = z;


	if (mZoomResetSwitch) {
		mZoomResetSwitch = false;
		setZoom(z);
	}

}

void Camera::setZoom(float z) {
	mZoom = z;
	mAimZoom = z;
}
void Camera::aimRange(float r) {
	mAimRange = r;
	if (mRangeResetSwitch) {
		mRange = mAimRange;
		mRangeResetSwitch = false;
	}
}
void Camera::setUpAngle(float z) {
	mUpAngle = z;
	mAimUpAngle = z;
}
void Camera::aimUpAngle(float r) {
	mAimUpAngle = r;
}

void Camera::vibrate(float x,float y,float z) {
	mVibrate.set(x,y,z);
}
void Camera::focus(const GameLib::Math::Vector3& pos) {
	mFocus = pos;
	mFocusRate += 0.07f;
	if (mFocusRate> 1.f) {
		mFocusRate = 1.f;
	}
};

//以下アクセッサ
float Camera::zoom()const {
	return mZoom;
};
float Camera::horizotalAngle()const {
	return mHorizotalAngle;
}
float Camera::verticalAngle()const {
	return mVerticalAngle;
}
const GameLib::Math::Vector3* Camera::aimTarget()const {
	return &mAimTarget;
}
const GameLib::Math::Vector3* Camera::eyeTarget()const {
	return &mEyeTarget;
}
const GameLib::Math::Vector3* Camera::eyePosition()const {
	return &mEyePosition;
}
const GameLib::Math::Vector3* Camera::upVector()const {
	return &mUpVector;
}
float Camera::range() const{
	return 30000.f / mRange;
}
float Camera::getAimRange()const{
	return mAimRange;
};
float Camera::getAimZoom()const{
	return mAimZoom;
};
void Camera::getAngle(GameLib::Math::Vector3* out)const {
	Math::angleToVector3(out,horizotalAngle(),verticalAngle());
	*out *= -1.f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//primitiveを使用する
//線を引く。
//もはやなんの意味もなしていない
void Draw::drawLine(int x1,int y1,int x2,int y2,float scroll,unsigned int c1,unsigned int c2) {
	if (mSkip) {
		return;
	}
	float p1,q1,p2,q2;

	if (scroll==0) {
		p1=x1;
		q1=y1;

		p2=x2;
		q2=y2;
	}
	else {
		p1=x1 + mWidth /2;
		q1=y1 + mHeight/2;
		p2=x2 + mWidth /2;
		q2=y2 + mHeight /2;
	}
	p1=  p1*2/mWidth -1.0f;
	q1=  q1*2/mHeight-1.0f;
	p2=  p2*2/mWidth -1.0f;
	q2=  q2*2/mHeight-1.0f;

	Draw::mPrimitiveRenderer.addLine(
		GameLib::Math::Vector3(p1,q1,1.0f),
		GameLib::Math::Vector3(p2,q2,1.0f),
		c1,c2);
};

void Draw::drawLine(const GameLib::Math::Vector3& p,const GameLib::Math::Vector3& q) {
	GameLib::Math::Vector3 s,t;
	getScreenCoordinate(&s,p);
	getScreenCoordinate(&t,q);

	drawLine(s.x,s.y,t.x,t.y);
}
void Draw::drawRect(int x1,int y1,int x2,int y2,unsigned int color,int z) {
	if (mSkip) {
		return;
	}
	float p1,q1,p2,q2;

	p1=x1 * PrimaryConfig::mConfig.windowWidth() / mWidth;
	q1=y1 * PrimaryConfig::mConfig.windowHeight() / mHeight;

	p2=x2 * PrimaryConfig::mConfig.windowWidth() / mWidth;
	q2=y2 * PrimaryConfig::mConfig.windowHeight() / mHeight;

	mPrimitiveRenderer.setBlendMode(GameLib::Graphics::BLEND_LINEAR);
	mPrimitiveRenderer.addRectangle(
		GameLib::Math::Vector2(p1,q1),
		GameLib::Math::Vector2(p2,q2),
		color,
		1.0f*z/mDepth);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scanimate::Scanimate() {
	mBatch=0;
}
Scanimate::~Scanimate() {
	//テクスチャにはバッチが残ってるような・・・
	if (mBatch) {
		delete []mBatch;
		mBatch=0;
	}
}

void Scanimate::create(const char* path,int count,int n) {
	if (n<0) {
		n=MyTexture::mBatchMax;
	}
	if (PrimaryConfig::mConfig.mOmit) {
		n /= 2;
	}
	mSplitNumber=n;

	mTexture.load(path,false,true);
	while (!mTexture.isReady()) {
		;
	}
	mBatch = new Batch*[n];

	//セパレートバッチをつくる
	float w=1.f * mTexture.mTexture.originalWidth() / n;
	float h=mTexture.mTexture.originalHeight();

	for (int i=0 ; i<n ; ++i) {
		float u=w*i;
		float v=0.f;
		mBatch[i] = mTexture.createBatch("",u,v,w,h);
	}

	mCount=count;
	//mCount=2048+256+64-480-480-60;
};

void Scanimate::draw(int x,int y) {
	if (mCount) {
		for (int i=0 ; i<mSplitNumber ; ++i) {
			float p=mTexture.mTexture.originalWidth()* i / mSplitNumber - 1.f*mTexture.mTexture.originalWidth()/2.f;
			float q=0;

			float u,v;
			BlendInfo bi;
			bi.setBlendMode(2);
			bi.mBlendElement.mDepthTest=false;
			//bi.mCamera=false;
			float tt=(mCount) + (1.f* mCount * i / 512);

			//以下変換部分
			u=GameLib::Math::sin( tt*tt/50.f  +tt*0.f ) * (tt*tt/4096.f + tt*2.f );//* (320.f/(tt+1.f));
			v=GameLib::Math::sin( tt*tt/128.f +tt*3.f ) * (tt*tt/1024.f + tt   );
			//bi.setZoom(1.f , GameLib::Math::cos(tt*8)*4096 / (tt+4096));
			bi.mBlendElement.mAmbient.set(
				1024.f / (tt+1024.f),
				1024.f / (tt+1024.f),
				1024.f / (tt+1024.f));

			//bi.mRotateX= tt*2;
			//bi.mRotateZ= tt*tt/256;
			bi.mRotate.x= tt*tt/256.f;
			bi.mRotate.z= tt*tt/512.f+tt*2.f;

			mBatch[i] ->draw(u+p+x,v+q+y,0,bi);
		};

		if (mCount>0) {
			--mCount;
		}
	}
	else {
		BlendInfo bi;
		bi.setBlendMode(1);
		bi.mBlendElement.mDepthTest=false;
		//bi.mCamera=false;

		mTexture.draw(x,y,0,bi);
	}
}


/////////////2//////////////////////////////////////////////////////////////////////////////////////////////////

//質感情報
Materials::Materials() {

	setBlendMode(0);
	setCullMode(false,true);

	mDeffuse.set(1.f,1.f,1.f);
	mSpecular=0.f;
	mSharpness=0.f;
	mLighting[0] = false;
	mLighting[1] = false;
	mLighting[2] = false;
	mLighting[3] = false;

	mAmbient.set(0.f,0.f,0.f);
	mEmission.set(0.f,0.f,0.f);
	mTransparency=1.f;

	mAngle.set(0.f,0.f,0.f);
	mScale.set(1.f,1.f,1.f);
	mPosition.set(0.f,0.f,0.f);

	mFrontEnd =false;
	mSprite = 0;
	mNegative = false;
};

void Materials::setBlendMode(int m) {
	if (m==0) {
		mBlendMode=GameLib::Graphics::BLEND_OPAQUE;			//OPAQUEだとαテストが有効になる。なんでもLINERにすればいいものではないので注意しよう。
		mDepthTest=true;
		mDepthWrite=true;
	}
	if (m==1) {	//半透明
		mBlendMode=GameLib::Graphics::BLEND_LINEAR;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==2) {	//加算
		mBlendMode=GameLib::Graphics::BLEND_ADDITIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==3) {	//乗算
		mBlendMode=GameLib::Graphics::BLEND_MULTIPLE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==4) {	//反転
		mBlendMode=GameLib::Graphics::BLEND_NEGATIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==5) {	//加算と合成のあいだ　いまいち使えない
		mBlendMode=GameLib::Graphics::BLEND_PSEUDOADDITIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	ASSERT(m<6);
}

void Materials::setCullMode(bool front,bool back){
	if (front) {
		if (back){
			mCullMode =GameLib::Graphics::CULL_MAX;
		}
		else {
			mCullMode =GameLib::Graphics::CULL_FRONT;
		}
	}
	else {
		if (back) {
			mCullMode =GameLib::Graphics::CULL_BACK;
		}
		else {
			mCullMode =GameLib::Graphics::CULL_NONE;
		}
	}
}

void Materials::merge(const Materials& src,float blend) {
	if (blend >= 1.f) {
		mCullMode = src.mCullMode;
		mLighting[0] = src.mLighting[0];
		mLighting[1] = src.mLighting[1];
		mLighting[2] = src.mLighting[2];
		mLighting[3] = src.mLighting[3];

		mFrontEnd = src.mFrontEnd;
		mSprite = src.mSprite;
		mNegative = src.mNegative;
	}
	if (blend != 0.f) {
		mDeffuse *= (1-blend);
		mDeffuse.madd(src.mDeffuse,blend);

		mSpecular *= (1-blend);
		mSpecular.madd(src.mSpecular,blend);

		mAmbient *= (1-blend);
		mAmbient.madd(src.mAmbient,blend);

		mEmission *= (1-blend);
		mEmission.madd(src.mEmission,blend);

		mAngle *= (1-blend);
		mAngle.madd(src.mAngle,blend);

		mScale *= (1-blend);
		mScale.madd(src.mScale,blend);

		mPosition *= (1-blend);
		mPosition.madd(src.mPosition,blend);

		mSharpness = mSharpness*(1-blend) + src.mSharpness*blend;
		mTransparency = mTransparency*(1-blend) + src.mTransparency*blend;

	}

};



//タグから読み込み
void Materials::read(const Tag& tag) {
	if (tag.attribute("blendMode")) {
		setBlendMode(tag.attribute("blendMode")->getIntValue());
	};
	if (tag.attribute("depthTest")) {
		mDepthTest = tag.attribute("depthTest")->getIntValue();
	}
	if (tag.attribute("depthWrite")) {
		mDepthWrite = tag.attribute("depthWrite")->getIntValue();
	}

	if (tag.attribute("cull")) {
		setCullMode(
			tag.attribute("cull")->getIntValue(0),
			tag.attribute("cull")->getIntValue(1));
	}
	if (tag.attribute("deffuse")) {
		mDeffuse.set(
			1.f * tag.attribute("deffuse")->getIntValue(0) / 100.f,
			1.f * tag.attribute("deffuse")->getIntValue(1) / 100.f,
			1.f * tag.attribute("deffuse")->getIntValue(2) / 100.f);
	}
	if (tag.attribute("specular")) {
		mSpecular.set(
			1.f * tag.attribute("specular")->getIntValue(0) / 100.f,
			1.f * tag.attribute("specular")->getIntValue(1) / 100.f,
			1.f * tag.attribute("specular")->getIntValue(2) / 100.f);
	}
	if (tag.attribute("sharpness")) {
		mSharpness = 1.f * tag.attribute("sharpness")->getIntValue() / 100.f;
	}
	if (tag.attribute("light")) {
		mLighting[0]=tag.attribute("light")->getIntValue(0);
		mLighting[1]=tag.attribute("light")->getIntValue(1);
		mLighting[2]=tag.attribute("light")->getIntValue(2);
		mLighting[3]=tag.attribute("light")->getIntValue(3);
	}
	if (tag.attribute("ambient")) {
		mAmbient.set(
			1.f * tag.attribute("ambient")->getIntValue(0) / 100.f,
			1.f * tag.attribute("ambient")->getIntValue(1) / 100.f,
			1.f * tag.attribute("ambient")->getIntValue(2) / 100.f);
	}
	if (tag.attribute("emission")) {
		mEmission.set(
			1.f * tag.attribute("emission")->getIntValue(0) / 100.f,
			1.f * tag.attribute("emission")->getIntValue(1) / 100.f,
			1.f * tag.attribute("emission")->getIntValue(2) / 100.f);
	}
	if (tag.attribute("angle")) {
		mAngle.set(
			tag.attribute("angle")->getIntValue(0),
			tag.attribute("angle")->getIntValue(1),
			tag.attribute("angle")->getIntValue(2));
	}
	if (tag.attribute("scale")) {
		mScale.set(
			1.f * tag.attribute("scale")->getIntValue(0) / 100.f,
			1.f * tag.attribute("scale")->getIntValue(1) / 100.f,
			1.f * tag.attribute("scale")->getIntValue(2) / 100.f);
	}
	if (tag.attribute("position")) {
		mPosition.set(
			1.f * tag.attribute("position")->getIntValue(0),
			1.f * tag.attribute("position")->getIntValue(1),
			1.f * tag.attribute("position")->getIntValue(2));
	}

	if (tag.attribute("transparency")) {
		mTransparency = 1.f * tag.attribute("transparency")->getIntValue() / 100.f;
	}
	if (tag.attribute("frontEnd")) {
		mFrontEnd=true;
	}
	if (tag.attribute("sprite")) {
		mSprite=tag.get("sprite");
	}
	if (tag.attribute("negative")) {
		mNegative=true;
	}

};

void BlendElement::initialize(){
	setBlendMode(-1);
	mNegative=false;
	mAmbient.set(1.f,1.f,1.f);
	mEmission.set(0.f,0.f,0.f);
	mTransparency=1.f;
	mFrontEnd = false;
	mPolygonIndex=0;
	mPolygonNumber=-1;
	mLighting[0] = true;
	mLighting[1] = true;
	mLighting[2] = true;
	mLighting[3] = true;
	mReverseCulling = false;
};
void BlendElement::setBlendMode(int m) {
	//合成モードをmaterialsに任せる。
	if (m<0) {
		mBlendMode=GameLib::Graphics::BLEND_MAX;
		mDepthTest=true;
		mDepthWrite=true;
	}

	if (m==0) {
		mBlendMode=GameLib::Graphics::BLEND_OPAQUE;			//OPAQUEだとαテストが有効になる。なんでもLINERにすればいいものではないので注意しよう。
		mDepthTest=true;
		mDepthWrite=true;
	}
	if (m==1) {	//半透明
		mBlendMode=GameLib::Graphics::BLEND_LINEAR;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==2) {	//加算
		mBlendMode=GameLib::Graphics::BLEND_ADDITIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==3) {	//乗算
		mBlendMode=GameLib::Graphics::BLEND_MULTIPLE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==4) {	//反転
		mBlendMode=GameLib::Graphics::BLEND_NEGATIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	if (m==5) {	//加算と合成のあいだ　いまいち使えない
		mBlendMode=GameLib::Graphics::BLEND_PSEUDOADDITIVE;
		mDepthTest=true;
		mDepthWrite=false;
	}
	ASSERT(m<6);
}
void BlendElement::setColor(unsigned int color) {
	Math::unsignedToColor(
		&mTransparency,
		&mAmbient.x,
		&mAmbient.y,
		&mAmbient.z,
		color);
}
const GameLib::Math::Vector3* BlendElement::angle() const{
	return &mAngle;
};

void BlendElement::mulColor(const BlendElement& be) {
	//mDeffuse *= be.mDeffuse;
	//mSpecular *= be.mSpecular;
	mAmbient *= be.mAmbient;
	mEmission *= be.mEmission;
	mTransparency *= be.mTransparency;
};


//合成情報
BlendInfo::BlendInfo() {
	mBlendElement.initialize();

	mReverseX=false;
	mReverseY=false;
	mReverseZ=false;

	mRotate=0.f;

	setZoom();
};

void BlendInfo::setBlendMode(int m) {
	mBlendElement.setBlendMode(m);
}
void BlendInfo::setZoom(float x,float y,float z){
	mScale.set(x,y,z);
}
void BlendInfo::setColor(unsigned int color) {
	mBlendElement.setColor(color);
}

//////////////////////////////////////////////////////////

//ライト設定
Light * Draw::light(int i){
	ASSERT(i<4);
	return &mLight[i];
}

Light::Light() {
	mIntensity=1.f;
	mPosition.set(0.f,0.f,10000.f);
	mColor.set(1.f,1.f,1.f);

	mIntensityLevel = 1.f;
	mAimIntensityLevel = 1.f;
}

void Light::setQuantity(float level) {
	mAimIntensityLevel = level;
};
void Light::set(float intensity,const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& color) {
	mAimIntensityLevel = intensity;
	setPosition(pos);
	setColor(color);
}

void Light::update(int index) {
	GameLib::Graphics::Manager m = GameLib::Graphics::Manager::instance();

	m.setLightPosition	(index,mPosition);			//光源位置
	m.setLightIntensity	(index,mIntensity);		//ライト強度
	m.setLightColor		(index,mColor);			//ライト色

	mIntensityLevel = mIntensityLevel*0.95f + mAimIntensityLevel*0.05f;
};

float Light::intensity() const {
	return mIntensity * mIntensityLevel;
}
void Light::setIntensity(float i) {
	mIntensity = i;
}

void Light::setPosition(const GameLib::Math::Vector3& pos) {
	mPosition = pos;
	mPosition *= 2.f / Draw::mWidth;
}
void Light::setColor(const GameLib::Math::Vector3& color) {
	mColor = color;
}

void Light::setPattern1() {
	//基本光源。ななめ上から。
	mPosition.set(5000.f,10000.f,10000.f);

	mIntensity = 1.0f*mPosition.length();
	mColor.set(1.f,1.f,1.f);
}

//下から
void Light::setPattern2() {
	mPosition.set(-2500.f,-5000.f,-10000.f);
	mIntensity = 0.8f*mPosition.length();		//上光源よりは弱くしとくか
	mColor.set(1.f,0.5f,0.f);

	//mPosition = Draw::instance()->mCamera.mPosition;
	//mPosition *= 1.f / (Draw::mWidth/2);

	//mIntensity = 1.0f*mPosition.length();	//正確には、注視点とカメラの距離でとる。
	//mColor.set(1.f,1.f,1.f);
}

//毎フレーム情報が送られてくる照明
//情報が途切れたら切れるように。
void Light::setPattern3() {
	//mPosition = *Draw::instance()->mCamera.eyeTarget();
	//mPosition.z += 4.f / (Draw::mWidth/2);
	mIntensity = 128.f / (Draw::mWidth/2);
	mAimIntensityLevel = 0.f;

	//mColor.set(1.f,1.f,1.f);
}

///逆光
void Light::setPattern4() {
	GameLib::Math::Vector3 pos;
	mPosition = *Draw::instance()->mCamera.eyePosition();
	mPosition -= *Draw::instance()->mCamera.eyeTarget();
	mPosition.normalize();
	mPosition *= -64.f;
	mPosition += *Draw::instance()->mCamera.eyeTarget();

	mIntensity = 64.f;
	mColor.set(1.f,1.f,1.f);
}

