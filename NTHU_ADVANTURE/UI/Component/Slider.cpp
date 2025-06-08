#include <algorithm>
#include <string>

#include "Engine/Point.hpp"
#include "Slider.hpp"

Slider::Slider(float x, float y, float w, float h)
    : ImageButton("stage-select/slider.png", "stage-select/slider-blue.png", x, y),
      Bar("stage-select/bar.png", x, y, w, h),
      End1("stage-select/end.png", x, y + h / 2, 0, 0, 0.5, 0.5),
      End2("stage-select/end.png", x + w, y + h / 2, 0, 0, 0.5, 0.5) {
    Position.x += w;
    Position.y += h / 2;
    Anchor = Engine::Point(0.5, 0.5);
}
void Slider::Draw() const {
    // TODO HACKATHON-5 (3/4): The slider's component should be drawn here.
    Bar.Draw();//constructor
    End1.Draw();
    End2.Draw();
    ImageButton::Draw();
}
//onValueChangedCallback是一個function指標，傳入value，回傳void，
void Slider::SetOnValueChangedCallback(std::function<void(float value)> onValueChangedCallback) {
    OnValueChangedCallback = onValueChangedCallback;
}
void Slider::SetValue(float value) {
    // TODO HACKATHON-5 (4/4): Set the value of the slider and call the callback.
    //傳進來的值是0到1的比例
    float barLeft = Bar.Position.x;
    float barWidth = Bar.Size.x;
    Position.x = barLeft + value * barWidth;//更新位置
    if (OnValueChangedCallback) OnValueChangedCallback(value);//調音量 OnValueChangedCallback是個物件 儲存之前通過SetOnValueChangedCallback設置的函式(這個只會在initializer被叫)
    //在initialize時有sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
    //設置OnValueChangedCallback函式，函式被調用時，它會執行 this->BGMSlideOnValueChanged(value)
}
void Slider::OnMouseDown(int button, int mx, int my) {
    if ((button & 1) && mouseIn)
        Down = true;
}
void Slider::OnMouseUp(int button, int mx, int my) {
    Down = false;
}
void Slider::OnMouseMove(int mx, int my) {
    ImageButton::OnMouseMove(mx, my);
    if (Down) {
        // Clamp
        float clamped = std::min(std::max(static_cast<float>(mx), Bar.Position.x), Bar.Position.x + Bar.Size.x);
        float value = (clamped - Bar.Position.x) / Bar.Size.x * 1.0f;
        SetValue(value);
    }
}
