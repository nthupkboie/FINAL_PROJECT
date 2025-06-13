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
#include "SmallEatScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "LogScene.hpp"
#include "UI/Component/Label.hpp"
#include "string"


const int SmallEatScene::MapWidth = 30, SmallEatScene::MapHeight = 16;
const int SmallEatScene::BlockSize = 64;

const int SmallEatScene::window_x = 30, SmallEatScene::window_y = 16;

std::vector<SmallEatScene::TileType> SmallEatScene::mapData;

// Engine::Point SmallEatScene::GetClientSize() {
//     return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
// }

Engine::Point SmallEatScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize); // 視角大小
}

Engine::Point SmallEatScene::cameraOffset = Engine::Point(0, 0);
void SmallEatScene::Initialize() {
    firstTime = true;
    // 初始化遊戲狀態
    lives = 3;
    //money = 0;
    //Shopper::canBuy = false;
    
    if (LogScene::money>=50 && !LogScene::haveAxe) canBuyAxe = true;
    else canBuyAxe = false;
    if (LogScene::money>=20) canBuySpeed = true;
    else canBuySpeed = false;

    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());      // 地圖圖層
    AddNewObject(PlayerGroup = new Group());       // 玩家角色
    AddNewObject(NPCGroup = new Group());
    AddNewObject(ShopperGroup = new Group());
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
                                            BlockSize * 5, BlockSize * 5,
                                            2, 3,  // 上 (第0列第2行)
                                            2, 0,  // 下
                                            2, 1,  // 左
                                            2, 2,  // 右
                                            64, 64)); // 圖塊大小

    NPC* Yang;
    auto YangAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    NPCGroup->AddNewObject(Yang = new NPC("Yang", YangAvatar, 
                                            "NPC/Yang/role/YangU.png",
                                            "NPC/Yang/role/YangD.png", 
                                            "NPC/Yang/role/YangL.png",
                                            "NPC/Yang/role/YangR.png",
                                            BlockSize * 8, BlockSize * 8
                                        ));
    
    //Shopper* axeman;
    auto axemanAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    ShopperGroup->AddNewObject(axeman = new Shopper("axeman", axemanAvatar, 
                                            "NPC/axeman/role/axemanU.png",
                                            "NPC/axeman/role/axemanD.png", 
                                            "NPC/axeman/role/axemanL.png",
                                            "NPC/axeman/role/axemanR.png",
                                            BlockSize * 9, BlockSize * 7
                                        ));

    Engine::Point axe0(BlockSize * 9 + BlockSize / 2, BlockSize * 7 + BlockSize / 2);
    Engine::Point axe1(BlockSize * 9 + BlockSize / 2, BlockSize * 11 + BlockSize / 2);
    Engine::Point axe2(BlockSize * 12 + BlockSize / 2, BlockSize * 11 + BlockSize / 2);
    Engine::Point axe3(BlockSize * 12 + BlockSize / 2, BlockSize * 7 + BlockSize / 2);

    axeman->AddPatrolPoint(axe0);
    axeman->AddPatrolPoint(axe1);
    axeman->AddPatrolPoint(axe2);
    axeman->AddPatrolPoint(axe3);

    auto LucyAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    ShopperGroup->AddNewObject(Lucy = new Shopper("Lucy醬", LucyAvatar, 
                                            "NPC/Lucy/role/lucyU.png",
                                            "NPC/Lucy/role/lucyD.png", 
                                            "NPC/Lucy/role/lucyL.png",
                                            "NPC/Lucy/role/lucyR.png",
                                            BlockSize * 25, BlockSize * 5
                                        ));

    // 初始化對話框
    dialog.Initialize();
    
    // 設置NPC的對話內容
    test->SetMessages({
        "你好，我是村民A！",
        "這個村莊最近不太平靜...",
        "晚上請小心行事。",
        "祝你好運，冒險者！",
        "Shawty had them Apple Bottom jeans, jeans"
    });

    Yang->SetMessages({
        "我是楊舜仁！",
        "我不會當人",
    });

    if (canBuyAxe){
        axeman->SetMessages({
            "這位相貌堂堂骨骼驚奇的客官，上好黑鐵斧頭要嗎?"
        });
    }
    else if (LogScene::haveAxe) {
        axeman->SetMessages({
            "你買過斧頭了，不要在這裡擋我拉客，走開!"
        });
    }
    else {
        axeman->SetMessages({
            "窮鬼滾"
        });
    }
    
    if (canBuySpeed){
        Lucy->SetMessages({
            "聽說Lucy時常做運動，身體健康精神好，跑步神速",
            "她其實有在嗑藥。",
            "要來杯好康的獨門配方嗎?"
        });
    }
    else if (LogScene::haveSpeedUp){
        Lucy->SetMessages({
            "哎呀，是熟客呀!",
            "今天這批很爽喔喔喔喔喔!!!!!(吸",
            "TIGHT TIGHT TIGHT YEAAAAH"
        });
    }
    else {
        Lucy->SetMessages({
            "聽說Lucy不喜歡沒錢的人",
            "滾，現在。",
        });
    }
    
    // 預載資源
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    
    // 開始背景音樂
    bgmId = AudioHelper::PlayBGM("XCB.ogg");

    //Lucy->canBuy = axeman->canBuy = false;

    

    //道具
    LabelGroup->AddNewObject(moneyLabel = new Engine::Label(std::to_string(LogScene::money), "title.ttf", 48, 130, 70, 255, 255, 255, 255, 0.5, 0.5));
    LabelGroup->AddNewObject(moneyImage = new Engine::Image("play/dollar.png", 20, 35, 56, 56));
    if (LogScene::haveAxe) LabelGroup->AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
    if (LogScene::haveSpeedUp){
        LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
        //haveSpeedUpInt = rou
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    }
}

void SmallEatScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void SmallEatScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    // if (justBuyedAxe) {
    //     LogScene::money -= 50;
    //     justBuyedAxe = false;
    // }
    // if (justBuyedSpeed){
    //     LogScene::money -= 20;
    //     justBuyedSpeed = false;
    // }

    //moneyLabel->Text = std::to_string(LogScene::money);
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

    // for (auto& obj : ShopperGroup->GetObjects()) {
    //     if (auto npc = dynamic_cast<Shopper*>(obj)) {
    //         npc->Update(deltaTime, player);
    //     }
    // }

    // // 更新對話框
    // if (dialog.IsDialogActive()) {
    //     dialog.Update(deltaTime);
    // }

    Shopper* currentShopper = nullptr;
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->Update(deltaTime, player);
            if (shopper->IsTalking() || shopper->canBuy) {
                currentShopper = shopper;
            }
        }
    }

    if (dialog.IsDialogActive()) {
        dialog.Update(deltaTime);
    }
    // } else if (currentShopper && currentShopper->IsTalking()) {
    //     // 若場景的 dialog 未啟動，但 Shopper 認為在對話，同步關閉
    //     currentShopper->isTalking = false;
    //     currentShopper->canBuy = true;
    //     Engine::LOG(Engine::INFO) << "Sync: Shopper '" << currentShopper->GetName() << "' dialog ended";
    // }

    // 檢查遊戲結束條件
    // if (lives <= 0) {
    //     Engine::GameEngine::GetInstance().ChangeScene("lose");
    // }
    //if (LogScene::money>=50 && !LogScene::haveAxe) canBuyAxe = true;
    if (LogScene::haveAxe && canBuyAxe){
        canBuyAxe = false;
        axeman->SetMessages({
            "你買過斧頭了，不要在這裡擋我拉客，走開!"
        });
    }
    if (LogScene::money<20 && canBuySpeed){
        canBuySpeed = false;
        Lucy->SetMessages({
            "聽說Lucy不喜歡沒錢的人",
            "滾，現在。",
        });
    }
    else if (LogScene::money>=20 && LogScene::haveSpeedUp == 1){
        Lucy->SetMessages({
            "哎呀，是熟客呀!",
            "今天這批很爽喔喔喔喔喔!!!!!(吸",
            "TIGHT TIGHT TIGHT YEAAAAH"
        });
    }

    if (currentShopper && !currentShopper->IsTalking()) Engine::LOG(Engine::WARN) << "!currentShopper->IsTalking()";
    if (!showShopButtons && !dialog.IsDialogActive() && currentShopper && currentShopper->canBuy) {
        Engine::LOG(Engine::WARN) << "Shop buttons displayed";
        // 移除舊按鈕
        if (buyButton) {
            RemoveObject(buyButton->GetObjectIterator());
            buyButton = nullptr;
        }
        if (cancelButton) {
            RemoveObject(cancelButton->GetObjectIterator());
            cancelButton = nullptr;
        }
        if (buyLabel) {
            RemoveObject(buyLabel->GetObjectIterator());
            buyLabel = nullptr;
        }
        if (cancelLabel) {
            RemoveObject(cancelLabel->GetObjectIterator());
            cancelLabel = nullptr;
        }

        if (currentShopper == axeman && canBuyAxe && axeman->canBuy) {
            readyToBuyAxe = true;
            Engine::LOG(Engine::INFO) << "Axe shop buttons created";
            buyButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 400, 300, 800, 100);
            buyButton->SetOnClickCallback(std::bind(&SmallEatScene::BuyOnClick, this, AXE));
            AddNewControlObject(buyButton);
            buyLabel = new Engine::Label("要要要", "title.ttf", 48, 960, 360, 0, 0, 0, 255, 0.5, 0.5);
            AddNewObject(buyLabel);

            cancelButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 400, 500, 800, 100);
            cancelButton->SetOnClickCallback(std::bind(&SmallEatScene::CancelOnClick, this));
            AddNewControlObject(cancelButton);
            cancelLabel = new Engine::Label("先不要", "title.ttf", 48, 960, 560, 0, 0, 0, 255, 0.5, 0.5);
            AddNewObject(cancelLabel);

            showShopButtons = true;
            axeman->canBuy = false;
        } else if (currentShopper == Lucy && canBuySpeed && Lucy->canBuy) {
            readyToBuySpeed = true;
            Engine::LOG(Engine::INFO) << "Speed shop buttons created";
            buyButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 400, 300, 800, 100);
            buyButton->SetOnClickCallback(std::bind(&SmallEatScene::BuyOnClick, this, SPEED));
            AddNewControlObject(buyButton);
            buyLabel = new Engine::Label("好...好啊", "title.ttf", 48, 960, 360, 0, 0, 0, 255, 0.5, 0.5);
            AddNewObject(buyLabel);

            cancelButton = new Engine::ImageButton("stage-select/full_1.png", "stage-select/full_1.png", 960 - 400, 500, 800, 100);
            cancelButton->SetOnClickCallback(std::bind(&SmallEatScene::CancelOnClick, this));
            AddNewControlObject(cancelButton);
            cancelLabel = new Engine::Label("先不要", "title.ttf", 48, 960, 560, 0, 0, 0, 255, 0.5, 0.5);
            AddNewObject(cancelLabel);

            showShopButtons = true;
            Lucy->canBuy = false;
        }
    }
}

