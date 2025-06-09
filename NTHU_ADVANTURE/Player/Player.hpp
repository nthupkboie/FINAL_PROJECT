#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <list>
#include <string>
#include <vector>

#include <allegro5/allegro.h> // new add

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class Bullet;
class PlayScene;
class Turret;

class Player : public Engine::Sprite {
protected:
    std::vector<Engine::Point> path;
    float speed;
    float hp;
    int money;
    PlayScene *getPlayScene();
    virtual void OnExplode();

    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle;
    std::shared_ptr<ALLEGRO_BITMAP> bmpUp1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpUp2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpDown1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpDown2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpLeft1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpLeft2;
    std::shared_ptr<ALLEGRO_BITMAP> bmpRight1;
    std::shared_ptr<ALLEGRO_BITMAP> bmpRight2;

    float animationTimer;
    bool isMoving = false;

    ALLEGRO_KEYBOARD_STATE lastKeyState;


public:
    float reachEndTime;
    std::list<Turret *> lockedTurrets;
    std::list<Bullet *> lockedBullets;
    Player(std::string img, float x, float y, float radius, float speed, float hp, int money);
    void Hit(float damage);
    void UpdatePath(const std::vector<std::vector<int>> &mapDistance);
    void Update(float deltaTime) override;
    void Draw() const override;

    
};
#endif   // PLAYER_HPP