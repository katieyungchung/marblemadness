#include "StudentWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "Actor.h"
#include "Level.h"
#include <string>
#include <iostream>
#include <sstream>
using namespace std;


GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// constructor
StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{}


// loads current level's maze from data file + constructs representation of current level
int StudentWorld::init()
{
    // initialize data members
    finishLevel = false;
    bonus = 1000;
    tick = 0;
    
    // load current maze details from a level data file
    string level = "level"; // set level text
    if (getLevel() < 10) {
        level += "0"; // only add 0 digit if level is less than 10
    }
    level += to_string(getLevel()) + ".txt";
   
    // TO BE DELETED !!
   //qlevel = "level07.txt";
    // TO BE DELETED !!
    
    Level lev(assetPath());
    Level::LoadResult result = lev.loadLevel(level);
    
    if (result == Level::load_fail_file_not_found || getLevel() > 99) {
        cerr << "Could not find data file\n";
        return GWSTATUS_PLAYER_WON;
    }
    else if (result == Level::load_fail_bad_format) {
        cerr << "Level not formatted correctly\n";
        return GWSTATUS_LEVEL_ERROR;
    }
    else if (result == Level::load_success) {
        cerr << "Successfully loaded level\n";
        
        // get contents at each point of 15x15 grid
        for (int r = 0; r < VIEW_HEIGHT + 1; r++) {
            for (int c = 0; c < VIEW_WIDTH + 1; c++) {
                Level::MazeEntry ge = lev.getContentsOf(r, c);
                switch(ge) {
                    case Level::empty:
                        break;
                    case Level::exit:
                        actorList.push_back(new Exit(this, r, c));
                        break;
                    case Level::player:
                        m_avatar = new Avatar(this, r, c);
                        break;
                    case Level::horiz_ragebot:
                        actorList.push_back(new RageBot(this, r, c, 0));
                        break;
                    case Level::vert_ragebot:
                        actorList.push_back(new RageBot(this, r, c, 270));
                        break;
                    case Level::thiefbot_factory:
                        actorList.push_back(new ThiefBotFactory(this, r, c, false));
                        break;
                    case Level::mean_thiefbot_factory:
                        actorList.push_back(new ThiefBotFactory(this, r, c, true));
                        break;
                    case Level::wall:
                        actorList.push_back(new Wall(this, r, c));
                        break;
                    case Level::marble:
                        actorList.push_back(new Marble(this, r, c));
                        break;
                    case Level::pit:
                        actorList.push_back(new Pit(this, r, c));
                        break;
                    case Level::crystal:
                        actorList.push_back(new Crystal(this, r, c));
                        crystalsLeft++;
                        break;
                    case Level::restore_health:
                        actorList.push_back(new RestoreHealth(this, r, c));
                        break;
                    case Level::extra_life:
                        actorList.push_back(new ExtraLife(this, r, c));
                        break;
                    case Level::ammo:
                        actorList.push_back(new Ammo(this, r, c));
                        break;
                }
            }
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

// asks actors to do something + disposing actors
int StudentWorld::move()
{
    if (bonus > 0) { bonus--; } // decrement bonus
    tick++; // update tick
    updateDisplayText(); // update game status line
    m_avatar->doSomething();
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getStatus()) { // if alive
            (*it)->doSomething();
    
            if (!m_avatar->getStatus()) {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            if (finishLevel) {
                increaseScore(2000 + bonus);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    
   // remove newly dead characters
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if (!(*it)->getStatus()) {
            delete *it;
            it = actorList.erase(it);
        }
    }
        
	return GWSTATUS_CONTINUE_GAME;
}

// called when player completes current level/loses life
void StudentWorld::cleanUp()
{
    // deletes avatar object
    delete m_avatar;
    m_avatar = nullptr;
    
    // looping through list and deallocating memory
    for (auto a : actorList)
        delete a;
    actorList.clear();
}

// destructor
StudentWorld::~StudentWorld()
{
    cleanUp();
}

// function for displaying text
void StudentWorld::updateDisplayText() {
    ostringstream oss; // declares ostringstream object
    oss << "Score: "; // appends "Score: "
    oss.fill('0'); // sets fill character to '0', will fill spaces
    oss << setw(7) << getScore(); // sets the width to 6 and adds score
    
    oss << "  Level: ";
    oss.fill('0');
    oss << setw(2) << getLevel();
    
    oss << "  Lives: ";
    oss.fill(' ');
    oss << setw(2) << getLives();

    oss << "  Health: ";
    oss << setw(3) << (m_avatar->getHealth() / 20.0) * 100 << "%";
    
    oss << "  Ammo: ";
    oss << setw(3) << m_avatar->getAmmo();
    
    oss << "  Bonus: ";
    oss << setw(4) << bonus;
    
    string s = oss.str();
    setGameStatText(s);
}

/* ///////////////////////////////////////////////////
 
 
                   HELPER FUNCTIONS
 
 
 ///////////////////////////////////////////////////*/

// returns the actor at a specific position
Actor* StudentWorld::getActorAtPos(double x, double y) {
    if (m_avatar->getX() == x && m_avatar->getY() == y)
        return m_avatar;
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getX() == x && (*it)->getY() == y)
            return *it;
    }
    return nullptr;
}

/* /////////////// PLAYER FUNCTIONS ///////////////////*/

int StudentWorld::getPlayerDirection() {
    return m_avatar->getDirection();
}

bool StudentWorld::onSameSquareAsPlayer(double x, double y) {
    return (m_avatar->getX() == x && m_avatar->getY() == y);
}

// check if player can move to location
bool StudentWorld::isInBounds(double x, double y) {
    if (x < 0 || x > VIEW_WIDTH - 1 || y < 0 || y > VIEW_HEIGHT - 1)
        return false;
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getX() == x && (*it)->getY() == y && !(*it)->avatarOverlap()) {
            if ((*it)->canPush())
                return true;
            else
                return false;
        }
    }
    return true;
}

