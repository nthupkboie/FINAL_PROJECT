//
// Created by Hsuan on 2024/4/10.
//

#ifndef INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_STARTSCENE_H
#define INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_STARTSCENE_H

#include <allegro5/allegro_audio.h>
#include <memory>
#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include <string>

class StartScene final : public Engine::IScene {
public:
    ALLEGRO_SAMPLE_ID bgmId;
    explicit StartScene() = default;
    void Initialize() override;
    void Terminate() override;
    void RefreshLabels();
    void PlayOnClick(int stage);
    void SettingsOnClick(int stage);
    void ScoreboardOnClick(int stage);
    void LogOnClick(int stage);
    void RegisterOnClick(int stage);
    void Update(float deltaTime) override;
    void AIChatOnClick(int stage);
    std::string currentLanguage;

private:
    Engine::Label* labelTitle;
    Engine::Label* labelPlay;
    Engine::Label* labelSettings;
    Engine::Label* labelScoreboard;
    Engine::Label* labelLog;
    Engine::Label* labelRegister;
    Engine::Label* labelAIChat;
};
#endif   // INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_STARTSCENE_H
