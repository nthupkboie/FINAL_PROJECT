#ifndef CUSTOM_IMAGEBUTTON_HPP
#define CUSTOM_IMAGEBUTTON_HPP

#include "ImageButton.hpp"

class CustomImageButton : public Engine::ImageButton {
public:
    using Engine::ImageButton::ImageButton; // 繼承建構子

    void SetImages(const std::string& newImgOut, const std::string& newImgIn);
};

#endif // CUSTOM_IMAGEBUTTON_HPP
