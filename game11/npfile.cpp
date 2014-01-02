
#include <stdio.h>
#include <string>
#include <fstream>

#include "GameLib/Framework.h"

#include "GameLib/FileIO/Manager.h"
#include "GameLib/FileIO/InFile.h"
#include "GameLib/FileIO/OutFile.h"
#include "GameLib/Threading/Functions.h"

#include "GameLib/Base/Compressor.h"
#include "GameLib/Base/Array.h"

#include "npfile.h"
#include "npstring.h"
#include "modellist.h"

nFile::nFile() {
	mData=0;
	mLength=0;
};

nFile::~nFile() {
	mFile.release();

	if (mData) {
		delete []mData;
		mData=0;
	}
}

void nFile::load(const char* path,bool bgm,bool code) {
	if (mData) {
		delete []mData;
		mData=0;
	}

	mFile=GameLib::FileIO::InFile::create(path);

	if (bgm) {
		;
	}
	else {

		while (!mFile.isFinished()) {
			//BGMファイルを読み込む際には、サウンドバッファの更新に行ってはいけない
	//		if (BGM::instance()) {
	//			BGM::instance()->update();
	//		}
			ASSERT(!mFile.isError());
			GameLib::Threading::sleep(1);
		}

		if (code) {
			mData= new char[(mFile.size()-1)*128];
			mLength = FileSystem::decode(mData,mFile.data(),mFile.size());
		};
	}
};
bool nFile::isFinished() {
	return mFile.isFinished();
};
bool nFile::isError() {
	return mFile.isError();
}
int nFile::size() {
	if (mData) {
		return mLength;
	}

	return mFile.size();
};
const char* nFile::data() {
	if (mData) {
		return mData;
	}

	return mFile.data();
};
const char nFile::data(int i) {
	if (mData) {
		return mData[i];
	}

	char a=mFile.getUnsignedShort(i) %256;
	return a;
};

unsigned int nFile::data(int index,int length,bool little){
	unsigned int result=0;
	unsigned int r;

	if (mData) {
		for (int i=0 ; i<length ; ++i) {
			char t=mData[i+index];
	
			if (little) {
				result += t  *  (1<<(8*i)) ;
			}
			else {
				result *= 256;	
				result += t;
			}
		}
	}
	else {
		for (int i=0 ; i<length ; ++i) {
			r=mFile.getUnsignedShort(i+index) %256;
	
			if (little) {
				//r=*(mData.data()+i+index);
				result += r  *  (1<<(8*i)) ;
			}
			else {
				result *= 256;	
				result += r;
			}
		}
	}
	return result;
};

