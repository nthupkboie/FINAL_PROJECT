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

    // // NPC
    // NPC* test;
    // // sheet路徑, x, y, 
    // // 上, 下, 左, 右, (先行在列)
    // // 圖塊寬, 圖塊高
    // auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/test_avatar.png");
    // NPCGroup->AddNewObject(test = new NPC("NPC",testAvatar, "NPC/test/role/test_sheet.png",
    //                                         BlockSize * 5, BlockSize * 5,
    //                                         2, 3,  // 上 (第0列第2行)
    //                                         2, 0,  // 下
    //                                         2, 1,  // 左
    //                                         2, 2,  // 右
    //                                         64, 64)); // 圖塊大小

    NPC* Yang;
    auto YangAvatar = Engine::Resources::GetInstance().GetBitmap("cool/foriegn.png");
    NPCGroup->AddNewObject(Yang = new NPC("foreign", YangAvatar, 
                                            "NPC/Yang/role/YangU.png",
                                            "NPC/Yang/role/YangD.png", 
                                            "NPC/Yang/role/YangL.png",
                                            "NPC/Yang/role/YangR.png",
                                            BlockSize * 8, BlockSize * 8
                                        ));
    
    //Shopper* axeman;
    auto axemanAvatar = Engine::Resources::GetInstance().GetBitmap("cool/old.png");
    ShopperGroup->AddNewObject(axeman = new Shopper("axeman", axemanAvatar, 
                                            "NPC/axeman/role/axemanU.png",
                                            "NPC/axeman/role/axemanD.png",
                                            "NPC/axeman/role/axemanL.png",
                                            "NPC/axeman/role/axemanR.png",
                                            BlockSize * 11, BlockSize * 8
                                        ));

    Engine::Point axe0(BlockSize * 11 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);
    Engine::Point axe1(BlockSize * 11 + BlockSize / 2, BlockSize * 13 + BlockSize / 2);
    Engine::Point axe2(BlockSize * 14 + BlockSize / 2, BlockSize * 13 + BlockSize / 2);
    Engine::Point axe3(BlockSize * 14 + BlockSize / 2, BlockSize * 8 + BlockSize / 2);

    axeman->AddPatrolPoint(axe0);
    axeman->AddPatrolPoint(axe1);
    axeman->AddPatrolPoint(axe2);
    axeman->AddPatrolPoint(axe3);

    auto LucyAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/Lucy/avatar/lucyAvatar.png");
    ShopperGroup->AddNewObject(Lucy = new Shopper("Lucy醬", LucyAvatar, 
                                            "NPC/Lucy/role/lucyU.png",
                                            "NPC/Lucy/role/lucyD.png", 
                                            "NPC/Lucy/role/lucyL.png",
                                            "NPC/Lucy/role/lucyR.png",
                                            BlockSize * 17, BlockSize * 7
                                        ));

    // 初始化對話框
    dialog.Initialize();
    
    Yang->SetMessages({
        "Who are you??????",
        "I am a foreigner",
        "and I am handsome ahahhahahahahah",
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
        LabelGroup->AddNewObject(speedLabel = new Engine::Label(std::to_string((int)LogScene::haveSpeedUp), "title.ttf", 48, 130, 210, 255, 255, 255, 255, 0.5, 0.5));
    }

    //建築
    AddBuildingZone(2, 2, 2, 2, "主地圖");
}

void SmallEatScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void SmallEatScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
   if(firstTime){
        std::vector<std::string> testMessages = { 
            "小吃部有新竹美食麥當勞、還不錯的蘇記、很賺的seven",
            "食物不重要 有插頭的座位才是本體",
            "玻璃屋也是個很讚的地方，猜猜是誰蓋的? 是顏東勇喔^^"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/smalleaticon.png");
        dialog.StartDialog("小吃部", testAvatar, testMessages);

        firstTime = false;
    }

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
    
    if (keyCode == ALLEGRO_KEY_I) {
        std::vector<std::string> testMessages = { 
            "小吃部有新竹美食麥當勞、還不錯的蘇記、很賺的seven",
            "食物不重要 有插頭的座位才是本體",
            "玻璃屋也是個很讚的地方，猜猜是誰蓋的? 是顏東勇喔^^"
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("NPC/test/avatar/smalleaticon.png");
        dialog.StartDialog("小吃部", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inSmallEat = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
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
        //if (player)  LogScene::lastPlayerPos = player->Position;
        
        for (const auto& zone : buildingZones) {
            if (IsPlayerNearBuilding(player, zone)) {
                std::cout << "Entering " << zone.buildingName << "!" << std::endl;

                if (zone.buildingName == "主地圖") {
                    Engine::GameEngine::GetInstance().ChangeScene("play");
                    PlayScene::inPlay = true;//記得改
                    PlayScene::inSmallEat = false;
                }
                

                break;  // 找到一個就跳出，不需要檢查更多建築
            }
        }
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
            case 'G': mapData.push_back(MC); break;
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
                    // imagePath = "cool/mc.png";
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
                case MC:
                    imagePath = "cool/mc.png";
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

    readyToBuyAxe = readyToBuySpeed = false;
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
    readyToBuyAxe = readyToBuySpeed = false;
}

 
bool SmallEatScene::IsPlayerNearBuilding(Player* player, const BuildingZone& zone) {
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

void SmallEatScene::ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY) {
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

void SmallEatScene::AddBuildingZone(int x, int y, int width, int height, const std::string& buildingName) {
    // 設置建築物的範圍區域並儲存
    buildingZones.push_back(BuildingZone{x, y, width, height, buildingName});
}

