// BatEnemy.cpp
#include "BatEnemy.hpp"

BatEnemy::BatEnemy(int x, int y)
    : Enemy(
        /*img=*/    "play/bat.png",  // 256×64px, 4 frames
        /*x,y=*/    x, y,
        /*radius=*/ 20,              // collision radius
        /*speed=*/  150,              // px/sec
        /*hp=*/     3,              // hit points
        /*money=*/  8                // reward
      )
{
    // 1) Chop the 256×64 into 4×1 frames @14fps:
    SetAnimation(/*cols=*/4, /*rows=*/1, /*fps=*/14.0f);

    // 2) Draw at native frame size (64×64 pixels):
    float frameW = GetBitmapWidth()  / 2.5f;  // =256/4 =64
    float frameH = GetBitmapHeight() / 0.65f;  // = 64/1 =64
    Size.x = frameW;
    Size.y = frameH;
}

// Note: we don't override Update or Hit—
// the base Enemy::Update handles movement, rotation & animation,
// and Enemy::Hit handles damage and death cleanup.
