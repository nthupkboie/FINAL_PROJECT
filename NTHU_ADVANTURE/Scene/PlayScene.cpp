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
#include "LogScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include <allegro5/allegro_primitives.h>
#include "UI/Component/Label.hpp"
#include "UI/Component/Image.hpp"


bool PlayScene::inPlay = true, PlayScene::inSmallEat = false;

//int PlayScene::money = 0;

const int PlayScene::MapWidth = 60, PlayScene::MapHeight = 32;
const int PlayScene::BlockSize = 64;

const int PlayScene::window_x = 30, PlayScene::window_y = 16;

std::vector<PlayScene::TileType> PlayScene::mapData;

// Engine::Point PlayScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point PlayScene::cameraOffset = Engine::Point(0, 0);
void PlayScene::Initialize() {
    // 初始化遊戲狀態
    lives = 3;
    //money = 0;

    PlayScene::inPlay = true;
    PlayScene::inSmallEat = false;
    //PlayScene::money = 0;
    
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
    NPC* test;
    // sheet路徑, x, y, 
    // 上, 下, 左, 右, (先行在列)
    // 圖塊寬, 圖塊高
    auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    NPCGroup->AddNewObject(test = new NPC("NPC",testAvatar, "NPC/test/role/test_sheet.png",
                                            BlockSize * 30, BlockSize * 10,
                                            2, 3,  // 上 (第0列第2行)
                                            2, 0,  // 下
                                            2, 1,  // 左
                                            2, 2,  // 右
                                            64, 64)); // 圖塊大小

    Engine::Point testPoint0(BlockSize * 30 + BlockSize / 2, BlockSize * 10 + BlockSize / 2);
    Engine::Point testPoint1(BlockSize * 28 + BlockSize / 2, BlockSize * 10 + BlockSize / 2);
    Engine::Point testPoint2(BlockSize * 28 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);
    Engine::Point testPoint3(BlockSize * 30 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);

    test->AddPatrolPoint(testPoint0);
    test->AddPatrolPoint(testPoint1);
    test->AddPatrolPoint(testPoint2);
    test->AddPatrolPoint(testPoint3);

    test->SetMoveSpeed(10.0f);

    // 設置NPC的對話內容
    test->SetMessages({
        "你好，我是村民A！",
        "這個村莊最近不太平靜...",
        "晚上請小心行事。",
        "祝你好運，冒險者！"
    });

    // 預載資源
    //Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    AddBuildingZone(8, 13, 2, 1, "新齋"); 
    AddBuildingZone(35, 12, 2, 1, "資電館");  
    AddBuildingZone(47, 12, 2, 1, "台達館");
    AddBuildingZone(8, 19, 2, 1, "風雲樓");
    AddBuildingZone(21, 19, 2, 1, "成功湖");
    AddBuildingZone(35, 19, 2, 1, "水木");
    AddBuildingZone(47, 19, 2, 1, "小吃部");

    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("play.ogg");

    //道具
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    if (LogScene::haveSpeedUp){
        LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    } 
}

