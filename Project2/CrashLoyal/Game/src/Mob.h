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

#pragma once

#include "Entity.h"

class Mob : public Entity {

public:
    Mob(const iEntityStats& stats, const Vec2& pos, bool isNorth);
    virtual void tick(float deltaTSec);

protected:
    void move(float deltaTSec);
    const Vec2* pickWaypoint();
    std::vector<Entity*> checkCollision();

    // Return the closest mob that is overlapping with the two given vectors. 
    Entity* getMostThreateningMob(Vec2 ahead, Vec2 ahead2);

    // Determine if the two given ahead vectors intersect and overlap with the given mob's position.
    bool lineIntersectsMob(Vec2 ahead, Vec2 ahead2, Entity* mob);

    // Check if any other mob is colliding with the mob. 
    // If it is, make adjustments to the mob's position to avoid collision. 
    // Do this while considering if the adjustment will cause any other collisions for the river, map, and buildings.
    void processCollision(Entity* otherMob, float elapsedTime);

    // Check if any building is colliding with the mob. 
    // If it is, make adjustments to the mob's position to avoid collision. 
    void checkBuildings(float elapsedTime, bool isNorth);

    // Check if the mob is colliding with or overlapping with the river. 
    // If it is, make adjustments to the mob's position to avoid collision/overlap. 
    void checkRiver(float elapsedTime);

    // Check if the mob is colliding with or overlapping with the edge of the map screen. 
    // If it is, make adjustments to the mob's position to avoid the mob going outside of the map.
    bool checkMapEdges(float elapsedTime);

    // Determine if the given Vec2 position is going outside of the map. 
    bool checkMapEdgesCollides(Vec2 newPos);

    // Determine if the given Vec2 position is overlapping with the river.
    bool checkRiverEdgesCollides(Vec2 newPos);

    // Determine if the given Vec2 position is overlapping/colliding with the building.
    bool checkBuildingsCollides(Vec2 newPos, bool isNorth);

private:
    const Vec2* m_pWaypoint;
    Vec2 targetPos;
    bool xStop;
    bool yStop;
};
