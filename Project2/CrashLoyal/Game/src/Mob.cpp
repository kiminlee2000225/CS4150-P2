// MIT License
// 
// Copyright(c) 2020 Arthur Bacon and Kevin Dill
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Mob.h"

#include "Constants.h"
#include "Game.h"
#include "Player.h"

#include <algorithm>
#include <vector>


Mob::Mob(const iEntityStats& stats, const Vec2& pos, bool isNorth)
    : Entity(stats, pos, isNorth)
    , m_pWaypoint(NULL)
{
    assert(dynamic_cast<const iEntityStats_Mob*>(&stats) != NULL);
}

void Mob::tick(float deltaTSec)
{
    // Tick the entity first.  This will pick our target, and attack it if it's in range.
    Entity::tick(deltaTSec);

    // if our target isn't in range, move towards it.
    if (!targetInRange())
    {
        move(deltaTSec);
    }
}

void Mob::move(float deltaTSec)
{
    // If we have a target and it's on the same side of the river, we move towards it.
    //  Otherwise, we move toward the bridge.
    bool bMoveToTarget = false;
    if (!!m_pTarget)
    {    
        bool imTop = m_Pos.y < (GAME_GRID_HEIGHT / 2);
        bool otherTop = m_pTarget->getPosition().y < (GAME_GRID_HEIGHT / 2);

        if (imTop == otherTop)
        {
            bMoveToTarget = true;
        }
    }

    Vec2 destPos;
    if (bMoveToTarget)
    { 
        m_pWaypoint = NULL;
        destPos = m_pTarget->getPosition();
    }
    else
    {
        if (!m_pWaypoint)
        {
            m_pWaypoint = pickWaypoint();
        }
        destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
    }


    // compare it to all mobs and turrets (for loop)
    // have a vector of distRemaining and moveVec
    // get the distance to other mobs

    // Actually do the moving
    Vec2 moveVec = destPos - m_Pos;
    float distRemaining = moveVec.normalize();
    float moveDist = m_Stats.getSpeed() * deltaTSec;

    // if we're moving to m_pTarget, don't move into it
    if (bMoveToTarget)
    {
        assert(m_pTarget);
        distRemaining -= (m_Stats.getSize() + m_pTarget->getStats().getSize()) / 2.f;
        distRemaining = std::max(0.f, distRemaining);
    }

    Vec2 velocityVector;
    velocityVector.x = m_Pos.x + moveDist;
    velocityVector.y = m_Pos.y + moveDist;

    if (moveDist <= distRemaining)
    {
        const float MAX_SEE_AHEAD = 5;
        Vec2 ahead;
        ahead.x = m_Pos.x + moveVec.normalize() *MAX_SEE_AHEAD;
        ahead.y = m_Pos.y + moveVec.normalize() *MAX_SEE_AHEAD;
        Vec2 ahead2 = ahead * 0.5;
        Entity* mostThreateningMob = getMostThreateningMob(ahead, ahead2);
        // std::pair<Entity*, const Vec2> pair = getMostThreateningMob();
        if (mostThreateningMob != nullptr) {
            std::cout << std::string(" most threatening mob is NOT null ") << std::endl;
            std::cout <<  std::string(" first ")  << std::endl;
            Vec2 avoidanceForce;
            avoidanceForce = ahead - mostThreateningMob->getPosition();
            avoidanceForce.normalize();
            float squareRadius = ((float)sqrt(2) * mostThreateningMob->getStats().getSize()) / 2;
            avoidanceForce *= squareRadius;
            avoidanceForce /= m_Stats.getMass();

            // 2) multiply combined steering force (acceleration) by elapsed time (deltaTSec), 
            // add to previous velocity
            avoidanceForce *= deltaTSec;

            // moveVec += avoidanceForce;
            // float newVelocity = m_Stats.getSpeed() + avoidanceForce;
            // float newVelocityX = m_Stats.getSpeed() + accX;
            // float newVelocityY = m_Stats.getSpeed() + accY;
            float newVelocityX = m_Stats.getSpeed() + avoidanceForce.x;
            float newVelocityY = m_Stats.getSpeed() + avoidanceForce.y;

            // 3) check and clamp so speed doesn't exceed max
            // v = d / t
            // float moveDist = m_Stats.getSpeed() * deltaTSec;
            // float newVelocity = distance / deltaTSec;
            // float newVelocity = m_Stats.getSpeed(); // change
            float maxVelocity = m_Stats.getSpeed();
            if (newVelocityX > maxVelocity) {
                newVelocityX = maxVelocity;
            }
            if (newVelocityY > maxVelocity) {
                newVelocityY = maxVelocity;
            }

            // 4) multipy the clamped velocity by elapsed time (deltaTSec), 
            // add to previous position
            // float newMoveDist = newVelocity * deltaTSec;
            float newMoveDistX = newVelocityX * deltaTSec;
            float newMoveDistY = newVelocityY * deltaTSec;

            // float newPosX = m_Pos.x + newMoveDistX;
            // float newPosY = m_Pos.y + newMoveDistY;
            // moveVec.x += newMoveDistX
            // m_Pos += moveVec * newMoveDist;
            // m_Pos += newMoveDist;

            // 5) check for position is off map

            // 6) set position to be the calculated/new position
            m_Pos.x += newMoveDistX;
            m_Pos.y += newMoveDistY;
        }
        else {
           //  std::cout << std::string(" most threatening mob is null ") << std::endl;
            m_Pos += moveVec * moveDist;
        }
    }
    else
    {
        m_Pos += moveVec * distRemaining;
        std::cout << std::string(" second ") << std::endl;

        // if the destination was a waypoint, find the next one and continue movement
        if (m_pWaypoint)
        {
            m_pWaypoint = pickWaypoint();
            destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
            moveVec = destPos - m_Pos;
            moveVec.normalize();
            m_Pos += moveVec * distRemaining;
        }
    }

    // PROJECT 2: This is where your collision code will be called from
    // Mob* otherMob = checkCollision();
    std::vector<Entity*> otherMobs = checkCollision();
    for (Entity* otherMob : otherMobs) {
        if (otherMob) {
            processCollision(otherMob, deltaTSec, moveVec);
        }
    }
}

