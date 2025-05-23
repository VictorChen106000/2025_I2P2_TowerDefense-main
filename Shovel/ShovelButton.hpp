// ShovelButton.hpp
#pragma once
#include "UI/Component/ImageButton.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Resources.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"

class ShovelButton : public Engine::ImageButton {
    Engine::Sprite  baseSpr, iconSpr;

public:
    ShovelButton(float x, float y)
      : ImageButton(
          "play/shovel-base.png",  // we just use this to size our button
          "play/shovel-base.png",  // (hovered image is ignored by us)
          x, y
        ),
        baseSpr("play/shovel-base.png",
                x, y,     // position (we’ll update these each frame)
                0, 0,     // region = full bitmap
                0, 0      // anchor = top-left
        ),
        iconSpr("play/shovel.png",
                x, y,     // position (we’ll update these each frame)
                0, 0,     // region = full bitmap
                0, 0      // anchor = top-left
        )
    {
        // ensure we participate in the control chain
        Enabled = true;
    }

    // — recalc position + hover every frame —
    void Update(float dt) override {
        float w = GetBitmapWidth(), h = GetBitmapHeight();

        // center both sprites on our button rectangle
        baseSpr.Position = Position;
        iconSpr.Position.x = Position.x + (w - iconSpr.GetBitmapWidth()) * 0.5f;
        iconSpr.Position.y = Position.y + (h - iconSpr.GetBitmapHeight()) * 0.5f;

        // bounding-box hover test
        Engine::Point m = Engine::GameEngine::GetInstance().GetMousePosition();
        bool hovering =  m.x >= Position.x
                      && m.x <  Position.x + w
                      && m.y >= Position.y
                      && m.y <  Position.y + h;

        // hide the base when hovered
        baseSpr.Visible = !hovering;

        // (we don’t need baseSpr.Update(dt) or iconSpr.Update(dt)
        //  because they’re static images)
    }

    // draw base + icon in the right order
    void Draw() const override {
        baseSpr.Draw();
        iconSpr.Draw();
    }

    // swallow the “press” event
    void OnMouseDown(int, int, int) override { }

    // fire our callback on *release* anywhere inside the rect
    void OnMouseUp(int button, int mx, int my) override {
        if (!(button & 1) || !OnClickCallback) return;

        float w = GetBitmapWidth(), h = GetBitmapHeight();
        bool hit = mx >= Position.x
                && mx <  Position.x + w
                && my >= Position.y
                && my <  Position.y + h;
        if (hit) {
            OnClickCallback();
        }
    }
};
