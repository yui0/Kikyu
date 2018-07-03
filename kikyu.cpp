//---------------------------------------------------------
//	Kikyu!
//
//		©2013 Yuichiro Nakada
//---------------------------------------------------------
// g++ -o kikyu kikyu.cpp -Iinclude -Llib.x86 -L/usr/X11R6/lib -lcatcake -lasound -lmad -lfreetype -lpng -ljpeg -lz -lGL -lXxf86vm -lpthread -lX11

#include <stdio.h>
#include "catcake_main.h"
#include "font.h"

// パラメータ
//#define SCREEN_WIDTH		640
#define SCREEN_WIDTH		854
#define SCREEN_HEIGHT		480
#define SCREEN_HZ		60
// プレイヤー
#define PLAYER_WIDTH		48	// 幅
#define PLAYER_HEIGHT		64	// 高さ
#define PLAYER_JUMP		-5	// ジャンプの強さ
#define PLAYER_GRAVIRY		0.25	// 重力
// 敵
#define ENEMY_WIDTH		96/3	// 幅
#define ENEMY_HEIGHT		32	// 高さ
#define ENEMY_SPEED		-5	// 移動スピード
#define ENEMY_HIT_LENGTH	20	// 衝突判定の領域サイズ
#define ENEMY_MAX		20	// 最大出現数
// アイテム
#define ITEM_WIDTH		16	// 幅
#define ITEM_HEIGHT		16	// 高さ
#define ITEM_SPEED		-4	// アイテムのスピード
#define ITEM_MAX		20	// 最大出現数
#define COIN_POINT		100	// コインのポイント
#define COIN_FRAME		14	// コイン画像のフレームインデックス
#define DIAMOND_POINT		1000	// ダイアモンドのポイント
#define DIAMOND_FRAME		64	// ダイアモンドのフレームインデックス
// 背景
#define BACKGROUND_WIDTH	1320
#define BACKGROUND_HEIGHT	320
#define SCROLL_SPEED		 0.004
// 画像
#if defined(__ANDROID__)
#define RESOURCE_PATH		"./sdcard/berry/"
#else
#define RESOURCE_PATH		"./assets/"
#endif
#define IMAGE_PLAYER		"Balloon.png"	// http://korcs.info/material.html
#define IMAGE_ENEMY		"enemy.png"
#define IMAGE_ITEM		"icon0.png"
#define IMAGE_BACKGROUND	"side01.jpg"
#define IMAGE_BACKGROUND2	"side02.jpg"
#define IMAGE_BACKGROUND3	"side03.jpg"
#define IMAGE_BACKGROUND4	"side04.jpg"
//#define IMAGE_BACKGROUND	"side01.png"
#define IMAGE_TITLE		"0627a.png"
// 音
#define TRACK_BGM1		0
#define TRACK_BGM2		1
#define TRACK_SE1		2
#define TRACK_SE2		3
#define BGM_VOL			240
#define BGM_STAGE1		"bgm_maoudamashii_4_vehicle03.mp3"	// http://maoudamashii.jokersounds.com
#define BGM_STAGE2		"bgm_maoudamashii_4_vehicle01.mp3"
#define BGM_STAGE3		"bgm_maoudamashii_4_vehicle02.mp3"
#define BGM_STAGE4		"bgm_maoudamashii_3_theme09.mp3"
//#define BGM_STAGE1		"arasuji_03.mp3"
//#define BGM_GAMESTART		"bgm_gamestart_1.wav"
#define BGM_GAMEOVER		"bgm_gameover_1.mp3"
#define SE_VOL			240
//#define SE_GAMESTART		"se_maoudamashii_system46.mp3"
#define SE_GAMESTART		"se_kusano_06_girl_ikimasu.mp3"	// http://www.otonomori.info/
#define SE_JUMP			"se_jump_short.mp3"	// ユウラボ8bitサウンド工房
//#define SE_PYUU			"se_pyuuuuu.mp3"	// ユウラボ8bitサウンド工房
#define SE_PYUU			"se_kusano_09_girl_u.mp3"
#define SE_ITEM_GET		"itemget.wav"