void PlayScene::AddBuildingZone(int x, int y, int width, int height, const std::string& buildingName) {
    // 設置建築物的範圍區域並儲存
    buildingZones.push_back(BuildingZone{x, y, width, height, buildingName});
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

    // 更新攝影機，直接設置偏移量
    cameraOffset.x = player->Position.x - window_x / 2 * BlockSize; // 置中：player.x - 192
    cameraOffset.y = player->Position.y - window_y / 2 * BlockSize; // 置中：player.y - 96
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    // // 更新攝影機
    // float targetX = player->Position.x - 3 * BlockSize; // 視角中心
    // float targetY = player->Position.y - 1.5 * BlockSize;
    // // 邊界限制
    // targetX = std::max(0.0f, std::min(targetX, static_cast<float>(MapWidth * BlockSize - 6 * BlockSize)));
    // targetY = std::max(0.0f, std::min(targetY, static_cast<float>(MapHeight * BlockSize - 3 * BlockSize)));
    // // 平滑插值（與玩家移動同步，0.3秒）
    // cameraOffset.x += (targetX - cameraOffset.x) * (deltaTime / 0.3f);
    // cameraOffset.y += (targetY - cameraOffset.y) * (deltaTime / 0.3f);
    
    for (const auto& zone : buildingZones) {
        if (IsPlayerNearBuilding(player, zone)) {
            ShowEnterPrompt(zone.buildingName, zone.x, zone.y);

        }
    }

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


    //道具
    moneyLabel->Position = Engine::Point(130 + cameraOffset.x, 70 + cameraOffset.y);
    moneyImage->Position = Engine::Point(20 + cameraOffset.x, 35 + cameraOffset.y);
    if (axeImage) axeImage->Position = Engine::Point(20 + cameraOffset.x, 105 + cameraOffset.y);
    if (speedImage) {
        speedImage->Position = Engine::Point(20 + cameraOffset.x, 175 + cameraOffset.y);
        speedLabel->Position = Engine::Point(130 + cameraOffset.x, 210 + cameraOffset.y);
    }

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

void PlayScene::ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY) {
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


void PlayScene::Draw() const {
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

    DrawMiniMap();


    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
    
    // 繪製對話框
    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }

    //IScene::Draw();

    
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

    if (keyCode == ALLEGRO_KEY_V) {
        Engine::GameEngine::GetInstance().ChangeScene("win");
    }
    if (keyCode == ALLEGRO_KEY_L) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }    
    if(keyCode == ALLEGRO_KEY_B){
        Engine::GameEngine::GetInstance().ChangeScene("battle");
        inPlay = false;
    }
    if(keyCode == ALLEGRO_KEY_1){
        Engine::GameEngine::GetInstance().ChangeScene("smalleat");
        inPlay = false;
        inSmallEat = true;
        //haveAxe = true;
    }
    if(keyCode == ALLEGRO_KEY_2){
        Engine::GameEngine::GetInstance().ChangeScene("waterwood");
        inPlay = false;
        inSmallEat = true;
        //haveAxe = true;
    }
    if(keyCode == ALLEGRO_KEY_3){
        Engine::GameEngine::GetInstance().ChangeScene("windcloud");
        inPlay = false;
        inSmallEat = true;
        //haveAxe = true;
    }
    if(keyCode == ALLEGRO_KEY_4){
        Engine::GameEngine::GetInstance().ChangeScene("EE");
        inPlay = false;
        inSmallEat = true;
    }
    if(keyCode == ALLEGRO_KEY_5){
        Engine::GameEngine::GetInstance().ChangeScene("talda");
        inPlay = false;
        inSmallEat = true;
        //haveAxe = true;
    }
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
        for (const auto& zone : buildingZones) {
            if (IsPlayerNearBuilding(player, zone)) {
                std::cout << "Entering " << zone.buildingName << "!" << std::endl;
                // 進入建築物的邏輯，例如進入新場景等
                break;
            }
        }
        for (const auto& zone : buildingZones) {
            if (IsPlayerNearBuilding(player, zone)) {
                std::cout << "Entering " << zone.buildingName << "!" << std::endl;

                if (zone.buildingName == "新齋") {
                    Engine::GameEngine::GetInstance().ChangeScene("new");
                } else if (zone.buildingName == "小吃部") {
                    Engine::GameEngine::GetInstance().ChangeScene("smalleat");
                    inPlay = false;
                    inSmallEat = true;
                } else if (zone.buildingName == "資電館") {
                    Engine::GameEngine::GetInstance().ChangeScene("EE");
                    inPlay = false;
                    inSmallEat = true;
                } else if (zone.buildingName == "台達館") {
                    Engine::GameEngine::GetInstance().ChangeScene("talda");
                    inPlay = false;
                    inSmallEat = true;
                } else if (zone.buildingName == "水木") {
                    Engine::GameEngine::GetInstance().ChangeScene("waterwood");
                    inPlay = false;
                    inSmallEat = true;
                } else if (zone.buildingName == "風雲樓") {
                    Engine::GameEngine::GetInstance().ChangeScene("windcloud");
                    inPlay = false;
                    inSmallEat = true;
                } else if (zone.buildingName == "成功湖") {
                    Engine::GameEngine::GetInstance().ChangeScene("lake");
                    inPlay = false;
                    inSmallEat = true;
                }

                break;  // 找到一個就跳出，不需要檢查更多建築
            }
        }
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
    std::string filename = std::string("Resource/mainworld") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case 'G': 
            case 'W': mapData.push_back(WATERWOOD); break;
            case 'L': mapData.push_back(LAKE); break;
            case 'C': mapData.push_back(WINDCLOUD); break;
            case 'E': mapData.push_back(SMALLEAT); break;
            case 'D': mapData.push_back(TALDA); break;
            case '$':
            case 'A':
            case '-': mapData.push_back(TILE_GRASS); break;
            case 'R': mapData.push_back(TILE_ROAD); break;
            case 'T': mapData.push_back(TILE_TREE); break;
            case 'S': mapData.push_back(TILE_STAIRS); break;
            case 'N': mapData.push_back(NEW); break;
            case 'n': mapData.push_back(TILE_NEW); break;
            case '=': mapData.push_back(NOTHING); break;
            case 'I': mapData.push_back(INFORMATIONELETRIC); break;
            //case 'A': mapData.push_back(TILE_AVANUE); break;
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
                    std::string imagePath = "mainworld/GAS.png";
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
                case TILE_GRASS: {
                    imagePath = "mainworld/GASS.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    // 使用隨機裝飾物（裝飾物圖檔要先準備好）
                    int randVal = rand() % 100; // 0~99 的隨機數

                    if (randVal < 10) {
                        // 10% 香菇
                        imagePath = "mainworld/MUSHROOM.png";
                    } else if (randVal < 20) {
                        // 10% 粉紅色花
                        imagePath = "mainworld/PINK.png";
                    } else if (randVal < 30) {
                        // 10% 紅色花
                        imagePath = "mainworld/RED.png";
                    } else if (randVal < 40) {
                        // 10% 草叢
                        imagePath = "mainworld/BUSH.png";
                    } else {
                        // 其他 60% 不加裝飾物
                        break;
                    }

                    // 貼上裝飾物圖層
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                }
                case TILE_ROAD: {
                    // 判斷周圍格子是否為草地
                    auto isGrass = [&](int nx, int ny) {
                        if (nx < 0 || ny < 0 || nx >= MapWidth || ny >= MapHeight)
                            return false;
                        return mapData[ny * MapWidth + nx] == TILE_GRASS;
                    };

                    std::string suffix = "";
                    if (isGrass(x, y - 1)) suffix += "U";
                    if (isGrass(x, y + 1)) suffix += "D";
                    if (isGrass(x - 1, y)) suffix += "L";
                    if (isGrass(x + 1, y)) suffix += "R";

                    if(suffix.empty()){
                        if (isGrass(x - 1, y - 1)) suffix += "0";
                        else if (isGrass(x + 1, y - 1)) suffix += "1";
                        else if (isGrass(x - 1, y + 1)) suffix += "2";
                        else if (isGrass(x + 1, y + 1)) suffix += "3";
                    }

                    // if (!suffix.empty())
                    //     suffix.pop_back(); // 移除最後一個底線

                    imagePath = + "mainworld/" + suffix + "ROAD.png";

                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                }
                case TILE_TREE:
                    imagePath = "mainworld/grass.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    imagePath = "mainworld/tree.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_STAIRS:
                    imagePath = "mainworld/stairs.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_AVANUE:
                    imagePath = "mainworld/avanue.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case NEW:
                    // imagePath = "mainworld/grass1.png";
                    // TileMapGroup->AddNewObject(
                    //     new Engine::Image(imagePath, 
                    //                     x * BlockSize, 
                    //                     y * BlockSize, 
                    //                     BlockSize * 7, 
                    //                     BlockSize * 7)
                    // );
                    imagePath = "mainworld/NEW.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
                case INFORMATIONELETRIC:
                    imagePath = "mainworld/informationeletric.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
                case SMALLEAT:
                    imagePath = "mainworld/smalleat.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
                case WATERWOOD:
                    imagePath = "mainworld/waterwood.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
                case WINDCLOUD:
                    imagePath = "mainworld/windcloud.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
                case LAKE:
                    imagePath = "mainworld/lake.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 10, 
                                        BlockSize * 8)
                    );
                    break;
                case TALDA:
                    imagePath = "mainworld/talda.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 8)
                    );
                    break;
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

