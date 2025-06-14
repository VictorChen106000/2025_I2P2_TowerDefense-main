#ifndef IMAGEBUTTON_HPP
#define IMAGEBUTTON_HPP
#include <allegro5/bitmap.h>
#include <functional>
#include <memory>
#include <string>
#include <cmath>
#include "Engine/IControl.hpp"
#include "Image.hpp"

namespace Engine {
    /// <summary>
    /// A clickable button, changes image when mouse move.
    /// </summary>
    class ImageButton : public Image, public IControl {
    protected:
        // Determines whether mouse is in the button.
        bool mouseIn = false;
        // The smart pointer that points to bitmap for mouse-out display.
        std::shared_ptr<ALLEGRO_BITMAP> imgOut;
        // The smart pointer that points to bitmap for mouse-in display.
        std::shared_ptr<ALLEGRO_BITMAP> imgIn;
        // The callback function to call when button clicked.
        std::function<void()> OnClickCallback;

    public:
        // Whether the button can be pressed.
        bool Enabled = true;
        /// <summary>
        /// Construct a image object.
        /// </summary>
        /// <param name="img">The mouse-out image path in 'resources/images/'</param>
        /// <param name="imgIn">The mouse-in image path in 'resources/images/'</param>
        /// <param name="x">X-coordinate.</param>
        /// <param name="y">Y-coordinate.</param>
        /// <param name="w">Width of the image, 0 indicates original size.</param>
        /// <param name="h">Height of the image, 0 indicates original size.</param>
        /// <param name="anchorX">The centerX of the object. (0, 0) means top-left, while (1, 0) means top-right.</param>
        /// <param name="anchorY">The centerY of the object. (0, 1) means bottom-left, while (1, 1) means bottom-right.</param>
        explicit ImageButton(std::string img, std::string imgIn, float x, float y, float w = 0, float h = 0, float anchorX = 0, float anchorY = 0);
        /// <summary>
        /// Set the callback function when clicked.
        /// </summary>
        /// <param name="onClickCallback"></param>
        void SetOnClickCallback(std::function<void(void)> onClickCallback);
        /// <summary>
        /// Delegated from scene when mouse button down.
        /// </summary>
        /// <param name="button">The button pressed.</param>
        /// <param name="mx">Mouse x coordinate in window space.</param>
        /// <param name="my">Mouse y coordinate in window space.</param>
        void OnMouseDown(int button, int mx, int my) override;
        /// <summary>
        /// Delegated from scene when mouse move.
        /// </summary>
        /// <param name="mx">Mouse x coordinate in window space.</param>
        /// <param name="my">Mouse y coordinate in window space.</param>
        void OnMouseMove(int mx, int my) override;
        void SetImage(const std::string &outPath, const std::string &inPath);
        void Draw() const override;
        void EnableBreathing(float amplitude = 0.05f, float period = 2.0f) {
            m_breatheEnabled   = true;
            m_breatheAmplitude = amplitude;
            m_breathePeriod    = period;
        }
        void EnableHoverScale(float scale = 0.9f) {
            m_hoverEnabled = true;
            m_hoverScale   = scale;
        }

    private:
        int   m_baseW, m_baseH;          // remember original size
        bool  m_breatheEnabled   = false;
        float m_breatheAmplitude = 0.05f; // ±5% size
        float m_breathePeriod    = 2.0f;  // 2-second loop
        bool  m_hoverEnabled   = false;
        float m_hoverScale     = 0.9f;  // 90% size when hovered
    };
}
#endif   // IMAGEBUTTON_HPP
