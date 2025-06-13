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
    char answer[5] = {'A', 'A', 'A', 'A', 'A'};
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
};
