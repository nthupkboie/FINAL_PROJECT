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
#include "WindCloudScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>


const int WindCloudScene::MapWidth = 30, WindCloudScene::MapHeight = 16;
const int WindCloudScene::BlockSize = 64;

const int WindCloudScene::window_x = 30, WindCloudScene::window_y = 16;

std::vector<WindCloudScene::TileType> WindCloudScene::mapData;

bool WindCloudScene::isPlayingWordle = false;

// Engine::Point WindCloudScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point WindCloudScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point WindCloudScene::cameraOffset = Engine::Point(0, 0);
void WindCloudScene::Initialize() {
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

    // NPC
    NPC* sister;
    // sheet路徑, x, y, 
    // 上, 下, 左, 右, (先行在列)
    // 圖塊寬, 圖塊高
    auto sisterAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/sister/avatar/sister.png");
    NPCGroup->AddNewObject(sister = new NPC("正在吃白醬鮮魚義大利麵加起司不加四季豆的學姐",sisterAvatar,
                                            "NPC/sister/role/sisterU.png",
                                            "NPC/sister/role/sisterD.png",
                                            "NPC/sister/role/sisterL.png",
                                            "NPC/sister/role/sisterR.png",
                                            BlockSize * 5, BlockSize * 5
                                            )); // 圖塊大小

    //NPC* Yang;
    auto YangAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/bro.png");
    NPCGroup->AddNewObject(Yang = new NPC("羊順人", YangAvatar,
                                            "NPC/Yang/role/YangU.png",
                                            "NPC/Yang/role/YangD.png", 
                                            "NPC/Yang/role/YangL.png",
                                            "NPC/Yang/role/YangR.png",
                                            BlockSize * 8, BlockSize * 8
                                        ));

    // NPCGroup->AddNewObject(Yang = new NPC("NPC",testAvatar, "NPC/test/role/test_sheet.png",
    //                                         BlockSize * 8, BlockSize * 8,
    //                                         2, 3,  // 上 (第0列第2行)
    //                                         2, 0,  // 下
    //                                         2, 1,  // 左
    //                                         2, 2,  // 右
    //                                         64, 64)); // 圖塊大小


    // 初始化對話框
    dialog.Initialize();
    
    // 設置NPC的對話內容
    std::string tmp = "唉呀 是可愛的" + LogScene::myName + "!";
    sister->SetMessages({
        tmp,
        "這學期還沒一起吃飯耶",
        "我本來想吃沐嵐，可是覺得要排太久了",
        "夜排檔也不錯，但有點普通",
        "去巨城嗎...搭完公車還要走路好麻煩",
        "東門市場又有點熱",
        "森森燒肉感覺很讚，但是好貴喔，如果有人能請我吃該多好><",
        "看來只好孤單地去吃金展了...",
    });

    if (!wordleFinished){
        Yang->SetMessages({
            "初次見面",
            "我是楊舜仁的弟弟",
            "羊順人"
        });
    }
    else {
        std::string tmp = "謝謝" + LogScene::myName + "陪我玩遊戲";
        Yang->SetMessages({
            tmp,
            "我會去說服哥哥不要當掉你"
        });
    }

    Yang->SetTriggerEvent([this]() {
        if (!wordleFinished) {
            isPlayingWordle = true;
            dialog.StartDialog("羊順人", Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/bro.png"), {
                "我來出個簡單的單字考考你，共五個字母、六次機會，死大斗！"
            });
        }
    });
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("windcloud.ogg");
    //道具
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    if (LogScene::haveSpeedUp){
        LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    }
    
    // wordleFinished = false;
    // wordleSuccess = false;
    // isPlayingWordle = false;
    // wordleAttempt = 0;
    // currentGuess = "";
    // wordleGuesses.clear();

    //建築
    AddBuildingZone(2, 2, 2, 2, "主地圖");
}

void WindCloudScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();

    wordleFinished = true;
    isPlayingWordle = false;
}

void WindCloudScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    //Engine::LOG(Engine::INFO) << LogScene::timer;

    if (wordleFinished) {
        std::string tmp = "謝謝" + LogScene::myName + "陪我玩遊戲";
        Yang->SetMessages({
            tmp,
            "我會去說服哥哥不要當掉你!"
        });
    }

    if(firstTime){
        std::vector<std::string> testMessages = {
            "風雲樓是清大最大的學餐建築，也有提供活動舉辦場所",
            "因為食物比較多，比較容易找到好吃的(大概)",
            "但是請不要隨意嘗試墨尼捲餅的拉茶和四樓的東西。",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/windcloudicon.png");
        dialog.StartDialog("風雲", testAvatar, testMessages);

        firstTime = false;
    }


    // 獲取玩家對象
    Player* player = nullptr;
    for (auto& obj : PlayerGroup->GetObjects()) {
        player = dynamic_cast<Player*>(obj);
        if (player) break;
    }
    
    if (!player) return; // 確保玩家存在

    //建築
    for (const auto& zone : buildingZones) {
        if (IsPlayerNearBuilding(player, zone)) {
            ShowEnterPrompt(zone.buildingName, zone.x, zone.y);

        }
    }

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

    if (keyInputCooldown > 0) keyInputCooldown -= deltaTime;

    if (isPlayingWordle && !wordleFinished) {
        
        if (keyInputCooldown <= 0) {
            ALLEGRO_KEYBOARD_STATE kbState;
            al_get_keyboard_state(&kbState);
        
            // 偵測鍵盤輸入 A-Z
            for (int k = ALLEGRO_KEY_A; k <= ALLEGRO_KEY_Z; ++k) {
                
                al_get_keyboard_state(&kbState);

                if (al_key_down(&kbState, k)) {

                    if (currentGuess.length() < 5) {
                        currentGuess += (char)(k - ALLEGRO_KEY_A + 'A');
                        keyInputCooldown = 0.2;
                    }
                }
            }

            // 刪除鍵
            al_get_keyboard_state(&kbState);

            if (al_key_down(&kbState, ALLEGRO_KEY_BACKSPACE)) {
                if (!currentGuess.empty())
                    currentGuess.pop_back();
                    keyInputCooldown = 0.2;
            }

            // 按 Enter 鍵提交
            if (al_key_down(&kbState, ALLEGRO_KEY_ENTER)) {
                if (currentGuess.length() == 5) {
                    wordleGuesses.push_back(currentGuess);
                    wordleAttempt++;
                    if (currentGuess == wordleAnswer) {
                        wordleSuccess = true;
                        wordleFinished = true;
                        LogScene::money += 100;
                        moneyLabel->Text = std::to_string(LogScene::money);
                    } else if (wordleAttempt >= 6) {
                        wordleFinished = true; 
                    }
                    currentGuess = "";
                    keyInputCooldown = 0.2;
                }
            }
        }

    }
    //建築
    bool nearAnyBuilding = false;
    for (const auto& zone : buildingZones) {
        if (IsPlayerNearBuilding(player, zone)) {
            ShowEnterPrompt(zone.buildingName,zone.x,zone.y);
            nearAnyBuilding = true;
            //break;
        }
    }
    if (!nearAnyBuilding && enterPromptLabel) {
        LabelGroup->RemoveObject(enterPromptLabel);  
        enterPromptLabel = nullptr;
        currentBuildingName = "";
    }

}

void WindCloudScene::Draw() const {
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

    if (isPlayingWordle) {
        const int startX = 100, startY = 100, boxSize = 64;
        for (int i = 0; i < wordleGuesses.size(); ++i) {
            for (int j = 0; j < 5; ++j) {
                char c = wordleGuesses[i][j];
                ALLEGRO_COLOR color = al_map_rgb(100, 100, 100);
                if (c == wordleAnswer[j]) {
                    color = al_map_rgb(0, 200, 0); // 綠
                } else if (wordleAnswer.find(c) != std::string::npos) {
                    color = al_map_rgb(200, 200, 0); // 黃
                } else {
                    color = al_map_rgb(50, 50, 50); // 灰
                }
                al_draw_filled_rectangle(startX + j * boxSize, startY + i * boxSize,
                                        startX + j * boxSize + boxSize, startY + i * boxSize + boxSize,
                                        color);
                al_draw_text(Engine::Resources::GetInstance().GetFont("pirulen.ttf", 32).get(),
                            al_map_rgb(255, 255, 255), startX + j * boxSize + boxSize / 2,
                            startY + i * boxSize + boxSize / 4, ALLEGRO_ALIGN_CENTER, std::string(1, c).c_str());
            }
        }

        // 畫出目前輸入
        for (int j = 0; j < currentGuess.length(); ++j) {
            char c = currentGuess[j];
            al_draw_filled_rectangle(startX + j * boxSize, startY + wordleGuesses.size() * boxSize,
                                    startX + j * boxSize + boxSize, startY + wordleGuesses.size() * boxSize + boxSize,
                                    al_map_rgb(80, 80, 80));
            al_draw_text(Engine::Resources::GetInstance().GetFont("pirulen.ttf", 32).get(),
                        al_map_rgb(255, 255, 255), startX + j * boxSize + boxSize / 2,
                        startY + wordleGuesses.size() * boxSize + boxSize / 4, ALLEGRO_ALIGN_CENTER, std::string(1, c).c_str());
        }

        if (wordleFinished) {
            std::string result = wordleSuccess ? "你成功了！獲得 100 元！" : "挑戰失敗，答案是 BINGO";
            al_draw_text(Engine::Resources::GetInstance().GetFont("title.ttf", 32).get(),
                        al_map_rgb(0, 0, 0), startX + 64, startY + 7 * boxSize,
                        0, result.c_str());
            //isPlayingWordle = false;
        }
    }
}

void WindCloudScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void WindCloudScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void WindCloudScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void WindCloudScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if (keyCode == ALLEGRO_KEY_ESCAPE && (wordleFinished || isPlayingWordle)) {
        isPlayingWordle = false;  // 確保這個標誌也被重置
    }
    
    if (keyCode == ALLEGRO_KEY_I && !isPlayingWordle) {
        std::vector<std::string> testMessages = {
            "風雲樓是清大最大的學餐建築，也有提供活動舉辦場所",
            "因為食物比較多，比較容易找到好吃的(大概)",
            "但是請不要隨意嘗試墨尼捲餅的拉茶和四樓的東西。",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/windcloudicon.png");
        dialog.StartDialog("風雲", testAvatar, testMessages);
    }
    if(keyCode == ALLEGRO_KEY_P && !isPlayingWordle){
        PlayScene::inPlay = true;
        PlayScene::inWindCloud = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
    // if (keyCode == ALLEGRO_KEY_ESCAPE && (wordleFinished || isPlayingWordle)){
    //     //wordleFinished = true;
    //     isPlayingWordle = false;
    // }
    // 按下 E 鍵進入建築物
    if (keyCode == ALLEGRO_KEY_E) {

        Player* player = nullptr;
        for (auto& obj : PlayerGroup->GetObjects()) {
            player = dynamic_cast<Player*>(obj);
            if (player) break;
        }
        if (!player) {
            std::cout << "Player not found!" << std::endl;
            return;
        }
        //if (player)  LogScene::lastPlayerPos = player->Position;
        
        for (const auto& zone : buildingZones) {
            if (IsPlayerNearBuilding(player, zone)) {
                std::cout << "Entering " << zone.buildingName << "!" << std::endl;

                if (zone.buildingName == "主地圖") {
                    PlayScene::inPlay = true;//記得改
                    PlayScene::inWindCloud = false;
                    Engine::GameEngine::GetInstance().ChangeScene("play");
                    
                }
                

                break;  // 找到一個就跳出，不需要檢查更多建築
            }
        }
    }
    
}

void WindCloudScene::ReadMap() {
    std::string filename = std::string("Resource/windcloud") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case 'W': mapData.push_back(TILE_WALL); break;
            case '^': mapData.push_back(TABLE); break;
            case 'F': mapData.push_back(TILE_FLOOR); break;
            case 'L': mapData.push_back(LSEAT); break;
            case 'R': mapData.push_back(RSEAT); break;
            case '=': mapData.push_back(NOTHING); break;
            case 'G': mapData.push_back(FOOD); break;
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
                    std::string imagePath = "smalleat/floor.png";
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
                case LSEAT:
                    imagePath = "smalleat/floor.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    imagePath = "smalleat/LSEAT.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case RSEAT:
                    imagePath = "smalleat/floor.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    imagePath = "smalleat/RSEAT.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                //////////////////////////////
                case FOOD:
                    imagePath = "cool/food.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 10)
                    );
                    break;
                case NOTHING:
                default:
                    continue;
            }
        }
    }
}

Engine::Point WindCloudScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool WindCloudScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
            return true;
        case TILE_WALL:
        case NOTHING:
        default:
            return false;
    }
}


bool WindCloudScene::IsPlayerNearBuilding(Player* player, const BuildingZone& zone) {
    // 將建築物格子坐標轉為像素坐標
    float zonePixelX = zone.x * BlockSize;
    float zonePixelY = zone.y * BlockSize;
    float zonePixelW = zone.width * BlockSize;
    float zonePixelH = zone.height * BlockSize;
    
    // 玩家中心點 (像素坐標)
    float playerX = player->Position.x + BlockSize/2;
    float playerY = player->Position.y + BlockSize/2;
    
    // 建築物中心點 (像素坐標)
    float buildingCenterX = zonePixelX + zonePixelW/2;
    float buildingCenterY = zonePixelY + zonePixelH/2;
    
    // 計算距離
    float dx = playerX - buildingCenterX;
    float dy = playerY - buildingCenterY;
    float distance = sqrt(dx*dx + dy*dy);
    
    // 檢測距離 (2.5個格子的範圍)
    return distance < (2.5f * BlockSize);
}

void WindCloudScene::ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY) {
    // 顯示提示文字（例如 "Press E to enter {buildingName}"）

    if (currentBuildingName == buildingName) return;  
                

    // 移除舊的提示
    if (enterPromptLabel) {
        LabelGroup->RemoveObject(enterPromptLabel);
        enterPromptLabel = nullptr;
    }

    currentBuildingName = buildingName; 
    
    // 計算提示文字顯示的位置
    float labelX = zoneX * BlockSize + BlockSize / 2;
    float labelY = zoneY * BlockSize ;



    enterPromptLabel = new Engine::Label(
        "Press E to enter " + buildingName,
        "Retro.ttf", 30,
        labelX, labelY,
        255, 255, 255, 255,
        0.5, 0.5
    );
    LabelGroup->AddNewObject(enterPromptLabel);

}

void WindCloudScene::AddBuildingZone(int x, int y, int width, int height, const std::string& buildingName) {
    // 設置建築物的範圍區域並儲存
    buildingZones.push_back(BuildingZone{x, y, width, height, buildingName});
}