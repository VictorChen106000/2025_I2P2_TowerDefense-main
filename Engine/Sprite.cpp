// Sprite.cpp
#include "Sprite.hpp"
#include <allegro5/allegro.h>

namespace Engine {

    Sprite::Sprite(std::string img,
                   float x, float y,
                   float w, float h,
                   float anchorX, float anchorY,
                   float rotation,
                   float vx, float vy,
                   unsigned char r, unsigned char g,
                   unsigned char b, unsigned char a)
      : Image(img, x, y, w, h, anchorX, anchorY),
        Rotation(rotation),
        Velocity(Point(vx, vy)),
        Tint(al_map_rgba(r, g, b, a))
    {
    }

    void Sprite::SetAnimation(int cols, int rows, float fps) {
        animCols    = std::max(1, cols);
        animRows    = std::max(1, rows);
        totalFrames = animCols * animRows;
        fps_        = fps;
        frameTime   = 1.0f / fps_;
        animTimer   = 0;
        curFrame    = 0;

        // compute each frame’s pixel size
        frameW = GetBitmapWidth()  / animCols;
        frameH = GetBitmapHeight() / animRows;

        // default sequence = every frame in row-major
        frameSequence.clear();
        frameSequence.reserve(totalFrames);
        for (int i = 0; i < totalFrames; i++)
            frameSequence.push_back(i);
    }

    void Sprite::SetFrameSequence(const std::vector<int>& seq) {
        frameSequence = seq;
        if (frameSequence.empty()) {
            // fallback to full range
            frameSequence.clear();
            for (int i = 0; i < totalFrames; i++)
                frameSequence.push_back(i);
        }
        curFrame = curFrame % frameSequence.size();
    }

    void Sprite::Update(float deltaTime) {
        // advance animation if any
        if (fps_ > 0 && frameSequence.size() > 1) {
            animTimer += deltaTime;
            while (animTimer >= frameTime) {
                animTimer -= frameTime;
                curFrame = (curFrame + 1) % frameSequence.size();
            }
        }
        // move
        Position.x += Velocity.x * deltaTime;
        Position.y += Velocity.y * deltaTime;
    }

    void Sprite::Draw() const {
        if (fps_ > 0 && frameSequence.size() > 1) {
            int actual = frameSequence[curFrame];
            int col    = actual % animCols;
            int row    = actual / animCols;

            al_draw_tinted_scaled_rotated_bitmap_region(
                bmp.get(),
                col * frameW, row * frameH,
                frameW,       frameH,
                Tint,
                Anchor.x * frameW,
                Anchor.y * frameH,
                Position.x,
                Position.y,
                Size.x / frameW,
                Size.y / frameH,
                Rotation,
                0
            );
        }
        else {
            // single‐frame fallback
            al_draw_tinted_scaled_rotated_bitmap(
                bmp.get(),
                Tint,
                Anchor.x * GetBitmapWidth(),
                Anchor.y * GetBitmapHeight(),
                Position.x,
                Position.y,
                Size.x / GetBitmapWidth(),
                Size.y / GetBitmapHeight(),
                Rotation,
                0
            );
        }
    }

}
