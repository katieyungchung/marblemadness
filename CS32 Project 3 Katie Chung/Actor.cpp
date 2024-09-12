// Actor.cpp
#include "Actor.h"
#include "StudentWorld.h"
#include <cstdlib>

/* ////////////// ACTOR //////////////////*/

// update position function for all actors
void Actor::updatePos(double& x, double& y, const int dir) {
    switch (dir) {
        case up:
            y++;
            break;
        case down:
            y--;
            break;
        case left:
            x--;
            break;
        case right:
            x++;
            break;
    }
}

/* ////////////// AVATAR //////////////////*/

// reads in input and moves avatar accordingly
void Avatar::doSomething() {
    int ch;
    if (getWorld()->getKey(ch)) {
        switch(ch) {
            case KEY_PRESS_LEFT: {
                setDirection(left);
                avatarMove(getX() - 1, getY());
                break;
            }
            case KEY_PRESS_RIGHT: {
                setDirection(right);
                avatarMove(getX() + 1, getY());
                break;
            }
            case KEY_PRESS_UP: {
                setDirection(up);
                avatarMove(getX(), getY() + 1);
                break;
            }
            case KEY_PRESS_DOWN: {
                setDirection(down);
                avatarMove(getX(), getY() - 1);
                break;
            }
            case KEY_PRESS_SPACE: {
                playerShoot();
                break;
            }
        }
    }
}

// checks if movement is valid
void Avatar::avatarMove(double x, double y) {
    if (getWorld()->isInBounds(x, y))
        moveTo(x, y);
}

// when the player presses space bar and shoots
void Avatar::playerShoot() {
    if (numPeas > 0) {
        double x = getX();
        double y = getY();
        int dir = getDirection();
        updatePos(x, y, dir);
        
        getWorld()->constructPea(x, y, dir);
        getWorld()->playSound(SOUND_PLAYER_FIRE);
        numPeas--;
    }
}

// how an avatar is attacked by pea
bool Avatar::canPeaDamage() {
    updateHealth(-2); // loses 2 hp
    getWorld()->playSound(SOUND_PLAYER_IMPACT);
    if (getHealth() <= 0) {
        updateStatus(false);
        getWorld()->playSound(SOUND_PLAYER_DIE);
    }
    return true;
}

/* ////////////// MARBLE //////////////////*/

void Marble::doSomething() {
    if (hitPoints <= 0)
        updateStatus(false);
}

// can only push if pushMarble returns true
bool Marble::canPush() {
    return pushMarble(getWorld()->getPlayerDirection());
}

// avatar pushes marble
bool Marble::pushMarble(int dir) {
    // get marble's current pos
    double x = getX();
    double y = getY();
    updatePos(x, y, dir);
   
    // move the marble if possible
    if (getWorld()->canMarbleMove(this, x, y)) {
        moveTo(x, y);
        return true;
    }
    return false;
}

bool Marble::canPeaDamage() {
    hitPoints -= 2;
    return true;
}

/* /////////// ROBOTS ///////////*/

// reverses current direction of robot
void Robot::reverseDirection() {
    int dir = getDirection();
    switch (dir) {
        case up:
            setDirection(down);
            break;
        case down:
            setDirection(up);
            break;
        case left:
            setDirection(right);
            break;
        case right:
            setDirection(left);
            break;
    }
}

bool Robot::canTakeAction() const {
    int tickFreq = ((28 - getWorld()->getCurrLevel()) / 4);
    if (tickFreq < 3) // least tickFreq is 3
        tickFreq = 3;
    
    return (getWorld()->getTick() % tickFreq == 0);
}

// if robot can shoot player + shoot player
bool Robot::canShoot() {
   // shoot peas
   int dir = 0;
   switch(getDirection()) {
       case up:
           dir = 1;
           break;
       case down:
           dir = 2;
           break;
       case left:
           dir = 3;
           break;
       case right:
           dir = 4;
           break;
   }

   // if player is in the line of sight of the robot
   if (getWorld()->robotCanShootPlayer(getX(), getY(), dir)) {
       double x = getX();
       double y = getY();
       int dir = getDirection();
       updatePos(x, y, dir); // updated position where pea should be constructed

       getWorld()->constructPea(x, y, dir);
       getWorld()->playSound(SOUND_ENEMY_FIRE);
       return true;
   }
   return false;
}

