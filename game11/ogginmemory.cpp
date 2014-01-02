// OggDecoderInMemory.cpp
//

//#pragma warning ( disable : 4267 )
//#pragma warning ( disable : 4244 )
//#pragma warning ( disable : 4996 )

#include "ogginmemory.h"
#include "npfile.h"
#include <stdio.h>
#include <string.h>

	// �R���X�g���N�^
	OggDecoderInMemory::OggDecoderInMemory() :
		buffer_	( 0 ),
		size_	( 0 ),
		curPos_	( 0 )
	{
		mOVFisReady=false;
	}

	// �R���X�g���N�^
	OggDecoderInMemory::OggDecoderInMemory( const char* filePath ) :
		buffer_		( 0 ),
		size_		( 0 ),
		curPos_		( 0 )
	{
		setSound( filePath );
		mOVFisReady=false;
	}

	// �f�X�g���N�^
	OggDecoderInMemory::~OggDecoderInMemory() {
		clear();
	}

	//! �N���A
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

	//! �X�g���[���̃o�b�t�@����I�u�W�F�N�g�|�C���^���擾
	OggDecoderInMemory* OggDecoderInMemory::getMyPtr( void* stream ) {
		OggDecoderInMemory *p = 0;
		memcpy( &p, stream, sizeof( OggDecoderInMemory* ) );
		return p;
	}

	//! �������ǂݍ���
	size_t OggDecoderInMemory::read( void* buffer, size_t size, size_t maxCount, void* stream ) {
		if ( buffer == 0 ) {
			return 0;
		}

		// �X�g���[������I�u�W�F�N�g�̃|�C���^���擾
		OggDecoderInMemory *p = getMyPtr( stream );

		// �擾�\�J�E���g�����Z�o
		int resSize = p->size_ - p->curPos_;
		size_t count = resSize / size;
		if ( count > maxCount ) {
			count = maxCount;
		}

		memcpy( buffer, (char*)stream + sizeof( OggDecoderInMemory* ) + p->curPos_, size * count );

		// �|�C���^�ʒu���ړ�
		p->curPos_ += size * count;

		return count;
	};

	//! �������V�[�N
	int OggDecoderInMemory::seek( void* buffer, ogg_int64_t offset, int flag ) {

		// �X�g���[������I�u�W�F�N�g�̃|�C���^���擾
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

	//! �������N���[�Y
	int OggDecoderInMemory::close( void* buffer ) {
		// �f�X�g���N�^��N���A����ɏ������Ă���Ă���̂�
		// �����͉������Ȃ�
		return 0;
	}

	//! �������ʒu�ʒB
	long OggDecoderInMemory::tell( void* buffer ) {
		return getMyPtr( buffer )->curPos_;
	}

	//! �T�E���h���Z�b�g
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
	int OggDecoderInMemory::copy(nFile& file) {	//�R�s�[�������߂�B 1�Ŋ����@2�ŃG���[�B

		bool finished=false;
		for (int i=0 ; i<(64*1024) ; ++i) {		//��t���[���ɉ��o�C�g�R�s�[���邩�H
			*(buffer_ + sizeof( OggDecoderInMemory* ) + mCopyPos) = file.data(mCopyPos);

			++mCopyPos;
			if ( mCopyPos >= size_ ) {
				finished =true;
				break;
			}
		};

		//�R�s�[����
		if (finished) {
			// �R�[���o�b�N�o�^
			ov_callbacks callbacks = {
				&OggDecoderInMemory::read,
				&OggDecoderInMemory::seek,
				&OggDecoderInMemory::close,
				&OggDecoderInMemory::tell
			};

			// Ogg�I�[�v��
			if ( ov_open_callbacks( buffer_, &ovf_ , 0, 0, callbacks ) != 0 ) {
				clear();
				return 2;		//�G���[�B
			}
			return 1;
			mOVFisReady=true;
		}
		return 0;
	}