//std::pair<Entity*, const Vec2> Mob::getMostThreateningMob() {
 Entity* Mob::getMostThreateningMob(Vec2 ahead, Vec2 ahead2) {
    Entity* mostThreateningMob = nullptr;
    // float minDist = FLT_MAX;
    //Vec2 ahead;
    const Player& northPlayer = Game::get().getPlayer(true);
    for (Entity* pOtherMob : northPlayer.getMobs())
    {
        if (this == pOtherMob)
        {
            continue;
        }

        //const float MAX_SEE_AHEAD = 5;
        //Vec2 currAhead;
        //Vec2 moveVec = pOtherMob->getPosition() - this->getPosition();
        //currAhead.x = this->getPosition().x + moveVec.normalize();
        //currAhead.y = this->getPosition().y + moveVec.normalize();
        //Vec2 ahead2 = currAhead * 0.5;

        bool collision = lineIntersectsMob(ahead, ahead2, pOtherMob);
        // std::cout << collision << std::endl;
        if (collision && (mostThreateningMob == nullptr || m_Pos.dist(pOtherMob->getPosition()) < m_Pos.dist(mostThreateningMob->getPosition()))) {
            mostThreateningMob = pOtherMob;
         //  ahead = currAhead;
        }
    }

    //const Player& southPlayer = Game::get().getPlayer(false);
    //for (Entity* pOtherMob : southPlayer.getMobs())
    //{
    //    if (this == pOtherMob)
    //    {
    //        continue;
    //    }

    //    const float MAX_SEE_AHEAD = 5;
    //    Vec2 currAhead;
    //    Vec2 moveVec = pOtherMob->getPosition() - this->getPosition();
    //    currAhead.x = (this->getPosition().x + moveVec.normalize()) * MAX_SEE_AHEAD;
    //    currAhead.y = (this->getPosition().y + moveVec.normalize()) * MAX_SEE_AHEAD;
    //    Vec2 ahead2 = currAhead * 0.5;

    //    bool collision = lineIntersectsMob(currAhead, ahead2, pOtherMob);
    //    std::cout << collision << std::endl;
    //    if (collision && (mostThreateningMob == nullptr || m_Pos.dist(pOtherMob->getPosition()) < m_Pos.dist(mostThreateningMob->getPosition()))) {
    //        mostThreateningMob = pOtherMob;
    //        ahead = currAhead;
    //    }
    //}
    //std::pair<Entity*, const Vec2> returnPair = std::make_pair(mostThreateningMob, ahead);
    //return returnPair;
    return mostThreateningMob;
}

 float distance(Entity* mob1, Vec2 ahead) {
    return sqrt((mob1->getPosition().x - ahead.x) * (mob1->getPosition().x - ahead.x) +
        (mob1->getPosition().y - ahead.y) * (mob1->getPosition().y - ahead.y));
 }

