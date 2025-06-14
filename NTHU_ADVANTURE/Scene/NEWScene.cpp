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
#include "NewScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"

const int NEWScene::MapWidth = 30, NEWScene::MapHeight = 16;
const int NEWScene::BlockSize = 64;

const int NEWScene::window_x = 30, NEWScene::window_y = 16;

std::vector<NEWScene::TileType> NEWScene::mapData;

// Engine::Point NEWScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point NEWScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point NEWScene::cameraOffset = Engine::Point(0, 0);
void NEWScene::Initialize() {
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
    PlayerGroup->AddNewObject(player = new Player("player/idle.png", BlockSize * 14, BlockSize * 2));

    // 初始化攝影機，確保玩家置中
    cameraOffset.x = player->Position.x - window_x / 2 * BlockSize; // 192
    cameraOffset.y = player->Position.y - window_y / 2 * BlockSize; // 96
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    NPC* bablo;
    auto babloAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/bablo/avatar/bablo.png");
    NPCGroup->AddNewObject(bablo = new NPC("李君萎", babloAvatar, 
                                            "NPC/bablo/role/babloU.png",
                                            "NPC/bablo/role/babloD.png", 
                                            "NPC/bablo/role/babloL.png",
                                            "NPC/bablo/role/babloR.png",
                                            BlockSize * 4, BlockSize * 10
                                        ));

    NPC* shan;
    auto shanAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/shan/avatar/shan.png");
    NPCGroup->AddNewObject(shan = new NPC("王率城", shanAvatar, 
                                            "NPC/shan/role/shan_up.png",
                                            "NPC/shan/role/shan_ idle.png", 
                                            "NPC/shan/role/shan_left.png",
                                            "NPC/shan/role/shan_right.png",
                                            BlockSize * 7, BlockSize * 7
                                        ));

    NPC* xi;
    auto xiAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/xi/avatar/xi.png");
    NPCGroup->AddNewObject(xi = new NPC("笑東東", xiAvatar, 
                                            "NPC/xi/role/xi_up.png",
                                            "NPC/xi/role/xi_idle.png", 
                                            "NPC/xi/role/xi_left.png",
                                            "NPC/xi/role/xi_right.png",
                                            BlockSize * 10, BlockSize * 7
                                        ));

    NPC* ruby;
    auto rubyAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/ruby/avatar/ruby.png");
    NPCGroup->AddNewObject(ruby = new NPC("糕仔君", rubyAvatar, 
                                            "NPC/ruby/role/girl_up.png",
                                            "NPC/ruby/role/girl_idle.png", 
                                            "NPC/ruby/role/girl_left.png",
                                            "NPC/ruby/role/girl_right.png",
                                            BlockSize * 13, BlockSize * 7
                                        ));
    
    ruby->SetMoveSpeed(3.0f);

    Engine::Point ruby0(BlockSize * 13 + BlockSize / 2, BlockSize * 7 + BlockSize / 2);
    Engine::Point ruby1(BlockSize * 14 + BlockSize / 2, BlockSize * 7 + BlockSize / 2);
    Engine::Point ruby2(BlockSize * 14 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);
    Engine::Point ruby3(BlockSize * 13 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);

    ruby->AddPatrolPoint(ruby0);
    ruby->AddPatrolPoint(ruby1);
    ruby->AddPatrolPoint(ruby2);
    ruby->AddPatrolPoint(ruby3);

    NPC* brainlin;
    auto brainlinAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/brainlin/avatar/brain.png");
    NPCGroup->AddNewObject(brainlin = new NPC("淋駁菌",brainlinAvatar, 
                                            "NPC/brainlin/role/brain_up.png",
                                            "NPC/brainlin/role/brain_idle.png", 
                                            "NPC/brainlin/role/brain_left.png",
                                            "NPC/brainlin/role/brain_right.png",
                                            BlockSize * 17, BlockSize * 7
                                        ));

    NPC* edchen;
    auto edchenAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/edchen/avatar/edchen.png");
    NPCGroup->AddNewObject(edchen = new NPC("塵停嗯",edchenAvatar, 
                                            "NPC/edchen/role/edchen_up.png",
                                            "NPC/edchen/role/edchen_idle.png", 
                                            "NPC/edchen/role/edchen_left.png",
                                            "NPC/edchen/role/edchen_right.png",
                                            BlockSize * 21, BlockSize * 7
                                        ));

    NPC* guen;
    auto guenAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/edchen/avatar/edchen.png");
    NPCGroup->AddNewObject(guen = new NPC("蟹夢俊", edchenAvatar, 
                                            "NPC/edchen/role/edchen_up.png",
                                            "NPC/edchen/role/edchen_idle.png", 
                                            "NPC/edchen/role/edchen_left.png",
                                            "NPC/edchen/role/edchen_right.png",
                                            BlockSize * 21, BlockSize * 10
                                        ));

    NPC* rayfu;
    auto rayfuAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/rayfu/avatar/rayfu.png");
    NPCGroup->AddNewObject(rayfu = new NPC("富康芮", rayfuAvatar, 
                                            "NPC/rayfu/role/rayfu_up.png",
                                            "NPC/rayfu/role/rayfu_idle.png", 
                                            "NPC/rayfu/role/rayfu_left.png",
                                            "NPC/rayfu/role/rayfu_right.png",
                                            BlockSize * 18, BlockSize * 10
                                        ));

    NPC* chi;
    auto chiAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/7/avatar/7.png");
    NPCGroup->AddNewObject(chi = new NPC("填菌歧", chiAvatar, 
                                            "NPC/7/role/7_up.png",
                                            "NPC/7/role/7_idle.png", 
                                            "NPC/7/role/7_left.png",
                                            "NPC/7/role/7_right.png",
                                            BlockSize * 15, BlockSize * 10
                                        ));

    NPC* ryanlin;
    auto ryanlinavatar = Engine::Resources::GetInstance().GetBitmap("NPC/ryanlin/avatar/ryanlin.png");
    NPCGroup->AddNewObject(ryanlin = new NPC("霖陳雨", ryanlinavatar, 
                                            "NPC/ryanlin/role/ryanlin_up.png",
                                            "NPC/ryanlin/role/ryanlin_idle.png", 
                                            "NPC/ryanlin/role/ryanlin_left.png",
                                            "NPC/ryanlin/role/ryanlin_right.png",
                                            BlockSize * 12, BlockSize * 10
                                        ));

    // 初始化對話框
    dialog.Initialize();
    
    bablo->SetMessages({
        "下次不會再喝酒了...",
        "王哨橙 摔角"
    });

    ruby->SetMessages({
        "我才是真正的ruby!!!",
        "要不要去操場跑十圈，走了啦!!!!!!!!!!!!!!!!!!!!!!!!!!",
    });

    xi->SetMessages({
        "唯一推武陵褲!!!",
        "我要來搞linux了"
    });

    shan->SetMessages({
        "有人能比我更帥????????????????????????????",
        "我是系核、我很帥、我超猛"
    });

    brainlin->SetMessages({
        "我不能盤腿坐:U",
        "(\"挑眉\" :D "
    });

    edchen->SetMessages({
        "我不是蟹夢俊!!!!!!!!",
        "我沒有拿過A-以下######",
    });

    guen->SetMessages({
        "我不是塵停嗯!!!!!!!!",
        "我要去讀書了 掰!",
    });

    rayfu->SetMessages({
        "富康芮超富!!!!!!",
        "我是副召!!!!!!!!!",
    });

    chi->SetMessages({
        "我是中壢人啦",
        "我要歧視你們 因為我是填菌歧是你們!!!!",
    });

    ryanlin->SetMessages({
        "我是音樂大師 幫你們寫譜",
        "ryanlinnnnnnnnnn!!!!!!!",
    });

    // 預載資源
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("Ruby.ogg");
    //道具
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    if (LogScene::haveSpeedUp){
        LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    }
}

void NEWScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void NEWScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    if(firstTime){
        std::vector<std::string> testMessages = {
            "在新齋的交誼廳內，常常能看到一群資工系學生和一群電腦",
            "或是一群很吵的資工系學生和一堆酒",
            "是充滿歡樂與回憶的地方!",
            "在期末前打開任意一間資工系學生的房間，會看到一群卷鬼",
            "在final project期間逛逛新齋，會看到一群卷到日出的鬼",
            "是新鮮的肝逐漸失去價值的地方。"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/NEWicon.png");
        dialog.StartDialog("新齋", testAvatar, testMessages);

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

void NEWScene::Draw() const {
    IScene::Draw();

    // 繪製遊戲世界（地圖、NPC、玩家等）
    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);

    TileMapGroup->Draw();
    PlayerGroup->Draw();
    NPCGroup->Draw();
    LabelGroup->Draw();

    // 恢復變換矩陣（確保後續繪製不受相機影響）
    al_identity_transform(&transform);
    al_use_transform(&transform);
    
    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
}

void NEWScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void NEWScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void NEWScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void NEWScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = {
            "在新齋的交誼廳內，常常能看到一群資工系學生和一群電腦",
            "或是一群很吵的資工系學生和一堆酒",
            "是充滿歡樂與回憶的地方!",
            "在期末前打開任意一間資工系學生的房間，會看到一群卷鬼",
            "在final project期間逛逛新齋，會看到一群卷到日出的鬼",
            "是新鮮的肝逐漸失去價值的地方。"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/NEWicon.png");
        dialog.StartDialog("新齋", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inNEW = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void NEWScene::ReadMap() {
    std::string filename = std::string("Resource/NEWblock") + ".txt";

    // 清空舊的地圖數據
    mapData.clear();
    
    // 讀取地圖文件
    char c;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '+': mapData.push_back(TILE_WOOD); break;
            case '.': mapData.push_back(TILE_WALL); break;
            case '^': mapData.push_back(TABLE); break;
            case 'F': mapData.push_back(TILE_FLOOR); break;
            case '&': mapData.push_back(NEWMAP); break;
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
            std::string imagePath = "NEW/newfloor.png";
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
                case TILE_WALL:
                    imagePath = "NEW/newwall.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case TILE_FLOOR: {
                    imagePath = "NEW/newfloor.png";
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
                case TILE_WOOD:
                    imagePath = "NEW/woodfloor.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize, 
                                        BlockSize)
                    );
                    break;
                case NEWMAP:
                    imagePath = "NEW/NEWmap.png";
                    TileMapGroup->AddNewObject(
                        new Engine::Image(imagePath, 
                                        x * BlockSize, 
                                        y * BlockSize, 
                                        BlockSize * 30, 
                                        BlockSize * 16)
                    );
                    Engine::LOG(Engine::INFO) << x << ", " << y;
                case NOTHING:
                default:
                    continue;
            }
        }
    }
    std::string imagePath = "NEW/NEWmap.png";
    TileMapGroup->AddNewObject(
                            new Engine::Image(imagePath, 
                                            0 * BlockSize, 
                                            0 * BlockSize, 
                                            BlockSize * 30, 
                                            BlockSize * 16)
                        );
}

Engine::Point NEWScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool NEWScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
        case TILE_WOOD:
        case NEWMAP:
        case NOTHING:
            return true;
        case TILE_WALL:
        default:
            return false;
    }
}