bool intersect(ckVec *a, r32 aw, r32 ah, ckVec *b, r32 bw, r32 bh) {
	aw /= 2;
	ah /= 2;
	bw /= 2;
	bh /= 2;
	if (b->x-bw <= a->x+aw && b->y-bh <= a->y+ah && b->x+bw >= a->x-aw && b->y+bh >= a->y-ah) {
	//if (b->x <= a->x+aw && b->y <= a->y+ah && b->x+bw >= a->x && b->y+bh >= a->y) {
//		printf("(%f,%f)-(%f,%f) (%f,%f)-(%f,%f)\n", a->x, a->y, aw, ah, b->x, b->y, bw, bh);
		return true;
	} else {
		return false;
	}
}

int game_frame;			// ゲームフレーム
int score, score_plus;		// スコア
int stage;			// ステージ
int enemy_freq;			// 敵発生周期

class Game : public ckTask
{
public:
	Game();
	~Game();

private:
	virtual void onUpdate();
	void (Game::*Scene)();
	void SceneTitleInit();
	void SceneTitle();
	void SceneGameInit();
	void SceneGame();
	void SceneGameOver();
	void Init();

	FontTex font;			// フォント

	ckSprt title_sprt;

	ckScr *m_scr;
	ckSprt bg_sprt;			// 背景
	float bg_scroll;

	ckSprt player_sprt;
	int player_frame;		// アニメーション
	float player_vx, player_vy;	// 移動速度

	ckSprt enemy_sprt;
	int enemy_frame[ENEMY_MAX];	// アニメーション
	int enemy_time[ENEMY_MAX];	// 存在時間

	ckSprt item_sprt;
	int item_frame[ITEM_MAX];	// アニメーション
};

void newGame()
{
	ckNewTask(Game);
}

void Game::onUpdate()
{
	(this->*Scene)();
}
Game::Game() : ckTask(ORDER_ZERO)
{
	ckSndMgr::openSoundDevice(ckSndMgr::CHANNEL_NUM_STEREO, ckSndMgr::SAMPLE_RATE_44KHZ, 50);

	// 背景
	m_scr = ckDrawMgr::newScreen(ckID::genID());
	//m_scr->setClearMode(false, true);
	//m_scr->setPerspective(false);
	//m_scr->moveLast();
//	m_scr->moveFirst();
	m_scr->moveBefore(ckDrawMgr::DEFAULT_2D_SCREEN_ID);

	SceneTitleInit();
}
Game::~Game()
{
	ckDrawMgr::deleteScreen(m_scr->getID());
	ckSndMgr::closeSoundDevice();
}

