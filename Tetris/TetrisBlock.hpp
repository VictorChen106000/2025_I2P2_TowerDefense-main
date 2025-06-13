#pragma once

#include <array>
#include <vector>
#include <utility>
#include "Engine/Sprite.hpp"       // for Engine::Sprite
namespace Engine { class Group; }  // forward-declare so the header stays light

enum class TetrominoType { T, L, I };

class TetrisBlock {
public:
    using Cell  = std::pair<int,int>;
    using Shape = std::array<Cell,4>;

    // Get the 4 cell-offsets so PlayScene can do placement checks itself
    const Shape& GetCells() const { return _cells; }

    // Create / destroy
    TetrisBlock(TetrominoType type, int tileSize);
    ~TetrisBlock();

    // Move the child-sprites into world coords
    void SetPosition(int mapX, int mapY);

    // Rotate 90° clockwise around (0,0)
    void Rotate();

    // Once you’re ready to “lock” it in, add all sprites into your TowerGroup
    void CommitToScene(Engine::Group* towerGroup);

private:
    TetrominoType                   _type;
    Shape                           _cells;
    int                             _tileSize;
    std::vector<Engine::Sprite*>    _children;

    // All of your T, L, I definitions in local grid coords
    static const Shape shapeDefs[3];
};
