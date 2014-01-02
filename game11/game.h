#ifndef INCLUDED_GAME9SEQ_GAME_H
#define INCLUDED_GAME9SEQ_GAME_H

#include "gamedata.h"
#include "title.h"

//�Q�[���̍������[������
//�^�C�g��
//�X�e�[�W�p�[�g

class Game {
public:
	GameData* mGameData;
	Title* mTitle;
	
	int mCount;
	bool mReset;

	int update();
	void reset();			//�^�C�g���ւ��ǂ�
	static Game* instance();
	static void create();
	static void destroy();
private:
	static Game* mInstance;
	Game();
	~Game();
	void release();

};




#endif
