#ifndef INCLUDED_GAMEDATA_H
#define INCLUDED_GAMEDATA_H

#include "interface.h"
class Stage;
class Ending;

class GameData{
	Ending* mEnding;
public:
	int mID;

	//�t�F�[�Y
	Stage* mStage;				//�X�N���v�g����̑�����󂯕t���邽��

	GameData();
	~GameData();
	void release();
	int update();
	void reset(int mode);			//0�v���[���@1�ŏ�����@2�f�o�b�O
	void draw(int x,int y);		//�_�Epower���\�� Flag�������̂�const�ɂł��Ȃ�
	void drawHistory(int x,int y);//�v���C����\��
};

#endif

