// [main.cpp]
// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"

#include "Scene/PlayScene.hpp"
#include "Scene/StartScene.h"
#include "Scene/SettingsScene.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/SmallEatScene.hpp"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>


// No USE
// #include "Scene/ScoreboardScene.hpp"
#include "Scene/LoseScene.hpp"
// #include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"

int main(int argc, char **argv) {
	Engine::LOG::SetConfig(true);
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();

	al_init_image_addon();


    // TODO HACKATHON-2 (2/3): Register Scenes here
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("settings", new SettingsScene());
	game.AddNewScene("start", new StartScene());
	game.AddNewScene("battle", new BattleScene());
	game.AddNewScene("smalleat", new SmallEatScene());

	// game.AddNewScene("scoreboard-scene", new ScoreboardScene());
	// game.AddNewScene("stage-select", new StageSelectScene());
	 game.AddNewScene("lose", new LoseScene());
	 game.AddNewScene("win", new WinScene());

	// start from start scene
	game.Start("start", 60, 1920, 1024);
	//game.Start("start", 60, 64*10, 64*5);
	return 0;
}
