#ifndef INCLUDED_NPDRAW_H
#define INCLUDED_NPDRAW_H
#include <string>

#include "GameLib/Framework.h"
#include "GameLib/Graphics/Manager.h"
#include "GameLib/Graphics/Texture.h"
#include "GameLib/Graphics/VertexBuffer.h"
#include "GameLib/Graphics/IndexBuffer.h"
#include "GameLib/Graphics/Vertex.h"
#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Matrix34.h"

#include "GameLib/Scene/PrimitiveRenderer.h"

#include "GameLib/Scene/Font.h"
#include "GameLib/Scene/StringRenderer.h"

#include "npstring.h"
#include "npmath.h"

class Tag;
class MyTexture;
class Batch;
class Model;
class BlendInfo;


//�摜����n


//�Ɩ�
class Light{
	float mIntensity;						//���x
	GameLib::Math::Vector3 mPosition;		//���C�g�ʒu
	GameLib::Math::Vector3 mColor;			//���C�g�F
	float mIntensityLevel;					//���x�W��
	float mAimIntensityLevel;
public:

	Light();

	void set(float intensity,const GameLib::Math::Vector3& position,const GameLib::Math::Vector3& color);

	void setIntensity(float);
	void setPosition(const GameLib::Math::Vector3&);
	void setColor(const GameLib::Math::Vector3&);
	void setQuantity(float level);	//�Ɩ��̋���

	void update(int index);
	float intensity() const;


	void setPattern1();		//�ȂȂߏォ��
	void setPattern2();		//������
	void setPattern3();		//�X�|�b�g���C�g
	void setPattern4();		//��ɋt��

};

//���̂̎����ݒ�
class Materials{
public:
	GameLib::Graphics::BlendMode mBlendMode;//�������[�h
	bool mDepthTest;						//Z�e�X�g
	bool mDepthWrite;						//Z��������

	GameLib::Graphics::CullMode mCullMode;	//���\���[�h
	//GameLib::Graphics::LightingMode mMode;	//�Ɩ��@���_/�s�N�Z���@����͎�������˂���Ȃ�
	bool mLighting[4];						//�e���������郉�C�g

	GameLib::Math::Vector3 mDeffuse;		//���ːF
	GameLib::Math::Vector3 mSpecular;		//�X�y�L����
	float mSharpness;						//�n�C���C�g�̉s��


	bool mFrontEnd;							//false�Ńt�����g�G���h���[�h
	int mSprite;							//�X�v���C�g���[�h�@��ɐ��ʂ�����
	bool mNegative;

//���Ԍo�߂ŕω�����p�����[�^
	GameLib::Math::Vector3 mAmbient;		//���F
	GameLib::Math::Vector3 mEmission;		//����
	float mTransparency;					//�����x

	GameLib::Math::Vector3 mAngle;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mPosition;


	Materials();
	void setBlendMode(int i);
	void setCullMode(bool front=false,bool back=false);		//���ʂ��w��
	void read(const Tag&);
	void merge(const Materials&,float blend);
	void getWorldMatrix(GameLib::Math::Matrix34* out)const;
};

//�ŏI�������
class BlendElement{
	//materials�R��
	GameLib::Math::Vector3 mDeffuse;		//���ːF
	GameLib::Math::Vector3 mSpecular;		//�X�y�L����
	float mSharpness;						//�n�C���C�g�̉s��
	GameLib::Graphics::CullMode mCullMode;
	bool mReverseCulling;

	void reverseCulling();

	//�Ǘ�
	GameLib::Math::Vector3 mAngle;			//�X�v���C�g�p�Ɏg��
public:
	//blendInfo�R��
	int mPolygonIndex,mPolygonNumber;

	//�����R��
	GameLib::Graphics::BlendMode mBlendMode;	//�������[�h
	bool mDepthTest;							//Z�e�X�g
	bool mDepthWrite;							//Z�o�b�t�@�������݃��[�h�@p370�Q��
	bool mNegative;

	GameLib::Math::Vector3 mAmbient;			//���F
	GameLib::Math::Vector3 mEmission;			//����
	float mTransparency;						//�񓧖��x
	bool mFrontEnd;								//�����I�Ƀt�����g�G���h�\������
	bool mLighting[4];

