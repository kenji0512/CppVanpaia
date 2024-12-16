#include <iostream>
#include <memory>
#include "stdafx.h"
#include "MainGameSequence.h"

/// <summary>
/// エントリポイント
/// NOTE: Unity/C#において等価のコードはないが、ゲームループの仕組みは全てのゲームが必ず持つ実装である
/// </summary>
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				 LPSTR lpCmdLine, int nCmdShow )
{
	// ウインドウモードに変更
	ChangeWindowMode(TRUE);

	if( DxLib_Init() == -1 )	// ＤＸライブラリ初期化処理
	{
		 return -1;				// エラーが起きたら直ちに終了
	}

	//バックバッファの設定
	SetDrawScreen(DX_SCREEN_BACK);

	//初期化
	Task::TaskManager::Create();
	SysPhysics::Create();

	//ゲームシーケンス作成
	auto gameSequence = std::make_shared<MainGameSequence>();
	gameSequence->NextState(MAIN_GAME_SEQUENCE::TITLE); //タイトルから始まる

	//FPS計測
	int tick = GetNowCount();
	int fps = 0;
	int fps_view = 0;

	// ESCキーが押されるまでループ
	while (CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面を初期化
		ClearDrawScreen();
		clsDx();

		//タスク削除
		TaskManager::Run(RUN_TYPE::DESTROY);

		//TODO:本来は別スレッドで動かす
		TaskManager::Run(RUN_TYPE::PHYSICS);

		//メインスレッド
		TaskManager::Run(RUN_TYPE::DO);

		//当たり判定
		SysPhysics::Run();
		
		//メインシーケンス
		gameSequence->Loop();

		//描画
		//別スレッドでやってLoopは同期するとよいが、今そこまでしなくてもいい
		TaskManager::Run(RUN_TYPE::DRAW);

		//FPS計測と表示
		fps++;
		if (GetNowCount() - tick > 1000)
		{
			tick += 1000;
			fps_view = fps;
			fps = 0;
		}
		DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", fps_view);

		//バックバッファ反映
		ScreenFlip();

		//プロセスメッセージ待ち
		if (ProcessMessage() == -1)
		{
			// エラーが起きたらループを抜ける
			break;
		}
	}

	//解放
	SysPhysics::Release();
	Task::TaskManager::Release();

	// ＤＸライブラリ使用の終了処理
	DxLib_End();

	return 0 ;
}
