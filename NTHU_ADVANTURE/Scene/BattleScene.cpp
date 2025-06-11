#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "random"

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
#include "Player/BattlePlayer.hpp"
#include "NPC/NPC.hpp"
#include "BattleScene.hpp"

const int BattleScene::MapWidth = 30, BattleScene::MapHeight = 16;
const int BattleScene::BlockSize = 64;

const int BattleScene::window_x = 30, BattleScene::window_y = 16;

// Engine::Point PlayScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point BattleScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point BattleScene::cameraOffset = Engine::Point(0, 0);
void BattleScene::Initialize() {
    // 初始化遊戲狀態
    lives = 3;
    money = 0;
    
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    
    // 讀取地圖
    //ReadMap();

    timer = 60.0f;          // 60 秒限制
    timeLimit = 60.0f;
    GenerateMaze();
    
    // 初始化玩家
    BattlePlayer* player;
    PlayerGroup->AddNewObject(player = new BattlePlayer("player/idle.png", 100, 100));

    // 初始化攝影機，確保玩家置中
    cameraOffset.x = player->Position.x - window_x / 2 * BlockSize; // 192
    cameraOffset.y = player->Position.y - window_y / 2 * BlockSize; // 96
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    // // NPC
    // NPC* test;
    // // sheet路徑, x, y, 
    // // 上, 下, 左, 右, (先行在列)
    // // 圖塊寬, 圖塊高
    // auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    // NPCGroup->AddNewObject(test = new NPC("NPC",testAvatar, "NPC/test/role/test_sheet.png",
    //                                         BlockSize * 2, BlockSize * 2,
    //                                         2, 3,  // 上 (第0列第2行)
    //                                         2, 0,  // 下
    //                                         2, 1,  // 左
    //                                         2, 2,  // 右
    //                                         64, 64)); // 圖塊大小

    // // 初始化對話框
    // dialog.Initialize();
    
    // // 設置NPC的對話內容
    // test->SetMessages({
    //     "你好，我是村民A！",
    //     "這個村莊最近不太平靜...",
    //     "晚上請小心行事。",
    //     "祝你好運，冒險者！"
    // });

    // 預載資源
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("play.ogg");

    
}

void BattleScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void BattleScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    // 計時器
    timer -= deltaTime;
    if (timer <= 0) {
        Engine::LOG(Engine::INFO) << "Time's up! Switching to lose scene.";
        Engine::GameEngine::GetInstance().ChangeScene("lose");
        return;
    }
    
    // 獲取玩家對象
    BattlePlayer* player = nullptr;
    for (auto& obj : PlayerGroup->GetObjects()) {
        player = dynamic_cast<BattlePlayer*>(obj);
        if (player) break;
    }
    
    if (!player) return; // 確保玩家存在

    // 檢查終點
    int gridX = static_cast<int>(std::floor(player->Position.x / BlockSize));
    int gridY = static_cast<int>(std::floor(player->Position.y / BlockSize));
    if (gridX == MapWidth - 1 && gridY == MapHeight - 1) { // (29, 15)
        Engine::LOG(Engine::INFO) << "Reached goal! Switching to win scene.";
        Engine::GameEngine::GetInstance().ChangeScene("win");
        return;
    }

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
    
    // // 更新所有NPC
    // for (auto& obj : NPCGroup->GetObjects()) {
    //     if (auto npc = dynamic_cast<NPC*>(obj)) {
    //         npc->Update(deltaTime, player);
    //     }
    // }

    // // 更新對話框
    // if (dialog.IsDialogActive()) {
    //     dialog.Update(deltaTime);
    // }

    // 檢查遊戲結束條件
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}

void BattleScene::Draw() const {
    //IScene::Draw();

    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);

    TileMapGroup->Draw();
    PlayerGroup->Draw();
    NPCGroup->Draw();

    al_identity_transform(&transform);
    al_use_transform(&transform);

    // 繪製計時器
    ALLEGRO_FONT* font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24).get();
    if (font) {
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "Time: %.1f", timer);
        al_draw_text(font, al_map_rgb(255, 255, 255), 20, 20, ALLEGRO_ALIGN_LEFT, timeStr);
    }

    // if (dialog.IsDialogActive()) {
    //     dialog.Draw();
    // }
}

void BattleScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void BattleScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void BattleScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void BattleScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        Engine::GameEngine::GetInstance().ChangeScene("play");
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