bool Mob::lineIntersectsMob(Vec2 ahead, Vec2 ahead2, Entity* mob){
    float mobRadius = ((float)sqrt(2) * mob->getStats().getSize()) / 2;
    if (this->getStats().getName() == mob->getStats().getName()) {
        //std::cout << std::string(" this mob name ") << this->getStats().getName() << std::endl;
        //std::cout << std::string(" other mob name ") << mob->getStats().getName() << std::endl;
        //std::cout << std::string(" mobRadius ") << mobRadius << std::endl;
        //std::cout << std::string(" dist ahead and mob ") << distance(mob, ahead) << std::endl;
        //std::cout << std::string(" dist ahead2 and mob ") << distance(mob, ahead2) << std::endl;
    }

    return distance(mob, ahead) <= mobRadius || distance(mob, ahead2) <= mobRadius;
}

const Vec2* Mob::pickWaypoint()
{
    float smallestDistSq = FLT_MAX;
    const Vec2* pClosest = NULL;

    for (const Vec2& pt : Game::get().getWaypoints())
    {
        // Filter out any waypoints that are behind (or barely in front of) us.
        // NOTE: (0, 0) is the top left corner of the screen
        float yOffset = pt.y - m_Pos.y;
        if ((m_bNorth && (yOffset < 1.f)) ||
            (!m_bNorth && (yOffset > -1.f)))
        {
            continue;
        }

        float distSq = m_Pos.distSqr(pt);
        if (distSq < smallestDistSq) {
            smallestDistSq = distSq;
            pClosest = &pt;
        }
    }

    return pClosest;
}

// PROJECT 2: 
//  1) return a vector of mobs that we're colliding with
//  2) handle collision with towers & river 
std::vector<Entity*> Mob::checkCollision()
{
    float thisSize = this->getStats().getSize();
    Vec2 thisPos = this->getPosition();

    std::vector<Entity*> collidingMobs;
    const Player& northPlayer = Game::get().getPlayer(true);
    for (Entity* pOtherMob : northPlayer.getMobs())
    {
        if (this == pOtherMob) 
        {
            continue;
        }

        // PROJECT 2: YOUR CODE CHECKING FOR A COLLISION GOES HERE
        float halfSize = pOtherMob->getStats().getSize() / 2;
        Vec2 pos = pOtherMob->getPosition();

        float r1RightEdge = pos.x + halfSize;
        float r1LeftEdge = pos.x - halfSize;
        float r1TopEdge = pos.y + halfSize;
        float r1BottomEdge = pos.y - halfSize;

        float r2RightEdge = thisPos.x + halfSize;
        float r2LeftEdge = thisPos.x - halfSize;
        float r2TopEdge = thisPos.y + halfSize;
        float r2BottomEdge = thisPos.y - halfSize;

        if (r1RightEdge >= r2LeftEdge &&
            r1LeftEdge <= r2RightEdge &&
            r1TopEdge >= r2BottomEdge &&
            r1BottomEdge <= r2TopEdge) {
            collidingMobs.push_back(pOtherMob);
             //std::cout << this->getStats().getName() + std::string(" and ") + pOtherMob->getStats().getName() << std::endl;
        }
    }

    const Player& southPlayer = Game::get().getPlayer(false);
    for (Entity* pOtherMob : southPlayer.getMobs())
    {
        if (this == pOtherMob)
        {
            continue;
        }

        // PROJECT 2: YOUR CODE CHECKING FOR A COLLISION GOES HERE
        float halfSize = pOtherMob->getStats().getSize() / 2;
        Vec2 pos = pOtherMob->getPosition();

        float r1RightEdge = pos.x + halfSize;
        float r1LeftEdge = pos.x - halfSize;
        float r1TopEdge = pos.y + halfSize;
        float r1BottomEdge = pos.y - halfSize;

        float r2RightEdge = thisPos.x + halfSize;
        float r2LeftEdge = thisPos.x - halfSize;
        float r2TopEdge = thisPos.y + halfSize;
        float r2BottomEdge = thisPos.y - halfSize;

        if (r1RightEdge >= r2LeftEdge &&
            r1LeftEdge <= r2RightEdge &&
            r1TopEdge >= r2BottomEdge &&
            r1BottomEdge <= r2TopEdge) {
            collidingMobs.push_back(pOtherMob);
            //std::cout << this->getStats().getName() + std::string(" and ") + pOtherMob->getStats().getName() << std::endl;
        }
    }

    return collidingMobs;
}