	void initialize();
	void setBlendMode(int m);		//0.opaque 1.liner 2.add
	void setColor(unsigned int color);
	void set(const Materials&);
	void set(const BlendInfo&);
	void mulColor(const BlendElement&);	//transparency,emission,ambient����Z

	void setGraphicsManager()const;

	void get(GameLib::Math::Matrix34* out,float x,float y,float z,const BlendInfo&,const Materials&);
	const GameLib::Math::Vector3* angle()const;
};

//�������
class BlendInfo {
public:
	BlendElement mBlendElement;

	bool mReverseX,mReverseY,mReverseZ;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mRotate;

	BlendInfo();
	void setBlendMode(int m);		//0.opaque 1.liner 2.add
	void setZoom(float x=1.0f,float y=1.0f,float z=1.0f);
	void setColor(unsigned int color);			//���F�Ɣ񓧖��x��ݒ�

	void getWorldMatrix(GameLib::Math::Matrix34* out,float x,float y,float z,int sprite,bool frontend)const;
	bool cullReverse()const;	//���]����H
	void getAngleMatrix(GameLib::Math::Matrix34* out,int sprite,bool frontend)const;
};


//���f���f�[�^
class Model {
public:
	//�p�^�p�^�A�j��1���[�v�Ԃ������킷
	class Anime{

		//�f�ޏ��̍����N���X
		class MaterialsQueue{
			//�A�j���[�V�����N���X
			class Animate{
			public:
				Interpolation3 mAmbient;		//���F
				Interpolation3 mEmission;		//����
				Interpolation3 mSpecular;		//�X�y�L����
				Interpolation mTransparency;	//�����x

				Interpolation3 mAngle;
				Interpolation3 mScale;
				Interpolation3 mPosition;

				void add(const Materials&,float t);
				void write(Materials* out,float t)const;
				void calculate();
				void read(const Tag&);
			private:
				void read(Interpolation* param,const Tag&,float devide);
				void read3(Interpolation3* param,const Tag&,const char* name,float devide);
			};

			Animate mAnimate;
			Materials mStart;	//�����l

		public:
			MaterialsQueue();
			void materials(Materials*,float count)const;

			void setStart(const Materials&);
			void add(const Materials&,float time);
			void read(const Tag&);		//animate�^�O�ǂݍ���
			void refresh();				//�v�Z����

			bool depthWrite() const;	//��܂��ɁAZ�`�����݂��邩�ǂ����̓��f���ł��܂�ƍl����
			int sprite() const;			//��܂��ɁAZ�`�����݂��邩�ǂ����̓��f���ł��܂�ƍl����
			bool frontEnd() const;
		};
		class Frame{
		public:
			Batch* mBatch;
			int mDelay;
			Math::Polygon mCollision;

			Frame();
		};

		static const int mMax = 16;
		Frame mFrame[mMax];
	public:
		MaterialsQueue mMaterials;

		Model* mModel;
		MyString mName;

		Anime();
		void add(Batch*,int delay,const Math::Polygon& collision);
		void draw(MyTexture* ,GameLib::Math::Matrix34 world,BlendElement be,float count,float scale) const;

		int getLoop(float x,float y, float z,float count) const;
		int getLoop(const GameLib::Math::Matrix34&,const BlendElement&,float count) const;
		Batch* batch(int i) const;
		Batch* batch(GameLib::Math::Matrix34* wm,BlendElement* be,int* loop,
			const GameLib::Math::Matrix34&,
			const BlendElement&,
			float count)const;
		const Frame* frame(GameLib::Math::Matrix34* wm,BlendElement* be,int* loop,
			const GameLib::Math::Matrix34&,
			const BlendElement&,
			float count)const;
	};

	const MyTexture* mParent;
	static const int mMax = 8;
	Anime* mAnime[mMax];	//�l��������Ώ[�����낤���ǁB
	MyString mName;
	Model* mNext;			//���[�v�����ɕʃ��f���Ɉȍ~����
	int mAfterimage;		//�c����
	int mAfterimageInterval;
	float mAfterimageScale;

