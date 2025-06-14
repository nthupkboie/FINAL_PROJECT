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
#include "DialogScene.hpp"
#include "PlayScene.hpp"
#include "Player/Player.hpp"
#include "NPC/NPC.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "LogScene.hpp"
#include <cmath>

const int DialogScene::MapWidth = 30, DialogScene::MapHeight = 16;
const int DialogScene::BlockSize = 64;
const int DialogScene::window_x = 30, DialogScene::window_y = 16;

Engine::Point DialogScene::GetClientSize() {
    return Engine::Point(window_x * BlockSize, window_y * BlockSize);
}

Engine::Point DialogScene::cameraOffset = Engine::Point(0, 0);
void DialogScene::Initialize() {
    firstTime = true;
    // 添加渲染群組
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(NPCGroup = new Group());
    AddNewObject(LabelGroup = new Group());
    
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    // 初始化對話框
    dialog.Initialize();
    
    // 開始背景音樂
    // bgmId = AudioHelper::PlayBGM("waterwood.ogg");
}

void DialogScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void DialogScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    LogScene::timer += deltaTime;
    
    // 更新攝影機，直接設置偏移量
    cameraOffset.x = std::max(0.0f, std::min(cameraOffset.x, static_cast<float>(MapWidth * BlockSize - window_x * BlockSize)));
    cameraOffset.y = std::max(0.0f, std::min(cameraOffset.y, static_cast<float>(MapHeight * BlockSize - window_y * BlockSize)));

    // 更新對話框
    if (dialog.IsDialogActive()) {
        dialog.Update(deltaTime);
    }
}

void DialogScene::Draw() const {
    //IScene::Draw();
    al_clear_to_color(al_map_rgb(0, 0, 0));

    ALLEGRO_TRANSFORM transform;
    al_copy_transform(&transform, al_get_current_transform());
    al_translate_transform(&transform, -cameraOffset.x, -cameraOffset.y);
    al_use_transform(&transform);

    TileMapGroup->Draw();
    NPCGroup->Draw();
    LabelGroup->Draw();

    al_identity_transform(&transform);
    al_use_transform(&transform);

    if (dialog.IsDialogActive()) {
        dialog.Draw();
    }
}

void DialogScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}

void DialogScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
}

void DialogScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
}

void DialogScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    
    // 按Enter鍵推進對話
    if (keyCode == ALLEGRO_KEY_ENTER && dialog.IsDialogActive()) {
        dialog.AdvanceDialog();
    }

    if (keyCode == ALLEGRO_KEY_1) {
        std::vector<std::string> testMessages = {
            "這是哪裡 我在哪?",
            "為甚麼眼睛睜不開 好暗",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("cool/me.png");
        dialog.StartDialog("???" , testAvatar, testMessages);
    }

    if (keyCode == ALLEGRO_KEY_2) {
        std::vector<std::string> testMessages = {
            "誒 " + LogScene::myName + "! 該起床了吧",
            "都睡多久了 整個路上你都在睡覺 也太誇張!!!!",
            "我們快到清大了喔!! " + LogScene::myName + " 起床!!!!", 
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("cool/dad.png");
        dialog.StartDialog("疑似是爸爸?", testAvatar, testMessages);
    }

    if (keyCode == ALLEGRO_KEY_3) {
        std::vector<std::string> testMessages = {
            "我是...? " + LogScene::myName,
            "喔對誒 那時候考完分科 隨便填一填好像上了清大 不知道甚麼系",
            "啊!!! 是上清大資工!!!",
            "喔乾 我要去新竹怪怪地方上學",
            "吼 想到就好煩 歐不不不不不不不不不",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("cool/me.png");
        dialog.StartDialog(LogScene::myName , testAvatar, testMessages);
    }

    if (keyCode == ALLEGRO_KEY_4) {
        std::vector<std::string> testMessages = {
            "誒 " + LogScene::myName + "!!!",
            "我們到了",
            "傳說中美食沙漠、要一直走路腳走到酸、每天都要爬坡的超級清華大學",
            "準備要下車了喔",
            "爸爸有事情要忙先走了 剩下的 " + LogScene::myName + "你就好好在清大生存下去吧 哈哈哈哈哈哈哈哈哈哈哈",
            "誒你先進去新齋認識些新朋友",
            "然後你等等好像要去台達館上課嗎?",
            "不知道啦 你先進去 認識朋友 不然我怕你便孤兒",
            "爸爸走囉 掰掰掰掰",
        };
        auto testAvatar = Engine::Resources::GetInstance().GetBitmap("cool/dad.png");
        dialog.StartDialog("爸爸", testAvatar, testMessages);
    }

    if(keyCode == ALLEGRO_KEY_P){
        PlayScene::inPlay = true;
        PlayScene::inWaterWood = false;
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

Engine::Point DialogScene::getCamera(){
    return Engine::Point(cameraOffset.x + 5 * BlockSize, cameraOffset.y + 2.5 * BlockSize);
}