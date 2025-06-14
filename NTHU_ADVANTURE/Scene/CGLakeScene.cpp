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
#include "CGLakeScene.hpp"
#include "UI/Component/ImageButton.hpp"


const int CGLakeScene::MapWidth = 30, CGLakeScene::MapHeight = 16;
const int CGLakeScene::BlockSize = 64;

const int CGLakeScene::window_x = 30, CGLakeScene::window_y = 16;

std::vector<std::vector<CGLakeScene::TileType>> CGLakeScene::mapState;
std::vector<CGLakeScene::TileType> CGLakeScene::mapData;

//bool canWalk = true;

// Engine::Point PlayScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point CGLakeScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point CGLakeScene::cameraOffset = Engine::Point(0, 0);
void CGLakeScene::Initialize() {
    // 初始化遊戲狀態
    lives = 3;
    money = 0;
    
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    AddNewObject(UIGroup = new Group()); // 新增 UIGroup
    
    // 讀取地圖
    ReadMap();

    //timer = 60.0f;          // 60 秒限制
    //timeLimit = 60.0f;
    //GenerateMaze();
    
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
    bgmId = AudioHelper::PlayBGM("wind.ogg");

    if (LogScene::haveAxe){
        Engine::ImageButton* axeButton;
        axeButton = new Engine::ImageButton("stage-select/axe.png", "stage-select/axe.png", 1700, 50, 150, 150);
        axeButton->SetOnClickCallback(std::bind(&CGLakeScene::AxeOnClick, this));
        AddNewControlObject(axeButton);
    }
    
}

void CGLakeScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    SaveToFile();
    IScene::Terminate();
}

void CGLakeScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    //Engine::LOG(Engine::INFO) << LogScene::timer;
    // 計時器
    // timer -= deltaTime;
    // if (timer <= 0) {
    //     Engine::LOG(Engine::INFO) << "Time's up! Switching to lose scene.";
    //     Engine::GameEngine::GetInstance().ChangeScene("lose");
    //     return;
    // }
    
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
    if (gridX == 6 && gridY == 0) { // (29, 15)
        Engine::LOG(Engine::INFO) << "Reached goal! Switching to play scene from lake.";
        LogScene::clearedLake++;
        Engine::GameEngine::GetInstance().ChangeScene("play");
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

    
}

void CGLakeScene::Draw() const {
    

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
    // ALLEGRO_FONT* font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24).get();
    // if (font) {
    //     char timeStr[16];
    //     snprintf(timeStr, sizeof(timeStr), "Time: %.1f", timer);
    //     al_draw_text(font, al_map_rgb(255, 255, 255), 20, 20, ALLEGRO_ALIGN_LEFT, timeStr);
    // }

    
    // if (dialog.IsDialogActive()) {
    //     dialog.Draw();
    // }
}

void CGLakeScene::OnMouseDown(int button, int mx, int my) {
    if ((button & 1) && canChop) {
        //int worldX = mx + cameraOffset.x;
        //int worldY = my + cameraOffset.y;
        int gridX = mx / BlockSize;
        int gridY = my / BlockSize;

        if (gridX >= 0 && gridX < MapWidth && gridY >= 0 && gridY < MapHeight &&
            !(gridX == MapWidth - 1 && gridY == MapHeight - 1)) {
            Engine::LOG(Engine::INFO) << "Clicked grid: (" << gridX << ", " << gridY << ")";
            //mapState[gridY][gridX] = TILE_ROAD;
            mapData[gridY * MapWidth + gridX] = TILE_GRASS;
            UpdateTileMap(gridX, gridY);
        }
    }
    else if((button & 2) && canChop){
        canChop = false; // 砍樹後禁用
        if (axeImage) {
            UIGroup->RemoveObject(axeImage->GetObjectIterator());
            axeImage = nullptr;
        }
    }
    
    IScene::OnMouseDown(button, mx, my);
}

void CGLakeScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    // 更新斧頭圖片位置
    if (axeImage) {
        axeImage->Position = Engine::Point(mx - 48, my - 48);
    }
}

void CGLakeScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void CGLakeScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        //LogScene::money += 10;
        PlayScene::inCGLake = false;
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

void CGLakeScene::ReadMap() {
    std::string filename = std::string("Resource/CGLake") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(TILE_GRASS); break;
            case '1': mapData.push_back(TILE_TREE); break;
            case '2': mapData.push_back(TILE_WATER); break;
            case '3': mapData.push_back(TILE_LOTUS); break;
            case '4': mapData.push_back(TILE_DSTN); break;

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
                    imagePath = "mainworld/grasss.png";
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
                case TILE_WATER:
                    imagePath = "mainworld/water.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_LOTUS:
                    imagePath = "mainworld/lotus.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_DSTN:
                    imagePath = "mainworld/grasss.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );

                    imagePath = "mainworld/running.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
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
void CGLakeScene::UpdateTileMap(int gridX, int gridY) {
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
    TileType tileType = mapData[gridY * MapWidth + gridX];
    std::string imagePath = "mainworld/grasss.png";
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

Engine::Point CGLakeScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool CGLakeScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_GRASS:
        case TILE_DSTN:
            return true;
        case TILE_WATER:
        case TILE_LOTUS:
        case TILE_TREE:
        case NOTHING:
        default:
            return false;
    }
}

void CGLakeScene::AxeOnClick() {
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

void CGLakeScene::SaveToFile(void){
    //LoadFromFile();
    
    std::ofstream ofs("Resource/CGLake.txt");
    if (!ofs.is_open()) {
        printf("Failed to open account.txt!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        return;
    }
    
    for (int i=0;i<MapHeight;i++){
        for (int j=0;j<MapWidth;j++){
            if (mapData[i * MapWidth + j] == TILE_GRASS) ofs << 0;
            else if (mapData[i * MapWidth + j] == TILE_TREE) ofs << 1;
            else if (mapData[i * MapWidth + j] == TILE_WATER) ofs << 2;
            else if (mapData[i * MapWidth + j] == TILE_LOTUS) ofs << 3;
            else if (mapData[i * MapWidth + j] == TILE_DSTN) ofs << 4;
        }
        ofs << "\n";
    }
    ofs.close();
}