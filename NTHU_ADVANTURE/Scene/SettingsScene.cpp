#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include <iostream>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/SettingsScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include <allegro5/allegro_primitives.h>
#include "Engine/LanguageManager.hpp"




void SettingsScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    AddNewObject(new Engine::Image("scene/setting.png", 0, 0, w, h));

    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/cloud.png", "stage-select/floor.png", halfW - 150, halfH * 3 / 2 - 130, 300, 200);
    btn->SetOnClickCallback(std::bind(&SettingsScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    //AddNewObject(new Engine::Label("Back", "title.ttf", 48, halfW, halfH * 3 / 2, 0, 0, 0, 255, 0.5, 0.5));
    labelBack = new Engine::Label("", "Retro.ttf", 48, halfW, halfH * 3 / 2, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelBack);


    Slider *sliderBGM, *sliderSFX;
    sliderBGM = new Slider(40 + halfW - 95, halfH - 50 - 32, 256, 64);
    sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
     AddNewControlObject(sliderBGM);
    // AddNewObject(new Engine::Label("BGM :     ", "title.ttf", 32, 40 + halfW - 60 - 95, halfH - 50, 255, 255, 255, 255, 0.5, 0.5));
     sliderSFX = new Slider(40 + halfW - 95, halfH + 50 - 32, 256, 64);
     sliderSFX->SetOnValueChangedCallback(std::bind(&SettingsScene::SFXSlideOnValueChanged, this, std::placeholders::_1));
     AddNewControlObject(sliderSFX);
    // AddNewObject(new Engine::Label("SFX :     ", "title.ttf", 32, 40 + halfW - 60 - 95, halfH + 50, 255, 255, 255, 255, 0.5, 0.5));

    labelBGM = new Engine::Label("", "Retro.ttf", 32, 40 + halfW - 60 - 95 - 50, halfH - 50, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(labelBGM);

    labelSFX = new Engine::Label("", "Retro.ttf", 32, 40 + halfW - 60 - 95 - 50, halfH + 50, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(labelSFX);

    // Not safe if release resource while playing, however we only free while change scene, so it's fine.
    bgmInstance = AudioHelper::PlaySample("nthu_school_song.ogg", true, AudioHelper::BGMVolume);
    sliderBGM->SetValue(AudioHelper::BGMVolume);
    sliderSFX->SetValue(AudioHelper::SFXVolume);



    // 語言切換按鈕
    auto langBtn = new Engine::ImageButton(
        "stage-select/cloud.png",    // 改成已存在的圖片
        "stage-select/cloud.png",
        halfW - 150, halfH / 2 - 110, 300, 200
    );


    langBtn->SetOnClickCallback([=]() {
        auto& langMgr = LanguageManager::GetInstance();
        std::string curLang = langMgr.GetCurrentLanguage();
        if (curLang == "en") langMgr.LoadLanguage("zh");
        else                 langMgr.LoadLanguage("en");
        RefreshLabels();
    });
    AddNewControlObject(langBtn);

    labelLanguage = new Engine::Label("", "Retro.ttf", 32, halfW, halfH / 2 + 20, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(labelLanguage);

    // 載入預設語言
    //LanguageManager::GetInstance().LoadLanguage("en");
    RefreshLabels();

    // 音樂播放
    bgmInstance = AudioHelper::PlaySample("nthu_school_song.ogg", true, AudioHelper::BGMVolume);
    sliderBGM->SetValue(AudioHelper::BGMVolume);
    sliderSFX->SetValue(AudioHelper::SFXVolume);
}

void SettingsScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void SettingsScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}


//OnValueChangedCallBack實際上會變這個
void SettingsScene::BGMSlideOnValueChanged(float value) {
    AudioHelper::ChangeSampleVolume(bgmInstance, value);
    AudioHelper::BGMVolume = value;
}
void SettingsScene::SFXSlideOnValueChanged(float value) {
    AudioHelper::SFXVolume = value;
}

void SettingsScene::RefreshLabels() {
    labelBack->Text = LanguageManager::GetInstance().GetText("back");
    labelBGM->Text = LanguageManager::GetInstance().GetText("bgm");
    labelSFX->Text = LanguageManager::GetInstance().GetText("sfx");
    labelLanguage->Text = LanguageManager::GetInstance().GetText("language");
    
    //std::cout << "[LANG] back = " << back << ", length = " << back.length() << std::endl;
    std::string back = LanguageManager::GetInstance().GetText("back");
    std::cout << "[LANG] back loaded. Length = " << back.length() << std::endl;


}

