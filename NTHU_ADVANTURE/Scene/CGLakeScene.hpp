#ifndef CGLakeScene_HPP
#define CGLakeScene_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

#include "NPC/NPCDialog.hpp"

#include "Player/BattlePlayer.hpp"

namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
}   // namespace Engine

class CGLakeScene final : public Engine::IScene {
private:
    
    
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

    NPCDialog dialog;  // 添加對話框成員

    // 攝影機偏移量
    static Engine::Point cameraOffset;
protected:
    int lives;
    int money;
    int SpeedMult;
    float timer;        // 計時器
    float timeLimit;    // 時間限制（秒）

public:
    enum TileType {
        TILE_ROAD,
        TILE_GRASS,
        TILE_TREE,
        TILE_STAIRS,
        TILE_WATER,
        TILE_LOTUS,
        TILE_DSTN,
        NEW, TILE_NEW,
        NOTHING,
    };

    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const std::vector<int> code;

    Group *TileMapGroup;

    


    // new add
    Group *PlayerGroup;
    Group *NPCGroup;
    Group *UIGroup;

    static std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<int> keyStrokes;
    //std::vector<TileType> mapData; // 修改為 int 以匹配 TileType
    static std::vector<CGLakeScene::TileType> mapData;

    ALLEGRO_KEYBOARD_STATE keyboardState; // new add

    static Engine::Point GetClientSize();
    explicit CGLakeScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnMouseDown(int button, int mx, int my) override;
    void OnMouseMove(int mx, int my) override;
    void OnMouseUp(int button, int mx, int my) override;
    void OnKeyDown(int keyCode) override;
    void ReadMap();
    void GenerateMaze();    // 新增：生成迷宮
    static Engine::Point getCamera();
    void UpdateTileMap(int gridX, int gridY);

    static const int window_x, window_y;

    static bool collision(int x, int y);
    void AxeOnClick();
    bool canChop = false; // 是否允許砍樹

    Engine::Image* axeImage = nullptr; // 跟隨玩家的斧頭圖片
    //static bool canWalk;

    //std::vector<CGLakeScene::TileType> mapData;
    void SaveToFile();

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
    bool IsPlayerNearBuilding(BattlePlayer* player, const BuildingZone& zone);

    // 顯示進入建築物的提示
    void ShowEnterPrompt(const std::string& buildingName, int zoneX, int zoneY);

    std::string currentBuildingName = ""; // <- 追蹤目前提示哪棟建築

    Engine::Label* enterPromptLabel = nullptr;
    Group* LabelGroup = nullptr;
};

#endif   // CGLakeScene_HPP