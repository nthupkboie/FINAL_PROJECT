#include "StartScene.h"
#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/StartScene.h"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include "Engine/LanguageManager.hpp"

void StartScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    Engine::ImageButton* btn;

    AddNewObject(new Engine::Image("scene/start-by.png", 0, 0, w, h));

    labelTitle = new Engine::Label("", "normal.ttf", 128, halfW, halfH / 3 + 50, 10, 100, 180, 255, 0.5, 0.5);
    AddNewObject(labelTitle);

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH / 2 + 70, 400, 100);
    btn->SetOnClickCallback(std::bind(&StartScene::RegisterOnClick, this, 1));
    AddNewControlObject(btn);
    labelRegister = new Engine::Label("", "title.ttf", 48, halfW, halfH / 2 + 120, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelRegister);

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH / 2 + 200, 400, 100);
    btn->SetOnClickCallback(std::bind(&StartScene::LogOnClick, this, 1));
    AddNewControlObject(btn);
    labelLog = new Engine::Label("", "title.ttf", 48, halfW, halfH / 2 + 250, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelLog);

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&StartScene::SettingsOnClick, this, 2));
    AddNewControlObject(btn);
    labelSettings = new Engine::Label("", "title.ttf", 48, halfW, halfH * 3 / 2, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelSettings);

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 - 180, 400, 100);
    btn->SetOnClickCallback(std::bind(&StartScene::ScoreboardOnClick, this, 2));
    AddNewControlObject(btn);
    labelScoreboard = new Engine::Label("", "title.ttf", 48, halfW, halfH * 3 / 2 - 120, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelScoreboard);

    currentLanguage = LanguageManager::GetInstance().GetCurrentLanguage();
    RefreshLabels();
}

void StartScene::Terminate() {
    IScene::Terminate();
}

void StartScene::PlayOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("play");
}

void StartScene::SettingsOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("settings");
}

void StartScene::ScoreboardOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
}

void StartScene::LogOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("login");
}

void StartScene::RegisterOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("register");
}

void StartScene::RefreshLabels() {
    labelTitle->SetText(LanguageManager::GetInstance().GetText("title"));
    labelRegister->SetText(LanguageManager::GetInstance().GetText("register"));
    labelLog->SetText(LanguageManager::GetInstance().GetText("test"));  // "log" 改為 "test"，如需恢復請調整
    labelSettings->SetText(LanguageManager::GetInstance().GetText("settings"));
    labelScoreboard->SetText(LanguageManager::GetInstance().GetText("scoreboard"));
}

void StartScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    std::string newLang = LanguageManager::GetInstance().GetCurrentLanguage();
    if (newLang != currentLanguage) {
        currentLanguage = newLang;
        RefreshLabels();
    }
}
