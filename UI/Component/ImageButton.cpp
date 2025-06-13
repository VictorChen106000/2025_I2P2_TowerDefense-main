#include <functional>
#include <memory>
#include <cmath>
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "Image.hpp"
#include "ImageButton.hpp"
#include "allegro5/allegro_primitives.h"

namespace Engine {
    ImageButton::ImageButton(std::string img, std::string imgIn, float x, float y, float w, float h, float anchorX, float anchorY) 
    : Image(img, x, y, w, h, anchorX, anchorY)
    , imgOut(Resources::GetInstance().GetBitmap(img))
    , imgIn(Resources::GetInstance().GetBitmap(imgIn)) 
    , m_baseW(int(w))
    , m_baseH(int(h))
    {
        Point mouse = GameEngine::GetInstance().GetMousePosition();
        mouseIn = Collider::IsPointInBitmap(Point((mouse.x - Position.x) * GetBitmapWidth() / Size.x + Anchor.x * GetBitmapWidth(), (mouse.y - Position.y) * GetBitmapHeight() / Size.y + Anchor.y * GetBitmapHeight()), bmp);
        if (!mouseIn || !Enabled) bmp = imgOut;
        else bmp = this->imgIn;
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
        // mouseIn = Collider::IsPointInBitmap(Point((mx - Position.x) * GetBitmapWidth() / Size.x + Anchor.x * GetBitmapWidth(), (my - Position.y) * GetBitmapHeight() / Size.y + Anchor.y * GetBitmapHeight()), bmp);
        float left   = Position.x - Anchor.x * Size.x;
        float top    = Position.y - Anchor.y * Size.y;
        float right  = left + Size.x;
        float bottom = top  + Size.y;

        mouseIn = (mx >= left && mx <= right && my >= top && my <= bottom);
        bmp = imgOut;
        // if (!mouseIn || !Enabled) bmp = imgOut;
        // else bmp = imgIn;
    }
    void ImageButton::SetImage(const std::string &outPath, const std::string &inPath) {
        // load new images
        imgOut = Resources::GetInstance().GetBitmap(outPath);
        imgIn  = Resources::GetInstance().GetBitmap(inPath);
        // immediately update the one we're drawing
        bmp    = (mouseIn && Enabled) ? imgIn : imgOut;
    }

    void ImageButton::Draw() const
    {
        // ALLEGRO_BITMAP* rawBmp = bmp.get();
        // 1) compute breathing scale (± amplitude) around 1.0
        float baseScale = 1.0f;
        if (m_breatheEnabled) {
            double t = al_get_time();  
            // sine oscillates –1…1, so scale goes 1–A … 1+A
            baseScale += m_breatheAmplitude
                   * std::sin((2.0f * M_PI / m_breathePeriod) * float(t));
        }

        
        // 2) if hover-shrink is enabled and mouseIn, apply it
        float finalScale = baseScale;
        if (m_hoverEnabled && mouseIn && Enabled) {
            finalScale *= m_hoverScale;
        }

        // 3) figure out new size and keep it centered on the original box
        int drawW = int(m_baseW * finalScale);
        int drawH = int(m_baseH * finalScale);

        // original top-left in screen coords:
        float left = Position.x - Anchor.x * m_baseW;
        float top  = Position.y - Anchor.y * m_baseH;

        // offset by half the growth/shrink so it stays centered:
        float drawX = left + (m_baseW - drawW) / 2.0f;
        float drawY = top  + (m_baseH - drawH) / 2.0f;

        // 3) draw whichever bitmap (out/in) is set in ‘bmp’
        al_draw_scaled_bitmap(
            bmp.get(),                     // current up/down image
            0, 0,                    // src origin
            al_get_bitmap_width(bmp.get()),
            al_get_bitmap_height(bmp.get()), 
            drawX, drawY,            // dest origin
            drawW, drawH,            // dest size
            0                        // flags
        );
    }
}