void SmallEatScene::Draw() const {
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

void SmallEatScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void SmallEatScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void SmallEatScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void SmallEatScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }
    
    if(keyCode == ALLEGRO_KEY_B){
        Engine::GameEngine::GetInstance().ChangeScene("battle");
    }
    
    if(firstTime){
        std::vector<std::string> testMessages = {
        "小吃部建築由本校校友張昌華設計。",
        "事實上，校園內許多建築都出自大師之手，包括原子爐、百齡堂(今水漾餐廳)、台北月涵堂等。他的作品外觀多呈簡潔的白色，且擁有明亮的大窗景。",
        "🍙小吃部的前身為「大餐廳」，一開始只在正餐時間營業，供應自助餐。",
        "核工碩79級校友、本校財規室李敏主任表示，他印象中的大餐廳擺設很簡單，就是一排排整齊的長桌。",
        "而且當年的自助餐不是自己夾菜，而是裝好一碟一碟的，想吃什麼就拿一盤到自己的托盤上。",
        "「吃得超飽只要8到10元！」李敏笑說，物價跟現在真的差很多呢。",
        "大餐廳的形式維持了近20年，直到1983年胡德總務長及學生代表討論後，改成提供多樣化餐點的「小吃部」，營業時間也更加彈性，讓同學即使錯過正餐時間也有「小吃」可以吃，當時可是全國大學首創的「攤位式餐飲」。"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
        dialog.StartDialog("小吃部", testAvatar, testMessages);

        firstTime = false;
    }
    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = {
        "小吃部建築由本校校友張昌華設計。",
        "事實上，校園內許多建築都出自大師之手，包括原子爐、百齡堂(今水漾餐廳)、台北月涵堂等。他的作品外觀多呈簡潔的白色，且擁有明亮的大窗景。",
        "🍙小吃部的前身為「大餐廳」，一開始只在正餐時間營業，供應自助餐。",
        "核工碩79級校友、本校財規室李敏主任表示，他印象中的大餐廳擺設很簡單，就是一排排整齊的長桌。",
        "而且當年的自助餐不是自己夾菜，而是裝好一碟一碟的，想吃什麼就拿一盤到自己的托盤上。",
        "「吃得超飽只要8到10元！」李敏笑說，物價跟現在真的差很多呢。",
        "大餐廳的形式維持了近20年，直到1983年胡德總務長及學生代表討論後，改成提供多樣化餐點的「小吃部」，營業時間也更加彈性，讓同學即使錯過正餐時間也有「小吃」可以吃，當時可是全國大學首創的「攤位式餐飲」。"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
        dialog.StartDialog("小吃部", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inSmallEat = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void SmallEatScene::ReadMap() {
    std::string filename = std::string("Resource/smalleat") + ".txt";

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
                case NOTHING:
                default:
                    continue;
            }
        }
    }
}