/* /////////// RAGEBOT //////////*/

void RageBot::doSomething() {
    if (!getStatus() || !canTakeAction()) // check if dead
        return;
    
    double x = getX();
    double y = getY();
    updatePos(x, y, getDirection());
    
    if (canShoot()) {
        // function will shoot
    }
    else {
        if (getWorld()->canRobotMove(this, x, y)) {
            moveTo(x, y);
        }
        else {
            reverseDirection();
            updatePos(x, y, getDirection());
            moveTo(x, y);
        }
    }
}

bool RageBot::canPeaDamage() {
    updateHealth(-2);
    getWorld()->playSound(SOUND_ROBOT_IMPACT);
    
    if (getHealth() == 0)
        die();
    
    return true;
}

void RageBot::die() {
    getWorld()->increaseScore(100);
    getWorld()->playSound(SOUND_ROBOT_DIE);
    updateStatus(false);
}

/* /////////// THIEFBOT //////////*/

int ThiefBot::distanceBeforeTurning() {
    // generate a random int between 1 and 6 inclusive
    int rand_distance = rand() % 6 + 1;
    return rand_distance;
}

bool ThiefBot::chanceToPickUpGoodie() {
    int randNum = rand() % 10 + 1;
    return randNum == 1;
}

int ThiefBot::randomDirection() {
    int randNum = rand() % 4 + 1;
    switch(randNum) {
        case 1:
            return up;
        case 2:
            return down;
        case 3:
            return left;
        case 4:
            return right;
    }
    return -1;
}

// what happens when thiefbot is on the same square as a goodie
void ThiefBot::doGoodieAction() {
    gotGoodie(); // sets hasGoodie to true
    setGoodie(getWorld()->getActorAtPos(getX(), getY())); // set pointer to goodie
    myGoodie()->setVisible(false); // goodie should now be invisible
    myGoodie()->setUndetectable(); // avatar/other actors can't detect it
    getWorld()->playSound(SOUND_ROBOT_MUNCH); // play munch sound
}

void ThiefBot::doSomething() {
    // check if alive or supposed to rest
    if (!getStatus() || !canTakeAction() || canShoot())
        return;
    
    double x = getX();
    double y = getY();
    updatePos(x, y, getDirection());
    
    // check if on the same square as a goodie
    if (!robotHasGoodie() && chanceToPickUpGoodie() && getWorld()->onSameSquareAsGoodie(getX(), getY()) ) {
        doGoodieAction();
        return;
    }
    // has not yet moved distanceBeforeTurning
    else if (getDistanceMoved() < getMaxDistance() && getWorld()->canRobotMove(this, x, y)) {
        moveTo(x, y);
        moveGoodie(x, y);
        incrementDistanceMoved();
        return;
    }
    // either has moved distanceBeforeTurning or encountered obstruction
    else if (getDistanceMoved() == getMaxDistance() || !getWorld()->canRobotMove(this, x, y)) {
        resetMaxDistance(); // new value of distanceBeforeTurning
        resetDistanceMoved(); // distance moved set back to 0
        
        int directions[4] = {up, down, left, right};
        int d = randomDirection();
        
        double x = getX();
        double y = getY();
        updatePos(x, y, d);
        
        if (moveRobot(this, x, y, d))
            return;
        
        for (int i = 0; i < 4; i++) { // trying the other directions
            if (directions[i] != d) { // except for the one already tried
                double x = getX();
                double y = getY();
                updatePos(x, y, directions[i]);
                if (moveRobot(this, x, y, directions[i]))
                    return;
            }
        }
        setDirection(d);
        return;
    }
}

