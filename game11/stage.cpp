
#include "npinput.h"
#include "stage.h"
#include "modellist.h"
#include "interface.h"
#include "game.h"

Stage::Stage(GameData* gamedata) :
	mSprite(512)
{
	mPanel.load("src/picture/panel",true);
	mRadar.load("src/picture/レーダー",true);
	mSilhouette.load("src/picture/機影",true);
	mPichun.load("src/picture/ぴちゅん",true);
	mMasterSpark.load(0,true,false,"src/picture/マスタースパーク");
	mBomber.load(0,true,false,"src/picture/ボンバー");
	mLaser.load("src/picture/レーザー",true);

	mAAG.load("src/picture/対空砲",true);
	mSnipeShot.load("src/picture/自機狙い",true);
	m2WayShot.load("src/picture/2way",true);
	mTensokuShot.load("src/picture/自機狙い",true,false,"src/picture/天則弾");
	mCutIn.load("src/picture/非想天則",true,false,"src/picture/スペル発動");
	mCutInNue.load("src/picture/ぬえカットイン",true,false);
	mSound.load("src/sound/sound.txt");

	mSprite.resizeArea(-400,-600,13,20,64,64);
	mGameData = gamedata;
	mScore = 0;
	mLeft = 2;

	mPhase = 0;
	mAppearCount = 0;
	mCastSpell = 0;

	mArea = 0;
	mScroll.set(0.f,0.f,0.f);
	mScrollOrder.set(0.f,0.f,0.f);

	BGM::instance()->change(0);
	ready();
}

Stage::~Stage() {
	release();
};

void Stage::release() {
	if (mPhase) {
		delete mPhase;
		mPhase = 0;
	}
	if (mCastSpell) {
		delete mCastSpell;
		mCastSpell = 0;
	}

	for (int i=0 ; i<mChara.number() ; ++i) {
		delete *mChara(i);
	}
	mChara.release();
	mSprite.removeObject();
}

void Stage::ready() {
	release();
	setCamera();

	GameLib::Math::Vector3 pos(0.f);
	createPlayer(pos);
	mShotCount = 0;
	mRestShot = 0;

	mMiss = false;
	mFlash = false;
	mFlashOrder = false;
	mFlashEffect = 0;
	mPlayer.initialize();

	mEnemyCount = 0;
	mPhase = new Ready;

	mReimuCount = 0;
	mMarisaCount = 0;
	mSanaeCount = 0;
	mAttackCount = 0;

	mMissCount = 0;
	mGameClear = false;
}


int Stage::update(GameData* gamedata) {
	Draw::instance()->mCamera.update();
	mScroll *= 0.95f;
	mScroll.madd(mScrollOrder,0.05f); 

	if (mPhase) {
		if (mPhase->update(this)) {
			delete mPhase;
			mPhase = 0;
		}
	}
	else {
		//敵出現
		enemyAppear();
		mRestShot = mPlayer.mShot - mShotCount;
		mShotCount = 0;
		//動かす処理ここで
		for (List<Chara*>::Iterator i(mChara) ; !i.end() ; i++) {
			(*i())->update(this);
		}
		mSprite.updateObject();

		//当たり判定チェックここで
		mSprite.updateObjectCollision();

		//死にチェック
		for (List<Chara*>::Iterator i(mChara) ; !i.end() ; i++) {
			(*i())->collision(this);
		}

		//消滅処理ここで
		for (int i=0 ; i<mChara.number() ; ) {
			if ((*mChara(i))->lost()) {
				delete *mChara(i);
				mChara.remove(i);
			}
			else {
				++i;
			}
		}

		draw();

		if (mPlayer.mSpecial) {
			--mPlayer.mSpecialLimit;
			if (mPlayer.mSpecialLimit < 0) {
				mPlayer.mSpecial = 0;
			}
		}
		mFlash = false;
		if (mFlashOrder) {
			mFlashOrder = false;
			mFlash = true;
			mFlashEffect = 16;
		}
		//ミスしてる？
		if (mMiss) {
			mMissCount = 128;
			mMiss = false;
		}
		if (mMissCount > 0) {

			--mMissCount;
			if (mMissCount == 0) {
				if (mLeft>0) {
					--mLeft;
					release();
					ready();
				}
				else {
					mPhase = new GameOver;
				}
			}
		}
	}
	if (mAppearCount > 60) {
		stageClear();
	}
	if (mCastSpell) {
		if (mCastSpell->update(this)) {
			delete mCastSpell;
			mCastSpell = 0;
		}
	}

	if (mGameClear) {
		return 1;
	}
	else {
		return 0;
	};
}
void Stage::draw() {
	//背景
	BlendInfo bi;
	ModelList::instance()->mBack.model()->draw(mScroll,bi);

	mSprite.drawObject();

	//フラッシュ
	if (mFlashEffect > 0) {
		if (mFlashEffect%3 == 0) {
			unsigned int color = 0xffffffff;
			Rect::draw(-400,300,800,600,color);
		}
		--mFlashEffect;
	}

	//コンパネ
	mPanel.model()->draw(0.f,0.f,0.f,bi);
	if (mPlayer.mSpecial) {
		float p = 1.f * mPlayer.mSpecialLimit / Stage::mSpecialLimit;
		Gauge::draw(p,0xffffff00,0x00000000,-400,280,800,16);
	}
	for (int i=0 ; i<mRestShot ; ++i) {
		float x = 32.f * (mPlayer.mShot-1) / 2.f;
		ModelList::instance()->mChara.mTexture.model("残弾")->draw(i*32 - x,280.f,0.f,bi);
	}

	//レーダー
	mRadar.model()->draw(0.f,-300.f +48.f,0.f,bi);
	for (List<Chara*>::Iterator i(mChara) ; !i.end() ; i++) {
		(*i())->drawRadar(this);
	}

	mTelop.update();

	Draw::instance()->text(-320,-300+48+24,"Score",0xffff0000,2);
	Draw::instance()->text(-320,-300+48   ,mScore,0xffffffff,2);
	Draw::instance()->text( 240,-300+48+24,"Left",0xffff0000,2);
	Draw::instance()->text( 240,-300+48   ,mLeft,0xffffffff,2);
};
void Stage::drawRadar(const GameLib::Math::Vector3& pos,int w,int h,unsigned int color) {
	//800x600を128*96に直す
	GameLib::Math::Vector3 p = pos;
	p *= 128.f / 800.f;

	if (p.y < 48.f &&
		p.y > -48.f &&
		p.x < 192.f &&
		p.x > -192.f) {

		p.y -= 300.f - 48.f;

		BlendInfo bi;
		bi.mScale.set(w,h,1.f);
		bi.mBlendElement.setColor(color);

		mSilhouette.model()->draw(p,bi);
	}
};

