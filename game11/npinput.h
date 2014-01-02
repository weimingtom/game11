#ifndef INCLUDED_NPINPUT_H
#define INCLUDED_NPINPUT_H

//�L�[���͌n

class cKey {
private:
	static int mFrame[27];		//������Ă���t���[����
	static bool mOn[27];		//������Ă��邩�ǂ���
	//static int mReplay;			//0�Ȃɂ��Ȃ��@1�^��@2�Đ�
	static int mCount;
	static bool mJoyStick;		//�N�����ɃX�e�B�b�N���`�F�b�N���悤
	static int mJoyStickNumber;
public:
	static int mAssign[12];

	cKey();
	static void update();		//���t���[���Ăяo����
	static void reset();		//�L�[�z�u�����Z�b�g
	static bool isOn(int);
	static int frame(int);		//������Ă���t���[����
	static bool get(int,int pulse=4);		//�J�[�\������p
	static bool trigger(int);	//�������t���[���̂ݓ���

	static void replay();		//���v���C�J�n
	static void record();		//�ۑ��J�n
	static void stop();			//���v���C�E�ۑ��X�g�b�v

	static void assigning();	//���蓖�Ē��B4�`9�̃L�[���͂�Y���@�{���E�X�N�V���@�\���I�t�ɂł��邼
	static int getPadOne();		//���蓖�ėp�B�p�b�h�ŉ�����Ă���L�[���ЂƂ���
	static int joyStickNumber();
};



#endif

