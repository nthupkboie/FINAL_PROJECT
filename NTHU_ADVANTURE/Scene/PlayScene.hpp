#ifndef PLAYSCENE_HPP
#define PLAYSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

#include "NPC/NPCDialog.hpp"

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
protected:
    int lives;
    int money;
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

    // new add
    Group *PlayerGroup;
    Group *NPCGroup;

    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<int> keyStrokes;

    ALLEGRO_KEYBOARD_STATE keyboardState; // new add

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

    static std::vector<PlayScene::TileType> mapData;

    static bool collision(int x, int y);
};

#endif   // PLAYSCENE_HPP