Engine::Point SmallEatScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}

bool SmallEatScene::collision(int x, int y){
    switch(mapData[y/BlockSize * MapWidth + x / BlockSize]){
        case TILE_FLOOR:
            return true;
        case TILE_WALL:
        case NOTHING:
        default:
            return false;
    }
}

void SmallEatScene::BuyOnClick(int item) {
    // if (LogScene::money >= 50) {
    //     LogScene::money -= 50;
    //     items++;
    //     Engine::LOG(Engine::WARN) << "Purchased item, money: " << LogScene::money << ", items: " << items;
    // } else {
    //     Engine::LOG(Engine::WARN) << "Not enough money to buy item";
    // }
    // 防止多次觸發
    if (!showShopButtons) return;
    // 移除按鈕
    if (buyButton) {
        RemoveObject(buyButton->GetObjectIterator());
        buyButton = nullptr;
        RemoveObject(buyLabel->GetObjectIterator());
        buyLabel = nullptr;
    }
    if (cancelButton) {
        RemoveObject(cancelButton->GetObjectIterator());
        cancelButton = nullptr;
        RemoveObject(cancelLabel->GetObjectIterator());
        cancelLabel = nullptr;
    }

    Engine::LOG(Engine::WARN) << "ITEM: " << item;

    showShopButtons = false;
    //Shopper::canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    

    if (readyToBuyAxe == true){
        if (!axeImage) AddNewObject(axeImage = new Engine::Image("stage-select/axe.png", 20, 105, 56, 56));
        LogScene::haveAxe = true;
        //Engine::LOG(Engine::WARN) << "axeeeeeeee";
        LogScene::money -= 50;
        axeman->canBuy = false;
        readyToBuyAxe = false;
        //justBuyedAxe = true;
    }
    else if (readyToBuySpeed == true){
        //Engine::LOG(Engine::WARN) << "Purchased";
        LogScene::haveSpeedUp+=1;
        LogScene::money -= 20;
        readyToBuySpeed = false;
        //justBuyedSpeed = true;
        Lucy->canBuy = false;
        //更新speed
        if (LogScene::haveSpeedUp == 1){
            LabelGroup->AddNewObject(speedImage = new Engine::Image("play/potion.png", 20, 175, 56, 56));
            LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
        } 
    }


    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }
    moneyLabel->Text = std::to_string(LogScene::money);
    if (LogScene::haveSpeedUp) speedLabel->Text = std::to_string((int)LogScene::haveSpeedUp);
}

void SmallEatScene::CancelOnClick() {
    //Engine::LOG(Engine::WARN) << "Purchase cancelled";
    
    // 移除按鈕
    if (buyButton) {
        RemoveObject(buyButton->GetObjectIterator());
        buyButton = nullptr;
        RemoveObject(buyLabel->GetObjectIterator());
        buyLabel = nullptr;
    }
    if (cancelButton) {
        RemoveObject(cancelButton->GetObjectIterator());
        cancelButton = nullptr;
        RemoveObject(cancelLabel->GetObjectIterator());
        cancelLabel = nullptr;
    }
    showShopButtons = false;
    Lucy->canBuy = false;
    axeman->canBuy = false;
    //Shopper::isTalking = false;
    NPCDialog::talking = false;
    // 重置所有 Shopper 的 canBuy
    for (auto& obj : ShopperGroup->GetObjects()) {
        if (auto shopper = dynamic_cast<Shopper*>(obj)) {
            shopper->canBuy = false;
        }
    }
}