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
#include <allegro5/allegro_primitives.h>
#include "RegisterScene.hpp"


void RegisterScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    name = pswd = "";
    ID_entered = 0;

    AddNewObject(new Engine::Image("scene/register.png", 0, 0, w, h));
    AddNewObject(new Engine::Label("Enter the ID you want to create:", "retro.ttf", 56, halfW, halfH / 2, 11, 23, 70, 255, 0.5, 0.5));
    AddNewObject(label_name = new Engine::Label(name, "title.ttf", 56, halfW, halfH / 2 + 100, 255, 97, 0, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Create its password:", "retro.ttf", 56, halfW, halfH / 2 + 200, 11, 23, 70, 255, 0.5, 0.5));
    AddNewObject(label_pswd = new Engine::Label(pswd, "title.ttf", 56, halfW, halfH / 2 + 300, 255, 97, 0, 255, 0.5, 0.5));
    
    //AddNewObject(imageGroup = new Group());
    AddNewObject(arrowImage = new Engine::Image("stage-select/right-arrow.png", halfW / 2 + 50, halfH / 2 + 40, 128, 128));

    // LoadFromFile();

    // AddNewObject(new Engine::Label("Scoreboard", "pirulen.ttf", 80, halfW, halfH / 10 + 50, 255, 255, 255, 255, 0.5, 0.5));
    // //AddNewObject(new Engine::Label(scores[0] + " " + names[0] + " " + times[0] + " " + mapIDs[0], "pirulen.ttf", 48, halfW, halfH / 10 + 100, 255, 255, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 + 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&RegisterScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "title.ttf", 48, halfW, halfH * 3 / 2 + 100, 0, 0, 0, 255, 0.5, 0.5));
 
    //這個不能刪 否則terminate時會刪掉不存在的東西 注意StageSelectScene刪掉的東西裡面也沒有這個 特別寫在下面了
    bgmInstance = AudioHelper::PlaySample("coconut.ogg", true, AudioHelper::BGMVolume);

    LoadFromFile();

}

void RegisterScene::OnKeyDown(int keyCode){
    IScene::OnKeyDown(keyCode);
    if (!ID_entered){
        if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
            char c = 'A' + (keyCode - ALLEGRO_KEY_A);
            name += c;
            if (warning1) warning1->Text = "";
        } 
        else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
            char c = '0' + (keyCode - ALLEGRO_KEY_0);
            name += c;
            if (warning1) warning1->Text = "";
        }
        else if (keyCode == ALLEGRO_KEY_BACKSPACE){
            name = name.substr(0, name.length() - 1);
            if (warning1) warning1->Text = "";
        }
        else if (keyCode == ALLEGRO_KEY_ENTER){
            if (checkID(name)) {
                int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
                int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
                int halfW = w / 2;
                int halfH = h / 2;
                AddNewObject(warning1 = new Engine::Label("The account exists", "title.ttf", 48, halfW, halfH / 2 + 400, 255, 255, 255, 255, 0.5, 0.5));
                
            }
            else {
                ID_entered = 1;
                arrowImage->Position.y = Engine::GameEngine::GetInstance().GetScreenSize().y / 4 + 240;
            }
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
            if (pswd == "") {
                ID_entered = 0;
                arrowImage->Position.y = Engine::GameEngine::GetInstance().GetScreenSize().y / 4 + 40;
            }
            else pswd = pswd.substr(0, pswd.length() - 1);
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
    //LoadFromFile();
    
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

bool RegisterScene::checkID(std::string ID){
    //bool flag = false;
    for (int i=0;i<IDs.size();i++){
        if (IDs[i] == ID) {
            return true;
        }
    }
    return false;
}

void RegisterScene::Draw() const {
    IScene::Draw(); // 繪製背景與按鈕等

    ALLEGRO_COLOR gray = al_map_rgb(200, 200, 200);
    float boxW = 600; // 固定寬度
    float boxH = 80;  // 固定高度

    // Label: ID
    if (label_name) {
        float x = label_name->Position.x;
        float y = label_name->Position.y;
        float drawX = x - boxW / 2;
        float drawY = y - boxH / 2;
        al_draw_filled_rectangle(drawX, drawY, drawX + boxW, drawY + boxH, gray);
        label_name->Draw();
    }

    // Label: Password
    if (label_pswd) {
        float x = label_pswd->Position.x;
        float y = label_pswd->Position.y;
        float drawX = x - boxW / 2;
        float drawY = y - boxH / 2;
        al_draw_filled_rectangle(drawX, drawY, drawX + boxW, drawY + boxH, gray);
        label_pswd->Draw();
    }
}
