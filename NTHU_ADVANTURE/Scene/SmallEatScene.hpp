#ifndef SmallEatScene_HPP
#define SmallEatScene_HPP
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

class SmallEatScene final : public Engine::IScene {
private:
    
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

    NPCDialog dialog;  // 添加對話框成員

    // 攝影機偏移量
    static Engine::Point cameraOffset;

    bool buyingAxe = false;

    bool showShopButtons = false; // 新增：控制購買按鈕顯示
    Engine::ImageButton* buyButton = nullptr; // 新增：購買按鈕
    Engine::ImageButton* cancelButton = nullptr; // 新增：取消按鈕
    Engine::Label* buyLabel;
    Engine::Label* cancelLabel; 
    Shopper* axeman = nullptr; // 新增：NPC 引用
    Shopper* Lucy = nullptr; // 新增：NPC 引用
    int items = 0;
    bool canBuyAxe = false;
    bool canBuySpeed = false;


    Engine::Label* moneyLabel;
    Engine::Image* moneyImage;
    Engine::Image* axeImage = nullptr;
    Engine::Image* speedImage = nullptr;
    Engine::Label* speedLabel;

protected:
    int lives;
    //int money;
    int SpeedMult;

public:
    enum TileType {
        TILE_WALL,
        TILE_FLOOR,
        TABLE,
        NOTHING,
        LSEAT,
        RSEAT,
        MC
    };

    enum ItemType {
        AXE,
        SPEED,
    };

    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const std::vector<int> code;
    Group *TileMapGroup;


    // new add
    Group *PlayerGroup;
    Group *NPCGroup;
    Group *ShopperGroup;
    Group *LabelGroup;

    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<int> keyStrokes;

    ALLEGRO_KEYBOARD_STATE keyboardState; // new add

    static Engine::Point GetClientSize();
    explicit SmallEatScene() = default;
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

    static std::vector<SmallEatScene::TileType> mapData;

    static bool collision(int x, int y);

    void BuyOnClick(int item);
    void CancelOnClick();

    bool readyToBuyAxe = false;
    bool readyToBuySpeed = false;

    void BuyOnClick2();
    void CancelOnClick2();

    bool firstTime = true;
};

#endif   // SmallEatScene_HPP
