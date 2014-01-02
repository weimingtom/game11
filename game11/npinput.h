#ifndef INCLUDED_NPINPUT_H
#define INCLUDED_NPINPUT_H

//キー入力系

class cKey {
private:
	static int mFrame[27];		//押されているフレーム数
	static bool mOn[27];		//おされているかどうか
	//static int mReplay;			//0なにもなし　1録画　2再生
	static int mCount;
	static bool mJoyStick;		//起動時にスティックをチェックしよう
	static int mJoyStickNumber;
public:
	static int mAssign[12];

	cKey();
	static void update();		//毎フレーム呼び出そう
	static void reset();		//キー配置をリセット
	static bool isOn(int);
	static int frame(int);		//押されているフレーム数
	static bool get(int,int pulse=4);		//カーソル操作用
	static bool trigger(int);	//押したフレームのみ得る

	static void replay();		//リプレイ開始
	static void record();		//保存開始
	static void stop();			//リプレイ・保存ストップ

	static void assigning();	//割り当て中。4～9のキー入力を忘れる　倍速・スクショ機能をオフにできるぞ
	static int getPadOne();		//割り当て用。パッドで押されているキーをひとつ得る
	static int joyStickNumber();
};



#endif

