// [main.cpp]
// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"
#include "Engine/LanguageManager.hpp"

#include "Scene/PlayScene.hpp"
#include "Scene/StartScene.h"
#include "Scene/SettingsScene.hpp"
#include "Scene/ScoreboardScene.hpp"
#include "Scene/LogScene.hpp"
#include "Scene/RegisterScene.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/SmallEatScene.hpp"
#include "Scene/WaterWoodScene.hpp"
#include "Scene/WindCloudScene.hpp"
#include "Scene/EEScene.hpp"
#include "Scene/TaldaScene.hpp"

#include "Scene/AIChatScene.hpp"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

// No USE
// #include "Scene/ScoreboardScene.hpp"
#include "Scene/LoseScene.hpp"
// #include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"

int main(int argc, char **argv) {

	LanguageManager::GetInstance().LoadLanguage("en");

	Engine::LOG::SetConfig(true);
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();

	al_init();                     // 初始化 Allegro
	al_init_image_addon();        // 初始化圖片載入
	al_init_primitives_addon();   // 初始化繪圖基本圖形

    // TODO HACKATHON-2 (2/3): Register Scenes here
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("settings", new SettingsScene());
	game.AddNewScene("start", new StartScene());
	game.AddNewScene("battle", new BattleScene());
	game.AddNewScene("smalleat", new SmallEatScene());
	game.AddNewScene("waterwood", new WaterWoodScene());
	game.AddNewScene("windcloud", new WindCloudScene());
	game.AddNewScene("scoreboard", new ScoreboardScene());
	game.AddNewScene("login", new LogScene());
	game.AddNewScene("register", new RegisterScene());
	game.AddNewScene("talda", new TaldaScene());
	game.AddNewScene("EE", new EEScene());
	// game.AddNewScene("stage-select", new StageSelectScene());
	game.AddNewScene("lose", new LoseScene());
	game.AddNewScene("win", new WinScene());

	//game.AddNewScene("aichat", new AIChatScene());

	// start from start scene
	game.Start("start", 60, 1920, 1024);
	//game.Start("start", 60, 64*10, 64*5);
	return 0;
}
