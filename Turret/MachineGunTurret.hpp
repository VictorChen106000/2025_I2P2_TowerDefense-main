#ifndef MACHINEGUNTURRET_HPP
#define MACHINEGUNTURRET_HPP
#include "Turret.hpp"

class MachineGunTurret : public Turret {
public:
    static const int Price;
    MachineGunTurret(float x, float y);
   // void CreateBullet() override;
//tambahan private
private:
    void applylevelstats()override;
    void CreateBullet() override;
     float GetDPS() const override {
        return 10.0f / coolDown;  // so if coolDown=0.5, DPS=20.0
    }
    void setlevelimages() override;
    
};
#endif   // MACHINEGUNTURRET_HPP
