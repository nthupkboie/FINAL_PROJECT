#ifndef LogScene_HPP
#define LogScene_HPP
#include <memory>
#include <string>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>



class LogScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<std::string>IDs;
    std::vector<std::string>passwords;
    //std::string right_name;
    std::string right_pswd;
    std::string name;
    std::string pswd;
    Engine::Label* label_name;
    Engine::Label* label_pswd;
    Engine::Label* warning1;
    Engine::Label* warning2;
    int ID_entered = 0;

public:
    explicit LogScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void LoadFromFile(void);

    void OnKeyDown(int keyCode) override;
    bool checkID(std::string ID);
    void Draw() const override;

    static int money;
    static bool haveAxe;
    static float haveSpeedUp;
    static int clearedLake;
};


#endif   // LogScene_HPP
