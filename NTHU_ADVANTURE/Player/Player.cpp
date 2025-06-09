#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include <cmath>

#include "Player.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Scene/PlayScene.hpp"

#include "Engine/Resources.hpp"

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

    // 儲存圖像
    bmpIdle = bmp;

    bmpUp1 = Engine::Resources::GetInstance().GetBitmap("play/run1_player.png");
    bmpUp2 = Engine::Resources::GetInstance().GetBitmap("play/run2_player.png");

    bmpDown1 = Engine::Resources::GetInstance().GetBitmap("play/run1_player.png");
    bmpDown2 = Engine::Resources::GetInstance().GetBitmap("play/run2_player.png");

    bmpLeft1 = Engine::Resources::GetInstance().GetBitmap("play/run1_player.png");
    bmpLeft2 = Engine::Resources::GetInstance().GetBitmap("play/run2_player.png");

    bmpRight1 = Engine::Resources::GetInstance().GetBitmap("play/run1_player.png");
    bmpRight2 = Engine::Resources::GetInstance().GetBitmap("play/run2_player.png");

    //對齊
    Position.x = std::round(Position.x / 64.0f) * 64.0f + 32.0f;
    Position.y = std::round(Position.y / 64.0f) * 64.0f + 32.0f;
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
void Player::Update(float deltaTime) { //deltaTime 是每幀的時間增量（通常為 ~0.016 秒，假設 60 FPS）

    animationTimer += deltaTime; //動畫切換需要追蹤累積時間（例如從遊戲開始或移動開始的總時間），而不是單幀的 deltaTime
    if (animationTimer >= 0.6f) animationTimer -= 0.6f; // 重置，每 0.6 秒一個循環
    // 獲取鍵盤狀態
    ALLEGRO_KEYBOARD_STATE kbState;
    al_get_keyboard_state(&kbState);

    Engine::Point tmp = Position;
    // 檢查鍵盤輸入
    if (al_key_down(&kbState, ALLEGRO_KEY_I)) {
        tmp.y -= 5;
        //tmp.x = Position.x;
        bmp = (fmod(animationTimer, 0.6f) < 0.3f) ? bmpUp1 : bmpUp2;
        isMoving = true;
    }
    else if (al_key_down(&kbState, ALLEGRO_KEY_K)) {
        tmp.y += 5;
        //tmp.x = Position.x;
        bmp = (fmod(animationTimer, 0.6f) < 0.3f) ? bmpDown1 : bmpDown2;
        isMoving = true;
    }
    else if (al_key_down(&kbState, ALLEGRO_KEY_J)) {
        tmp.x -= 5;
        //tmp.y = Position.y;
        bmp = (fmod(animationTimer, 0.6f) < 0.3f) ? bmpLeft1 : bmpLeft2;
        isMoving = true;
    }
    else if (al_key_down(&kbState, ALLEGRO_KEY_L)) {
        tmp.x += 5;
        //tmp.y = Position.y;
        bmp = (fmod(animationTimer, 0.6f) < 0.3f) ? bmpRight1 : bmpRight2;
        isMoving = true;
    }

    //else if (isMoving && !al_key_down(&kbState, ALLEGRO_KEY_L) && !al_key_down(&kbState, ALLEGRO_KEY_J) && !al_key_down(&kbState, ALLEGRO_KEY_K) && !al_key_down(&kbState, ALLEGRO_KEY_I)){
    else if (!al_key_down(&kbState, ALLEGRO_KEY_L) && !al_key_down(&kbState, ALLEGRO_KEY_J) && !al_key_down(&kbState, ALLEGRO_KEY_I) && !al_key_down(&kbState, ALLEGRO_KEY_K)) {
        bmp = bmpIdle;
        isMoving = false;
    }

    Position = tmp;
    //PlayScene::collision(Position.x, Position.y);

    Sprite::Update(deltaTime);
}
void Player::Draw() const {
    Sprite::Draw();
    if (PlayScene::DebugMode) {
        // Draw collision radius.
        al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(255, 0, 0), 2);
    }
}