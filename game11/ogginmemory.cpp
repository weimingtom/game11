// OggDecoderInMemory.cpp
//

//#pragma warning ( disable : 4267 )
//#pragma warning ( disable : 4244 )
//#pragma warning ( disable : 4996 )

#include "ogginmemory.h"
#include "npfile.h"
#include <stdio.h>
#include <string.h>

	// コンストラクタ
	OggDecoderInMemory::OggDecoderInMemory() :
		buffer_	( 0 ),
		size_	( 0 ),
		curPos_	( 0 )
	{
		mOVFisReady=false;
	}

	// コンストラクタ
	OggDecoderInMemory::OggDecoderInMemory( const char* filePath ) :
		buffer_		( 0 ),
		size_		( 0 ),
		curPos_		( 0 )
	{
		setSound( filePath );
		mOVFisReady=false;
	}

	// デストラクタ
	OggDecoderInMemory::~OggDecoderInMemory() {
		clear();
	}

	//! クリア
	void OggDecoderInMemory::clear() {
		if( buffer_ ) {
			delete[] buffer_;
			buffer_ = 0;
			size_ = 0;
			curPos_ = 0;
	
			if (mOVFisReady) {
				ov_clear(&ovf_);
				mOVFisReady=false;
			}

		}
	}

	//! ストリームのバッファからオブジェクトポインタを取得
	OggDecoderInMemory* OggDecoderInMemory::getMyPtr( void* stream ) {
		OggDecoderInMemory *p = 0;
		memcpy( &p, stream, sizeof( OggDecoderInMemory* ) );
		return p;
	}

	//! メモリ読み込み
	size_t OggDecoderInMemory::read( void* buffer, size_t size, size_t maxCount, void* stream ) {
		if ( buffer == 0 ) {
			return 0;
		}

		// ストリームからオブジェクトのポインタを取得
		OggDecoderInMemory *p = getMyPtr( stream );

		// 取得可能カウント数を算出
		int resSize = p->size_ - p->curPos_;
		size_t count = resSize / size;
		if ( count > maxCount ) {
			count = maxCount;
		}

		memcpy( buffer, (char*)stream + sizeof( OggDecoderInMemory* ) + p->curPos_, size * count );

		// ポインタ位置を移動
		p->curPos_ += size * count;

		return count;
	};

	//! メモリシーク
	int OggDecoderInMemory::seek( void* buffer, ogg_int64_t offset, int flag ) {

		// ストリームからオブジェクトのポインタを取得
		OggDecoderInMemory *p = getMyPtr( buffer );

		switch( flag ) {
		case SEEK_CUR:
			p->curPos_ += offset;
			break;

		case SEEK_END:
			p->curPos_ = p->size_ + offset;
			break;

		case SEEK_SET:
			p->curPos_ = offset;
			break;

		default:
			return -1;
		}

		if ( p->curPos_ > p->size_ ) {
			p->curPos_ = p->size_;
			return -1;
		} else if ( p->curPos_ < 0 ) {
			p->curPos_ = 0;
			return -1;
		}

		return 0;
	}

	//! メモリクローズ
	int OggDecoderInMemory::close( void* buffer ) {
		// デストラクタやクリアが先に処理してくれているので
		// ここは何もしない
		return 0;
	}

	//! メモリ位置通達
	long OggDecoderInMemory::tell( void* buffer ) {
		return getMyPtr( buffer )->curPos_;
	}

	//! サウンドをセット
	void OggDecoderInMemory::setSound( const char* fileName ) {
		clear();

		nFile f;
		f.load(fileName,true);
	}

	void OggDecoderInMemory::setSound(nFile& file) {
		clear();

		size_=file.size();

		buffer_ = new char[ size_ + sizeof( OggDecoderInMemory* ) ];
		OggDecoderInMemory* p = this;

		memcpy( buffer_, &p, sizeof( OggDecoderInMemory* ) );
		mCopyPos=0;
	}
	int OggDecoderInMemory::copy(nFile& file) {	//コピーをすすめる。 1で完了　2でエラー。

		bool finished=false;
		for (int i=0 ; i<(64*1024) ; ++i) {		//一フレームに何バイトコピーするか？
			*(buffer_ + sizeof( OggDecoderInMemory* ) + mCopyPos) = file.data(mCopyPos);

			++mCopyPos;
			if ( mCopyPos >= size_ ) {
				finished =true;
				break;
			}
		};

		//コピー完了
		if (finished) {
			// コールバック登録
			ov_callbacks callbacks = {
				&OggDecoderInMemory::read,
				&OggDecoderInMemory::seek,
				&OggDecoderInMemory::close,
				&OggDecoderInMemory::tell
			};

			// Oggオープン
			if ( ov_open_callbacks( buffer_, &ovf_ , 0, 0, callbacks ) != 0 ) {
				clear();
				return 2;		//エラー。
			}
			return 1;
			mOVFisReady=true;
		}
		return 0;
	}

