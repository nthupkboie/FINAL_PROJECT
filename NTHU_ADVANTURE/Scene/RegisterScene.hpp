#ifndef RegisterScene_HPP
#define RegisterScene_HPP
#include <memory>
#include <string>
#include <vector>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>
#include "UI/Component/Label.hpp"
#include "Engine/Resources.hpp"



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
    Engine::Image* arrowImage;
    int ID_entered = 0;
    //Group* imageGroup;
    Engine::Label* labelID;
    Engine::Label* labelpw;
    Engine::Label* labelback;

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
    std::string currentLanguage;
    void Update(float deltaTime) override;
    void RefreshLabels();


};


#endif   // LogScene_HPP
