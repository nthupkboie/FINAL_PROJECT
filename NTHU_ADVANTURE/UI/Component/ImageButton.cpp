#include <functional>
#include <memory>

#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "Image.hpp"
#include "ImageButton.hpp"

namespace Engine {
    ImageButton::ImageButton(std::string img, std::string imgIn, float x, float y, float w, float h, float anchorX, float anchorY) : Image(img, x, y, w, h, anchorX, anchorY), imgOut(Resources::GetInstance().GetBitmap(img)), imgIn(Resources::GetInstance().GetBitmap(imgIn)) {
        Point mouse = GameEngine::GetInstance().GetMousePosition();
        mouseIn = Collider::IsPointInBitmap(Point((mouse.x - Position.x) * GetBitmapWidth() / Size.x + Anchor.x * GetBitmapWidth(), (mouse.y - Position.y) * GetBitmapHeight() / Size.y + Anchor.y * GetBitmapHeight()), bmp);
        if (!mouseIn || !Enabled) bmp = imgOut;
        else bmp = this->imgIn;
    }

    void ImageButton::Draw() const {
        ALLEGRO_BITMAP* bitmap = mouseIn ? imgIn.get() : imgOut.get();
        int alpha = mouseIn ? 128 : 255;  // 滑鼠移上去時透明
        al_draw_tinted_scaled_bitmap(bitmap, al_map_rgba(255, 255, 255, alpha),
            0, 0, al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap),
            Position.x - Anchor.x * Size.x,
            Position.y - Anchor.y * Size.y,
            Size.x, Size.y, 0);
    }


    void ImageButton::SetOnClickCallback(std::function<void(void)> onClickCallback) {
        OnClickCallback = onClickCallback;
    }
    void ImageButton::OnMouseDown(int button, int mx, int my) {
        if ((button & 1) && mouseIn && Enabled) {
            if (OnClickCallback)
                OnClickCallback();
        }
    }
    void ImageButton::OnMouseMove(int mx, int my) {
        mouseIn = Collider::IsPointInBitmap(Point((mx - Position.x) * GetBitmapWidth() / Size.x + Anchor.x * GetBitmapWidth(), (my - Position.y) * GetBitmapHeight() / Size.y + Anchor.y * GetBitmapHeight()), bmp);
        if (!mouseIn || !Enabled) bmp = imgOut;
        else bmp = imgIn;
    }
}
