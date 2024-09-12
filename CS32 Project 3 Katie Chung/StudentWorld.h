#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GraphObject.h"
#include "Level.h"
#include <string>
#include <list>
#include <iomanip>
using namespace std;

class Actor;
class Avatar;

class StudentWorld : public GameWorld
{
public:
    // constructor/destructor
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    
    // main functions
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    // helper functions
    void updateDisplayText();
    
    // accessor functions
    Actor* getActorAtPos(double x, double y);
    int getTick() { return tick; }
    int getCurrLevel() { return getLevel(); }
    int getPlayerDirection(); 
    
    // goodie functions
    void restoreHealth();
    void restoreLife();
    void restoreAmmo();
    
    // crystal functions
    void foundCrystal();
    int numCrystals() const { return crystalsLeft; }
    void levelCompleted() { finishLevel = true; playSound(SOUND_FINISHED_LEVEL); }
    
    // pea related functions
    void constructPea(double x, double y, int direction);
    int overlapPea(double x, double y);
    
    // marble related functions
    bool canMarbleMove(Actor* actor, double x, double y);
    bool canRobotMove(Actor* actor, double x, double y) const;
    void killMarble();
    
    // movement functions
    bool isInBounds(double x, double y);
    bool onSameSquareAsPlayer(double x, double y);
    bool sameRowColAsPlayer(double x, double y, int dir);
    
    // robot related functions
    bool robotCanShootPlayer(double x, double y, int dir);
    bool onSameSquareAsGoodie(double x, double y);
    int countSurroundingThiefBots(double x, double y);
    void constructMeanThiefBot(double x, double y);
    void constructRegularThiefBot(double x, double y);
    bool onSameSquareAsThiefBot(double x, double y);
    
private:
    list<Actor*> actorList;
    Avatar* m_avatar;
    
    bool finishLevel;
    int crystalsLeft;
    unsigned int bonus;
    int tick;
};

#endif // STUDENTWORLD_H_
