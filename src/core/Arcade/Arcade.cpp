//
// Created by rmaks on 26-Apr-26.
//

#include "Arcade.h"


//============= Enemies ==================
void Enemies::add(EnemyType enemyType) {
    std::rotate(this->enemies.begin(), this->enemies.begin() + 1, this->enemies.end());
    this->enemies.back() = enemyType;
}

void Enemies::shift() {
    std::rotate(this->enemies.begin(), this->enemies.begin() + 1, this->enemies.end());
    this->enemies.back() = emptyEnemy;
}

LEDState Enemies::get() const {
    return this->enemies;
}

void Enemies::clear() {
    std::fill(this->enemies.begin(), this->enemies.end(), emptyEnemy);
}

LEDState *Enemies::getForEdit() {
    return &enemies;
}

//============= Arcade ==================

void Arcade::setup(LEDStripService *LEDService) {
    this->lEDService = LEDService;
    auto maxCount = this->lEDService->getLedCount();
    this->enemies = new Enemies(maxCount);
    randomSeed(analogRead(0));
}

void Arcade::process() {
    // 1. If Game Over, stop logic updates and let render() handle the animation
    if (isGameOver) {
        this->render();
        return;
    }

    // 2. CHECK LOSE CONDITION: Did an enemy reach the base (index 0)?
    // We check this BEFORE moving anything to ensure it's frame-accurate.
    if (this->enemies->get().at(0) != CRGB(Black)) {
        isGameOver = true;
        wastedFrame = 0; // Prepare for the outward red animation
        logger.log("[Arcade] WASTED! Score: ", score);
        return;
    }

    // 3. UPDATE POSITIONS
    shiftMisiles(); // Move missiles forward

    // Spawn new enemies at a regular interval
    EVERY_N_MILLISECONDS(200) {
        this->enemies->add(getNewEnemy());
    }

    // 4. COLLISION & COLOR-MATCH LOGIC
    auto it = misiles.begin();
    while (it != misiles.end()) {
        auto* enemyList = this->enemies->getForEdit();

        // Safety check to prevent out-of-bounds
        if (it->position < enemyList->size()) {
            CRGB enemyColor = enemyList->at(it->position);

            // Is there an enemy at the missile's current pixel?
            if (enemyColor != CRGB(Black)) {

                // COLOR MATCH CHECK
                if (CRGB(it->type) == enemyColor) {
                    // Success! Remove enemy and gain a point
                    enemyList->at(it->position) = Black;
                    score++;
                } else {
                    // Mismatch! Trigger the White flash penalty
                    isPenaltyActive = true;
                    penaltyStartTime = millis();
                }

                // In either case, the missile is destroyed on impact
                it = misiles.erase(it);
                continue;
            }
        }
        ++it;
    }

    this->render();
}



void Arcade::render() {
    auto count = lEDService->getLedCount();

    if (isGameOver) {
        // --- PHASE 1: ANIMATED OUTWARD FILL ---
        if (wastedFrame < count) {
            this->lEDService->fillStepFromPosition(this->lEDService->getLedCount()/2, wastedFrame, Red);
            wastedFrame++;
        }
            // --- PHASE 2: SHOW SCORE PROGRESS BAR ---
        else {
            // We create a temporary state to represent the score
            LEDState scoreFrame;
            scoreFrame.resize(count); // Ensure it matches strip length

            for (int i = 0; i < count; i++) {
                scoreFrame[i] = (i < score) ? Green : Red;
            }


            this->lEDService->updateColorsWithState(scoreFrame);
            this->lEDService->render();
        }
        return;
    }

    // --- NORMAL RENDERING ---
    if (isPenaltyActive && (millis() - penaltyStartTime < penaltyDuration)) {
        this->lEDService->fillColor(White);
    } else {
        LEDState currentFrame = this->enemies->get();
        for (const auto& m : this->misiles) {
            if (m.position < currentFrame.size()) {
                currentFrame[m.position] = m.type;
            }
        }
        this->lEDService->updateColorsWithState(currentFrame);
    }
    this->lEDService->render();
}




EnemyType Arcade::getNewEnemy() {
    long randNumber = random(0, 10);

    switch (randNumber) {
        case 1:
            return Red;
//        case 4:
//            return Green;
        case 8:
            return Blue;
        default:
            return Black;
    }
}

void Arcade::hit(EnemyType enemyType) {
    unsigned long currentTime = millis();

    if (currentTime - lastFireTime >= fireCooldown) {
        this->misiles.push_back({0, enemyType});
        lastFireTime = currentTime;
        logger.log("[Arcade] Shot fired!");
    } else {
        logger.log("[Arcade] Weapon cooling down...");
    }
}

void Arcade::shiftMisiles() {
    if (this->misiles.empty()) return;

    auto count = lEDService->getLedCount();
    auto it = misiles.begin();

    while (it != misiles.end()) {
        it->position++;

        // If missile goes off the end of the strip
        if (it->position >= count) {
            it = misiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Arcade::resetGame() {
    this->enemies->clear();
    this->misiles.clear();
    this->isGameOver = false;
    this->isPenaltyActive = false;
    score = 0;
    logger.log("[Arcade] Game Restarted");
}
