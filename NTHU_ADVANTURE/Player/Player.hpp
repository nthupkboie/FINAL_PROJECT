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