	Model(const MyTexture& parent);
	~Model();
	//void create(Batch* batch);	//�o�b�`�ЂƂ����Ȃ�A����ōς܂�
	void add(int animeindex,Batch*,int delay,const Math::Polygon& collision);

	void draw(float x,float y,float z,const BlendInfo&,float anime=0.f) const;
	void draw(const GameLib::Math::Vector3& pos,const BlendInfo&,float anime=0.f) const;
	void draw(MyTexture*,float x,float y,float z,const BlendInfo&,float anime=0.f) const;
	void draw(MyTexture*,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;

	Anime* anime(const GameLib::Math::Matrix34 wm,const BlendElement&) const; 
	Anime* anime(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& ang,bool frontend) const;
	
	void getWorldMatrix(GameLib::Math::Matrix34* out,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;
	void getCollision(Math::Polygon* out,GameLib::Math::Matrix34 wm,BlendElement,float anime) const;
	int getAnimeNumber()const;

	//materials�Ŏw�肷�邪�Amodel���ł͕s�ςȂ̂�O��ɂ���l
	bool depthWrite() const;
	int sprite() const;
	bool frontEnd() const;
};

//�o�b�`�쐬���A�Ō�ɍs������
//�ړ��E�g��E��]
class VertexTransform{
public:
	GameLib::Math::Vector3 mPosition;
	GameLib::Math::Vector3 mScale;
	GameLib::Math::Vector3 mAngle;
	bool mReverseNormal;

	VertexTransform();
	void position(float x,float y,float z);
	void scale(float x,float y,float z);
	void rotate(float x,float y,float z);
	void transform(int start,int number,float* x,float* y,float* z,GameLib::Math::Vector3* normal) const;
};

class BatchData;
//�o�b�`
class Batch {
	GameLib::Graphics::VertexBuffer vBuffer;	//���_�o�b�t�@
	GameLib::Graphics::IndexBuffer iBuffer;		//�C���f�b�N�X�o�b�t�@
	int mPolygon;								//�|���S����
public:
	bool isEnable;
	MyTexture* mTexture;		//�g�p�e�N�X�`��
	Materials mMaterials;		//����
	MyString mName;

	//�����蔻��.���S�����_�B
	float mCollisionX[4];
	float mCollisionY[4];

	Batch();
	~Batch();
	void release();

	//�o�b�`������Ƃ��̓e�N�X�`���ɂ��肢���邱�ƁB
	void create(MyTexture* tex,const char* name,float u,float v,float w,float h,unsigned int color,float sx=0,float sy=0,float sz=0,float zoomx=1.f,float zoomy=1.f,bool reversenormal=false,bool cut1pix=false);		//�l�p�`���ȒP�o�^
	void create(MyTexture* tex,const char* name,const float* u,const float* v,const float* x,const float* y,const float* z,const unsigned int* color,const int* index,int vtnumber,int indexnumber,const GameLib::Math::Vector3* normal = 0);		//���G�ȃo�b�`
	void create(MyTexture* tex,const char* name,const BatchData&);

	void draw(float x,float y,float z,const BlendInfo& bi = BlendInfo(),const Materials* materials = 0)const;
	void draw(MyTexture* tex,float x,float y,float z,const BlendInfo& bi,const Materials* materials = 0) const;
	void draw(const GameLib::Math::Matrix34& wm,const BlendElement&)const;	//�Ō�ɒʂ�draw
	void draw(MyTexture* tex,const GameLib::Math::Matrix34& wm,const BlendElement&)const;	//�Ō�ɒʂ�draw
};

//�e�N�X�`��
class MyTexture {
public:
	static const int mBatchMax = 128;
	static const int mModelMax = 64;
private:
	MyString mBatchPath;					//�o�b�`�t�@�C���p�X
	Batch* mBatch[mBatchMax];				//������ő{����
	Model* mModel[mModelMax];
public:
	static int mLoadingNumber;				//���[�h���̐�
	static const int mLoadingTextureMax=128;
	static MyTexture* mLoadingTexture[mLoadingTextureMax];

	bool isEnable;
	bool isLoading;							//���[�h���ł���
	bool isLoadingBatch;					//�o�b�`���[�h�҂�
	GameLib::Graphics::Texture mTexture;	//�e�N�X�`��
	bool mExistTexture;						//�e�N�X�`������́H

	MyTexture();
	~MyTexture();
	void release();
	void load(const char* path, bool batch,bool wait=true,const char* batchpath=0);	//path�Ƀk���Ńe�N�X�`���͂�Ȃ�
	void read(const Tag&,Materials mt = Materials());
	void loadUpdate();							//���[�h�A�b�v�f�[�g
	static void loadUpdateAll();
	void draw( float x, float y, float z,const BlendInfo &bi);
	Batch* createBatch(const Tag&,Materials);	//NML����o�b�`�쐬
	Model* createModel(const Tag&,Materials);

	int getNewBatch() const;
	Batch* createBatch(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff,const VertexTransform& vt = VertexTransform());									//�l�p�`���ȒP�o�^
	Batch* createBatch(const char* name,float* u,float* v,float* x,float* y,float* z,unsigned int* color,int* index,int vtnumber,int indexnumber,GameLib::Math::Vector3* normal = 0);	//���G�ȃo�b�`
	Batch* createBatch(const char* name,const BatchData&);	//���G�ȃo�b�`

	Batch* createChip(const char* name,int chipwidth,int chipheight,int number,int* u,int* v,int* x,int* y,int* z);	//�`�b�v�^�C�v�̃o�b�`
	Batch* createMap(const char* name,int chipwidth,int chipheight,int height,int number,int* u,int* v,int* x,int* y,int* z,int* slant);	//3D�}�b�v�^�C�v�̃o�b�`

	Batch* createTriangle(const char* name,float u,float v,float w,float h,unsigned int color1,unsigned int color2,unsigned int color3);	//���O�p�`
	Batch* createPyramid(const char* name,float u,float v,float w,float h,unsigned int color);		//�����`
	Batch* createHexWall(const char* name,float u,float v,float w,float h,unsigned int color);
	Batch* createGlass(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff);
	Batch* createLaputa(const char* name,float u,float v,float w,float h,unsigned int color=0xffffffff,int i=10,int j=8,int k=4);

	bool isReady();
	bool isError() const;
	int width() const;
	int height() const;
	int originalWidth() const;
	int originalHeight() const;

	Batch* batch(const char*)const;	//���O����o�b�`��{��
	Batch* batch(int i=0) const;
	Model* model(const char*,bool assert=true)const;
	Model* model(int i=0) const;
	void drawModel(const GameLib::Math::Vector3& pos,const BlendInfo&,float  animecount = 0);
};

class TextureRect{
public:
	float mU,mV,mW,mH;	//�e�N�X�`�����W
	unsigned int mColor;
	GameLib::Math::Vector3 mSize;
	TextureRect();
	void set(float u,float v,float w, float h,unsigned int color);
};

class BatchData{
public:
	float* mU;
	float* mV;
	float* mX;
	float* mY;
	float* mZ;
	GameLib::Math::Vector3* mNormal;
	unsigned int* mColor;
	int* mIndex;

	int mVertexMax;
	int mVertexNumber;
	int mIndexMax;
	int mIndexNumber;

	BatchData();
	~BatchData();
	void create(int vertexMax,int indexMax);
	void setColor(unsigned int color,int start=0,int number=-1);
	void set(const VertexTransform&, int start=0,int number=-1);
	void read(const Tag&);	//batch�^�O����͂��đ���
	void checkError();
	void setGradation(unsigned int color1,unsigned int color2,const GameLib::Math::Vector3&,int start,int number=-1);

	void addTriangle(const GameLib::Math::Vector2* uv,const GameLib::Math::Vector3* ver,unsigned int color = 0xffffffff,const VertexTransform& vt = VertexTransform());	//�O�p�`�ǉ�
	void addTriangle(float* u,float* v,float* x,float* y,float* z,unsigned int color = 0xffffffff,const VertexTransform& vt = VertexTransform());			//�O�p�`�ǉ�
	void addSphere(const TextureRect&,const VertexTransform& vt = VertexTransform());

	void addSolidOfRevolution(const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addSolidOfRevolution(const float* x,const float * z,const GameLib::Math::Vector2* normal,int number,int split,int min,int max,const TextureRect&,const VertexTransform& vt);

	void addRect			(const TextureRect&,const VertexTransform& vt = VertexTransform(),float normalMode=0.f);
	void addTree			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int number = 2);
	void addCube			(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addUFO				(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addBullet			(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool soko=true);
	void addCylinder		(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool top = true,bool bottom=true,float ratio = 0.f);	//�~���o�b�`
	void addTorus			(const TextureRect&,const VertexTransform& vt = VertexTransform());
	void addHole			(const TextureRect&,const VertexTransform& vt = VertexTransform());

	void addOctahedron		(const TextureRect&,const VertexTransform& vt = VertexTransform());						//���ʑ�
	void addRectangularPyramid(const TextureRect&,const VertexTransform& vt = VertexTransform(),bool soko=true);	//�l�p��

	void addPrism			(const float* x,const float * y,int number,const TextureRect&,const VertexTransform& vt = VertexTransform(),bool futa=true,bool soko=true);//���K���������W��n������
	void addPrism			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=3,bool futa = true ,bool soko = true);
	void addGear			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=3);
	void addStar			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=5);
	void addConvexPolytope	(const GameLib::Math::Vector3* ver,int n,const TextureRect&,const VertexTransform& vt = VertexTransform());	//���K���������W��n������
	void addRock			(const TextureRect&,const VertexTransform& vt = VertexTransform(),int n=2);
};


//�J�����N���X
//�����ׂ����
//�����_�@�p�x�@�Y�[����
//�����_����J�����̋����E������x�N�g���͌Œ�ɂ���

class Camera {
	bool mActive;							//�������ǂ����BAim���w�肳���Ǝ�����on
	//�Čv�Z�p
	GameLib::Math::Vector3 mUpVector;		//�����
	GameLib::Math::Vector3 mEyeTarget;		//�U�����܂ޒ����_
	GameLib::Math::Vector3 mEyePosition;

