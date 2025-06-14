// Sprite.hpp
#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <allegro5/color.h>
#include <string>
#include <vector>
#include <algorithm>

#include "Engine/Point.hpp"
#include "UI/Component/Image.hpp"

namespace Engine {

    /// <summary>
    /// Image that supports rotation, velocity, tint, collision radius,
    /// and (optionally) grid‐based animation with arbitrary frame sequences.
    /// </summary>
    class Sprite : public Image {
    public:
        float Rotation;         // radians
        Point Velocity;         // px/sec
        ALLEGRO_COLOR Tint;
        float CollisionRadius = 0;

        explicit Sprite(std::string img,
                        float x, float y,
                        float w = 0, float h = 0,
                        float anchorX = 0.5f, float anchorY = 0.5f,
                        float rotation = 0,
                        float vx = 0, float vy = 0,
                        unsigned char r = 255, unsigned char g = 255,
                        unsigned char b = 255, unsigned char a = 255);

        void Draw()   const override;
        void Update(float deltaTime) override;

        /// Set up a cols×rows grid at fps.  By default uses all cols*rows frames in row-major order.
        void SetAnimation(int cols, int rows, float fps);

        /// Override the auto-generated 0..(cols*rows-1) sequence.
        /// Only frames in this list will be drawn (looping in order).
        void SetFrameSequence(const std::vector<int>& seq);

    private:
        // grid
        int animCols    = 1,
            animRows    = 1;
        // cell size
        int frameW      = 0,
            frameH      = 0;
        // how many cells in the grid
        int totalFrames = 1;
        // which index into frameSequence
        int curFrame    = 0;
        // timing
        float fps_      = 0,
              frameTime = 0,
              animTimer = 0;

        // which actual cell-indices we play
        std::vector<int> frameSequence;
    };

}

#endif // SPRITE_HPP
