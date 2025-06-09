// #ifndef REALSCENE_HPP
// #define REALSCENE_HPP

// #include <vector>
// #include <memory>
// #include "Engine/IScene.hpp"
// #include "Engine/Point.hpp"

// #include "Player/Player.hpp"  // 您的 Player 類

// namespace Engine {
//     class Group;
//     class Image;
// }

// class RealScene final : public Engine::IScene {
// public:
//     // 地圖格子類型
//     enum TileType {
//         TILE_GRASS,   // 可行走
//         TILE_WALL,    // 不可行走
//         TILE_WATER    // 不可行走
//     };

// private:
//     std::unique_ptr<Player> player;
//     std::vector<std::vector<TileType>> mapData;

// public:
//     // 遊戲組
//     std::unique_ptr<Engine::Group> mapGroup;
//     std::unique_ptr<Engine::Group> playerGroup;

//     // 地圖常量
//     static const int MAP_WIDTH = 20;
//     static const int MAP_HEIGHT = 15;
//     static const int TILE_SIZE = 64;

//     explicit RealScene();
//     void Initialize() override;
//     void Terminate() override;
//     void Update(float deltaTime) override;
//     void Draw() const override;

//     // 輸入處理
//     void OnKeyDown(int keyCode) override;
//     void OnKeyUp(int keyCode) override;
//     void OnMouseMove(int mx, int my) override;

//     // 玩家移動
//     void HandlePlayerMovement(float deltaTime);
    
//     // 地圖相關
//     void LoadDefaultMap();
//     bool IsPositionWalkable(float x, float y) const;
// };

// #endif // REALSCENE_HPP