void BattleScene::ReadMap() {
    std::string filename = std::string("Resource/battle") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(TILE_GRASS); break;
            case '1': mapData.push_back(TILE_TREE); break;

            // case '-': mapData.push_back(TILE_GRASS); break;
            // case 'R': mapData.push_back(TILE_ROAD); break;
            // case 'T': mapData.push_back(TILE_TREE); break;
            // case 'S': mapData.push_back(TILE_STAIRS); break;
            // case 'N': mapData.push_back(NEW); break;
            // case 'n': mapData.push_back(TILE_NEW); break;
            // case '=': mapData.push_back(NOTHING); break;

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
    
    // 繪製地圖
    for (int y = 0; y < MapHeight; y++) {
        for (int x = 0; x < MapWidth; x++) {
            int tileType = mapData[y * MapWidth + x];
            std::string imagePath;
            
            switch(tileType) {
                case TILE_GRASS:
                    imagePath = "mainworld/grasss.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_ROAD:
                    imagePath = "mainworld/road.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
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
                case NEW:
                    imagePath = "mainworld/grass1.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 7, 
                                        BlockSize * 7)
                    );

                    imagePath = "mainworld/NEW.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 7, 
                                        BlockSize * 7)
                    );
                    break;
                case TILE_NEW:
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
void BattleScene::GenerateMaze() {
    // 初始化隨機數生成器
    std::random_device rd;
    std::mt19937 gen(rd());

    // 初始化地圖為牆壁
    mapState.resize(MapHeight, std::vector<TileType>(MapWidth, TILE_TREE));
    mapData.resize(MapWidth * MapHeight, TILE_TREE);

    // 遞迴分割
    auto divide = [&](auto& self, int x1, int y1, int x2, int y2) -> void {
        if (x2 - x1 < 2 || y2 - y1 < 2) return;

        // 隨機選擇分割方向
        bool horizontal = (y2 - y1 > x2 - x1) || (y2 - y1 == x2 - x1 && std::uniform_int_distribution<>(0, 1)(gen) == 0);

        if (horizontal) {
            // 水平分割
            int y = std::uniform_int_distribution<>(y1 + 1, y2 - 1)(gen);
            // 畫牆壁
            for (int x = x1; x <= x2; x++) {
                mapState[y][x] = TILE_TREE;
                mapData[y * MapWidth + x] = TILE_TREE;
            }
            // 隨機開一個通道
            int passageX = std::uniform_int_distribution<>(x1, x2)(gen);
            mapState[y][passageX] = TILE_ROAD;
            mapData[y * MapWidth + passageX] = TILE_ROAD;

            // 遞迴
            self(self, x1, y1, x2, y - 1);
            self(self, x1, y + 1, x2, y2);
        } else {
            // 垂直分割
            int x = std::uniform_int_distribution<>(x1 + 1, x2 - 1)(gen);
            for (int y = y1; y <= y2; y++) {
                mapState[y][x] = TILE_TREE;
                mapData[y * MapWidth + x] = TILE_TREE;
            }
            int passageY = std::uniform_int_distribution<>(y1, y2)(gen);
            mapState[passageY][x] = TILE_ROAD;
            mapData[passageY * MapWidth + x] = TILE_ROAD;

            self(self, x1, y1, x - 1, y2);
            self(self, x + 1, y1, x2, y2);
        }
    };

    // 初始化邊界為路徑
    for (int x = 0; x < MapWidth; x++) {
        mapState[0][x] = TILE_ROAD;
        mapState[MapHeight - 1][x] = TILE_ROAD;
        mapData[x] = TILE_ROAD;
        mapData[(MapHeight - 1) * MapWidth + x] = TILE_ROAD;
    }
    for (int y = 0; y < MapHeight; y++) {
        mapState[y][0] = TILE_ROAD;
        mapState[y][MapWidth - 1] = TILE_ROAD;
        mapData[y * MapWidth] = TILE_ROAD;
        mapData[y * MapWidth + MapWidth - 1] = TILE_ROAD;
    }

    // 執行分割
    divide(divide, 1, 1, MapWidth - 2, MapHeight - 2);

    // 設置起點和終點
    mapState[0][0] = TILE_ROAD;
    mapData[0] = TILE_ROAD;
    mapState[MapHeight - 1][MapWidth - 1] = TILE_STAIRS;
    mapData[(MapHeight - 1) * MapWidth + MapWidth - 1] = TILE_STAIRS;

    // 繪製地圖
    for (int y = 0; y < MapHeight; y++) {
        for (int x = 0; x < MapWidth; x++) {
            TileType tileType = mapState[y][x];
            std::string imagePath;

            // 底圖：草地
            imagePath = "mainworld/grass.png";
            TileMapGroup->AddNewObject(
                new Engine::Image(imagePath, x * BlockSize, y * BlockSize, BlockSize, BlockSize)
            );

            switch (tileType) {
                case TILE_ROAD:
                    // 路徑無需額外圖層
                    break;
                case TILE_TREE:
                    imagePath = "mainworld/tree.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, x * BlockSize, y * BlockSize, BlockSize, BlockSize)
                    );
                    break;
                case TILE_STAIRS:
                    imagePath = "mainworld/stairs.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, x * BlockSize, y * BlockSize, BlockSize, BlockSize)
                    );
                    break;
                default:
                    break;
            }
        }
    }
}

Engine::Point BattleScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}