void Game::Init()
{
	int i;

	// 背景
	bg_sprt.init(2, m_scr->getID());
	bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND));
	bg_sprt.setBlendMode(ckDraw::BLEND_HALF, true);

	bg_sprt.dataPos(1).set(0, 0);	// 真ん中・スプライトの中央の座標
	bg_sprt.setDataSize(1, SCREEN_WIDTH, SCREEN_HEIGHT);
	bg_sprt.setDataUV(1, 0.0f, 0.0f, (float)SCREEN_WIDTH/BACKGROUND_WIDTH, /*(float)SCREEN_HEIGHT/BACKGROUND_HEIGHT*/1);
	bg_scroll = 0;

	bg_sprt.setDataSize(0, SCREEN_WIDTH, SCREEN_HEIGHT);
	bg_sprt.setDataUV(0, 0.0f, 0.0f, (float)SCREEN_WIDTH/BACKGROUND_WIDTH, 1);

	// プレイヤー
	player_sprt.init(1/*NUM*/, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	player_sprt.setTextureID(ckID_(IMAGE_PLAYER));
	player_sprt.setBlendMode(ckDraw::BLEND_HALF, true);

	player_sprt.dataPos(0).set(70.0f-SCREEN_WIDTH/2, -PLAYER_HEIGHT/2.0);
	player_sprt.setDataSize(0, PLAYER_WIDTH, PLAYER_HEIGHT);
	player_sprt.setDataUV(0, 0.0f, 0.0f, 1.0/6, 1.0/4);
	player_frame = 0;
	player_vy = 0;

	// 敵
	enemy_sprt.init(ENEMY_MAX, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	enemy_sprt.setTextureID(ckID_(IMAGE_ENEMY));
	enemy_sprt.setBlendMode(ckDraw::BLEND_HALF, true);

	for (i=0; i<ENEMY_MAX; i++) {
		enemy_sprt.dataPos(i).set(-ENEMY_WIDTH-SCREEN_WIDTH/2, 0);
		enemy_sprt.setDataSize(i, ENEMY_WIDTH, ENEMY_HEIGHT);
		enemy_sprt.setDataUV(i, 0.0f, 0.0f, 1.0/3, 1.0);
		enemy_frame[i] = 0;
	}

	// アイテム
	item_sprt.init(ITEM_MAX, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	item_sprt.setTextureID(ckID_(IMAGE_ITEM));
	item_sprt.setBlendMode(ckDraw::BLEND_HALF, true);

	for (i=0; i<ITEM_MAX; i++) {
		item_sprt.dataPos(i).set(-ITEM_WIDTH-SCREEN_WIDTH/2, 0);
		item_sprt.setDataSize(i, ITEM_WIDTH, ITEM_HEIGHT);
		item_sprt.setDataUV(i, 0.0f, 0.0f, 1.0/16, 1.0/5);
		item_frame[i] = 0;
	}
}
void Game::SceneGameInit()
{
	Init();

	// 全体
	score = 0;
	score_plus = 0;
	game_frame = 0;
	stage = 1;
	enemy_freq = 60;
	//bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND));
	Scene = &Game::SceneGame;
	ckSndMgr::play(TRACK_BGM1, ckID_(BGM_STAGE1), BGM_VOL, true);
	ckSndMgr::fadeTrackVolume(TRACK_BGM1, BGM_VOL, 40);

	//font.DrawEString(0, 0, (char*)"GAME START!!", 70);
}

void Game::SceneGame()
{
	int i;
	game_frame++;

	// 終了
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_ESCAPE)) {
		ckEndCatcake();
	}

	// スコア加算
	//score++;

	// スクロール
	bg_scroll += SCROLL_SPEED;
	if (bg_scroll >= 1.0) {
		bg_scroll = 0;
		bg_sprt.dataPos(1).set(0, 0);
	}
	if (bg_scroll+(float)SCREEN_WIDTH/BACKGROUND_WIDTH >= 1.0) {
		float a = SCREEN_WIDTH * (bg_scroll-(1-(float)SCREEN_WIDTH/BACKGROUND_WIDTH)) * 1/((float)SCREEN_WIDTH/BACKGROUND_WIDTH);
		//printf("%f\n", a);
		bg_sprt.dataPos(1).set(-a, 0);
		bg_sprt.dataPos(0).set(SCREEN_WIDTH-a, 0);

		// 伸びる
//		bg_sprt.setDataUV(1, bg_scroll, 0.0f, 1, 1);
	} else {
		// 範囲をずらす
		bg_sprt.setDataUV(1, bg_scroll, 0.0f, bg_scroll+(float)SCREEN_WIDTH/BACKGROUND_WIDTH, 1);
	}
	//sprt.dataAng(0) -= 1;

	// プレイヤー処理
	player_vy += PLAYER_GRAVIRY;		// 重力
	player_sprt.dataPos(0).y -= player_vy;	// 移動

	// フレームアニメーション
	player_frame = game_frame/4 % 24;

	// 画面からはみ出た際はゲームオーバー
	if (player_sprt.dataPos(0).y + PLAYER_HEIGHT < -SCREEN_HEIGHT/2) {
		//ckSndMgr::stop(TRACK_BGM);
		ckSndMgr::play(TRACK_BGM1, ckID_(BGM_GAMEOVER), BGM_VOL, false);
		Scene = &Game::SceneGameOver;
		//ckEndCatcake();
	} else if (player_sprt.dataPos(0).y > SCREEN_HEIGHT/2) {
		ckSndMgr::play(TRACK_BGM1, ckID_(BGM_GAMEOVER), BGM_VOL, false);
		Scene = &Game::SceneGameOver;
		//ckEndCatcake();
	}

	// ジャンプ
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_SPACE) || ckKeyMgr::isPressed(ckKeyMgr::KEY_LBUTTON)) {
		player_vy = PLAYER_JUMP;
		ckSndMgr::play(TRACK_SE1, ckID_(SE_JUMP), SE_VOL, false);
	}

	// 敵を生成
	if (game_frame % /*30*//*40*/enemy_freq == 0) {
		for (i=0; i<ENEMY_MAX; i++) if (enemy_sprt.dataPos(i).x < -ENEMY_WIDTH-SCREEN_WIDTH/2) break;
		if (i<ENEMY_MAX) {
			enemy_frame[i] = enemy_time[i] = 0;
			enemy_sprt.dataPos(i).set(SCREEN_WIDTH/2+30, ckMath::rand(-SCREEN_HEIGHT/2, SCREEN_HEIGHT/2-ENEMY_HEIGHT));
		}
	}

	for (i=0; i<ENEMY_MAX; i++) {
		if (enemy_sprt.dataPos(i).x < -ENEMY_WIDTH-SCREEN_WIDTH/2) continue;

		// 移動
		enemy_sprt.dataPos(i).x += ENEMY_SPEED;
		//enemy_sprt.dataPos(i).y += ckMath::cos_s32(time[i]*5*3.14/180);
		enemy_sprt.dataPos(i).y += ckMath::cos_s32(enemy_time[i]*2 % 180);

		// フレームアニメーション
		if (enemy_time[i] % 5 == 0) {
			enemy_frame[i] += 1;
			enemy_frame[i] %= 3;
		}
		float ux = (enemy_frame[i]%3)*1.0/3;
		float uy = (enemy_frame[i]/3)*1.0/1;
		enemy_sprt.setDataUV(i, ux, uy, ux+1.0/3, uy+1.0/1);

		// プレイヤーとの衝突判定
		if (intersect(&enemy_sprt.dataPos(i),
			enemy_sprt.dataW(i)/3, enemy_sprt.dataH(i)/4,
			  &player_sprt.dataPos(0),
			      player_sprt.dataW(0), player_sprt.dataH(0))) {
			// SE 再生
			ckSndMgr::play(TRACK_SE2, ckID_(SE_PYUU), SE_VOL, false);
			//ckSndMgr::fadeTrackVolume(TRACK_BGM1, 0, 40);
			ckSndMgr::play(TRACK_BGM1, ckID_(BGM_GAMEOVER), BGM_VOL, false);
			Scene = &Game::SceneGameOver;
		}
		/*if (this.within(player, ENEMY_HIT_LENGTH)) {
			gameOver("鳥と衝突してしまいました. 残念!!");
		}*/

		// タイムを進める
		enemy_time[i]++;
	}

	// アイテムを生成
	if (game_frame % /*50*/60 == 0) {
		for (i=0; i<ITEM_MAX; i++) if (item_sprt.dataPos(i).x < -ITEM_WIDTH-SCREEN_WIDTH/2) break;
		if (i<ITEM_MAX) {
			int r = ckMath::rand(0, 100);
			if (r < 10) {
				item_frame[i] = DIAMOND_FRAME;
			} else {
				item_frame[i] = COIN_FRAME;
			}
			float ux = (item_frame[i]%16)*1.0/16;
			float uy = (item_frame[i]/16)*1.0/5;
			item_sprt.setDataUV(i, ux, uy, ux+1.0/16, uy+1.0/5);
			item_sprt.dataPos(i).set(SCREEN_WIDTH/2+30, ckMath::rand(-SCREEN_HEIGHT/2, SCREEN_HEIGHT/2-ITEM_HEIGHT));
		}
	}

	for (i=0; i<ITEM_MAX; i++) {
		if (item_sprt.dataPos(i).x < -ITEM_WIDTH-SCREEN_WIDTH/2) continue;

		// 移動
		item_sprt.dataPos(i).x += ITEM_SPEED;

		// 衝突判定
		if (intersect(&item_sprt.dataPos(i),
			item_sprt.dataW(i), item_sprt.dataH(i),
			  &player_sprt.dataPos(0),
			      player_sprt.dataW(0), player_sprt.dataH(0))) {
			// 削除
			item_sprt.dataPos(i).x = -ITEM_WIDTH-SCREEN_WIDTH/2-1;
			if (item_frame[i] == COIN_FRAME) {
				// スコア加算
				score += COIN_POINT -20;
				score_plus += 20;
				font.DrawEString(player_sprt.dataPos(0).x-PLAYER_WIDTH/2, player_sprt.dataPos(0).y, (char*)"+100", 50);
			} else {
				// スコア加算
				score += DIAMOND_POINT -40;
				score_plus += 40;
				font.DrawEString(player_sprt.dataPos(0).x-PLAYER_WIDTH/2, player_sprt.dataPos(0).y, (char*)"+1000", 50);
			}
			// SE 再生
			ckSndMgr::play(TRACK_SE2, ckID_(SE_ITEM_GET), SE_VOL, false);
		}
	}
	font.effect();

	// スコア表示
	if (score_plus>0) {
		score_plus --;
		score ++;
	}
	char msg[256];
	sprintf(msg, "SCORE: %d", score);
	font.clear();
	font.DrawString(-SCREEN_WIDTH/2+16, SCREEN_HEIGHT/2-16, msg); // 0,0
	sprintf(msg, "%dm", game_frame*2);
	font.DrawStringRight(SCREEN_WIDTH/2, SCREEN_HEIGHT/2-16, msg);
	/*ckDbgMgr::drawString(0, 0, ckCol::FULL, 1, "SCORE: "+s);*/

	// ステージ処理
	if (game_frame*2 >= 10000) {
		game_frame = 0;
		stage++;
		switch (stage) {
		case 2:
			bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND2));
			enemy_freq = 50;
			break;
		case 3:
			bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND3));
			enemy_freq = 40;
			break;
		case 4:
			bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND4));
			enemy_freq = 30;
		}
	}
}

