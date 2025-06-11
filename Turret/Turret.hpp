#ifndef TURRET_HPP
#define TURRET_HPP
#include <allegro5/base.h>
#include <list>
#include <string>
#include <vector>
#include "Engine/Sprite.hpp"

class Enemy;
class PlayScene;

class Turret : public Engine::Sprite {
protected:
    int price;
    float coolDown;
    float reload = 0;
    float rotateRadian = 2 * ALLEGRO_PI;
    Sprite imgBase;
    std::list<Turret *>::iterator lockedTurretIterator;
    PlayScene *getPlayScene();
    // Reference: Design Patterns - Factory Method.
    virtual void CreateBullet() = 0;

    //updrage turret
    int level =1;
    int maxlevel =3;
    std::vector<int>updrageprice;
    std::vector<std::string>levelImages;
    virtual void applylevelstats()=0;
    virtual void setlevelimages()=0;
    virtual Enemy* AcquireTarget();



public:
    bool Enabled = true;
    bool Preview = false;
    Enemy *Target = nullptr;
   
    Turret(std::string imgBase, std::string imgTurret, float x, float y, float radius, int price, float coolDown);


    void Update(float deltaTime) override;
    void Draw() const override;
    int GetPrice() const;

    // tambahan turret

    bool canupdrage() const{
        return level< maxlevel;
    }

    int getupdragecost()const{
        if(!canupdrage())return 0;
        return updrageprice[level-1];
    }

    bool updrage(){
        if(!canupdrage()) return false;
        level++;
        applylevelstats();
        ChangeImage();
        return true;
    }

    int getlevel()const{
        return level;
    }
    
    void ChangeImage(){
        if(level>=1 && level<=levelImages.size()){
            imgBase = Sprite(levelImages[level-1],Position.x,Position.y);
        }
    }

};
#endif   // TURRET_HPP
