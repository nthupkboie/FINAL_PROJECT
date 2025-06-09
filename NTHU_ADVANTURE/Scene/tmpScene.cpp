// #include "RealScene.hpp"

// // all need engine
// #include "Engine/Allegro5Exception.hpp"
// #include "Engine/AudioHelper.hpp"
// #include "Engine/Collider.hpp"
// #include "Engine/GameEngine.hpp"
// #include "Engine/Group.hpp"
// #include "Engine/IControl.hpp"
// #include "Engine/IObject.hpp"
// #include "Engine/IScene.hpp"
// #include "Engine/LOG.hpp"
// #include "Engine/Point.hpp"
// #include "Engine/Resources.hpp"

// #include <allegro5/allegro_primitives.h>

// RealScene::RealScene() {
//     mapGroup = std::make_unique<Engine::Group>();
//     playerGroup = std::make_unique<Engine::Group>();
// }

// void RealScene::Initialize() {
//     // 加載默認地圖
//     LoadDefaultMap();
    
//     // 初始化玩家
//     player = std::make_unique<Player>();
//     player->Position = Engine::Point(TILE_SIZE * 2, TILE_SIZE * 2); // 初始位置
//     playerGroup->AddNewObject(player.get());
// }

// void RealScene::Terminate() {
//     mapGroup.reset();
//     playerGroup.reset();
//     player.reset();
//     mapData.clear();
// }

// void RealScene::Update(float deltaTime) {
//     HandlePlayerMovement(deltaTime);
//     mapGroup->Update(deltaTime);
//     playerGroup->Update(deltaTime);
// }

// void RealScene::Draw() const {
//     // 先繪製地圖
//     mapGroup->Draw();
//     // 再繪製玩家
//     playerGroup->Draw();
    
//     // 調試繪製：顯示碰撞框
//     al_draw_rectangle(
//         player->Position.x - player->GetWidth()/2,
//         player->Position.y - player->GetHeight()/2,
//         player->Position.x + player->GetWidth()/2,
//         player->Position.y + player->GetHeight()/2,
//         al_map_rgb(255, 0, 0), 2);
// }

// void RealScene::LoadDefaultMap() {
//     // 初始化地圖數據 (0=草地, 1=牆, 2=水)
//     mapData = std::vector<std::vector<TileType>>(MAP_HEIGHT, std::vector<TileType>(MAP_WIDTH, TILE_GRASS));
    
//     // 添加邊界牆
//     for (int x = 0; x < MAP_WIDTH; x++) {
//         mapData[0][x] = TILE_WALL;
//         mapData[MAP_HEIGHT-1][x] = TILE_WALL;
//     }
//     for (int y = 0; y < MAP_HEIGHT; y++) {
//         mapData[y][0] = TILE_WALL;
//         mapData[y][MAP_WIDTH-1] = TILE_WALL;
//     }
    
//     // 添加一些障礙物
//     mapData[5][5] = TILE_WALL;
//     mapData[5][6] = TILE_WALL;
//     mapData[6][5] = TILE_WALL;
    
//     // 添加水域
//     for (int x = 10; x < 15; x++) {
//         for (int y = 3; y < 8; y++) {
//             mapData[y][x] = TILE_WATER;
//         }
//     }
    
//     // 創建地圖可視化 (簡單的色塊)
//     for (int y = 0; y < MAP_HEIGHT; y++) {
//         for (int x = 0; x < MAP_WIDTH; x++) {
//             auto tile = std::make_shared<Engine::Image>();
//             tile->Position = Engine::Point(x * TILE_SIZE, y * TILE_SIZE);
            
//             switch (mapData[y][x]) {
//                 case TILE_GRASS:
//                     tile->GetBitmap() = Engine::Resources().GetBitmap("grass");
//                     break;
//                 case TILE_WALL:
//                     tile->GetBitmap() = Engine::Resources().GetBitmap("wall");
//                     break;
//                 case TILE_WATER:
//                     tile->GetBitmap() = Engine::Resources().GetBitmap("water");
//                     break;
//             }
            
//             mapGroup->AddNewObject(tile);
//         }
//     }
// }

// void RealScene::HandlePlayerMovement(float deltaTime) {
//     if (!player) return;
    
//     float moveX = 0, moveY = 0;
//     const float moveSpeed = player->GetMoveSpeed() * deltaTime;
    
//     // 鍵盤控制 (WASD)
//     if (Engine::InputManager::GetInstance().IsKeyDown(ALLEGRO_KEY_W)) moveY -= moveSpeed;
//     if (Engine::InputManager::GetInstance().IsKeyDown(ALLEGRO_KEY_S)) moveY += moveSpeed;
//     if (Engine::InputManager::GetInstance().IsKeyDown(ALLEGRO_KEY_A)) moveX -= moveSpeed;
//     if (Engine::InputManager::GetInstance().IsKeyDown(ALLEGRO_KEY_D)) moveX += moveSpeed;
    
//     // 檢查碰撞並移動
//     if (moveX != 0 || moveY != 0) {
//         Engine::Point newPos = player->Position;
//         newPos.x += moveX;
//         newPos.y += moveY;
        
//         // 只更新有效移動
//         if (IsPositionWalkable(newPos.x, player->Position.y)) {
//             player->Position.x = newPos.x;
//         }
//         if (IsPositionWalkable(player->Position.x, newPos.y)) {
//             player->Position.y = newPos.y;
//         }
//     }
// }

// bool RealScene::IsPositionWalkable(float x, float y) const {
//     // 轉換為地圖格子坐標
//     int gridX = static_cast<int>(x / TILE_SIZE);
//     int gridY = static_cast<int>(y / TILE_SIZE);
    
//     // 檢查邊界
//     if (gridX < 0 || gridY < 0 || gridX >= MAP_WIDTH || gridY >= MAP_HEIGHT) {
//         return false;
//     }
    
//     // 檢查格子類型
//     switch (mapData[gridY][gridX]) {
//         case TILE_GRASS:
//             return true;
//         case TILE_WALL:
//         case TILE_WATER:
//         default:
//             return false;
//     }
// }

// // 輸入處理 (簡單實現)
// void RealScene::OnKeyDown(int keyCode) {
//     // 可以添加其他按鍵處理
// }

// void RealScene::OnKeyUp(int keyCode) {
//     // 可以添加按鍵釋放處理
// }

// void RealScene::OnMouseMove(int mx, int my) {
//     // 可以添加鼠標移動處理
// }