/* ///////////////// MARBLE FUNCTION //////////////////*/

bool StudentWorld::canMarbleMove(Actor* actor, double x, double y) {
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getX() == x && (*it)->getY() == y && !(*it)->marbleOverlap()) {
            return false;
        }
        if ((*it)->getX() == x && (*it)->getY() == y && (*it)->marbleOverlap()) { // only object it can overlap is a pit
            // delete the marble
            actor->updateStatus(false);
            actor->setVisible(false);
            
            // destroy the pit
            (*it)->die();
            return true;
        }
    }
    return true;
}

/* ////// INTERACTABLE OBJECTS /////////*/

void StudentWorld::restoreHealth() {
    m_avatar->setHealth(20);
    increaseScore(500);
}

void StudentWorld::restoreLife() {
    incLives();
    increaseScore(1000);
}

void StudentWorld::restoreAmmo() {
    m_avatar->updatePeas(20);
    increaseScore(100);
}

void StudentWorld::foundCrystal() {
    playSound(SOUND_GOT_GOODIE);
    increaseScore(50);
    crystalsLeft--;
    
    if (crystalsLeft == 0)
        playSound(SOUND_REVEAL_EXIT);
}

/* //////////// PEA FUNCTIONS /////////////*/

// returns 1, 2, or 3 depending on collision
int StudentWorld::overlapPea(double x, double y) {
    if (m_avatar->getX() == x && m_avatar->getY() == y && m_avatar->canPeaDamage()) {
        //cerr << "damage player" << endl;
        return 1;
    }
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getX() == x && (*it)->getY() == y) {
            if ((*it)->canPeaDamage()) {
                //cerr << "damage robot" << endl;
                return 1;
            }
            if ((*it)->canPeaHit()) {
                //cerr << "thing2" << endl;
                return 2;
            }
            if (!(*it)->canPeaDamage() && !(*it)->canPeaHit() && (*it)->isObjectDetectable()) {
                //cerr << "thing3" << endl;
                return 3;
            }
        }
    }
    return -1; // error
}

// creates pea object and adds to actorList
void StudentWorld::constructPea(double x, double y, int direction) {
    actorList.push_back(new Pea(this, x, y, direction));
}

/* ///////////////// ROBOT FUNCTIONS /////////////////*/

void StudentWorld::constructMeanThiefBot(double x, double y) {
    actorList.push_back(new MeanThiefBot(this, x, y));
    playSound(SOUND_ROBOT_BORN);
}

void StudentWorld::constructRegularThiefBot(double x, double y) {
    actorList.push_back(new RegularThiefBot(this, x, y));
    playSound(SOUND_ROBOT_BORN);
}

bool StudentWorld::onSameSquareAsGoodie(double x, double y) {
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->isCollectable() && x == (*it)->getX() && y == (*it)->getY())
            return true;
    }
    return false;
}

int StudentWorld::countSurroundingThiefBots(double x, double y) {
    int count = 0;
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        int posX = (*it)->getX();
        int posY = (*it)->getY();
        if ((*it)->isThiefBot() && posX > x - 3 && posX < x + 3 && posY > y - 3 && posY < y + 3) // in the 3 squares radius
            count++;
    }
    return count;
}

bool StudentWorld::canRobotMove(Actor* actor, double x, double y) const {
    if (m_avatar->getX() == x && m_avatar->getY() == y)
        return false;
    
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if ((*it)->getX() == x && (*it)->getY() == y && !(*it)->avatarOverlap()) {
            return false;
        }
    }
    return true;
}

bool StudentWorld::robotCanShootPlayer(double x, double y, int dir) {
    // up - 1, down - 2, left - 3, right - 4
    
    if (!sameRowColAsPlayer(x, y, dir))
        return false;
    
    double playerX = m_avatar->getX();
    double playerY = m_avatar->getY();

    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        // check if there is an obstacle in that row/column
        
        double obstacleX = (*it)->getX();
        double obstacleY = (*it)->getY();
        
        switch(dir) {
            case 1:
                if ((*it)->canPeaHit() && obstacleX == x && obstacleY > y && obstacleY < playerY)
                    return false;
                break;
            case 2:
                if ((*it)->canPeaHit() && obstacleX == x && obstacleY < y && obstacleY > playerY)
                    return false;
                break;
            case 3:
                if ((*it)->canPeaHit() && obstacleY == y && obstacleX < x && obstacleX > playerX)
                    return false;
                break;
            case 4:
                if ((*it)->canPeaHit() && obstacleY == y && obstacleX > x && obstacleX < playerX)
                    return false;
                break;
        }
    }
    return true;
}

bool StudentWorld::sameRowColAsPlayer(double x, double y, int dir) {
    // up - 1, down - 2, left - 3, right - 4
        
    double playerX = m_avatar->getX();
    double playerY = m_avatar->getY();
    
    // check if player is in the same row/col
    switch(dir) {
        case 1:
            if (playerX == x && playerY > y)
                return true;
            break;
        case 2:
            if (playerX == x && playerY < y)
                return true;
            break;
        case 3:
            if (playerX < x && playerY == y)
                return true;
            break;
        case 4:
            if (playerX > x && playerY == y)
                return true;
            break;
    }
    return false;
}


bool StudentWorld::onSameSquareAsThiefBot(double x, double y) {
    for (auto it = actorList.begin(); it != actorList.end(); it++) {
        if (x == (*it)->getX() && y == (*it)->getY() && (*it)->isThiefBot())
            return true;
    }
    return false;
}
