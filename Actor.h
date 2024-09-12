#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

/*
 Polymorphism/Class Hierarchy
 
 GraphObject
    Actor
        DynamicActor
            Avatar
            Robot
                RageBot
                ThiefBot
                    RegularThiefBot
                    MeanThiefBot
        InteractableObject
            Crystal
            Goodie
                ExtraLife
                RestoreHealth
                Ammo
        Pea
        Marble
        Pit
        Factory
        Wall
        Exit
 */

class Actor : public GraphObject {
public:
    Actor(StudentWorld* world, int imageID, double startX, double startY, int startDirection) : GraphObject(imageID, startX, startY, startDirection), m_world(world), m_status(true), canDetect(true) { setVisible(true); }
    
    // pure virtual functions
    virtual void doSomething() = 0;
    
    virtual bool avatarOverlap() = 0;
    virtual bool marbleOverlap() = 0;
    
    virtual bool canPush() = 0;
    virtual bool canPeaHit() = 0;
    virtual bool canPeaDamage() = 0;
    
    virtual bool isCollectable() = 0;
    virtual bool isThiefBot() = 0;
    
    // accessor functions
    bool getStatus() { return m_status; }
    StudentWorld* getWorld() const { return m_world; }
    bool isObjectDetectable() { return canDetect; }
    
    // modifier functions
    void updatePos(double& x, double& y, const int dir);
    void updateStatus(bool status) { m_status = status; }
    virtual void die() {
        updateStatus(false);
        setVisible(false);
    }
    void setUndetectable() { canDetect = false; }
    void setBackToDetectable() { canDetect = true; }
    
private:
    StudentWorld* m_world; // pointer to StudentWorld
    bool m_status; // if actor is alive
    bool canDetect;
};

/* ///////////// DYNAMIC ACTORS /////////////*/

class DynamicActor : public Actor {
public:
    DynamicActor(StudentWorld* world, int imageID, double startX, double startY, int startDirection, int hp) : Actor(world, imageID, startX, startY, startDirection), hitPoints(hp) {}
    
    // accessors
    int getHealth() const { return hitPoints; }
    
    // modifiers
    void setHealth(int num) { hitPoints = num; }
    void updateHealth(int num) { hitPoints += num; }
    
private:
    int hitPoints;
};

class Avatar : public DynamicActor {
public:
    Avatar(StudentWorld* world, double initX, double initY) : DynamicActor(world, IID_PLAYER, initX, initY, right, 20), numPeas(20) {}
    virtual void doSomething();
    virtual bool canPeaDamage();
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
   
    virtual bool canPush() { return false; }
    virtual bool canPeaHit() { return false; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
    
    // accessors
    int getAmmo() const { return numPeas; }
    
    // mutators
    void avatarMove(double x, double y);
    void updatePeas(int num) { numPeas += num; }
    void playerShoot();
    
private:
    int numCrystals;
    int numPeas;
};

/* ////////////// ROBOTS ////////////////*/

class Robot : public DynamicActor {
public:
    Robot(StudentWorld* world, int imageID, double x, double y, int dir, int hp) : DynamicActor(world, imageID, x, y, dir, hp) {}
    
    // accessors
    bool canTakeAction() const;
    
    // modifiers
    void reverseDirection();
    virtual bool canShoot();
    virtual void die() = 0;
    
private:
};

class RageBot : public Robot {
public:
    RageBot(StudentWorld* world, double x, double y, int dir) : Robot(world, IID_RAGEBOT, x, y, dir, 10) {}
    virtual void doSomething();
    virtual bool canPeaDamage();
   
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPush() { return false; }
    virtual bool canPeaHit() { return true; }
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
    virtual void die();
    
    
private:
};

class ThiefBot : public Robot {
public:
    ThiefBot(StudentWorld* world, int imageID, double x, double y, int hp) : Robot(world, imageID, x, y, right, hp), distanceMoved(0), hasGoodie(false), m_goodie(nullptr) {
        maxDistance = distanceBeforeTurning();
    }
    
    virtual void doSomething();
    virtual bool canPeaDamage();
    virtual void updateScore() = 0;
    
    // accessors
    int getDistanceMoved() { return distanceMoved; }
    int getMaxDistance() { return maxDistance; }
    bool robotHasGoodie() { return hasGoodie; }
    Actor* myGoodie() { return m_goodie; }
    
    // modifiers
    void incrementDistanceMoved() { distanceMoved++; }
    void resetDistanceMoved() { distanceMoved = 0; }
    void resetMaxDistance() { maxDistance = distanceBeforeTurning(); }
    void gotGoodie() { hasGoodie = true; }
    void lostGoodie() { hasGoodie = false; }
    void setGoodie(Actor* actor) { m_goodie = actor; }
    
    void doGoodieAction();
    void moveGoodie(double x, double y);
    virtual void die();
    bool moveRobot(Actor* robot, double x, double y, int dir);
    
    // utility functions
    int distanceBeforeTurning();
    bool chanceToPickUpGoodie();
    int randomDirection();
    
    // thiefbot traits
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    virtual bool canPush() { return false; }
    virtual bool canPeaHit() { return true; }
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return true; }
    
private:
    int distanceMoved;
    int maxDistance;
    bool hasGoodie;
    Actor* m_goodie;
};

