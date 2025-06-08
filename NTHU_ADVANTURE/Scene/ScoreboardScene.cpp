#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include <fstream>
#include <sstream>
#include <algorithm>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/ScoreboardScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

ScoreRecord Score;

void ScoreboardScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    LoadFromFile();

    AddNewObject(new Engine::Label("Scoreboard", "pirulen.ttf", 80, halfW, halfH / 10 + 50, 255, 255, 255, 255, 0.5, 0.5));
    //AddNewObject(new Engine::Label(scores[0] + " " + names[0] + " " + times[0] + " " + mapIDs[0], "pirulen.ttf", 48, halfW, halfH / 10 + 100, 255, 255, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 + 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&ScoreboardScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 3 / 2 + 100, 0, 0, 0, 255, 0.5, 0.5));


    if (current_page+1 < total_pages){
        btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW * 3 / 2 - 100, halfH * 3 / 2 + 50, 400, 100);
        btn->SetOnClickCallback(std::bind(&ScoreboardScene::NextOnClick, this, 1));
        AddNewControlObject(btn);
        AddNewObject(new Engine::Label("-->", "pirulen.ttf", 48, halfW * 3 / 2 + 100, halfH * 3 / 2 + 100, 0, 0, 0, 255, 0.5, 0.5));
    }
    
    if (current_page > 0){
        btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW / 2 - 300, halfH * 3 / 2 + 50, 400, 100);
        btn->SetOnClickCallback(std::bind(&ScoreboardScene::LastOnClick, this, 1));
        AddNewControlObject(btn);
        AddNewObject(new Engine::Label("<--", "pirulen.ttf", 48, halfW / 2 - 100, halfH * 3 / 2 + 100, 0, 0, 0, 255, 0.5, 0.5));
    }
    
    AddNewObject(new Engine::Label("page " + std::to_string(current_page+1), "pirulen.ttf", 48, halfW, halfH * 3 / 2, 255, 255, 255, 255, 0.5, 0.5));
    
    //這個不能刪 否則terminate時會刪掉不存在的東西 注意StageSelectScene刪掉的東西裡面也沒有這個 特別寫在下面了
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);


    for (int i=current_page*5;i<records.size() && i<(current_page+1)*5;i++){
        AddNewObject(new Engine::Label(std::to_string(i+1) + " " + records[i].score + " " + records[i].timestamp + " " + records[i].playerName + " " + records[i].mapId + "\n", "pirulen.ttf", 48, halfW / 4, halfH / 10 + 150 + 75*(i - current_page*5), 255, 255, 255, 255, 0.0, 0.5));
    }
    
}

void ScoreboardScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void ScoreboardScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void ScoreboardScene::NextOnClick(int stage) {
    current_page++;
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard-scene");
}

void ScoreboardScene::LastOnClick(int stage) {
    current_page--;
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard-scene");
}

bool cmp(const ScoreRecord& a, const ScoreRecord& b){
    int score1 = std::stoi(a.score);
    int score2 = std::stoi(b.score);
    if (score1 > score2) return true;
    else if (score1 < score2) return false;
    else return a.timestamp < b.timestamp; //string可以直接這樣
}

void ScoreboardScene::LoadFromFile() {
    std::ifstream ifs("C:/2025_I2P2_TowerDefense-main/Resource/scoreboard.txt");//要用絕對路徑 不然會跑到build去 每次都要重來
    if (!ifs.is_open()) {
        printf("Scoreboard file not found, starting empty!!!!!!!!!!!!!!!!!!!!!!!\n");
        return;
    }
    records.clear();
    // scores.clear();
    // times.clear();
    // names.clear();
    // mapIDs.clear();
    std::string line;
    std::getline(ifs, line); // 跳過標頭
    while (std::getline(ifs, line)) { //先讀進line 再讀進ss 然後從ss轉成四個對應字串
        std::stringstream ss(line);
        std::string scoreStr, playerName, timestamp, mapIdStr;

        std::getline(ss, scoreStr, ',');
        std::getline(ss, timestamp, ',');
        std::getline(ss, playerName, ',');
        std::getline(ss, mapIdStr, ',');
        
        ScoreRecord r;
        r.score = scoreStr;
        r.timestamp = timestamp;
        r.playerName = playerName;
        r.mapId = mapIdStr;
        records.push_back(r);
        
    }
    ifs.close();
    std::sort(records.begin(), records.end(), cmp);
    total_pages = (records.size() % 5 == 0)? records.size() / 5: records.size() / 5 + 1;
}