void ThiefBot::moveGoodie(double x, double y) {
    if (robotHasGoodie())
        myGoodie()->moveTo(x, y); // goodie should move w/ robot
}

bool ThiefBot::moveRobot(Actor* robot, double x, double y, int dir) {
    if (getWorld()->canRobotMove(robot, x, y)) {
        setDirection(dir);
        moveTo(x, y);
        moveGoodie(x, y);
        incrementDistanceMoved();
        return true;
    }
    return false;
}

bool ThiefBot::canPeaDamage() {
    updateHealth(-2);
    getWorld()->playSound(SOUND_ROBOT_IMPACT);
    if (getHealth() <= 0)
        die();
    return true;
}

void ThiefBot::die() {
    if (robotHasGoodie()) {
        myGoodie()->setVisible(true);
        myGoodie()->setBackToDetectable();
        lostGoodie();
        setGoodie(nullptr);
    }
    updateScore();
    getWorld()->playSound(SOUND_ROBOT_DIE);
    updateStatus(false);
}

void RegularThiefBot::updateScore() {
    getWorld()->increaseScore(10);
}

void MeanThiefBot::updateScore() {
    getWorld()->increaseScore(20);
}

/* /////////// THIEFBOT FACTORY /////////*/

void ThiefBotFactory::doSomething() {
    double x = getX();
    double y = getY();
    
    if (getWorld()->countSurroundingThiefBots(x, y) < 3 && chanceCreateThiefBot()) {
        if (isMeanThiefBotFactory()) {
            getWorld()->constructMeanThiefBot(x, y);
            return;
        }
        else {
            getWorld()->constructRegularThiefBot(x, y);
            return;
        }
    }
    return;
}

bool ThiefBotFactory::canPeaHit() {
    if (getWorld()->onSameSquareAsThiefBot(getX(), getY())) {
        setUndetectable();
        return false;
    }
    return true;
}

bool ThiefBotFactory::chanceCreateThiefBot() {
    int chance = rand() % 50 + 1;
    return chance == 1;
}

/* /////////// GOODIES ///////////////// */

void Goodie::doSomething() {
    if (!getStatus())
        return;
    
    if (canPlayerPickUp && getWorld()->onSameSquareAsPlayer(getX(), getY())) {
        getWorld()->playSound(SOUND_GOT_GOODIE);
        useGoodie();
        die();
    }
}

void RestoreHealth::useGoodie() {
    getWorld()->restoreHealth();
}

void ExtraLife::useGoodie() {
    getWorld()->restoreLife();
}

void Ammo::useGoodie() {
    getWorld()->restoreAmmo();
}

// increase score by 50, notify studentWorld, remove crystal
void Crystal::doSomething() {
    if (!getStatus()) // dead
        return;
    
    if (getWorld()->onSameSquareAsPlayer(getX(), getY())) {
        getWorld()->foundCrystal();
        die();
    }
}

/* ////////////// PEA /////////////////*/

void Pea::doSomething() {
    if (!getStatus()) // if dead
        return;
    
    if (firstShot) {
        firstShot = false;
        return;
    }
    
    int collisionType = getWorld()->overlapPea(getX(), getY());
    if (collisionType == 1 || collisionType == 2)
        die();

    // update position
    int dir = getDirection();
    double x = getX();
    double y = getY();
    
    updatePos(x, y, dir);
    
    if (collisionType == 3)  // pea goes over - everything else
        moveTo(x, y);
    
    // another check for updated position
    int collisionType2 = getWorld()->overlapPea(getX(), getY());
    if (collisionType2 == 1 || collisionType2 == 2)
        die();
}

/* ///////////// PIT & EXIT ///////////////*/

void Pit::doSomething() {
    if (!getStatus())
        return;
}

// if all crystals collected, make exit visible
// once player is on exit, level has been completed
void Exit::doSomething() {
    if (getWorld()->numCrystals() == 0) {
        setVisible(true);
        if (getWorld()->onSameSquareAsPlayer(getX(), getY()))
            getWorld()->levelCompleted();
    }
}


