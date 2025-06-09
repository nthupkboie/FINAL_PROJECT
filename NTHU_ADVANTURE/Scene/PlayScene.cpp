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
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"

const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;

Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}

void PlayScene::Initialize() {
    // 初始化遊戲狀態
    lives = 3;
    money = 0;
    
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    
    // 讀取地圖
    ReadMap();
    
    // 初始化玩家
    Player* player;
    PlayerGroup->AddNewObject(player = new Player("player/idle.png", 100, 100));

    // NPC
    NPC* test;
    // sheet路徑, x, y, 
    // 上, 下, 左, 右, (先行在列)
    // 圖塊寬, 圖塊高
    NPCGroup->AddNewObject(test = new NPC("NPC/test/role/test_sheet.png",
                                            64, 64,
                                            2, 3,  // 上 (第0列第2行)
                                            2, 0,  // 下
                                            2, 1,  // 左
                                            2, 2,  // 右
                                            64, 64)); // 圖塊大小

    // 初始化對話框
    dialog.Initialize();
    
    // 設置NPC的對話內容
    test->SetMessages({
        "你好，我是村民A！",
        "這個村莊最近不太平靜...",
        "晚上請小心行事。",
        "祝你好運，冒險者！"
    });

    // 獲取NPC頭像 (使用NPC的站立圖像)
    auto npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/icon/test_icon.png");
    
    // 預載資源
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("play.ogg");
}

void PlayScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void PlayScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    
    // 獲取玩家對象
    Player* player = nullptr;
    for (auto& obj : PlayerGroup->GetObjects()) {
        player = dynamic_cast<Player*>(obj);
        if (player) break;
    }
    
    if (!player) return; // 確保玩家存在
    
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

void PlayScene::Draw() const {
    IScene::Draw();
    
    // 繪製對話框
    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
}

void PlayScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }
    
    // // 按T鍵測試開啟對話 (可選)
    // if (keyCode == ALLEGRO_KEY_T) {
    //     std::vector<std::string> testMessages = {
    //         "這是按T鍵觸發的對話!",
    //         "第二條測試訊息。",
    //         "最後一條測試訊息。"
    //     };
    //     auto npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/icon/test_icon.png");
    //     dialog.StartDialog("測試NPC", npcAvatar, testMessages);
    // }
}

void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map1") + ".txt";
    
    // read map1.txt
    std::vector<int> mapData;
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(0); break; // grass walkable
            case '1': mapData.push_back(1); break; // rock  not walkable
            case '\n':
            case '\r':
            default: break;
        }
    }
    fin.close();
    
    // confirm map data
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    
    // draw
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            switch(mapData[i * MapWidth + j]){
                case 0:
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 1:
                    TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                default:
                    break;
            }
        }
    }
}