void Game::SceneGameOver()
{
	//ckDbgMgr::drawString(0, 0, ckCol::FULL, 1, "Game Over");
	font.clear();
	font.DrawStringCenter(16, (char*)"Game Over", 32, 32);
	char msg[256];
	sprintf(msg, "SCORE: %d", score);
	font.DrawString(-SCREEN_WIDTH/2+16, SCREEN_HEIGHT/2-16, msg); // 0,0
	sprintf(msg, "%dm", game_frame*2);
	font.DrawStringRight(SCREEN_WIDTH/2, SCREEN_HEIGHT/2-16, msg);

	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_ENTER) || ckKeyMgr::isPressed(ckKeyMgr::KEY_LBUTTON)) {
		SceneTitleInit();
	}
	// 終了
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_ESCAPE)) {
		ckEndCatcake();
	}
}

void Game::SceneTitleInit()
{
	// 消す
	Init();
	player_sprt.dataPos(0).set(-PLAYER_WIDTH-SCREEN_WIDTH, 0);
	//player_sprt.init(1, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	//enemy_sprt.init(ENEMY_MAX, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	//item_sprt.init(ITEM_MAX, ckDrawMgr::DEFAULT_2D_SCREEN_ID);

	// 背景
	bg_sprt.init(2, m_scr->getID());
	bg_sprt.setTextureID(ckID_(IMAGE_BACKGROUND4));
	bg_sprt.setBlendMode(ckDraw::BLEND_HALF, true);

	bg_sprt.dataPos(1).set(0, 0);	// 真ん中・スプライトの中央の座標
	bg_sprt.setDataSize(1, SCREEN_WIDTH, SCREEN_HEIGHT);
	bg_sprt.setDataUV(1, 0.0f, 0.0f, (float)SCREEN_WIDTH/BACKGROUND_WIDTH, 1);

	// タイトル
	title_sprt.init(1, ckDrawMgr::DEFAULT_2D_SCREEN_ID);
	title_sprt.setTextureID(ckID_(IMAGE_TITLE));
	title_sprt.setBlendMode(ckDraw::BLEND_HALF, true);
	//title_sprt.dataPos(0).set(SCREEN_WIDTH/2-196/2, -SCREEN_HEIGHT/2+278/2);
	title_sprt.dataPos(0).set(0, 0);
	title_sprt.setDataSize(0, 196, 278);
	title_sprt.setDataUV(0, 0.0f, 0.0f, 1, 1);

	font.clear();
	font.DrawStringCenter(64, (char*)"Kikyu!");
	//font.DrawPString(-80, 84, (char*)"♥キキュウ♥Aeronaut!");
	//font.DrawPString(-120, -SCREEN_HEIGHT/2+80, (char*)"Press Return to Embark");
	//font.DrawPString(-100, -SCREEN_HEIGHT/2+40, (char*)"(C)2013 YUICHIRO NAKADA");
	font.DrawPStringCenter(84, (char*)"♥キキュウ♥Aeronaut!");
	font.DrawPStringCenter(-SCREEN_HEIGHT/2+80, (char*)"Press Return to Embark");
	font.DrawPStringCenter(-SCREEN_HEIGHT/2+40, (char*)"(C)2013 YUICHIRO NAKADA");

	Scene = &Game::SceneTitle;
}

