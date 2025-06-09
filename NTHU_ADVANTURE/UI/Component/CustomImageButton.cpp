#include "CustomImageButton.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

void CustomImageButton::SetImages(const std::string& newImgOut, const std::string& newImgIn) {
    imgOut = std::shared_ptr<ALLEGRO_BITMAP>(
        al_load_bitmap(("resources/images/" + newImgOut).c_str()),
        [](ALLEGRO_BITMAP* bmp) { al_destroy_bitmap(bmp); }
    );

    imgIn = std::shared_ptr<ALLEGRO_BITMAP>(
        al_load_bitmap(("resources/images/" + newImgIn).c_str()),
        [](ALLEGRO_BITMAP* bmp) { al_destroy_bitmap(bmp); }
    );
}
