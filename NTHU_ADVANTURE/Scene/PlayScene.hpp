#ifndef PLAYSCENE_HPP
#define PLAYSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"
#include "NPC/NPC.hpp"
#include "NPC/NPCDialog.hpp"
#include "Player/Player.hpp"

class Player;
class Turret;
namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
    
}   // namespace Engine

class PlayScene final : public Engine::IScene {
private:
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

    NPCDialog dialog;  // 添加對話框成員

    // 攝影機偏移量
    static Engine::Point cameraOffset;

    Engine::Label* moneyLabel;
    Engine::Image* moneyImage = nullptr;
    Engine::Image* axeImage;
    Engine::Image* speedImage = nullptr;
    Engine::Label* speedLabel;
    NPC* Kao = nullptr;
    NPC* dyy = nullptr;
   
    
    struct BuildingZone {
        int x;          // x 座標 (像素)
        int y;          // y 座標 (像素)
        int width;      // 寬度 (像素)
        int height;     // 高度 (像素)
        std::string buildingName;  // 建築物名稱
    };

    // 存儲所有建築物的範圍
    std::vector<BuildingZone> buildingZones;

protected:
    int lives;
    //int money;
    int SpeedMult;

public:
    enum TileType {
        TILE_ROAD,
        TILE_GRASS,
        TILE_AVANUE,
        TILE_TREE,
        TILE_STAIRS,
        NEW, TILE_NEW,
        NOTHING,
        INFORMATIONELETRIC,
        SMALLEAT,
        WINDCLOUD,
        WATERWOOD,
        TALDA,
        LAKE,
    };

    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const std::vector<int> code;
    Group *TileMapGroup;
    Player* player;

    // new add
    Group *PlayerGroup;
    Group *NPCGroup;
    Group *LabelGroup;

    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<int> keyStrokes;
    std::string currentBuildingName = ""; // <- 追蹤目前提示哪棟建築

    ALLEGRO_KEYBOARD_STATE keyboardState; // new add
    Engine::Label* enterPromptLabel = nullptr;
    

    static Engine::Point GetClientSize();
    explicit PlayScene() = default;
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
    void DrawMiniMap() const;

    static const int window_x, window_y;
    static bool inPlay;
    static bool inSmallEat;
    static bool inBattle;
    static bool inCGLake;
    static bool inEE;
    static bool inTalda;
    static bool inWaterWood;
    static bool inWindCloud;
    static bool inNEW;
    //static bool haveAxe;

    static std::vector<PlayScene::TileType> mapData;

    static bool collision(int x, int y);

    // 初始化建築物範圍
    void AddBuildingZone(int x, int y, int width, int height, const std::string& buildingName);

    // 檢查玩家是否在建築物範圍內
    bool IsPlayerNearBuilding(Player* player, const BuildingZone& zone);

    // 顯示進入建築物的提示
    void ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY);

    bool firstTime = true;
};

#endif   // PLAYSCENE_HPP