void Game::SceneTitle()
{
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_ENTER) || ckKeyMgr::isPressed(ckKeyMgr::KEY_LBUTTON)) {
		title_sprt.dataPos(0).set(-196-SCREEN_WIDTH, 0);
		ckSndMgr::play(TRACK_SE2, ckID_(SE_GAMESTART), SE_VOL, false);
		SceneGameInit();
		//Scene = &Game::SceneGame;
	}
	// フルスクリーン
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_F)) {
		ckSysMgr::toggleFullScreen(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	// 終了
	if (ckKeyMgr::isPressed(ckKeyMgr::KEY_ESCAPE)) {
		ckEndCatcake();
	}
}

//void newHelloCatcake();
ckMain()
{
	ckCreateCatcake("Kikyu!", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_HZ);

	ckResMgr::loadResource(RESOURCE_PATH IMAGE_FONT, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_FONT_P, true);

	ckResMgr::loadResource(RESOURCE_PATH BGM_STAGE1, true);
//	ckResMgr::loadResource(RESOURCE_PATH BGM_STAGE2, true);
//	ckResMgr::loadResource(RESOURCE_PATH BGM_STAGE3, true);
//	ckResMgr::loadResource(RESOURCE_PATH BGM_STAGE4, true);
	//ckResMgr::loadResource(RESOURCE_PATH BGM_GAMESTART, true);
	ckResMgr::loadResource(RESOURCE_PATH SE_GAMESTART, true);
	ckResMgr::loadResource(RESOURCE_PATH BGM_GAMEOVER, true);
	ckResMgr::loadResource(RESOURCE_PATH SE_JUMP, true);
	ckResMgr::loadResource(RESOURCE_PATH SE_PYUU, true);
	ckResMgr::loadResource(RESOURCE_PATH SE_ITEM_GET, true);

	ckResMgr::loadResource(RESOURCE_PATH IMAGE_PLAYER, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_ENEMY, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_ITEM, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_BACKGROUND, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_BACKGROUND2, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_BACKGROUND3, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_BACKGROUND4, true);
	ckResMgr::loadResource(RESOURCE_PATH IMAGE_TITLE, true);

	ckMath::srand(static_cast<u32>(ckSysMgr::getUsecTime()));

	newGame();
	/*ckResMgr::loadResource("/sdcard/catcake_logo_71x14.png", true);
	ckResMgr::loadResource("/sdcard/stonsans.ttf", true);
	newHelloCatcake();*/

	ckStartCatcake();
#if ! defined(__ANDROID__)
	ckDestroyCatcake();
#endif
}

/*#if defined(__ANDROID__)
#include <unistd.h>
#include <jni.h>
//#include "ck_low_level_api.h"
extern "C"
{
	extern JNIEXPORT void JNICALL Java_com_kitaoworks_catcake_Catcake_nativeUpdate(JNIEnv*, jobject);

	void android_main(struct android_app*)
	{
		ckMain_();

		while (true) {
			Java_com_kitaoworks_catcake_Catcake_nativeUpdate(0, 0);
		}
	}
}
#endif*/
