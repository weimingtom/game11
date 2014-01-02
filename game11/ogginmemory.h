// OggDecoderInMemory.h
//

#ifndef IKD_DIX_OGGDECODERINMEMORY_H
#define IKD_DIX_OGGDECODERINMEMORY_H

#include "vorbis/vorbisfile.h"
#include "npfile.h"

class OggDecoderInMemory {
	public:
		OggVorbis_File ovf_;	//!< Ogg Vorbis�t�@�C�����
		bool mOVFisReady;

		OggDecoderInMemory();
		OggDecoderInMemory( const char* filePath );
		virtual ~OggDecoderInMemory();

		//! �N���A
		virtual void clear();

		//! �T�E���h���Z�b�g
		virtual void setSound( const char* fileName );
		virtual void setSound(nFile& );

		int mCopyPos;
		int copy(nFile&);

	protected:
		//! �X�g���[���̃o�b�t�@����I�u�W�F�N�g�|�C���^���擾
		static OggDecoderInMemory* getMyPtr( void* stream );

		//! �������ǂݍ���
		static size_t read( void* buffer, size_t size, size_t maxCount, void* stream );

		//! �������V�[�N
		static int seek( void* buffer, ogg_int64_t offset, int flag );

		//! �������N���[�Y
		static int close( void* buffer );

		//! �������ʒu�ʒB
		static long tell( void* buffer );

	protected:
		char*	buffer_;	// Ogg�t�@�C���o�b�t�@
		int		size_;		// �o�b�t�@�T�C�Y
		long	curPos_;	// ���݂̈ʒu


};

#endif