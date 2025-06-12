// [main.cpp]
// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"
#include "Scene/LoseScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"
#include "Scene/StartScene.h"
#include "Scene/SettingsScene.hpp"
#include "Scene/Scoreboard.hpp"
#include "Account/LoginScene.hpp"
#include "Account/RegisterScene.hpp"
#include "Scene/ShopScene.hpp"
#include "Account/LocalAndOnlineScene.hpp"
#include "Account/LoginOnlineScene.hpp"
#include "Account/RegisterOnlineScene.hpp"
#include "Account/ScoreboardOnline.hpp"
#include "Scene/ModeSelectionScene.hpp"

int main(int argc, char **argv) {
	Engine::LOG::SetConfig(true);

	ScoreboardOnline::Initialize(
		"towerdefensegame-1b01e-default-rtdb.asia-southeast1.firebasedatabase.app",
		"AIzaSyAarObWywZJ_rQ2AlXDd6czNdnRqqSTRbo"
	);
	  
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();

	

    // TODO HACKATHON-2 (2/3): Register Scenes here
	game.AddNewScene("shop",new ShopScene());
	game.AddNewScene("local-online",new LocalAndOnlineScene());
	game.AddNewScene("login-online",new LoginOnlineScene());
	game.AddNewScene("register-online", new RegisterOnlineScene());
	game.AddNewScene("login", new LoginScene());
	game.AddNewScene("register", new RegisterScene());
	game.AddNewScene("start", new StartScene());
	game.AddNewScene("settings", new SettingsScene());
    game.AddNewScene("stage-select", new StageSelectScene());
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("lose", new LoseScene());
	game.AddNewScene("win", new WinScene());
	game.AddNewScene("scoreboard", new Scoreboard());
	game.AddNewScene("mode-selection", new ModeSelectionScene());
	

    // TODO HACKATHON-1 (1/1): Change the start scene
	game.Start("local-online",  60, 1600, 832);
	// game.Start("mode-selection",  60, 1600, 832);
	return 0;
}
