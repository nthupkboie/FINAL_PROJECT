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
#include "LogScene.hpp"
#include "Player/BattlePlayer.hpp"
#include "NPC/NPC.hpp"
#include "BattleScene.hpp"
#include "UI/Component/ImageButton.hpp"


const int BattleScene::MapWidth = 30, BattleScene::MapHeight = 16;
const int BattleScene::BlockSize = 64;

const int BattleScene::window_x = 30, BattleScene::window_y = 16;

std::vector<std::vector<BattleScene::TileType>> BattleScene::mapState;

//bool canWalk = true;

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
    AddNewObject(UIGroup = new Group()); // 新增 UIGroup
    
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

    if (LogScene::haveAxe){
        Engine::ImageButton* axeButton;
        axeButton = new Engine::ImageButton("stage-select/axe.png", "stage-select/axe.png", 1700, 50, 150, 150);
        axeButton->SetOnClickCallback(std::bind(&BattleScene::AxeOnClick, this));
        AddNewControlObject(axeButton);
    }
    
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
    

    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);

    TileMapGroup->Draw();
    PlayerGroup->Draw();
    NPCGroup->Draw();

    al_identity_transform(&transform);
    al_use_transform(&transform);

    UIGroup->Draw(); // 繪製斧頭圖片
    IScene::Draw(); //畫斧頭 要放在最後
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
    if ((button & 1) && canChop) {
        //int worldX = mx + cameraOffset.x;
        //int worldY = my + cameraOffset.y;
        int gridX = mx / BlockSize;
        int gridY = my / BlockSize;

        if (gridX >= 0 && gridX < MapWidth && gridY >= 0 && gridY < MapHeight &&
            !(gridX == MapWidth - 1 && gridY == MapHeight - 1)) {
            Engine::LOG(Engine::INFO) << "Clicked grid: (" << gridX << ", " << gridY << ")";
            mapState[gridY][gridX] = TILE_ROAD;
            mapData[gridY * MapWidth + gridX] = static_cast<int>(TILE_ROAD);
            UpdateTileMap(gridX, gridY);
            canChop = false; // 砍樹後禁用
        }
        if (axeImage) {
                UIGroup->RemoveObject(axeImage->GetObjectIterator());
                axeImage = nullptr;
            }
    }
    
    IScene::OnMouseDown(button, mx, my);
}

void BattleScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    // 更新斧頭圖片位置
    if (axeImage) {
        axeImage->Position = Engine::Point(mx - 48, my - 48);
    }
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
        LogScene::money += 10;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }

    //else if (keyCode == ALLEGRO_KEY_X) PlayScene::haveAxe = true;
    
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

// 新增函數：更新指定格子的圖塊
void BattleScene::UpdateTileMap(int gridX, int gridY) {
    // 使用迭代器遍歷 TileMapGroup 的物件
    auto objects = TileMapGroup->GetObjects();
    std::vector<std::list<IObject*>::iterator> iteratorsToRemove;
    
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        Engine::Image* img = dynamic_cast<Engine::Image*>(*it);
        if (img && static_cast<int>(img->Position.x / BlockSize) == gridX &&
                static_cast<int>(img->Position.y / BlockSize) == gridY) {
            iteratorsToRemove.push_back(it);
        }
    }

    // 移除舊圖塊
    // for (auto it : iteratorsToRemove) {
    //     TileMapGroup->RemoveObject(it);
    // }

    // 根據 mapState 添加新圖塊
    TileType tileType = mapState[gridY][gridX];
    std::string imagePath = "mainworld/grass.png";
    TileMapGroup->AddNewObject(
        new Engine::Image(imagePath, gridX * BlockSize, gridY * BlockSize, BlockSize, BlockSize)
    );

    switch (tileType) {
        case TILE_ROAD:
            break;
        case TILE_TREE:
            imagePath = "mainworld/tree.png";
            TileMapGroup->AddNewObject(
                new Engine::Image(imagePath, gridX * BlockSize, gridY * BlockSize, BlockSize, BlockSize)
            );
            break;
        case TILE_STAIRS:
            imagePath = "mainworld/stairs.png";
            TileMapGroup->AddNewObject(
                new Engine::Image(imagePath, gridX * BlockSize, gridY * BlockSize, BlockSize, BlockSize)
            );
            break;
        default:
            break;
    }

    Engine::LOG(Engine::INFO) << "Updated tile at (" << gridX << ", " << gridY << ") to type " << tileType;
}