	float mRange,mAimRange;

	GameLib::Math::Vector3 mFocus;		//�t�H�[�J�X�B���t���[�����葱���Ȃ���΂Ȃ�Ȃ�
	float mFocusRate;					//�t�H�[�J�X���ǂꂾ���K�p���邩

	GameLib::Math::Vector3 mTarget;					//�J�����ʒu�����߂钍���_
	GameLib::Math::Vector3 mAimTarget;				//�����_

	float mVerticalAngle,mAimVerticalAngle;
	float mHorizotalAngle,mAimHorizotalAngle;
	float mUpAngle,mAimUpAngle;
	float mZoom,mAimZoom;							//�Y�[����

	GameLib::Math::Vector3 mVibrate;				//�U��

	bool mAngleResetSwitch;
	bool mTargetResetSwitch;
	bool mZoomResetSwitch;
	bool mRangeResetSwitch;
public:

	Camera();
	void reset();	//���ɗ����ڕW�l�����̂܂ܒl�ɂ���
	void initialize();
	void update();

	//������ݒ�
	void aimAngle(float horizotal,float vertical);				//��x�N�g���͌Œ�ł����̂œ��ōς܂�
	void autoAngle(float horizotal,float vertical);				//�߂����̉�]�������g���Ă����
	void aimAngle(const GameLib::Math::Vector3&);				//�Ώۍ��W-�w����W��ʂ�悤�ɃJ�������������߂�
	void setAngle(float horizotal,float vertical);				//���ʂɑ������
	void addAimAngle(float horizotal,float vertical=0.f);		//����
	void aimVerticalAngle(float v);
	void aimHorizotalAngle(float h);
	void untie();	//����ȏ�̊p�x���Ȃ��������Ƃɂ���

