#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

// engine
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "Engine/IObject.hpp"
#include "Engine/IScene.hpp"

// new add
#include "TaldaScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "NPC/Shopper.hpp"
#include "NPC/NPCDialog.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"

const int TaldaScene::MapWidth = 30, TaldaScene::MapHeight = 16;
const int TaldaScene::BlockSize = 64;

const int TaldaScene::window_x = 30, TaldaScene::window_y = 16;

std::vector<TaldaScene::TileType> TaldaScene::mapData;

// Engine::Point TaldaScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point TaldaScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point TaldaScene::cameraOffset = Engine::Point(0, 0);
void TaldaScene::Initialize() {
    firstTime = true;
    // 初始化遊戲狀態
    lives = 3;
    money = 0;
    
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    AddNewObject(LabelGroup = new Group());
    
    // 讀取地圖
    ReadMap();
    
    // 初始化玩家
    Player* player;
    PlayerGroup->AddNewObject(player = new Player("player/idle.png", 100, 100));

    // 初始化攝影機，確保玩家置中
    cameraOffset.x = player->Position.x - window_x / 2 * BlockSize; // 192
    cameraOffset.y = player->Position.y - window_y / 2 * BlockSize; // 96
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    // 初始化對話框
    dialog.Initialize();
   
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("play.ogg");
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));

    // 黑板的顯示位置與尺寸
    float bb_x = 1920 / 4 - 350;
    float bb_y = 1024 / 4 - 50;
    float bb_width = 1650;
    float bb_height = 650;

    // 顯示黑板背景
    LabelGroup->AddNewObject(new Engine::Image("scene/bb.png", bb_x, bb_y, bb_width, bb_height));

    bb_x = 1920 / 2 ;
    // 顯示「地點功能」的規則文字，每行一個Label，文字置左
    LabelGroup->AddNewObject(new Engine::Label("地點功能:", "Retro.ttf", 30, bb_x + 30, bb_y + 40, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("1. 新齋 : 人見人愛的八卦地點", "Retro.ttf", 30, bb_x + 30, bb_y + 80, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("2. 小吃部 : 可以購買你需要的技能，但這是屬於有錢人的地盤，如果沒錢請前往迷宮賺錢喔", "Retro.ttf", 30, bb_x + 30, bb_y + 120, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("3. 迷宮 : 若在時間內走出則可贏取金幣", "Retro.ttf", 30, bb_x + 30, bb_y + 160, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("4. 水木 : 想知道本日運勢嗎? 求神問卜好所在", "Retro.ttf", 30, bb_x + 30, bb_y + 200, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("5. 風雲樓 : 待開放", "Retro.ttf", 30, bb_x + 30, bb_y + 240, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("6. 台達 : 就是這裡啦，這是我最愛的秘密基地，在台達可以學到好多有趣的密技喔，資工系的學生都說讚", "Retro.ttf", 30, bb_x + 30, bb_y + 280, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("7. 資電 : 哎呀呀，成敗關鍵之地，被當與否，就看你有沒有認真囉", "Retro.ttf", 30, bb_x + 30, bb_y + 320, 255, 255, 255, 255, 0.5, 0.5));

    // 顯示「贏得遊戲的方法」文字，手動換行
    LabelGroup->AddNewObject(new Engine::Label("贏得遊戲的方法:", "Retro.ttf", 30, bb_x + 30, bb_y + 470, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("在各位小朋友好好努力學習與探索校園後，回答幾個小問題就可以決定你們能否通過遊戲考驗，", "Retro.ttf", 30, bb_x + 30, bb_y + 520, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(new Engine::Label("但是，這個遊戲是有玄機的，我們的排名方式要依照通關速度還有剩餘金錢喔，請大家務必認真學習。", "Retro.ttf", 30, bb_x + 30, bb_y + 560, 255, 255, 255, 255, 0.5, 0.5));
}

void TaldaScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void TaldaScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    if(firstTime){
        std::vector<std::string> testMessages = {
            "台達館對申請入學的人而言，是對資工系的第一印象",
            "每年五月都會出現許多新鮮的肝、有神的眼睛在此，",
            "去年曾在牆上出現坑洞，據說是被生氣的學生敲的",
            "勸新鮮的肝們進資工系前要三思(嘆"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/taldaicon.png");
        dialog.StartDialog("台達館", testAvatar, testMessages);

        firstTime = false;
    }
    
    // 獲取玩家對象
    Player* player = nullptr;
    for (auto& obj : PlayerGroup->GetObjects()) {
        player = dynamic_cast<Player*>(obj);
        if (player) break;
    }
    
    if (!player) return; // 確保玩家存在

    // 更新攝影機，直接設置偏移量
    cameraOffset.x = player->Position.x - window_x / 2 * BlockSize; // 置中：player.x - 192
    cameraOffset.y = player->Position.y - window_y / 2 * BlockSize; // 置中：player.y - 96
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));
    
    // 更新所有NPC
    for (auto& obj : NPCGroup->GetObjects()) {
        if (auto npc = dynamic_cast<NPC*>(obj)) {
            npc->Update(deltaTime, player);
        }
    }

    // 更新對話框
    if (dialog.IsDialogActive()) {
        dialog.Update(deltaTime);
    }

    // 檢查遊戲結束條件
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}

void TaldaScene::Draw() const {
    //IScene::Draw();

    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);

    TileMapGroup->Draw();
    PlayerGroup->Draw();
    NPCGroup->Draw();
    LabelGroup->Draw();

    al_identity_transform(&transform);
    al_use_transform(&transform);

    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
    
    // 繪製對話框
    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
}

void TaldaScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void TaldaScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void TaldaScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void TaldaScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    // if(firstTime){
    //     std::vector<std::string> testMessages = {
    //         "台達館座落於原紅樓舊址，",
    //         "由台達電公司捐款，並配合政府補助，",
    //         "清華大學「邁向頂尖大學」款項興建，",
    //         "於2011年落成啟用。",

    //         "台達館設計時納入台達電致力於環境保護，",
    //         "推廣綠建築的理念，",
    //         "更以「兼重人文藝術與科技」、「綠能校園」為主要訴求。",

    //         "設計時將地形、氣候、風向及溫濕度等因素考量在內，",
    //         "以創造一個永續、宜人、節能且環保的綠建築。"
    //     };
    //     auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/taldaicon.png");
    //     dialog.StartDialog("台達館", testAvatar, testMessages);

    //     firstTime = false;
    // }
    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = {
            "台達館對申請入學的人而言，是對資工系的第一印象",
            "每年五月都會出現許多新鮮的肝、有神的眼睛在此，",
            "去年曾在牆上出現坑洞，據說是被生氣的學生敲的",
            "勸新鮮的肝們進資工系前要三思(嘆"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/talicon.png");
        dialog.StartDialog("台達館", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inTalda = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void TaldaScene::ReadMap() {
    std::string filename = std::string("Resource/talda") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            // case '0': mapData.push_back(TILE_GRASS); break;
            // case 'R': mapData.push_back(TILE_ROAD); break;
            // case 'T': mapData.push_back(TILE_TREE); break;
            // case 'S': mapData.push_back(TILE_STAIRS); break;
            case 'W': mapData.push_back(TILE_WALL); break;
            case '^': mapData.push_back(TABLE); break;
            case 'F': mapData.push_back(TILE_FLOOR); break;
            // case 'N': mapData.push_back(NEW); break;
            // case 'n': mapData.push_back(TILE_NEW); break;
            case '=': mapData.push_back(NOTHING); break;
            case '\n':
            case '\r':
            default: break;
        }
    }
    fin.close();
    
    // // 確認地圖數據完整
    // if (static_cast<int>(mapData.size()) != MapWidth * MapHeight) {
    //     throw std::ios_base::failure("Map data is corrupted.");
    // }

    Engine::LOG(Engine::INFO) << "mapData.size() " << mapData.size();
    Engine::LOG(Engine::INFO) << "MapWidth * MapHeight " << MapWidth * MapHeight;

    // init init
    for(int y = 0; y < MapHeight; y++){
        for(int x = 0; x < MapWidth; x++){
                    std::string imagePath = "mainworld/grasss.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
        }
    }
    
    // 繪製地圖
    for (int y = 0; y < MapHeight; y++) {
        for (int x = 0; x < MapWidth; x++) {
            int tileType = mapData[y * MapWidth + x];
            std::string imagePath;
            
            switch(tileType) {
                ///////////////////////
                case TILE_WALL:
                    imagePath = "smalleat/wall.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_FLOOR: {
                    imagePath = "smalleat/floor.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );

                    // int randVal = rand() % 100; // 0~99 的隨機數

                    // if (randVal < 5) {
                    //     imagePath = "smalleat/BAG.png";
                    // }
                    // else {
                    //     break;
                    // }

                    // TileMapGroup->AddNewObject(
                    //     new Engine::Image(imagePath, 
                    //                     x * BlockSize, 
                    //                     y * BlockSize, 
                    //                     BlockSize, 
                    //                     BlockSize)
                    // );
                    // break;
                }
                break;
                case TABLE:
                    imagePath = "smalleat/floor.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );

                    imagePath = "smalleat/TABLE.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break; 
                //////////////////////////////
                case NOTHING:
                default:
                    continue;
            }
            
            // TileMapGroup->AddNewObject(
            //     new Engine::Image(imagePath, 
            //                       x * BlockSize, 
            //                       y * BlockSize, 
            //                       BlockSize, 
            //                       BlockSize)
            // );
        }
    }
}

Engine::Point TaldaScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool TaldaScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
            return true;
        case TILE_WALL:
        case NOTHING:
        default:
            return false;
    }
}