void Mob::processCollision(Entity* otherMob, float deltaTSec, Vec2 moveVec) 
{
    // PROJECT 2: YOUR COLLISION HANDLING CODE GOES HERE
    
    // 1) combine steering forces to apply together (acceleration)
    // ^ is this the avoidance force?
    //Vec2 avoidanceForce;
    //avoidanceForce = moveVec - otherMob->getPosition();
    //avoidanceForce.normalize();
    //float squareRadius = ((float) sqrt(2) * otherMob->getStats().getSize()) / 2;
    //avoidanceForce *= squareRadius;
    //avoidanceForce /= m_Stats.getMass();

    //// 2) multiply combined steering force (acceleration) by elapsed time (deltaTSec), 
    //// add to previous velocity
    //avoidanceForce *= deltaTSec;

    //// moveVec += avoidanceForce;
    //// float newVelocity = m_Stats.getSpeed() + avoidanceForce;
    //// float newVelocityX = m_Stats.getSpeed() + accX;
    //// float newVelocityY = m_Stats.getSpeed() + accY;
    //float newVelocityX = m_Stats.getSpeed() + avoidanceForce.x;
    //float newVelocityY = m_Stats.getSpeed() + avoidanceForce.y;

    //// 3) check and clamp so speed doesn't exceed max
    //// v = d / t
    //// float moveDist = m_Stats.getSpeed() * deltaTSec;
    //// float newVelocity = distance / deltaTSec;
    //// float newVelocity = m_Stats.getSpeed(); // change
    //float maxVelocity = m_Stats.getSpeed();
    //if (newVelocityX > maxVelocity) {
    //    newVelocityX = maxVelocity;
    //}
    //if (newVelocityY > maxVelocity) {
    //    newVelocityY = maxVelocity;
    //}

    //// 4) multipy the clamped velocity by elapsed time (deltaTSec), 
    //// add to previous position
    //// float newMoveDist = newVelocity * deltaTSec;
    //float newMoveDistX = newVelocityX * deltaTSec;
    //float newMoveDistY = newVelocityY * deltaTSec;

    //// float newPosX = m_Pos.x + newMoveDistX;
    //// float newPosY = m_Pos.y + newMoveDistY;
    //// moveVec.x += newMoveDistX
    //// m_Pos += moveVec * newMoveDist;
    //// m_Pos += newMoveDist;

    //// 5) check for position is off map

    //// 6) set position to be the calculated/new position
    // m_Pos.x += newMoveDistX;
    // m_Pos.y += newMoveDistY;

    // m_Pos += moveVec * moveDist;
    // m_Pos.x = newPosX + (moveVec.x * newMoveDistX);
    // m_Pos.y = newPosY + (moveVec.y * newMoveDistY);
}