void Stage::enemyAppear() {
	++mEnemyCount;
	if (mEnemyCount > (60.f/mPlayer.difficulty())) {
		mEnemyCount = 0;

		bool reverse = false;
		GameLib::Math::Vector3 pos(0.f);
		pos.x = Random::instance()->getFloat(640.f) - 320.f;
		pos.y = Random::instance()->getFloat( 300.f );

		reverse =Random::instance()->getBool();

		Enemy* e = 0;
		if (mArea == 0) {
			pos.y -= 400.f;

			int r = Random::instance()->getInt(5);
			if (r == 0) {
				e = createReimu(pos, reverse);
			}
			if (r == 1) {
				e = createMarisa(pos, reverse);
			}
			if (r == 2) {
				e = createSanae(pos, reverse);
			}
			if (r == 3) {
				e = createYousei(pos, reverse);
			}
			if (r == 4) {
				e = createUFO(pos, reverse);
			}
		}
		if (mArea == 1) {
			pos.y += 128.f;
			int r = Random::instance()->getInt(6);
			if (r == 0) {
				e = createTenko(pos, reverse);
			}
			if (r == 1) {
				e = createIku(pos, reverse);
			}
			if (r == 2) {
				e = createYorihime(pos, reverse);
			}
			if (r == 3) {
				e = createToyohime(pos, reverse);
			}
			if (r == 4) {
				e = createYousei(pos, reverse);
			}
			if (r == 5) {
				e = createUFO(pos, reverse);
			}

		}
		if (e) {
			e->object()->setAnimeSpeed(mPlayer.difficulty());
		}

	}
};

Player* Stage::createPlayer(const GameLib::Math::Vector3 &pos) {
	Player* player = new Player(this);;
	mChara.addInitializing(player);
	player->object()->setPosition(0.f,0.f,0.f);
	return player;
}
void Stage::createShot(const GameLib::Math::Vector3& pos) {
	Shot* shot = new Shot(this,pos,false);
	mChara.addInitializing(shot);
	mSound.play("攻撃");
}
void Stage::createFourShot(const GameLib::Math::Vector3& pos) {
	for (int i=0 ; i<4 ; ++i) {
		GameLib::Math::Vector3 p = pos;
		p.x += i*32 - 48;

		Shot* shot = new Shot(this,p,false);
		mChara.addInitializing(shot);
	}
	mSound.play("攻撃");
}

void Stage::createMasterSpark(const GameLib::Math::Vector3 &pos) {
	Object* obj = mSprite.addObject(mMasterSpark.model());
	
	obj->setPosition(pos);
	obj->mAutoRemove = 1;

	obj->mCollision.mFlag = 4;
	mSound.play("攻撃");
}
void Stage::createBomber(const GameLib::Math::Vector3 &pos) {
	Object* obj = mSprite.addObject(mBomber.model());
	
	obj->setPosition(pos);
	obj->mAutoRemove = 1;

	obj->mCollision.mFlag = 4+16;
	mSound.play("ボンバー");
}
void Stage::createBomb(const GameLib::Math::Vector3& pos) {
	Shot* shot = new Shot(this,pos,true);
	mChara.addInitializing(shot);
	mSound.play("攻撃");
}
void Stage::createAAG(const GameLib::Math::Vector3 &pos) {
	int n=1;
	if (mPlayer.mWave) {
		n=3;
	}
	for (int i=0 ; i<n ; ++i) {
		Shot* shot = new Shot(this,pos,false,mAAG.model());
		shot->object()->setAnimeSpeed(mPlayer.shotSpeed() );
		mChara.addInitializing(shot);

		if (i==1) {
			shot->object()->setRotate(0.f,0.f,15.f);
		}
		if (i==2) {
			shot->object()->setRotate(0.f,0.f,-15.f);
		}
	}
	mSound.play("攻撃");
}
void Stage::createCanon(const GameLib::Math::Vector3 &pos) {
	int n=1;
	if (mPlayer.mWave) {
		n=3;
	}
	for (int i=0 ; i<n ; ++i) {
		Shot* shot = new Shot(this,pos,false,mAAG.model());
		mChara.addInitializing(shot);

		shot->object()->setAnimeSpeed(mPlayer.shotSpeed() );
		shot->object()->setRotate(0.f,0.f,-90.f);
		if (i==1) {
			shot->object()->setRotate(0.f,0.f,-90.f + 15.f);
		}
		if (i==2) {
			shot->object()->setRotate(0.f,0.f,-90.f -15.f);
		}
	}
	mSound.play("攻撃");
}