void nFile::release() {
	mFile.release();
};	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void nSave(const char* path,const char* data,int length,bool code) {
	if (length<0)  {
		length=strlen(data);
	}
	GameLib::FileIO::OutFile of;

	if (code) {
		char* res= new char[length*2];
		int l=FileSystem::encode(res,data,length);

		of=GameLib::FileIO::OutFile::create(path,res,l);
		while (!of.isFinished()) {
			;
		}

		delete []res;
	}
	else {
		of=GameLib::FileIO::OutFile::create(path,data,length);
		while (!of.isFinished()) {
			;
		}
	}
	ModelList::instance()->mSoundSet.sound("saved")->play();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int FileSystem::encode(char*result,const char* original,int length) {
	//本体コピーついでにビット反転
	int tmplength = length+5;
	char* tmp = new char[tmplength];

	int key = 0;
	for (int i = 0; i<length ; ++i) {
		tmp[i] = ~original[i];
		key += original[i];
	}
	//末尾にファイルのサイズと鍵を保存しておこう
	tmp[length+0] = (length /256/256/256)%256;
	tmp[length+1] = (length /256/256)%256;
	tmp[length+2] = (length /256)%256;
	tmp[length+3] = length %256;
	tmp[length+4] = key%256;

	GameLib::Array<char> res(tmplength*2);

	int rel;
	GameLib::Compressor::compress(&res,&rel,tmp,tmplength);

	//コピー
	for (int i=0 ; i<res.size(); ++i) {
		result[i] = res[i];
	}
	delete []tmp;
	tmp = 0;

	return res.size();

/*

	//暗号化？するぞ
	char* tmp= new char[length+1];

	//いったん差分にしてやる
	char c=0;
	for (int i=0 ; i<length; ++i) {
		tmp[i] = original[i] - c;
		c=original[i];
	}

	//圧縮してやる
	int p=0;
	int q=0;
	while (true) {
		int l;
		for ( l=0 ; (tmp[q]==tmp[q+l]) && ( (q+l)<length ) && (l<255); ++l) {
			;
		}
		result[p]=l;
		result[p+1]=tmp[q];

		p+=2;
		q+=l;

		if (q == length) {
			break;
		}
	}

	//鍵をかけよう
	char k=0;
	for (int i=0 ; i<p ; ++i) {
		k+=result[i] * i;
	}
	result[p]=k;
	++p;

	delete []tmp;

	return p;
*/
}
int FileSystem::decode(char*result,const char* original,int length) {
	const int outmax = 65536*8;
	char* outdata = new char[outmax];	//小さすぎると死ぬ
	int outsize;
	GameLib::Compressor::decompress(outdata,&outsize,original,length);

	STRONG_ASSERT(outsize < outmax);//ちゃんと展開できた？
	
	//本体コピー
	int key = 0;
	for (int i=0 ; i <outsize-5 ; ++i) {
		result[i] = ~outdata[i];
		key += result[i];
	}

	//鍵とサイズチェック
	int s = 0;
	for (int i=0 ; i<4 ; ++i) {
		s *= 256;
		unsigned char c = outdata[outsize -5 +i];
		s += c;
	}
	unsigned char k = outdata[outsize-1];

	delete []outdata;

	if ((s == outsize-5) &&
		(key %256 == k)) {
			return outsize-5;
	}
	else {
		return 0;
	}

/*

	int r=0;
	//鍵をチェック
	char k=0;
	for (int i=0; i<length-1 ; ++i) {
		k+=original[i] * i;
	}
	if (k != original[length-1] ) {
		r=0;		//鍵があわない！
	}
	else {
		//連長圧縮を戻す
		int p=0;
		for (int i=0 ; i<(length-1)/2  ; ++i) {
			unsigned char c=original[i*2];
			for (int j=0 ; j< c  ;  ++j) {
				result[p]=original[i*2+1];
				++p;
			}
		}
		r=p;

		//差分になっているのを戻す
		char c=0;
		for (int i=0 ; i<r  ; ++i) {
			char n=result[i]+c;
			c=n;
			result[i]=n;
		}
	}

	return r;
*/
}
void FileSystem::setData(char* data,unsigned int i,int index,int length,bool little) {
	for (int w=0 ; w<length ; ++w) {
		char j=0;
		if (little) {
			j=i / ( 1 << (8*w) ) % 256;
		}
		else {
			j=i / ( 1 << (8*(length - w -1))) % 256;
		}

		data[index + w] = j;
	};
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FMF::FMF() {

};

FMF::~FMF() {
	mFile.release();
}

void FMF::load(const char* filename,const unsigned int layer) {
	MyString path(256);
	path<<  filename <<".fmf";
	mFile.load(path.data());

	//識別子チェック  FMF_
	ASSERT(mFile.data(0)=='F');
	ASSERT(mFile.data(1)=='M');
	ASSERT(mFile.data(2)=='F');
	ASSERT(mFile.data(3)=='_');

	mWidth=mFile.data(8,4,true);
	mHeight=mFile.data(12,4,true);
	mChipWidth=mFile.data(16,1,true);	
	mChipHeight=mFile.data(17,1,true);

	ASSERT(mFile.data(18,1,true) == layer);		//
	mLayer = layer;

	mBitCount=mFile.data(19,1,true);
};

unsigned int FMF::data(unsigned int layer, unsigned int x, unsigned int y) {
	ASSERT(layer <= mLayer);

	unsigned int a = mFile.data((mHeight*mWidth*layer + y* mWidth + x)*(mBitCount/8)+20,mBitCount/8,true);

	if (mBitCount==8) {
		if (a!=255) {
			a = (a/16)*256 + (a%16);
		}
	}

	return a;
};
bool FMF::seek(int* x,int* y,unsigned int layer,unsigned int i,unsigned int j) {
	bool result =false;

	for (int p=0 ; p< mWidth ; ++p) {
		for (int q=0 ; q<mHeight ; ++q) {
			unsigned int a=data(layer,p,q);
			if (i == a%256 &&
				j == a/256) {

				*x=p;
				*y=q;
				result =true;
				break;
			}
		}
	}
	return result;
};

//////////////////////////////////////////////////////////////////////////////////////////////

void loadFile( MyString* out, const char* filename ) {
	ifstream in( filename, ifstream::binary );
	in.seekg( 0, ifstream::end );
	streamsize size = in.tellg();
	in.seekg( 0, ifstream::beg );
	char* s = new char[ size + 1 ];
	in.read( s, size );
	s[ size ] = '\0';

	*out = s;
	delete []s;
}


//ファイルがあるか確認（アーカイブは知らん）　実際にあけて見るという・・
bool fileExist(const char *filename){

	FILE  *fp;
	fp = fopen(filename, "r");

	if (fp) {
	    fclose(fp);
	    return true;
	}

	return false;
}


