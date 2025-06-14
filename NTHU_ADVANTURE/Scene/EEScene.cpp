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
#include "EEScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"
#include "ScoreboardScene.hpp"

const int EEScene::MapWidth = 30, EEScene::MapHeight = 16;
const int EEScene::BlockSize = 64;

const int EEScene::window_x = 30, EEScene::window_y = 16;

std::vector<EEScene::TileType> EEScene::mapData;

// Engine::Point EEScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point EEScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point EEScene::cameraOffset = Engine::Point(0, 0);
void EEScene::Initialize() {
    firstTime = true;
    // 初始化遊戲狀態
    lives = 3;
    money = 0;
    
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    AddNewObject(LabelGroup = new Group());
    AddNewObject(ShopperGroup = new Group());
    
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
    //NPC* test;
    // sheet路徑, x, y, 
    // 上, 下, 左, 右, (先行在列)
    // 圖塊寬, 圖塊高
    

    // NPC* Yang;
    // auto YangAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    // NPCGroup->AddNewObject(Yang = new NPC("Yang", YangAvatar, 
    //                                         "NPC/Yang/role/YangU.png",
    //                                         "NPC/Yang/role/YangD.png", 
    //                                         "NPC/Yang/role/YangL.png",
    //                                         "NPC/Yang/role/YangR.png",
    //                                         BlockSize * 8, BlockSize * 8
    //                                     ));

    auto YangAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang.png");
    ShopperGroup->AddNewObject(yang = new Shopper("乂卍煞氣a楊舜仁卍乂", YangAvatar, 
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
    //if (LogScene::clearedLake){
        yang->SetMessages({
            "同學，既然你會出現在這裡，就代表你已經準備接受我 乂卍煞氣a楊舜仁卍乂 的制裁了!!",
            "接下來這幾題，全對才能活著走出這裡!!",
            "否則你將被永遠困在期末project地獄，每天寫程式到天亮!!!",
            "第一題，double佔幾個byte? (A) 2 (B) 4 (c) 8763 (D) 8",
        });
    //}
    // else {
    //     yang->SetMessages({
    //         "先去練練再來吧",
    //     });
    // }
    

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

void EEScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void EEScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    if(firstTime){
        std::vector<std::string> testMessages = {
            "聽說門口的白色鏤空立方體從不同角度可看到EE及CS",
            "不過資電館對資工系來說，就是一座監獄。",
            "真希望下次上機能夠破台嗚嗚嗚嗚"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/EEicon.png");
        dialog.StartDialog("資電館", testAvatar, testMessages);

        firstTime = false;
    }
    
    // 獲取玩家對象
    Player* player = nullptr;
    for (auto& obj : PlayerGroup->GetObjects()) {
        player = dynamic_cast<Player*>(obj);
        if (player) break;
    }

    //更新shopper
    Shopper* currentShopper = nullptr;
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->Update(deltaTime, player);
            if (shopper->IsTalking() || shopper->canBuy) {
                currentShopper = shopper;
                //Engine::LOG(Engine::WARN) << "Yang shun ren displayed";
            }
        }
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

    if (/*LogScene::clearedLake && */yang->canBuy && index < 4) {
        Engine::LOG(Engine::WARN) << "Shop buttons displayed";
        // 移除舊按鈕
        if (oneButton) {
            RemoveObject(oneButton->GetObjectIterator());
            oneButton = nullptr;
            RemoveObject(oneLabel->GetObjectIterator());
            oneLabel = nullptr;

            RemoveObject(twoButton->GetObjectIterator());
            twoButton = nullptr;
            RemoveObject(twoLabel->GetObjectIterator());
            twoLabel = nullptr;
            
            RemoveObject(threeButton->GetObjectIterator());
            threeButton = nullptr;
            RemoveObject(threeLabel->GetObjectIterator());
            threeLabel = nullptr;

            RemoveObject(fourButton->GetObjectIterator());
            fourButton = nullptr;
            RemoveObject(fourLabel->GetObjectIterator());
            fourLabel = nullptr;
        }
        

        //if (currentShopper == yang && yang->canBuy) {
            //readyToBuyAxe = true;
        //int pingyi = 50;
        Engine::LOG(Engine::INFO) << "Yang shop buttons created";
        oneButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 350, 400, 100, 100);
        oneButton->SetOnClickCallback(std::bind(&EEScene::oneOnClick, this));
        AddNewControlObject(oneButton);
        oneLabel = new Engine::Label("A", "title.ttf", 48, 960 - 298, 460, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(oneLabel);

        twoButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 150, 400, 100, 100);
        twoButton->SetOnClickCallback(std::bind(&EEScene::twoOnClick, this));
        AddNewControlObject(twoButton);
        twoLabel = new Engine::Label("B", "title.ttf", 48, 960 - 298 + 200, 460, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(twoLabel);

        threeButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960  + 50, 400, 100, 100);
        threeButton->SetOnClickCallback(std::bind(&EEScene::threeOnClick, this));
        AddNewControlObject(threeButton);
        threeLabel = new Engine::Label("C", "title.ttf", 48, 960 - 298 + 400, 460, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(threeLabel);

        fourButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 + 250, 400, 100, 100);
        fourButton->SetOnClickCallback(std::bind(&EEScene::fourOnClick, this));
        AddNewControlObject(fourButton);
        fourLabel = new Engine::Label("D", "title.ttf", 48, 960+ -298 + 600, 460, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(fourLabel);

        showShopButtons = true;
        yang->canBuy = false;
        index++;
        //}
    }
    //Engine::LOG(Engine::INFO) << LogScene::timer;
    if (yang->canBuy && index == 4) {
        ScoreboardScene::AddScore(LogScene::myName, LogScene::timer);
        Engine::GameEngine::GetInstance().ChangeScene("win");
    }
    else if (yang->canBuy && index == 5) Engine::GameEngine::GetInstance().ChangeScene("lose");
}

void EEScene::Draw() const {
    //IScene::Draw();

    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);
    
    
    TileMapGroup->Draw();
    PlayerGroup->Draw();
    NPCGroup->Draw();
    LabelGroup->Draw();
    ShopperGroup->Draw();
    IScene::Draw();
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

void EEScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void EEScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void EEScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void EEScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }
    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = {
            "聽說門口的白色鏤空立方體從不同角度可看到EE及CS",
            "不過資電館對資工系來說，就是一座監獄。",
            "真希望下次上機能夠破台嗚嗚嗚嗚"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/EEicon.png");
        dialog.StartDialog("資電館", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inEE = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void EEScene::ReadMap() {
    std::string filename = std::string("Resource/EE") + ".txt";

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

Engine::Point EEScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool EEScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
            return true;
        case TILE_WALL:
        case NOTHING:
        default:
            return false;
    }
}

void EEScene::oneOnClick() {
    
    if (oneButton) {
        RemoveObject(oneButton->GetObjectIterator());
        oneButton = nullptr;
        RemoveObject(oneLabel->GetObjectIterator());
        oneLabel = nullptr;

        RemoveObject(twoButton->GetObjectIterator());
        twoButton = nullptr;
        RemoveObject(twoLabel->GetObjectIterator());
        twoLabel = nullptr;
        
        RemoveObject(threeButton->GetObjectIterator());
        threeButton = nullptr;
        RemoveObject(threeLabel->GetObjectIterator());
        threeLabel = nullptr;

        RemoveObject(fourButton->GetObjectIterator());
        fourButton = nullptr;
        RemoveObject(fourLabel->GetObjectIterator());
        fourLabel = nullptr;
    }
    if (answer[index] != 'A') index = 5;
    

    //Engine::LOG(Engine::WARN) << "ITEM: " << item;

    showShopButtons = false;
    //Shopper::canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    

    // if (readyToBuyAxe == true){
    //     if (!axeImage) AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    //     LogScene::haveAxe = true;
    //     //Engine::LOG(Engine::WARN) << "axeeeeeeee";
    //     LogScene::money -= 50;
    //     axeman->canBuy = false;
    //     readyToBuyAxe = false;
    //     //justBuyedAxe = true;
    // }

    // else if (readyToBuySpeed == true){
    //     //Engine::LOG(Engine::WARN) << "Purchased";
    //     LogScene::haveSpeedUp+=1;
    //     LogScene::money -= 20;
    //     readyToBuySpeed = false;
    //     //justBuyedSpeed = true;
    //     Lucy->canBuy = false;
    //     //更新speed
    //     if (LogScene::haveSpeedUp == 1){
    //         LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
    //         LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    //     } 
    // }


    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }

    openingDialog();
    //moneyLabel->Text = std::to_string(LogScene::money);
    //if (LogScene::haveSpeedUp) speedLabel->Text = std::to_string((int)LogScene::haveSpeedUp);
}

void EEScene::twoOnClick() {
    //Engine::LOG(Engine::WARN) << "Purchase cancelled";
    
    // 移除按鈕
    if (oneButton) {
        RemoveObject(oneButton->GetObjectIterator());
        oneButton = nullptr;
        RemoveObject(oneLabel->GetObjectIterator());
        oneLabel = nullptr;

        RemoveObject(twoButton->GetObjectIterator());
        twoButton = nullptr;
        RemoveObject(twoLabel->GetObjectIterator());
        twoLabel = nullptr;
        
        RemoveObject(threeButton->GetObjectIterator());
        threeButton = nullptr;
        RemoveObject(threeLabel->GetObjectIterator());
        threeLabel = nullptr;

        RemoveObject(fourButton->GetObjectIterator());
        fourButton = nullptr;
        RemoveObject(fourLabel->GetObjectIterator());
        fourLabel = nullptr;
    }
    if (answer[index] != 'B') index = 5;
    showShopButtons = false;
    //Lucy->canBuy = false;
    //axeman->canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    // 重置所有 Shopper 的 canBuy
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }
    openingDialog();
}
void EEScene::threeOnClick() {
    //Engine::LOG(Engine::WARN) << "Purchase cancelled";
    
    // 移除按鈕
    if (oneButton) {
        RemoveObject(oneButton->GetObjectIterator());
        oneButton = nullptr;
        RemoveObject(oneLabel->GetObjectIterator());
        oneLabel = nullptr;

        RemoveObject(twoButton->GetObjectIterator());
        twoButton = nullptr;
        RemoveObject(twoLabel->GetObjectIterator());
        twoLabel = nullptr;
        
        RemoveObject(threeButton->GetObjectIterator());
        threeButton = nullptr;
        RemoveObject(threeLabel->GetObjectIterator());
        threeLabel = nullptr;

        RemoveObject(fourButton->GetObjectIterator());
        fourButton = nullptr;
        RemoveObject(fourLabel->GetObjectIterator());
        fourLabel = nullptr;
    }
    if (answer[index] != 'C') index = 5;
    showShopButtons = false;
    //Lucy->canBuy = false;
    //axeman->canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    // 重置所有 Shopper 的 canBuy
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }
    openingDialog();
}
void EEScene::fourOnClick() {
    //Engine::LOG(Engine::WARN) << "Purchase cancelled";
    
    // 移除按鈕
    if (oneButton) {
        RemoveObject(oneButton->GetObjectIterator());
        oneButton = nullptr;
        RemoveObject(oneLabel->GetObjectIterator());
        oneLabel = nullptr;

        RemoveObject(twoButton->GetObjectIterator());
        twoButton = nullptr;
        RemoveObject(twoLabel->GetObjectIterator());
        twoLabel = nullptr;
        
        RemoveObject(threeButton->GetObjectIterator());
        threeButton = nullptr;
        RemoveObject(threeLabel->GetObjectIterator());
        threeLabel = nullptr;

        RemoveObject(fourButton->GetObjectIterator());
        fourButton = nullptr;
        RemoveObject(fourLabel->GetObjectIterator());
        fourLabel = nullptr;
    }
    if (answer[index] != 'D') index = 5;
    showShopButtons = false;
    //Lucy->canBuy = false;
    //axeman->canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    // 重置所有 Shopper 的 canBuy
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }
    openingDialog();
}