Enemy* Stage::createReimu(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Reimu(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createMarisa(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Marisa(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createSanae(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Sanae(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createYousei(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Yousei(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createUFO(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new UFO(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createTenko(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Tenko(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createIku(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Iku(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createToyohime(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Toyohime(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createYorihime(const GameLib::Math::Vector3 &pos, bool reverse) {
	Enemy* enemy = new Yorihime(this);
	createEnemy(enemy,pos,reverse);

	return enemy;
};
Enemy* Stage::createHisoutensoku() {
	Enemy* enemy = new Hisoutensoku(this);

	createEnemy(enemy,enemy->position(),false);

	return enemy;
};


void Stage::createEnemy(Enemy* enemy,const GameLib::Math::Vector3 &pos, bool reverse) {
	enemy->object()->setPosition(pos);
	enemy->object()->setRotate(0.f,180.f * reverse,0.f);
	mChara.addInitializing(enemy);
}

void Stage::createEnemyShot(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,0);
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemyLaser(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,mLaser.model());
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemyOnbasira(const GameLib::Math::Vector3& pos) {
	Onbasira* shot = new Onbasira(this,pos);
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemyKanameisi(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,ModelList::instance()->mChara.mTexture.model("要石"));
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemyFan(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,ModelList::instance()->mChara.mTexture.model("扇子"));
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemySnipe(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,mSnipeShot.model());
	shot->object()->go(6.f,0.f,mPlayerPosition);
	mChara.addInitializing(shot);
	mSound.play("敵弾");
}
void Stage::createEnemy2way(const GameLib::Math::Vector3& pos) {
	GameLib::Math::Vector3 ang = mPlayerPosition;
	ang -= pos;

	float r;
	Math::vector3ToAngle(&r,0,ang);

	for (int i=0 ; i < 2 ;++i) {
		EnemyShot* shot = new EnemyShot(this,pos,m2WayShot.model());
		shot->object()->setRotate(0.f,0.f,r + (30.f*i) - 15.f);
		shot->object()->mAutoRemove = 0;
		shot->object()->setLimit(300);
		mChara.addInitializing(shot);
	}
	mSound.play("敵弾");
}
void Stage::createEnemyTensokuA(const GameLib::Math::Vector3& pos) {
	EnemyShot* shot = new EnemyShot(this,pos,mTensokuShot.model("A"));
	shot->object()->randomRotate(false,false,true);
	mChara.addInitializing(shot);
	mSound.play("連続弾");
}
void Stage::createEnemyTensokuB(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& target) {
	float r;
	GameLib::Math::Vector3 ang = target;
	ang -= pos;
	Math::vector3ToAngle(&r,0,ang);

	//for (int i=0 ; i<16 ;++i ) {
	//	GameLib::Math::Vector3 p = pos;
	//	p.x += (i-8) * 4;

	//	EnemyShot* shot = new EnemyShot(this,p,mTensokuShot.model("B"));
	//	shot->object()->setRotate(0.f,0.f,r);
	//	mChara.addInitializing(shot);
	//}
	for (int i=0 ; i<8 ; ++i) {
		EnemyShot* shot = new EnemyShot(this,pos,mTensokuShot.model("ブレストファイヤー"));
		shot->object()->go(6.f + 0.10f*i,0.f,target);
		mChara.addInitializing(shot);
	}

//	for (int i=1 ; i<3 ; ++i) {
//		for (int j = 0 ; j<2 ; ++j) {
//			int s = (j*2)-1;
//			EnemyShot* shot = new EnemyShot(this,pos,mTensokuShot.model("B"));
//			shot->object()->setRotate(0.f,0.f,r-15.f*i*s);
//			mChara.addInitializing(shot);
//			shot->object()->setAnimeSpeed(0.5f);
//		}
//	}


	mSound.play("連続弾");
}
void Stage::createEnemyTensokuC(const GameLib::Math::Vector3& pos,const GameLib::Math::Vector3& target) {
	float r;
	GameLib::Math::Vector3 ang = target;
	ang -= pos;
	Math::vector3ToAngle(&r,0,ang);

	EnemyShot* shot = new EnemyShot(this,pos,mTensokuShot.model("C"));
	shot->object()->setRotate(0.f,0.f,r);
	mChara.addInitializing(shot);

	{
		EnemyShot* shot = new EnemyShot(this,pos,mTensokuShot.model("ストリング"));
		shot->object()->setRotate(0.f,0.f,r);
		mChara.addInitializing(shot);
	}

	mSound.play("敵弾");

}

void Stage::createItemOneUp(const GameLib::Math::Vector3& pos) {
	Item* item = new OneUp(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemShot(const GameLib::Math::Vector3& pos) {
	Item* item = new ShotNumber(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemSpeed(const GameLib::Math::Vector3& pos) {
	Item* item = new SpeedUp(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemWave(const GameLib::Math::Vector3& pos) {
	Item* item = new Wave(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemShotSpeed(const GameLib::Math::Vector3& pos) {
	Item* item = new ShotSpeed(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemFullPower(const GameLib::Math::Vector3& pos) {
	Item* item = new FullPower(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemFlash(const GameLib::Math::Vector3& pos) {
	Item* item = new Flash(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemFourShot(const GameLib::Math::Vector3& pos) {
	Item* item = new FourShot(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemLaser(const GameLib::Math::Vector3& pos) {
	Item* item = new Laser(this,pos);
	mChara.addInitializing(item);
};
void Stage::createItemBomb(const GameLib::Math::Vector3& pos) {
	Item* item = new Bomb(this,pos);
	mChara.addInitializing(item);
};

void Stage::createExplode(const GameLib::Math::Vector3 &pos) {
	Object* obj = mSprite.addObject(ModelList::instance()->mChara.mTexture.model("爆発"));
	obj->setPosition(pos);
	obj->mAutoRemove = 1;
}
void Stage::createPichun(const GameLib::Math::Vector3 &pos) {
	Object* obj = mSprite.addObject(mPichun.model());
	obj->setPosition(pos);
	obj->randomRotate();
	obj->mAutoRemove = 1;
}
void Stage::miss() {

	mMiss = true;
};
void Stage::stageClear() {
	if (mPhase == 0) {
		mPhase = new StageClear;
	}
}
void Stage::gameClear() {
	mGameClear=true;
};

void Stage::destroy(Object* obj) {
	ASSERT(mPhase == 0);
	mPhase = new Destroy(obj);


}
void Stage::castSpell(const char *name) {
	if (mCastSpell) {
		delete mCastSpell;
		mCastSpell = 0;
	}
	mCastSpell = new CastSpell(mCutIn.model(),name,-120);
	flushBullet();

};

void Stage::castNue() {
	if (mCastSpell) {
		delete mCastSpell;
		mCastSpell = 0;
	}
	mCastSpell = new CastSpell(mCutInNue.model(),0,0);
};

void Stage::flush() {
	mSprite.removeObject(2+4+8+16+32);
};
void Stage::flushBullet() {
	mSprite.removeObject(8+16);
};


void Stage::setCamera() {
	if (mArea == 0) {
		Draw::instance()->mCamera.aimTarget(0,-240,0);
		mScrollOrder.set(0.f,0.f,0.f);
	}
	if (mArea == 1) {
		Draw::instance()->mCamera.aimTarget(0,160,0);
		mScrollOrder.set(0.f,0.f,0.f);
	}
	if (mArea == 2) {
		Draw::instance()->mCamera.aimTarget(0,160,0);
		mScrollOrder.set(0.f,-160.f,0.f);
	}
}
void Stage::startStage() {
	if (mArea == 2) {
		createHisoutensoku();

		GameLib::Math::Vector3 pos(-800.f,200.f,0.f);
		createUFO(pos,false);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Stage::PlayerState::initialize() {
	mWave = false;
	mShot = 4;
	mShotSpeed = 0;
	mSpeed = 0;
	mSpecial = 0;
	mSpecialLimit = 0;
}

float Stage::PlayerState::difficulty() {
	float result = 20.f;
	result += 1.f * (mShot -4) / 3.f;
	result += mSpeed;
	result += mShotSpeed;

	result += mWave * 2;

	result /= 24.f;

	return result;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Stage::Telop::Telop() {
	mCount = 0;
	mModel = 0;
	mTexture.load("src/picture/テロップ",true,false);
};

void Stage::Telop::update() {
	if (mModel) {
		--mCount;
		if (mCount > 0) {
			BlendInfo bi;
			mModel->draw(0.f,50.f,0.f,bi);
		}
	}
};
void Stage::Telop::set( const char* name) {
	ASSERT(mTexture.model(name));
	mModel = mTexture.model(name);
	mCount = 60;

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Stage::Phase::~Phase() {
};

Stage::Ready::Ready() {
	mCount = 0;
}
int Stage::Ready::update(Stage* stage) {
	stage->draw();

	if (mCount == 0) {
		stage->mTelop.set("Ready");
	}

	++mCount;
	if (mCount > 60) {
		stage->startStage();
		return 1;
	}
	return 0;
};

Stage::GameOver::GameOver() {
	mCount = 0;
	mTexture.load("src/picture/ゲームオーバー",true);
	BGM::instance()->change(-1);
}

int Stage::GameOver::update(Stage* stage) {
	++mCount;
	BlendInfo bi;
	mTexture.model()->draw(0.f,0.f,0.f,bi);
	if (mCount > 120) {
		Game::instance()->mReset = true;
	}
	return 0;
};

void Stage::getScore(int s) {
	ASSERT(s >= 0);
	mScore += s;
};

Stage::StageClear::StageClear() {
	mCount = 0;
};
int Stage::StageClear::update(Stage* stage) {
	stage->draw();

	if (mCount == 0) {
		stage->mArea = (stage->mArea+1) %3;
		stage->mAppearCount = 0;
		stage->setCamera();

		stage->flush();
	}

	++mCount;
	if (mCount > 64) {
		stage->startStage();

		return 1;
	}
	return 0;
};

Stage::Destroy::Destroy(Object* obj) {
	mCount = 0;
	mObject = obj;

	obj->setAnimeSpeed(0.f);
}
int Stage::Destroy::update(Stage* stage) {
	stage->mSprite.updateObject();
	stage->draw();

	if (mCount < 180) {
		GameLib::Math::Vector3 pos;
		mObject->getWorldPositionFinal(&pos);
		GameLib::Math::Vector3 r;
		Random::instance()->getVector3(&r,96.f,96.f,0.f);
		pos += r;
		stage->createExplode(pos);
	}
	if (mCount == 120) {
		GameLib::Math::Vector3 pos;
		mObject->getWorldPositionFinal(&pos);
		stage->createBossExplode(pos);

	}

	if (mCount == 180) {
		stage->mSound.play("ボス爆発");
		mObject->remove();


	}

	++mCount;

	if (mCount > 240) {
		stage->gameClear();
		return 1;
	}
	return 0;
}
void Stage::createBossExplode(const GameLib::Math::Vector3& pos) {
	Object* ob = mSprite.addObject(mTensokuShot.model("爆発"));
	ob->setPosition(pos);
	ob->mAutoRemove = 1;

}
Stage::CastSpell::CastSpell(Model* model,const char* name,int time) {
	mName = name;
	mCount = time;
	mModel = model;
};
int Stage::CastSpell::update(Stage* stage) {
	if (mCount == 0) {
		stage->mSound.play("スペル");
	}
	if (mCount >= 0) {
		int t = mCount;

		int x = -(t-60)*(t-60)/10 +120;
		Draw::instance()->text(x,-120,mName.data(),0xffffffff,1);

		BlendInfo bi;
		mModel->draw(0.f,0.f,0.f,bi,t);
	}
	++mCount;

	if (mCount >= 60) {
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Chara::Chara() {
	mObject = 0;
	mSilhouetteColor = 0xffffffff;
	mSilhouetteWidth = 4;
	mSilhouetteHeight = 4;

};
Chara::~Chara() {
	mObject = 0;
};
Object* Chara::object() const{
	return mObject;
};
Object* Chara::setObject(Stage* stage,Model* model) {
	ASSERT(mObject == 0);
	mObject = stage->mSprite.addObject(model);
	mObjectID = mObject->id();
	return mObject;
}
void Chara::removeObject() {
	mObject = 0;
}
bool Chara::lost() const{
	if (mObject ==0) {
		return true;
	}
	if (!mObject->isEnable()) {
		return true;
	}
	if (mObject->id() != mObjectID) {
		return true;
	}
	return false;
};
void Chara::drawRadar(Stage* stage) {
	if (mSilhouetteColor) {
		GameLib::Math::Vector3 pos;
		Draw::instance()->getScreenCoordinate(&pos,position());

		stage->drawRadar(
			pos,
			mSilhouetteWidth,
			mSilhouetteHeight,
			mSilhouetteColor);
	}
}
GameLib::Math::Vector3 Chara::position() {
	GameLib::Math::Vector3 result;
	object()->getWorldPositionFinal(&result);
	return result;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Player::Player(Stage* stage) {
	setObject(stage,ModelList::instance()->mChara.mTexture.model("自機"));
	object()->mCollision.mFlag = 1;
	object()->mCollision.mFlagTarget = 2+8;

};
int Player::update(Stage* stage) {

	stage->mPlayerPosition = position();

	//移動
	GameLib::Math::Vector3 pos;
	pos = *object()->worldPosition();
	float v = 1.5f + (0.5f * stage->mPlayer.mSpeed);

	if (cKey::isOn(1)) {
		pos.x += v;
	}
	if (cKey::isOn(3)) {
		pos.x -= v;
	}

	if (pos.x < -360.f) {
		pos.x = -360.f;
	}
	if (pos.x > 360.f) {
		pos.x = 360.f;
	}
	if (stage->mArea == 2) {

		if (cKey::isOn(0)) {
			pos.y += v;
		}
		if (cKey::isOn(2)) {
			pos.y -= v;
		}

		if (pos.y < 0.f) {
			pos.y = 0.f;
		}
		if (pos.y > 480.f) {
			pos.y = 480.f;
		}
	}

	object()->setPosition(pos);

	if (stage->mArea == 0) {
		//ショット
		if (stage->mPlayer.mSpecial == 0) {
			if (stage->mRestShot > 0) {
				if (cKey::trigger(5)) {
					stage->createShot(*object()->worldPosition());
				}
			}
		}
		//四連
		if (stage->mPlayer.mSpecial == 1) {
			if (stage->mRestShot > 0) {
				if (cKey::trigger(5)) {
					stage->createFourShot(*object()->worldPosition());
				}
			}
		}
		//恋符「マスタースパーク」
		if (stage->mPlayer.mSpecial == 2) {
			if (cKey::frame(5)%4 == 1) {
				stage->createMasterSpark(position());
			}
		}
		//ボム
		if (stage->mPlayer.mSpecial == 3) {
			if (stage->mRestShot > 0) {
				if (cKey::trigger(5)) {
					stage->createBomb(*object()->worldPosition());
				}
			}
		}
	}
	if (stage->mArea == 1) {
		//対空砲
		if (stage->mRestShot > 0) {
			if (cKey::trigger(5)) {
				stage->createAAG(*object()->worldPosition());
			}
		}
	}
	if (stage->mArea == 2) {
		//主砲
		if (stage->mRestShot > 0) {
			if (cKey::trigger(5)) {
				stage->createCanon(*object()->worldPosition());
			}
		}
	}



	return 0;
};
void Player::collision(Stage *stage) {
	if (object()->mCollision.objectNumber() > 0) {
		stage->miss();
		stage->createPichun(position());
		object()->remove();
		removeObject();
		stage->mSound.play("ぴちゅん");
	}
}

///////////////////////////////////////////////////////////////////////////////////
Enemy::Enemy(){
};
Enemy::~Enemy(){
};

void Enemy::collision(Stage*stage) {
	if (object()->mCollision.objectNumber() > 0) {
		drop(stage);
		if (kill(stage)) {
			stage->getScore(mScore);
			stage->mSound.play("撃破");
		}
	}
	else {
		if (stage->mFlash) {
			kill(stage);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
Zako::Zako() {
	mAttackPoint = Random::instance()->getInt(800) - 400;
};

int Zako::update(Stage* stage) {
	attack(stage);
	return 0;
};
void Zako::attack(Stage * stage) {
	GameLib::Math::Vector3 pos;
	Draw::instance()->getScreenCoordinate(&pos,position());

	if (pos.x < 400.f &&
		pos.x > -400.f &&
		pos.y > -300.f &&
		pos.y < 300.f) {

		++stage->mAttackCount;


		float t = pos.x - mAttackPoint;
		t *= 512.f * t / (400.f*400.f);

		if (stage->mAttackCount > (t+Stage::mAttackTime)) {
			shot(stage);
		}
	}
}
bool Zako::kill(Stage * stage) {
	stage->createPichun(position());

	object()->remove();
	removeObject();

	return true;
};

///////////////////////////////////////////////////////////////////////////////////

Reimu::Reimu(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("霊夢"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	++stage->mAppearCount;
}

void Reimu::shot(Stage * stage) {
	stage->mAttackCount = 0;
	GameLib::Math::Vector3 pos;
	object()->getWorldPositionFinal(&pos);

	stage->createEnemyShot(pos);
};
void Reimu::drop(Stage* stage) {
	++stage->mReimuCount;
	if (stage->mReimuCount>3) {
		stage->createItemFourShot(position());
		stage->mReimuCount = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////
Marisa::Marisa(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("魔理沙"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	++stage->mAppearCount;
}

void Marisa::shot(Stage * stage) {
	stage->mAttackCount = 0;
	GameLib::Math::Vector3 pos;
	object()->getWorldPositionFinal(&pos);

	stage->createEnemyLaser(pos);

};
void Marisa::drop(Stage* stage) {
	++stage->mMarisaCount;
	if (stage->mMarisaCount>3) {
		stage->createItemLaser(position());
		stage->mMarisaCount = 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////
Sanae::Sanae(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("早苗"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	mBullet = 4;
	++stage->mAppearCount;
}

void Sanae::shot(Stage * stage) {
	if (mBullet > 0) {
		--mBullet;
		stage->mAttackCount = 0;
		stage->createEnemyOnbasira(position());
	}
};
void Sanae::drop(Stage* stage) {
	++stage->mSanaeCount;
	if (stage->mSanaeCount>3) {
		stage->createItemBomb(position());
		stage->mSanaeCount = 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////
Yousei::Yousei(Stage* stage) {
	mScore = 10;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("妖精"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;
}

void Yousei::shot(Stage * stage) {
};
void Yousei::drop(Stage* stage) {
}

///////////////////////////////////////////////////////////////////////////////////

Tenko::Tenko(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("天子"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	++stage->mAppearCount;

}

void Tenko::shot(Stage * stage) {
	stage->mAttackCount = 0;

	stage->createEnemyKanameisi(position());
};
void Tenko::drop(Stage* stage) {
}
///////////////////////////////////////////////////////////////////////////////////

Iku::Iku(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("衣玖"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	mBullet = 3;

	++stage->mAppearCount;
}

void Iku::shot(Stage * stage) {
	if (mBullet > 0) {
		stage->mAttackCount = 0;

		stage->createEnemy2way(position());

		--mBullet;
	}
};
void Iku::drop(Stage* stage) {
}
///////////////////////////////////////////////////////////////////////////////////

Yorihime::Yorihime(Stage* stage) {
	mBullet = 1;

	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("依姫"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;


	++stage->mAppearCount;
}

void Yorihime::shot(Stage * stage) {
	if (mBullet > 0) {
		stage->mAttackCount = 0;
		stage->createEnemySnipe(position());

		--mBullet;
	}
};
void Yorihime::drop(Stage* stage) {
}
///////////////////////////////////////////////////////////////////////////////////

Toyohime::Toyohime(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mScore = 100;

	setObject(stage,ModelList::instance()->mChara.mTexture.model("豊姫"));
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	++stage->mAppearCount;
}

void Toyohime::shot(Stage * stage) {
	stage->mAttackCount = 0;

	stage->createEnemyFan(position());
};
void Toyohime::drop(Stage* stage) {
}

///////////////////////////////////////////////////////////////////////////////////
UFO::UFO(Stage* stage) {
	mScore = 1000;

	setObject(stage,ModelList::instance()->mChara.mUFO.model());
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;
	mSilhouetteColor = 0xffffff00;

	Object* obj = stage->mSprite.addObject(ModelList::instance()->mChara.mUFO.model(1));
	obj->setParent(object());
}
void UFO::shot(Stage * stage) {
};

void UFO::drop(Stage* stage) {
	stage->randomDrop(position());
}
void Stage::randomDrop(const GameLib::Math::Vector3& pos) {
	while(true) {
		int r = Random::instance()->getInt(8);
		if (r == 0) {
			createItemOneUp(pos);
			break;
		}
		if (r == 1) {
			createItemShot(pos);
			break;
		}
		if (r == 2) {
			createItemSpeed(pos);
			break;
		}
		if (r == 3) {
			createItemWave(pos);
			break;
		}
		if (r == 4) {
			if (Random::instance()->getBool()) {
				createItemFullPower(pos);
				break;
			}
			continue;
		}
		if (r == 5) {
			createItemFlash(pos);
			break;
		}
		if (r == 7) {
			createItemShotSpeed(pos);
			break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////

Hisoutensoku::Hisoutensoku(Stage* stage) {
	mSilhouetteColor = 0xffff0000;
	mSilhouetteWidth = 32;
	mSilhouetteHeight = 32;

	mScore = 100000;

	setObject(stage,ModelList::instance()->mChara.mBoss.model());

	object()->mCollision.mFlag = 2;
	object()->mCollision.mFlagTarget = 4;

	mHP = mMaxHP;
	mCount = 0;
	mPhase = 0;

	object()->setPosition(1200.f,0.f,0.f);
	GameLib::Math::Vector3 end(256.f,300.f,0.f);
	mMove = new Move(this,end);
	stage->castSpell("鉄符「ブレストストーム」");

	//シールド
	{
		Object* obj = stage->mSprite.addObject(ModelList::instance()->mChara.mBoss.model("シールド"));
		obj->setParent(object());
		obj->mCollision.mFlag = 2;
	}
	//shoot
	{
		Object* obj = stage->mSprite.addObject(ModelList::instance()->mChara.mTexture.model("shoot"));
		obj->setParent(object());
		obj->mAutoRemove = 1;
		obj->setAnimeCount(-120);
		obj->setPosition(-64.f,86.f,0.f);
	}
}
Hisoutensoku::~Hisoutensoku() {
	if (mMove) {
		delete mMove;
		mMove = 0;
	}
}
int Hisoutensoku::update(Stage * stage) {
	object()->mBlendInfo.mEmission *= 0.5f;
	if (mMove) {
		if (mMove->update(stage)) {
			delete mMove;
			mMove = 0;
		}
		return 0;
	}
	//ファイヤー
	if (mPhase == 0) {
		//if ((mCount/40)%2==0) {
		if (mCount %40 == 0) {
			GameLib::Math::Vector3 pos = position();
			pos.y += 40.f;
			stage->createEnemyTensokuB(pos,stage->mPlayerPosition);
		}
	}
	//ヨーヨー
	if (mPhase == 1) {
		if (mCount %96 == 0) {
			int i=(mCount/96)%2;
			GameLib::Math::Vector3 pos = position();
			pos.x += i*160 - 80;
			stage->createEnemyTensokuC(pos,stage->mPlayerPosition);
		}
	}

	//全方位
	if (mPhase == 2) {
		if (mCount %2 == 0) {
			stage->createEnemyTensokuA(position());
		}
	}



	++mCount;
	return 0;
};
bool Hisoutensoku::kill(Stage * stage) {
	if (mMove == 0) {
		--mHP;
		object()->mBlendInfo.mEmission.set(1.f,1.f,1.f);
		if (mHP < 1) {
			if (mPhase == 0) {
				mHP = mMaxHP;
				mPhase = 1;

				GameLib::Math::Vector3 pos(256.f,100.f,0.f);
				mMove = new Move(this,pos);
				stage->castSpell("電磁「トリック・ダブルループ」");
				mCount = 0;
				stage->randomDrop(position());
			}
			else {
				if (mPhase == 1) {
					mHP = mMaxHP;

					GameLib::Math::Vector3 pos(220.f,180.f,0.f);
					mMove = new Move(this,pos);
					stage->castSpell("ぶっぱなせ「ヒソウコレダー」");
					mPhase = 2;
					mCount = 0;
					stage->randomDrop(position());
				}
				else {
					if (mPhase == 2) {

						stage->destroy(object());
						removeObject();
						return true;
					}
				}
			}
		}
	}
	return false;
};
void Hisoutensoku::drop(Stage* stage) {
}

Hisoutensoku::Move::Move(Hisoutensoku *hi, const GameLib::Math::Vector3 &end) {
	mParent = hi;
	mEnd = end;
	mStart = hi->position();
	mCount = 0;
}
int Hisoutensoku::Move::update(Stage *stage) {
	++mCount;

	if (mCount == 60) {
		mParent->object()->setModel("移動");
	}
	if (mCount > 60 && mCount <= 120) {

		GameLib::Math::Vector3 pos;
		float t = 1.f * (mCount-60) / 60.f;
		Math::lerp(&pos,t,mStart,mEnd);

		mParent->object()->setPosition(pos);
	}

	if (mCount == 120) {
		mParent->object()->setModel("メイン");
	}
	if (mCount == 180) {
		return 1;
	}
	return 0;
};

float Stage::PlayerState::shotSpeed() {
	return 1.f * (mShotSpeed + 8.f) / 8.f;
}
///////////////////////////////////////////////////////////////////////////////////
Shot::Shot(Stage* stage,const GameLib::Math::Vector3& pos,bool bomb,Model* model) {

	if (model == 0) {
		if (stage->mPlayer.mWave) {
			setObject(stage,ModelList::instance()->mChara.mTexture.model("ウェーブ弾"));
		}
		else {
			setObject(stage,ModelList::instance()->mChara.mTexture.model("弾"));
		}
	}
	else {
		setObject(stage,model);
	}
	
	object()->setAnimeSpeed(stage->mPlayer.shotSpeed() );

	object()->setPosition(pos);
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 4;
	object()->mCollision.mFlagTarget = 2+16;

	mBomb = bomb;
	
	mSilhouetteColor=0;
};
int Shot::update(Stage* stage) {
	++stage->mShotCount;

	return 0;
};
void Shot::collision(Stage* stage) {
	if (object()->mCollision.objectNumber() > 0) {
		if (mBomb) {
			stage->createBomber(position());
		}
		else {
			GameLib::Math::Vector3 pos;
			object()->getWorldPositionFinal(&pos);
			stage->createExplode(pos);
		}

		object()->remove();
		removeObject();
	}
}
///////////////////////////////////////////////////////////////////////////////////
EnemyShot::EnemyShot(Stage* stage,const GameLib::Math::Vector3& pos,Model* model) {
	 mSilhouetteColor = 0;
	if (model == 0) {
		setObject(stage,ModelList::instance()->mChara.mTexture.model("敵弾"));
	}
	else {
		setObject(stage,model);
	}
	object()->setPosition(pos);
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 8;
	object()->mCollision.mFlagTarget = 1;
};
int EnemyShot::update(Stage* stage) {

	return 0;
};
void EnemyShot::collision(Stage* stage) {
	if (object()->mCollision.objectNumber() > 0 || 
		stage->mFlash) {

		GameLib::Math::Vector3 pos;
		object()->getWorldPositionFinal(&pos);

		object()->remove();
		removeObject();
	}
}

Onbasira::Onbasira(Stage* stage,const GameLib::Math::Vector3& pos) {
	setObject(stage,ModelList::instance()->mChara.mTexture.model("オンバシラ"));
	object()->setPosition(pos);
	object()->mAutoRemove = 1;

	object()->mCollision.mFlag = 8;
	object()->mCollision.mFlagTarget = 1;

	mCount = 0;
	mAngle = 90.f;
	mSilhouetteColor = 0;
};
int Onbasira::update(Stage* stage) {
	++mCount;

	//追尾
	if (mCount < 64) {
		GameLib::Math::Vector3 pos = stage->mPlayerPosition;
		pos -= position();

		GameLib::Math::Vector3 ang;
		Math::angleToVector3(&ang,mAngle,0.f);

		GameLib::Math::Vector3 cross;
		cross.setCross(pos,ang);
		if (cross.z < 0.f) {
			mAngle += 1.f;
		}
		else {
			mAngle -= 1.f;
		}
	}
	else {
		if (mAngle < 90.f) {
			mAngle += 1.f;
		}
		else {
			mAngle -= 1.f;
		}

	}

	object()->setVelocity(
		2.f * GameLib::Math::cos(mAngle),
		2.f * GameLib::Math::sin(mAngle),
		0.f
		);
	object()->setRotate(0.f,0.f,mAngle-90.f);

	return 0;
};
void Onbasira::collision(Stage* stage) {
	if (object()->mCollision.objectNumber() > 0 || 
		stage->mFlash) {

		object()->remove();
		removeObject();
	}
}




///////////////////////////////////////////////////////////////////////////////////
Item::Item() {
	mEnd = false;
	 mSilhouetteColor = 0;
}
int Item::update(Stage* stage) {
	if (!mEnd) {
		GameLib::Math::Vector3 pos;
		pos = *object()->worldPosition();
		if  (stage->mArea == 0) {
			if (pos.y > 0.f) {
				object()->setVelocity(0,0,0);
				object()->setLimit(240);
				mEnd = true;
			}
		}
		if (stage->mArea == 1) {
			if (pos.y < 0.f) {
				object()->setLimit(240);
				mEnd = true;
			}
		}
	}
	return 0;
}
void Item::collision(Stage* stage) {
	if (object()->mCollision.objectNumber() > 0) {
		get(stage);

		GameLib::Math::Vector3 pos;
		object()->getWorldPositionFinal(&pos);

		object()->remove();
		removeObject();

		stage->mSound.play("アイテム");

	}
}
void Item::create(Stage* stage,const GameLib::Math::Vector3& pos,const char* name) {
	setObject(stage,ModelList::instance()->mChara.mTexture.model(name));
	object()->setPosition(pos);

	object()->mCollision.mFlag = 32;
	object()->mCollision.mFlagTarget = 1;
	if (stage->mArea == 0) {
		object()->setVelocity(0.f,1.f,0.f);
	}
	if (stage->mArea == 1) {
		object()->setVelocity(0.f,-1.f,0.f);
	}
	if (stage->mArea == 2) {
		object()->setVelocity(-1.f,0.f,0.f);
		object()->setLimit(800);
		mEnd = true;
	}
};

///////////////////////////////////////////////////////////////////////////////////

OneUp::OneUp(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"1up");
};

void OneUp::get(Stage* stage) {
	++stage->mLeft;
	stage->mTelop.set("1up");
};


ShotNumber::ShotNumber(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"弾数アップ");
};

void ShotNumber::get(Stage* stage) {
	if (stage->mPlayer.mShot < 16) {
		++stage->mPlayer.mShot;
		stage->mTelop.set("ShotMaxUp");
	}
};

SpeedUp::SpeedUp(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"スピードアップ");
};

void SpeedUp::get(Stage* stage) {
	if (stage->mPlayer.mSpeed < 4) {
		++stage->mPlayer.mSpeed;
		stage->mTelop.set("SpeedUp");
	}
};
FullPower::FullPower(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"フルパワー");
};

void FullPower::get(Stage* stage) {
	if (stage->mPlayer.mShot < 12) {
		stage->mPlayer.mShot = 12;	//16
	}
	if (stage->mPlayer.mSpeed < 3) {
		stage->mPlayer.mSpeed = 3;	//4
	}
	if (stage->mPlayer.mShotSpeed < 3) {
		stage->mPlayer.mShotSpeed = 3;	//4
	}

	stage->mPlayer.mWave = true;
	stage->mTelop.set("FullPower");

};

Wave::Wave(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"ウェーブ");
};

void Wave::get(Stage* stage) {
	stage->mPlayer.mWave = true;
	stage->mTelop.set("WaveShooter");
};
ShotSpeed::ShotSpeed(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"雲山");
};

void ShotSpeed::get(Stage* stage) {
	if (stage->mPlayer.mShotSpeed < 4) {
		++stage->mPlayer.mShotSpeed;
		stage->mTelop.set("ShotSpeedUp");
	}
};

Flash::Flash(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"ボム");
};

void Flash::get(Stage* stage) {
	stage->mFlashOrder = true;;
	stage->mTelop.set("FlushBomb");
	stage->castNue();
};

FourShot::FourShot(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"四連ショット");
};

void FourShot::get(Stage* stage) {
	stage->mPlayer.mSpecial = 1;
	stage->mPlayer.mSpecialLimit = Stage::mSpecialLimit;
	stage->mTelop.set("BroadShot");
};

Laser::Laser(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"貫通");
};
void Laser::get(Stage* stage) {
	stage->mPlayer.mSpecial = 2;
	stage->mPlayer.mSpecialLimit = Stage::mSpecialLimit;
	stage->mTelop.set("Laser");
};



Bomb::Bomb(Stage* stage,const GameLib::Math::Vector3& pos) {
	create(stage,pos,"爆弾");
};
void Bomb::get(Stage* stage) {
	stage->mPlayer.mSpecial = 3;
	stage->mPlayer.mSpecialLimit = Stage::mSpecialLimit;
	stage->mTelop.set("Bomber");
};
