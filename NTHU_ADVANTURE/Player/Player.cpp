#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "Player.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"

#include "Scene/PlayScene.hpp"

PlayScene *Player::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}
void Player::OnExplode() {;
//     getPlayScene()->EffectGroup->AddNewObject(new ExplosionEffect(Position.x, Position.y));
//     std::random_device dev;
//     std::mt19937 rng(dev());
//     std::uniform_int_distribution<std::mt19937::result_type> distId(1, 3);
//     std::uniform_int_distribution<std::mt19937::result_type> dist(1, 20);
//     for (int i = 0; i < 10; i++) {
//         // Random add 10 dirty effects.
//         getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-" + std::to_string(distId(rng)) + ".png", dist(rng), Position.x, Position.y));
//     }
}
Player::Player(std::string img, float x, float y, float radius, float speed, float hp, int money) : Engine::Sprite(img, x, y), speed(speed), hp(hp), money(money) {
    CollisionRadius = radius;
    reachEndTime = 0;
}
void Player::Hit(float damage) {
//     hp -= damage;
//     if (hp <= 0) {
//         OnExplode();
//         // Remove all turret's reference to target.
//         for (auto &it : lockedTurrets)
//             it->Target = nullptr;
//         for (auto &it : lockedBullets)
//             it->Target = nullptr;
//         getPlayScene()->EarnMoney(money);
//         getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
//         AudioHelper::PlayAudio("explosion.wav");
//     }
}
void Player::UpdatePath(const std::vector<std::vector<int>> &mapDistance) {;
//     int x = static_cast<int>(floor(Position.x / PlayScene::BlockSize));
//     int y = static_cast<int>(floor(Position.y / PlayScene::BlockSize));
//     if (x < 0) x = 0;
//     if (x >= PlayScene::MapWidth) x = PlayScene::MapWidth - 1;
//     if (y < 0) y = 0;
//     if (y >= PlayScene::MapHeight) y = PlayScene::MapHeight - 1;
//     Engine::Point pos(x, y);
//     int num = mapDistance[y][x];
//     if (num == -1) {
//         num = 0;
//         Engine::LOG(Engine::ERROR) << "Enemy path finding error";
//     }
//     path = std::vector<Engine::Point>(num + 1);
//     while (num != 0) {
//         std::vector<Engine::Point> nextHops;
//         for (auto &dir : PlayScene::directions) {
//             int x = pos.x + dir.x;
//             int y = pos.y + dir.y;
//             if (x < 0 || x >= PlayScene::MapWidth || y < 0 || y >= PlayScene::MapHeight || mapDistance[y][x] != num - 1)
//                 continue;
//             nextHops.emplace_back(x, y);
//         }
//         // Choose arbitrary one.
//         std::random_device dev;
//         std::mt19937 rng(dev());
//         std::uniform_int_distribution<std::mt19937::result_type> dist(0, nextHops.size() - 1);
//         pos = nextHops[dist(rng)];
//         path[num] = pos;
//         num--;
//     }
//     path[0] = PlayScene::EndGridPoint;
}
void Player::Update(float deltaTime) {
    // 計算移動速度
    //float moveSpeed = speed * deltaTime; // speed = 10
    //Velocity = Engine::Point(0, 0);

    // 獲取鍵盤狀態
    ALLEGRO_KEYBOARD_STATE kbState;
    al_get_keyboard_state(&kbState);

    // 檢查鍵盤輸入
    if (al_key_down(&kbState, ALLEGRO_KEY_I)) {
        Position.y -= 10;
    }
    if (al_key_down(&kbState, ALLEGRO_KEY_K)) {
        Position.y += 10;
    }
    if (al_key_down(&kbState, ALLEGRO_KEY_J)) {
        Position.x -= 10;
    }
    if (al_key_down(&kbState, ALLEGRO_KEY_L)) {
        Position.x += 10;
    }

    // 計算新位置
    //Engine::Point newPosition = Position + Velocity;

    // 檢查邊界（畫面大小 1280x832）
    // if (newPosition.x < CollisionRadius) newPosition.x = CollisionRadius;
    // if (newPosition.x > 1280 - CollisionRadius) newPosition.x = 1280 - CollisionRadius;
    // if (newPosition.y < CollisionRadius) newPosition.y = CollisionRadius;
    // if (newPosition.y > 832 - CollisionRadius) newPosition.y = 832 - CollisionRadius;

    // 檢查地圖碰撞
    // int gridX = static_cast<int>(floor(newPosition.x / PlayScene::BlockSize));
    // int gridY = static_cast<int>(floor(newPosition.y / PlayScene::BlockSize));
    // if (gridX >= 0 && gridX < PlayScene::MapWidth && gridY >= 0 && gridY < PlayScene::MapHeight) {
    //     if (getPlayScene()->mapState[gridY][gridX] == PlayScene::TILE_FLOOR) {
    //         Position = newPosition; // 有效格子，更新位置
    //     } else {
    //         Engine::LOG(Engine::INFO) << "Player collision at (" << gridX << ", " << gridY << ")";
    //     }
    // } else {
    //     Engine::LOG(Engine::INFO) << "Player out of map bounds at (" << gridX << ", " << gridY << ")";
    // }

    Sprite::Update(deltaTime);
}
void Player::Draw() const {
    Sprite::Draw();
    if (PlayScene::DebugMode) {
        // Draw collision radius.
        al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(255, 0, 0), 2);
    }
}
