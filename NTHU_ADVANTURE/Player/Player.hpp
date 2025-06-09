#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <list>
#include <string>
#include <vector>

#include <map>

#include <allegro5/allegro.h> // new add

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class PlayScene;

class Player : public Engine::Sprite {
protected:
    PlayScene *getPlayScene();

    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_down;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_left;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_right;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_up;
    std::shared_ptr<ALLEGRO_BITMAP> bmpUp1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpUp2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpDown1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpDown2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpLeft1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpLeft2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpRight1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpRight2;

    bool last_left, last_up, last_down, last_right;

    float animationTimer;

    // 移動控制
    bool isMoving = false;
    ALLEGRO_KEYBOARD_STATE lastKeyState;
    Engine::Point startPos, targetPos; //startPos記錄角色開始移動時的起始格子中心位置，targetPos記錄角色移動的目標格子中心位置
    float moveProgress; //追蹤移動的進度，範圍從 0.0（開始）到 1.0（完成）
    // 按鍵時間追蹤
    std::map<int, float> keyPressTimes;
    int lastDirection; // 記錄最後方向（ALLEGRO_KEY_*）
    float gameTime; // 累計遊戲時間
    
public:
    Player(std::string img, float x, float y);
    void Update(float deltaTime) override;
    void Draw() const override;

    int GetLastDirection() const { return lastDirection; }
};
#endif   // PLAYER_HPP