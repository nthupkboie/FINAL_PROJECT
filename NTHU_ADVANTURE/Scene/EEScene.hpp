#pragma once
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

#include "NPC/NPCDialog.hpp"
#include "NPC/Shopper.hpp"
#include "UI/Component/ImageButton.hpp"

class Turret;
namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
}   // namespace Engine

class EEScene final : public Engine::IScene {
private:
    
    
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

    NPCDialog dialog;  // 添加對話框成員

    // 攝影機偏移量
    static Engine::Point cameraOffset;

    Engine::Label* moneyLabel;
    Engine::Image* moneyImage;
    Engine::Image* axeImage = nullptr;
    Engine::Image* speedImage = nullptr;
    Engine::Label* speedLabel;

    bool showShopButtons = false; // 新增：控制購買按鈕顯示
    Engine::ImageButton* oneButton = nullptr; // 新增：購買按鈕
    Engine::ImageButton* twoButton = nullptr; // 新增：取消按鈕
    Engine::ImageButton* threeButton = nullptr;
    Engine::ImageButton* fourButton = nullptr;
    Engine::Label* oneLabel;
    Engine::Label* twoLabel;
    Engine::Label* threeLabel;
    Engine::Label* fourLabel;
    Shopper* yang = nullptr; // 新增：NPC 引用

    int index = -1;
    char answer[5] = {'B', 'D', 'C', 'A', 'D'};
    std::shared_ptr<ALLEGRO_BITMAP> bmp1;
protected:
    int lives;
    int money;
    int SpeedMult;

public:
    enum TileType {
        TILE_WALL,
        TILE_FLOOR,
        TABLE,
        NOTHING,
        EE,
    };

    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const std::vector<int> code;
    Group *TileMapGroup;


    // new add
    Group *PlayerGroup;
    Group *LabelGroup;
    Group *NPCGroup;
    Group *ShopperGroup;

    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<int> keyStrokes;

    ALLEGRO_KEYBOARD_STATE keyboardState; // new add

    static Engine::Point GetClientSize();
    explicit EEScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnMouseDown(int button, int mx, int my) override;
    void OnMouseMove(int mx, int my) override;
    void OnMouseUp(int button, int mx, int my) override;
    void OnKeyDown(int keyCode) override;
    void ReadMap();
    static Engine::Point getCamera();

    static const int window_x, window_y;

    static std::vector<EEScene::TileType> mapData;

    static bool collision(int x, int y);

    bool firstTime = true;

    void oneOnClick();
    void twoOnClick();
    void threeOnClick();
    void fourOnClick();
    void openingDialog();
    void AddScore(const std::string& playerName, int score);

    struct BuildingZone {
        int x;          // x 座標 (像素)
        int y;          // y 座標 (像素)
        int width;      // 寬度 (像素)
        int height;     // 高度 (像素)
        std::string buildingName;  // 建築物名稱
    };

    // 存儲所有建築物的範圍
    std::vector<BuildingZone> buildingZones;

    // 初始化建築物範圍
    void AddBuildingZone(int x, int y, int width, int height, const std::string& buildingName);

    // 檢查玩家是否在建築物範圍內
    bool IsPlayerNearBuilding(Player* player, const BuildingZone& zone);

    // 顯示進入建築物的提示
    void ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY);

    std::string currentBuildingName = ""; // <- 追蹤目前提示哪棟建築

    Engine::Label* enterPromptLabel = nullptr;
};