void BattleScene::GenerateMaze() {
    // 初始化隨機數生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // 初始化地圖
    mapState.resize(MapHeight, std::vector<TileType>(MapWidth, TILE_ROAD));
    mapData.resize(MapWidth * MapHeight, static_cast<int>(TILE_ROAD));

    // 隨機生成牆壁（70% 機率）
    int treeCount = 0;
    for (int y = 1; y < MapHeight - 1; y++) {
        for (int x = 1; x < MapWidth - 1; x++) {
            if (dis(gen) < 0.65) {
                mapState[y][x] = TILE_TREE;
                mapData[y * MapWidth + x] = static_cast<int>(TILE_TREE);
                treeCount++;
            }
        }
    }
    Engine::LOG(Engine::INFO) << "Initial tree count: " << treeCount << " (" << (float)treeCount / ((MapWidth - 2) * (MapHeight - 2)) * 100 << "%)";

    // 設置邊界為牆壁
    for (int x = 0; x < MapWidth; x++) {
        mapState[0][x] = TILE_TREE;
        mapState[MapHeight - 1][x] = TILE_TREE;
        mapData[x] = static_cast<int>(TILE_TREE);
        mapData[(MapHeight - 1) * MapWidth + x] = static_cast<int>(TILE_TREE);
    }
    for (int y = 0; y < MapHeight; y++) {
        mapState[y][0] = TILE_TREE;
        mapState[y][MapWidth - 1] = TILE_TREE;
        mapData[y * MapWidth] = static_cast<int>(TILE_TREE);
        mapData[y * MapWidth + MapWidth - 1] = static_cast<int>(TILE_TREE);
    }

    // 設置起點和終點
    mapState[2][2] = TILE_ROAD;
    mapData[2 * MapWidth + 2] = static_cast<int>(TILE_ROAD);
    mapState[MapHeight - 1][MapWidth - 1] = TILE_STAIRS;
    mapData[(MapHeight - 1) * MapWidth + MapWidth - 1] = static_cast<int>(TILE_STAIRS);

    // 非遞迴 DFS
    std::vector<std::vector<bool>> visited(MapHeight, std::vector<bool>(MapWidth, false));
    std::stack<std::pair<int, int>> stack;
    std::vector<std::pair<int, int>> path; // 記錄最終路徑
    bool found = false;
    int pathLength = 0;
    const int maxSteps = 200; // 限制路徑長度

    stack.push({2, 2});
    visited[2][2] = true;
    path.push_back({2, 2});

    while (!stack.empty() && pathLength < maxSteps) {
        auto [x, y] = stack.top();
        if (x == MapWidth - 1 && y == MapHeight - 1) {
            found = true;
            break;
        }

        // 方向：右、下、左、上
        std::vector<std::pair<int, int>> directions = {
            {1, 0},  // 右
            {0, 1},  // 下
            {-1, 0}, // 左
            {0, -1}  // 上
        };
        std::vector<float> weights = {0.45f, 0.45f, 0.05f, 0.05f}; // 右下 45%，左上 5%

        // 隨機打亂方向
        for (int i = 0; i < 4; i++) {
            int j = i + (int)(dis(gen) * (4 - i));
            std::swap(directions[i], directions[j]);
            std::swap(weights[i], weights[j]);
        }

        bool moved = false;
        for (int i = 0; i < 4; i++) {
            int nx = x + directions[i].first;
            int ny = y + directions[i].second;
            if (nx >= 0 && nx < MapWidth && ny >= 0 && ny < MapHeight && !visited[ny][nx]) {
                stack.push({nx, ny});
                visited[ny][nx] = true;
                path.push_back({nx, ny});
                pathLength++;
                moved = true;
                break;
            }
        }

        if (!moved) {
            stack.pop();
            path.pop_back();
            pathLength--;
        }
    }

    // 若未找到路徑或路徑過長，重生成
    if (!found || pathLength >= maxSteps) {
        Engine::LOG(Engine::WARN) << "No path found or path too long (" << pathLength << " steps), regenerating maze...";
        GenerateMaze();
        return;
    }

    // 僅將最終路徑設為 TILE_ROAD
    for (const auto& [x, y] : path) {
        if (mapState[y][x] != TILE_STAIRS) {
            mapState[y][x] = TILE_ROAD;
            mapData[y * MapWidth + x] = static_cast<int>(TILE_ROAD);
        }
    }

    // 統計最終樹數量
    treeCount = 0;
    for (int y = 0; y < MapHeight; y++) {
        for (int x = 0; x < MapWidth; x++) {
            if (mapState[y][x] == TILE_TREE) treeCount++;
        }
    }
    Engine::LOG(Engine::INFO) << "Path length: " << pathLength;
    Engine::LOG(Engine::INFO) << "Final tree count: " << treeCount << " (" << (float)treeCount / (MapWidth * MapHeight) * 100 << "%)";

    // 額外增加樹密度（10% 非路徑格子）
    for (int y = 1; y < MapHeight - 1; y++) {
        for (int x = 1; x < MapWidth - 1; x++) {
            if (mapState[y][x] == TILE_ROAD && !visited[y][x] && dis(gen) < 0.1) {
                mapState[y][x] = TILE_TREE;
                mapData[y * MapWidth + x] = static_cast<int>(TILE_TREE);
                treeCount++;
            }
        }
    }
    Engine::LOG(Engine::INFO) << "Final tree count after extra: " << treeCount << " (" << (float)treeCount / (MapWidth * MapHeight) * 100 << "%)";

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

bool BattleScene::collision(int x, int y){
    if (mapState[y/BlockSize][x/BlockSize] == TILE_TREE) return false;
    else return true;
}

void BattleScene::AxeOnClick() {
    canChop = true;
    if (!axeImage) {
        axeImage = new Engine::Image("stage-select/axe.png", 1750, 100, 96, 96);
        UIGroup->AddNewObject(axeImage);
        if (!Engine::Resources::GetInstance().GetBitmap("stage-select/sword.png")) {
            Engine::LOG(Engine::ERROR) << "Failed to load stage-select/sword.png for axeImage";
        }
    }
    Engine::LOG(Engine::INFO) << "Axe activated, canChop = true";
}