	//�����_�w��
	void aimTarget(float x,float y,float z);
	void aimTarget(const GameLib::Math::Vector3&);
	void setTarget(float x,float y,float z);
	void setTarget(const GameLib::Math::Vector3&);
	void addAimTarget(float x,float y,float z);
	//�Y�[��
	void aimZoom(float z);
	void setZoom(float z);
	//���_����
	void aimRange(float r);
	void vibrate(float x,float y,float z);
	//������x�N�^�̌X��
	void aimUpAngle(float );
	void setUpAngle(float );

	void focus(const GameLib::Math::Vector3&);			//�t�H�[�J�X

	//���
	float zoom()const;
	float horizotalAngle()const;
	float verticalAngle()const;
	const GameLib::Math::Vector3* aimTarget() const;
	const GameLib::Math::Vector3* target() const;
	const GameLib::Math::Vector3* position() const;
	const GameLib::Math::Vector3* upVector() const;
	const GameLib::Math::Vector3* eyePosition() const;
	const GameLib::Math::Vector3* eyeTarget() const;
	void getAngle(GameLib::Math::Vector3* out) const;//�J�������璍���_�ւ̃x�N�^
	float range() const;	//���ۂɎg������
	float getAimRange()const;	//mAimRange�̒l
	float getAimZoom()const;	//mAimRange�̒l

};

//�摜�\������
class Draw {

public:
	static const int mWidth=800;				//�����ň����Ƃ��̍��W
	static const int mHeight=600;
	static const int mDepth=512;
	static const int mFontMax = 3;
	static int mFontSize[mFontMax ];
private:
	GameLib::Scene::Font mFont[mFontMax ];
	GameLib::Scene::StringRenderer mStringRenderer;