void EEScene::openingDialog()
{
    //yang->canBuy = true;
    if (index == 0){
        yang->SetMessages({
            "同學不錯嘛，繼續接招!!",
        });
    }
    else if (index == 1){
        yang->npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang2.png");
        yang->SetMessages({
            "哼哼，看來戰鬥開始變有趣了www",
        });
    }
    else if (index == 2){
        yang->npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang3.png");
        yang->SetMessages({
            "嘛，看來這天還是到了，總算遇到像樣的敵人(歪笑)",
        });
    }
    else if (index == 3){
        yang->npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang4.png");
        std::string tmp = "可惡...我絕對不會讓你活著踏出這裡!!!看來只好使出那招了!!!自盡吧，" + LogScene::myName;
        yang->SetMessages({
            tmp,
        });
    }
    else if (index == 4){
        yang->npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang5.png");
        yang->SetMessages({
            "沒...沒想到我乂卍煞氣a楊舜仁卍乂竟然敗給這種小摟摟...呃啊(吐血)(癱倒)(伸手向太陽)(陰暗爬行)(失去意識)",
        });
    }
    else if (index == 5){
        yang->npcAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Yang/avatar/Yang.png");
        yang->SetMessages({
            "唔姆.. 你輸了 (茶",
            "去死吧 wwwww (咳咳",
        });
    }

    yang->dialog.StartDialog(yang->GetName(), yang->npcAvatar, yang->messages);
    yang->enterWasDown = true;
    yang->isTalking = true;
}

