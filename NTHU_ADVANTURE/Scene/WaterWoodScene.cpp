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
#include "WaterWoodScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"
#include <cmath>

const int WaterWoodScene::MapWidth = 30, WaterWoodScene::MapHeight = 16;
const int WaterWoodScene::BlockSize = 64;

const int WaterWoodScene::window_x = 30, WaterWoodScene::window_y = 16;

std::vector<WaterWoodScene::TileType> WaterWoodScene::mapData;

// Engine::Point WaterWoodScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point WaterWoodScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point WaterWoodScene::cameraOffset = Engine::Point(0, 0);
void WaterWoodScene::Initialize() {
    firstTime = true;
    // 初始化遊戲狀態
    lives = 3;
    //money = 0;
    
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

    //NPC* nineSky;
    auto nineSkyAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/nineSky/avatar/auntie.png");
    NPCGroup->AddNewObject(nineSky = new NPC("九天玄女唯一指定姊妹 廖麗芳", nineSkyAvatar, 
                                            "NPC/nineSky/role/nineSky1.png",
                                            "NPC/nineSky/role/nineSky2.png", 
                                            "NPC/nineSky/role/nineSky3.png",
                                            "NPC/nineSky/role/nineSkyR.png",
                                            BlockSize * 18, BlockSize * 7
                                        ));

    // 初始化對話框
    dialog.Initialize();

    nineSky->SetMessages({
        "貴人吉像",
        "相見便是有緣 來根小籤吧",
        "#lottery"
    });

    // 預載資源
    //Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("waterwood.ogg");
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    if (LogScene::haveSpeedUp){
        LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
        //haveSpeedUpInt = rou
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    }

    bmp1 = Engine::Resources::GetInstance().GetBitmap("NPC/nineSky/role/nineSky1.png");
    bmp2 = Engine::Resources::GetInstance().GetBitmap("NPC/nineSky/role/nineSky2.png");
    bmp3 = Engine::Resources::GetInstance().GetBitmap("NPC/nineSky/role/nineSky3.png");
}

void WaterWoodScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void WaterWoodScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    //Engine::LOG(Engine::INFO) << LogScene::timer;
    timer += deltaTime;
    if (timer >= 2.0f) timer -= 2.0f;
    if (std::fmod(timer, 2.0f) < 0.5f) nineSky->bmp = bmp1;
    else if (std::fmod(timer, 2.0f) < 1.0f) nineSky->bmp = bmp2;
    else if (std::fmod(timer, 2.0f) < 1.5f) nineSky->bmp = bmp1;
    else nineSky->bmp = bmp3;
    
    if(firstTime){
        std::vector<std::string> testMessages = {
            "水木是清華大學的學餐",
            "一走進去就會聞到金展的香(臭)味",
            "特產是冷掉的四海遊龍煎餃和高麗菜沒熟的弘謙炒飯",
            "水木理髮廳可以幫你的頭髮變得很清爽 •̀ ωˊ•",
            "建議去風雲。",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/waterwoodicon.png");
        dialog.StartDialog("水木", testAvatar, testMessages);

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

void WaterWoodScene::Draw() const {
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
}

void WaterWoodScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void WaterWoodScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void WaterWoodScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void WaterWoodScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = {
            "水木是清華大學的學餐",
            "一走進去就會聞到金展的香(臭)味",
            "特產是冷掉的四海遊龍煎餃和高麗菜沒熟的弘謙炒飯",
            "水木理髮廳可以幫你的頭髮變得很清爽 阿嬤最愛 •̀ ωˊ•",
            "建議去風雲。",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/waterwoodicon.png");
        dialog.StartDialog("水木", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inWaterWood = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void WaterWoodScene::ReadMap() {
    std::string filename = std::string("Resource/waterwood") + ".txt";

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
            case 'G': mapData.push_back(GIN); break;
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
                case GIN:
                    imagePath = "cool/gin.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 8, 
                                        BlockSize * 10)
                    );
                    break;
                //////////////////////////////
                case NOTHING:
                default:
                    continue;
            }
        }
    }
}

Engine::Point WaterWoodScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool WaterWoodScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
            return true;
        case TILE_WALL:
        case NOTHING:
        default:
            return false;
    }
}