class RegularThiefBot : public ThiefBot {
public:
    RegularThiefBot(StudentWorld* world, double x, double y) : ThiefBot(world, IID_THIEFBOT, x, y, 5) {}
    virtual void updateScore();
    virtual bool canShoot() { return false; }
    //virtual void die();
private:
};

class MeanThiefBot : public ThiefBot {
public:
    MeanThiefBot(StudentWorld* world, double x, double y) : ThiefBot(world, IID_MEAN_THIEFBOT, x, y, 8) {}
    virtual void updateScore();
    //virtual void die();
private:
};

/* ///////////// INTERACTABLE OBJECTS /////////////*/

class Interactable_Object : public Actor {
public:
    Interactable_Object(StudentWorld* world, int imageID, double startX, double startY) : Actor(world, imageID, startX, startY, none) {}
    virtual bool avatarOverlap() { return true; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPeaDamage() { return false; }
    virtual bool canPeaHit() { return false; }
    virtual bool canPush() { return false; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
private:
};

/* /////////////// CRYSTAL ////////////////////*/

class Crystal : public Interactable_Object {
public:
    Crystal(StudentWorld* world, double x, double y) : Interactable_Object(world, IID_CRYSTAL, x, y) {}
    virtual void doSomething();

private:
};

/* ///////////// GOODIES ///////////////////*/

class Goodie : public Interactable_Object {
public:
    Goodie(StudentWorld* world, int imageID, double x, double y) : Interactable_Object(world, imageID, x, y), canPlayerPickUp(true) {}
    virtual void useGoodie() = 0;
    virtual void doSomething();
    void playerCanPickUp() { canPlayerPickUp = true; }
    void playerCannotPickUp() { canPlayerPickUp = false; }
    virtual bool isCollectable() { return true; }
    
private:
    bool canPlayerPickUp;
};

class ExtraLife : public Goodie {
public:
    ExtraLife(StudentWorld* world, double x, double y) : Goodie(world, IID_EXTRA_LIFE, x, y) { }
    virtual void useGoodie();
private:
};

class RestoreHealth : public Goodie {
public:
    RestoreHealth(StudentWorld* world, double x, double y) : Goodie(world, IID_RESTORE_HEALTH, x, y) { }
    virtual void useGoodie();
private:
};

class Ammo: public Goodie {
public:
    Ammo(StudentWorld* world, double x, double y) : Goodie(world, IID_AMMO, x, y) { }
    virtual void useGoodie();
private:
};

/* ///////////// OTHER ACTORS ///////////////*/

class Pea : public Actor {
public:
    Pea(StudentWorld* world, double x, double y, int direction) : Actor(world, IID_PEA, x, y, direction), firstShot(true) {};
    virtual void doSomething();
    virtual bool canPeaDamage() { return false; }
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPush() { return false; }
    virtual bool canPeaHit() { return false; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
private:
    bool firstShot;
};

class Marble : public Actor {
public:
    Marble(StudentWorld* world, double x, double y) : Actor(world, IID_MARBLE, x, y, none), hitPoints(10) { }
    virtual void doSomething();
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPeaDamage();
    virtual bool canPush();
    virtual bool canPeaHit() { return true; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
    
    // utility functions
    bool pushMarble(int dir);

private:
    int hitPoints;
};

class Pit : public Actor {
public:
    Pit(StudentWorld* world, double x, double y) : Actor(world, IID_PIT, x, y, none) {}
    virtual void doSomething();
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return true; }
    
    virtual bool canPush() { return false; }
    virtual bool canPeaDamage() { return false; }
    virtual bool canPeaHit() { return false; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }

private:
};

class ThiefBotFactory : public Actor {
public:
    ThiefBotFactory(StudentWorld* world, double x, double y, bool isMean) : Actor(world, IID_ROBOT_FACTORY, x, y, none), isMeanFactory(isMean) {}
    virtual void doSomething();
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPeaDamage() { return false; }
    virtual bool canPush() { return false; }
    virtual bool canPeaHit(); 
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
    
    bool isMeanThiefBotFactory() { return isMeanFactory; }
private:
    bool isMeanFactory; // whether regular factory or mean factory
    bool chanceCreateThiefBot(); // utility function
};

class Wall : public Actor {
public:
    Wall(StudentWorld* world, double initX, double initY) : Actor(world, IID_WALL, initX, initY, none) {}
    virtual void doSomething() { return; } // wall can't do anything
    
    virtual bool avatarOverlap() { return false; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPush() { return false; }
    virtual bool canPeaDamage() { return false; }
    virtual bool canPeaHit() { return true; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
private:
};


class Exit : public Actor {
public:
    Exit(StudentWorld* world, double x, double y) : Actor(world, IID_EXIT, x, y, none) { setVisible(false); }
    virtual void doSomething();
    
    virtual bool avatarOverlap() { return true; }
    virtual bool marbleOverlap() { return false; }
    
    virtual bool canPush() { return false; }
    virtual bool canPeaDamage() { return false; }
    virtual bool canPeaHit() { return false; }
    
    virtual bool isCollectable() { return false; }
    virtual bool isThiefBot() { return false; }
private:
};

#endif // ACTOR_H_
