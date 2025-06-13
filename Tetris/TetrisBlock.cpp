// Tetris/TetrisBlock.cpp

#include "Tetris/TetrisBlock.hpp"
#include "Engine/Resources.hpp"
#include "Engine/Group.hpp"    // brings in the full definition of Engine::Group

// Prototype definitions in local grid coords (T, L, I)
const TetrisBlock::Shape TetrisBlock::shapeDefs[3] = {
    {{ {0,0},{1,0},{2,0},{1,1} }},  // T
    {{ {0,0},{0,1},{0,2},{1,2} }},  // L
    {{ {0,0},{1,0},{2,0},{3,0} }}   // I
};

TetrisBlock::TetrisBlock(TetrominoType type, int tileSize)
  : _type(type)
  , _tileSize(tileSize)
{
    // copy the shape offsets
    _cells = shapeDefs[static_cast<int>(_type)];

    // create one Sprite per cell, initially invisible
    for (auto [dx,dy] : _cells) {
        // use the full Sprite constructor: (img, x, y, w, h,
        //                                 anchorX, anchorY,
        //                                 rotation,
        //                                 vx, vy,
        //                                 r, g, b, a)
        auto* s = new Engine::Sprite(
            "Tetris/red-tetris.png",        // image path
            0, 0,                           // x, y (will be repositioned later)
            _tileSize, _tileSize,           // width, height
            0, 0,                           // anchorX, anchorY
            0,                              // initial rotation
            0, 0,                           // initial velocity x,y
            255,255,255,255                 // full-opacity white tint
        );
        s->Visible = false;                // hide until we commit
        _children.push_back(s);
    }
}

TetrisBlock::~TetrisBlock() {
   
}

void TetrisBlock::SetPosition(int mapX, int mapY) {
    // snap each child sprite to its cell position in world coords
    for (size_t i = 0; i < _children.size(); ++i) {
        auto [dx,dy] = _cells[i];
        _children[i]->Position.x = mapX + dx * _tileSize;
        _children[i]->Position.y = mapY + dy * _tileSize;

    }
}

void TetrisBlock::Rotate() {
    // rotate each (x,y) offset → (y, -x)
    for (auto& cell : _cells) {
        int x = cell.first;
        int y = cell.second;
        cell = { y, -x };
    }
}

void TetrisBlock::CommitToScene(Engine::Group* towerGroup) {
    // make them visible and hand off to the scene’s tower group
    for (auto* c : _children) {
        c->Visible = true;
        towerGroup->AddNewObject(c);
      }
      _children.clear();
}