Engine::Point PlayScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

void PlayScene::DrawMiniMap() const {
    const int minimapX = MapWidth*BlockSize*(0.4) - 10;  // 右上角 x
    const int minimapY = 10;  // 右上角 y
    const float scale = 0.1f; // 縮小比例

    for (int y = 0; y < MapHeight; ++y) {
        for (int x = 0; x < MapWidth; ++x) {
            int tile = mapData[y * MapWidth + x];
            ALLEGRO_COLOR color;
            switch (tile) {
                case TILE_GRASS: color = al_map_rgb(34, 139, 34); break; // 綠色
                case TILE_ROAD:  color = al_map_rgb(128, 128, 128); break; // 灰色
                case TILE_TREE:  color = al_map_rgb(0, 100, 0); break; // 深綠
                case TILE_STAIRS:color = al_map_rgb(255, 255, 255); break; // 白
                case TILE_AVANUE:color = al_map_rgb(50, 255, 255); break;
                case NEW:
                case NOTHING:
                    color = al_map_rgb(255, 132, 132);
                break;
                default: color = al_map_rgb(255, 132, 132); break;
            }
            al_draw_filled_rectangle(
                minimapX + x * BlockSize * scale,
                minimapY + y * BlockSize * scale,
                minimapX + (x + 1) * BlockSize * scale,
                minimapY + (y + 1) * BlockSize * scale,
                color
            );
        }
    }

    // 畫出玩家位置
    for (auto obj : PlayerGroup->GetObjects()) {
        if (auto* player = dynamic_cast<Player*>(obj)) {
            float px = minimapX + player->Position.x * scale;
            float py = minimapY + player->Position.y * scale;
            al_draw_filled_circle(px, py, 3, al_map_rgb(255, 0, 0)); // 紅色小圓點
        }
    }

    // 畫出視窗範圍框
    float cx = minimapX + cameraOffset.x * scale;
    float cy = minimapY + cameraOffset.y * scale;
    float w = window_x * BlockSize * scale;
    float h = window_y * BlockSize * scale;
    al_draw_rectangle(cx, cy, cx + w, cy + h, al_map_rgb(255, 255, 0), 1); // 黃框
}

//walkable
bool PlayScene::collision(int x, int y){
    //printf("NOOOOOOOOOOO\n");
    switch (mapData[y/BlockSize *MapWidth + x/BlockSize]){
        case TILE_AVANUE:
        case TILE_GRASS:
        case TILE_ROAD:
        case TILE_STAIRS:
            return true;
        case TILE_TREE:
        case NEW:
        case INFORMATIONELETRIC:
        case NOTHING:
        default:
            return false;
    }
    // if (mapState[y/BlockSize][x/BlockSize] == TILE_TREE) return false;
    // else return true;
}

bool PlayScene::IsPlayerNearBuilding(Player* player, const BuildingZone& zone) {
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