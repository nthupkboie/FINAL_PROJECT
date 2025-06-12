#ifndef RegisterScene_HPP
#define RegisterScene_HPP
#include <memory>
#include <string>
#include <vector>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>



class RegisterScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<std::string>IDs;
    std::vector<std::string>passwords;
    std::string name;
    std::string pswd;
    Engine::Label* label_name;
    Engine::Label* label_pswd;
    Engine::Label* warning1;
    int ID_entered = 0;

public:
    explicit RegisterScene() = default;
    void Initialize() override;
    //void Update(float deltaTime) override;
    void OnKeyDown(int keyCode) override;
    void Terminate() override;
    void BackOnClick(int stage);
    void LoadFromFile(void);
    void SaveToFile(void);
    bool checkID(std::string ID);
    void Draw() const override;

};


#endif   // LogScene_HPP