	Light mLight[4];					//���C�e�B���O
	int mCount;

	//�قƂ�Ńf�o�b�O�p�̋@�\�ł��邱�Ƃɒ��ӁB
	GameLib::Scene::PrimitiveRenderer mPrimitiveRenderer;

	//�t�F�[�h ff�ł܂�����B
	int mFade;
	int mFadeAim;
public:
	int mPolygonPerFrame;	//�f�o�b�O�p�J�E���^
	bool mSkip;				//�`�悵�Ȃ����[�h�B�^�[�{�̕`��X�L�b�v�Ɏg�����炢���ȁB

	Camera mCamera;
	//�[���o�b�`
	Batch mZeroBatch;

	//�r���[�ϊ�	
	GameLib::Math::Matrix44 mPVM;
	GameLib::Math::Matrix44 mFrontEndPVM;
	GameLib::Math::Matrix34 mInvertWMforSprite1;
	GameLib::Math::Matrix34 mInvertWMforSprite2;

	//�֐��S
	void update();

	//primitive���g���B�x���̂Œ��ӁB
	void drawLine( int x1,int y1,int x2, int y2,float scroll=1.0f,unsigned int c1=0xffffffff,unsigned int c2=0xffffffff);
	void drawRect( int x1,int y1,int x2, int y2,unsigned int color=0xffffffff,int z=512);

	void drawLine(const GameLib::Math::Vector3&,const GameLib::Math::Vector3&);
	void draw();
	void setFade(int i);
	void aimFade(int i);
	void renderString();
	void nega() const;		//�l�K�e�B�u�ɂ���

	//�����\��
	void text(int x, int y,const char* txt,unsigned int color,int size);
	void text(int x, int y,char           ,unsigned int color,int size);
	void text(int x, int y,int            ,unsigned int color,int size);
	void text(int x, int y,unsigned int   ,unsigned int color,int size);
	void text(int x, int y,bool           ,unsigned int color,int size);

	//�Ɩ�����
	Light* light(int i);

	void getScreenCoordinate(GameLib::Math::Vector3* out,const GameLib::Math::Vector3& in) const;	//�X�N���[�����W�𓾂�

	static Draw* instance();
	static void create();
	static void destroy();
private:
	static Draw* mInstance;
	Draw();
	~Draw();
};

class Scanimate{
public:
	MyTexture mTexture;
	Batch** mBatch;
	//Batch* ba;
	int mSplitNumber;
	int mCount;

	Scanimate();
	~Scanimate();
	void create(const char* path,int count,int i = -1);
	void draw(int x,int y);
};

#endif
