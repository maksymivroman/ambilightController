//
// Created by rmaks on 26-Apr-26.
//

#ifndef AMBILIGHTCONTROLLER_ARCADE_H
#define AMBILIGHTCONTROLLER_ARCADE_H


#include "../LEDService/LEDStripService.h"
#include "../Global/Global.hpp"
#include <algorithm>

typedef HTMLColorCode EnemyType;


//============= Items ====================

struct Misile {
    u_int position;
    EnemyType type;
};

typedef std::vector<Misile> Misiles;

//============= Enemies ==================
class Enemies {
public:
    Enemies(u_int maxCount){
        this->enemies.reserve(maxCount);
        this->enemies.resize(maxCount, emptyEnemy);
        logger.log("[Enemies] Init ", maxCount);
    }

    void add(EnemyType enemyType);

    void shift();

    LEDState get() const;
    LEDState *getForEdit();

    void clear();

private:
    LEDState enemies;

    EnemyType emptyEnemy{Black};

};


//============= Arcade ==================

class Arcade {

public:
    void setup(LEDStripService *LEDService);
    void process();
    void hit(EnemyType enemyType);
    void resetGame();

private:
    void render();
    void shiftMisiles();

    EnemyType getNewEnemy();

    LEDStripService *lEDService{nullptr};

    Enemies *enemies;

    Misiles misiles;

    unsigned long lastFireTime = 0;
    const uint16_t fireCooldown = 150;
    bool isPenaltyActive = false;
    unsigned long penaltyStartTime = 0;
    const uint16_t penaltyDuration = 50;

    bool isGameOver = false;

    uint16_t score = 0;
    uint16_t wastedFrame = 0;
};



#endif //AMBILIGHTCONTROLLER_ARCADE_H
