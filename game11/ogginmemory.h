// OggDecoderInMemory.h
//

#ifndef IKD_DIX_OGGDECODERINMEMORY_H
#define IKD_DIX_OGGDECODERINMEMORY_H

#include "vorbis/vorbisfile.h"
#include "npfile.h"

class OggDecoderInMemory {
	public:
		OggVorbis_File ovf_;	//!< Ogg Vorbisファイル情報
		bool mOVFisReady;

		OggDecoderInMemory();
		OggDecoderInMemory( const char* filePath );
		virtual ~OggDecoderInMemory();

		//! クリア
		virtual void clear();

		//! サウンドをセット
		virtual void setSound( const char* fileName );
		virtual void setSound(nFile& );

		int mCopyPos;
		int copy(nFile&);

	protected:
		//! ストリームのバッファからオブジェクトポインタを取得
		static OggDecoderInMemory* getMyPtr( void* stream );

		//! メモリ読み込み
		static size_t read( void* buffer, size_t size, size_t maxCount, void* stream );

		//! メモリシーク
		static int seek( void* buffer, ogg_int64_t offset, int flag );

		//! メモリクローズ
		static int close( void* buffer );

		//! メモリ位置通達
		static long tell( void* buffer );

	protected:
		char*	buffer_;	// Oggファイルバッファ
		int		size_;		// バッファサイズ
		long	curPos_;	// 現在の位置


};

#endif