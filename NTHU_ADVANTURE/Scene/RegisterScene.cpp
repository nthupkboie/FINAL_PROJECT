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

#include "RegisterScene.hpp"


void RegisterScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    name = pswd = "";
    ID_entered = 0;

    AddNewObject(new Engine::Label("Enter the ID you want to create:", "title.ttf", 48, halfW, halfH / 2, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(label_name = new Engine::Label(name, "title.ttf", 48, halfW, halfH / 2 + 100, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Create its password:", "title.ttf", 48, halfW, halfH / 2 + 200, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(label_pswd = new Engine::Label(pswd, "title.ttf", 48, halfW, halfH / 2 + 300, 255, 255, 255, 255, 0.5, 0.5));


    // LoadFromFile();

    // AddNewObject(new Engine::Label("Scoreboard", "pirulen.ttf", 80, halfW, halfH / 10 + 50, 255, 255, 255, 255, 0.5, 0.5));
    // //AddNewObject(new Engine::Label(scores[0] + " " + names[0] + " " + times[0] + " " + mapIDs[0], "pirulen.ttf", 48, halfW, halfH / 10 + 100, 255, 255, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 + 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&RegisterScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "title.ttf", 48, halfW, halfH * 3 / 2 + 100, 0, 0, 0, 255, 0.5, 0.5));
 
    //這個不能刪 否則terminate時會刪掉不存在的東西 注意StageSelectScene刪掉的東西裡面也沒有這個 特別寫在下面了
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);

}

void RegisterScene::OnKeyDown(int keyCode){
    IScene::OnKeyDown(keyCode);
    if (!ID_entered){
        if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
            char c = 'A' + (keyCode - ALLEGRO_KEY_A);
            name += c;
        } 
        else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
            char c = '0' + (keyCode - ALLEGRO_KEY_0);
            name += c;
        }
        else if (keyCode == ALLEGRO_KEY_BACKSPACE){
            name = name.substr(0, name.length() - 1);
        }
        else if (keyCode == ALLEGRO_KEY_ENTER){
            
            ID_entered = 1;
        }
        label_name->Text = name;
    }
    else if (ID_entered == 1) {
        if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
            char c = 'A' + (keyCode - ALLEGRO_KEY_A);
            pswd += c;
        } 
        else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
            char c = '0' + (keyCode - ALLEGRO_KEY_0);
            pswd += c;
        }
        else if (keyCode == ALLEGRO_KEY_BACKSPACE){
            pswd = pswd.substr(0, pswd.length() - 1);
        }
        else if (keyCode == ALLEGRO_KEY_ENTER){
            
            int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
            int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
            int halfW = w / 2;
            int halfH = h / 2;
            AddNewObject(new Engine::Label("Account created", "title.ttf", 48, halfW, halfH / 2 + 400, 255, 255, 255, 255, 0.5, 0.5));
            SaveToFile();
            ID_entered = 2;
        }
        label_pswd->Text = pswd;
    }
    
    
}
void RegisterScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void RegisterScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}


void RegisterScene::LoadFromFile() {
    std::ifstream ifs("C:/FINAL_PROJECT/NTHU_ADVANTURE/Resource/account.txt");//要用絕對路徑 不然會跑到build去 每次都要重來
    // if (!ifs.is_open()) {
    //     printf("Scoreboard file not found, starting empty!!!!!!!!!!!!!!!!!!!!!!!\n");
    //     return;
    // }
    IDs.clear();
    passwords.clear();

    std::string line;
    // std::getline(ifs, line); // 跳過標頭
    while (std::getline(ifs, line)) { //先讀進line 再讀進ss 然後從ss轉成四個對應字串
        std::stringstream ss(line);
        std::string ID, password;

        std::getline(ss, ID, ',');
        std::getline(ss, password);
    //     std::getline(ss, playerName, ',');
    //     std::getline(ss, mapIdStr, ',');
        
    //     ScoreRecord r;
    //     r.score = scoreStr;
    //     r.timestamp = timestamp;
    //     r.playerName = playerName;
    //     r.mapId = mapIdStr;
    //     records.push_back(r);
        IDs.push_back(ID);
        passwords.push_back(password);
    }
    ifs.close();
    // std::sort(records.begin(), records.end(), cmp);
    // total_pages = (records.size() % 5 == 0)? records.size() / 5: records.size() / 5 + 1;
}

void RegisterScene::SaveToFile(void){
    LoadFromFile();
    
    std::ofstream ofs("C:/FINAL_PROJECT/NTHU_ADVANTURE/Resource/account.txt");
    if (!ofs.is_open()) {
        printf("Failed to open account.txt!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        return;
    }
    //ofs << "score,timestamp,playerName,mapId\n";
    for (int i=0;i<IDs.size();i++){
        ofs << IDs[i] << "," << passwords[i] << "\n";
    }

    ofs << name << "," <<  pswd << "\n"; //新的
    ofs.close();
}
