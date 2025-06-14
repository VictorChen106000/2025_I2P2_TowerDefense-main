#ifndef INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_MODESELECTIONSCENE_H
#define INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_MODESELECTIONSCENE_H

#include <allegro5/allegro_audio.h>
#include <memory>

#include "Engine/IScene.hpp"

class ModeSelectionScene final : public Engine::IScene {
public:
    void SetNextStage(int stage) {nextStage = stage;}
    explicit ModeSelectionScene() = default;
    void Initialize() override;
    void Terminate() override;

    // Called when the user clicks "Normal Mode"
    void NormalModeOnClick(int mode);
    // Called when the user clicks "Survival Mode"
    void SurvivalModeOnClick(int mode);
    // Called when the user clicks "Back"
    void BackOnClick(int);

private:
    int nextStage = 1;
};

#endif  // INC_2025_I2P2_TOWERDEFENSE_WITH_ANSWER_MODESELECTIONSCENE_H
