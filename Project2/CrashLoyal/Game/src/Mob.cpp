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

	targetPos = destPos;
	// Actually do the moving
	Vec2 moveVec = destPos - m_Pos;
	Vec2 normalizedVec = moveVec / moveVec.length();
	Vec2 velocityVector = normalizedVec * m_Stats.getSpeed();

	float distRemaining = moveVec.normalize();
	float moveDist = m_Stats.getSpeed() * deltaTSec;

	// if we're moving to m_pTarget, don't move into it
	if (bMoveToTarget)
	{
		assert(m_pTarget);
		distRemaining -= (m_Stats.getSize() + m_pTarget->getStats().getSize()) / 2.f;
		distRemaining = std::max(0.f, distRemaining);
	}

	//Vec2 velocityVector;
	//velocityVector.x = m_Pos.x + moveDist;
	//velocityVector.y = m_Pos.y + moveDist;

	if (moveDist <= distRemaining)
	{
		// float offsetVector = velocityVector * deltaTSec; // <-- distance
		 //Vec2 nextPos;
		 //nextPos.x = m_Pos.x + offsetVector;
		 //nextPos.y = m_Pos.y + offsetVector;
		 //m_Pos = nextPos;

		// std::cout << std::string(" first ") << std::endl;
		const float MAX_SEE_AHEAD = 5;
		Vec2 ahead;
		ahead = m_Pos + normalizedVec;
		ahead *= MAX_SEE_AHEAD;
		//ahead.x = m_Pos.x + normalizedVec *MAX_SEE_AHEAD;
		//ahead.y = m_Pos.y + normalizedVec *MAX_SEE_AHEAD;
		Vec2 ahead2 = ahead * 0.5;
		Entity* mostThreateningMob = getMostThreateningMob(ahead, ahead2);
		// std::pair<Entity*, const Vec2> pair = getMostThreateningMob();
		if (mostThreateningMob != nullptr) {
			std::cout << std::string(" most threatening mob is NOT null ") << std::endl;
			// std::cout << std::string(" most threatening mob is NOT null ") << std::endl;
			 // std::cout <<  std::string(" first ")  << std::endl;

			 // notes from Kevin Dill:
			 // calculate for vectors (velocity) with speed and direction, not just speed and its ending position. 
			 // my acc is being used as an offset. 
			 // might not be multiplying by elapsedTime which is deltaTSec
			 // newPos is the previous position + velocity * deltaTSec
			 // combine all steering forces to get the overall steering force. 
			 // m / s^2   * s .     acc * elapsedTime = velocity
			 // 
			 // use assert() for debugging.

			Vec2 avoidanceForce;
			avoidanceForce = ahead - mostThreateningMob->getPosition();
			float squareRadius = ((float)sqrt(2) * mostThreateningMob->getStats().getSize()) / 2;
			avoidanceForce = avoidanceForce / avoidanceForce.length();
			Vec2 force = avoidanceForce * squareRadius;
			Vec2 acc = force / m_Stats.getMass();
			// float acc = force / m_Stats.getMass();

			 // 2) multiply combined steering force (acceleration) by elapsed time (deltaTSec), 
			 // add to previous velocity
			Vec2 newVelocity = acc * deltaTSec; // <-- velocity
			Vec2 nextNewVelocity = velocityVector + newVelocity;

			if (nextNewVelocity.length() > m_Stats.getSpeed()) {
				nextNewVelocity = nextNewVelocity / nextNewVelocity.length();
				nextNewVelocity *= m_Stats.getSpeed();
			}
			m_Velocity = nextNewVelocity;

			Vec2 newOffsetVector = nextNewVelocity * deltaTSec;
			m_Pos += newOffsetVector;
		}
		else {
			// std::cout << std::string(" most threatening mob is null ") << std::endl;
			// Vec2 nextPos = m_Pos + (moveVec * moveDist);
			Vec2 velocity = velocityVector;
			Vec2 velocityNormal;
			Vec2 offset;
			//  std::cout << std::string("a: ") << velocity.length() << std::endl;
			if (velocity.length() > m_Stats.getSpeed()) {
				velocity /= velocity.length();
				velocity *= m_Stats.getSpeed();
			}
			Vec2 distance = velocity * deltaTSec;

			Vec2 nextPos = m_Pos + distance;
			// nextPos = checkBuildingCollision(nextPos, deltaTSec, velocity); // <-- comment this out 
			//Vec2 buildingVelocity = checkBuildingCollision(nextPos, deltaTSec);
			//velocity = velocityVector + buildingVelocity;
			//if (velocity.length() > m_Stats.getSpeed()) {
			//    velocity /= velocity.length();
			//    velocity *= m_Stats.getSpeed();
			//}
			//distance = velocity * deltaTSec;
			//nextPos = m_Pos + distance;
			m_Pos = nextPos;
		}
	}
	else
	{
		//  m_Pos += moveVec * distRemaining;
		 //Vec2 nextPos = m_Pos + (moveVec * distRemaining);
		 //nextPos = checkBuildingCollision(nextPos, deltaTSec);
		 //// nextPos = checkMobCollision(nextPos);
		 //m_Pos = nextPos;
		// std::cout << std::string(" second ") << std::endl;

		 // if the destination was a waypoint, find the next one and continue movement
		if (m_pWaypoint)
		{
			m_pWaypoint = pickWaypoint();
			//destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
			//moveVec = destPos - m_Pos;
			//moveVec.normalize();

			//Vec2 nextPos = m_Pos + (moveVec * distRemaining);
			//nextPos = checkBuildingCollision(nextPos, deltaTSec);
			//// nextPos = checkMobCollision(nextPos);
			//m_Pos = nextPos;
			// m_Pos += moveVec * distRemaining;
		}
	}

	// PROJECT 2: This is where your collision code will be called from
	// Mob* otherMob = checkCollision();
	// Might be better to do this before the position has been updated. Mgiht make it smoother. 
	std::vector<Entity*> otherMobs = checkCollision();
	for (Entity* otherMob : otherMobs) {
		if (otherMob) {
			processCollision(otherMob, deltaTSec);
		}
	}
	checkBuildings(deltaTSec, true);
	checkBuildings(deltaTSec, false);
	checkRiver(deltaTSec);
	checkMapEdges(deltaTSec);
}


void Mob::checkMapEdges(float elapsedTime) {
	float shiftSize = 0;
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	Vec2 p = Vec2(this->m_Pos.x - targetPos.x, this->m_Pos.y - targetPos.y);
	p.normalize();
	bool adjusted = false;
	if (this->getPosition().x > SCREEN_WIDTH_PIXELS) {
		shiftSize = this->getPosition().x - SCREEN_WIDTH_PIXELS;
		shiftSize = std::min(maxDist, shiftSize);
		p.x -= shiftSize;
		adjusted = true;
	}
	if (this->getPosition().x < 0.0f) {
		shiftSize = std::abs(this->getPosition().x);
		shiftSize = std::min(maxDist, shiftSize);
		p.x += shiftSize;
		adjusted = true;
	}
	if (this->getPosition().y > SCREEN_HEIGHT_PIXELS) {
		shiftSize = this->getPosition().y - SCREEN_HEIGHT_PIXELS;
		shiftSize = std::min(maxDist, shiftSize);
		p.y -= shiftSize;
		adjusted = true;
	}
	if (this->getPosition().y < 0) {
		shiftSize = std::abs(this->getPosition().y);
		shiftSize = std::min(maxDist, shiftSize);
		p.y += shiftSize;
		adjusted = true;
	}
	if (adjusted) {
		p *= (float)this->getStats().getSpeed();
		p *= (float)elapsedTime;
		m_Pos += p;
	}
}

void Mob::checkRiver(float elapsedTime) {
	float shiftSize = 0.5f;
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	std::shared_ptr<Vec2>river;
	if (this->getPosition().x >= RIVER_LEFT_X
		&& this->getPosition().x <= RIVER_LEFT_X + (LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.0)
		&& this->getPosition().y >= RIVER_TOP_Y
		&& this->getPosition().y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2(RIVER_LEFT_X, RIVER_TOP_Y));
	}
	else if (this->getPosition().x >= (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5)
		&& this->getPosition().x <= (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5) + (SCREEN_WIDTH_PIXELS - RIGHT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.0)
		&& this->getPosition().y >= RIVER_TOP_Y
		&& this->getPosition().y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2((RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5), RIVER_TOP_Y));
	}
	else if (this->getPosition().x >= (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5)
		&& this->getPosition().x <= (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5) + (RIGHT_BRIDGE_CENTER_X - LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH)
		&& this->getPosition().y >= RIVER_TOP_Y
		&& this->getPosition().y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2((LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5), RIVER_TOP_Y));
	}
	else {
		return;
	}
	Vec2 p = Vec2(this->m_Pos.x - targetPos.x, this->m_Pos.y - targetPos.y);
	p.normalize();
	if (river->x == (BRIDGE_WIDTH + LEFT_BRIDGE_CENTER_X / 2.0) - 0.5) {
		if (this->m_Pos.x > river->x + ((RIGHT_BRIDGE_CENTER_X - LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH) / 2)) {
			shiftSize = std::min(maxDist, shiftSize);
			p.x += shiftSize;
		}
		else {
			shiftSize = std::min(maxDist, shiftSize);
			p.x -= shiftSize;
		}
	}
	else if (river->x == RIVER_LEFT_X) {
		shiftSize = std::min(maxDist, shiftSize);
		p.x += shiftSize;
	}
	else {
		shiftSize = std::min(maxDist, shiftSize);
		p.x -= shiftSize;
	}
	p *= (float)this->getStats().getSpeed();
	p *= (float)elapsedTime;
	m_Pos += p;
}

void Mob::processCollision(Entity* otherMob, float elapsedTime) {
	float shiftVal = 0.1f;
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	if (this->getStats().getMass() > otherMob->getStats().getMass()) {
		return;
	}
	else {
		Vec2 p = Vec2(this->m_Pos.x - otherMob->getPosition().x,
			this->m_Pos.y - otherMob->getPosition().y);
		p.normalize();
		float otherMobHalfSize = (float)otherMob->getStats().getSize() / 2;
		Vec2 otherMobPos = otherMob->getPosition();
		float thisSize = this->getStats().getSize();
		float halfSize = (float)thisSize / 2;

		float r1RightEdge = otherMobPos.x + otherMobHalfSize;
		float r1LeftEdge = otherMobPos.x - otherMobHalfSize;
		float r1TopEdge = otherMobPos.y + otherMobHalfSize;
		float r1BottomEdge = otherMobPos.y - otherMobHalfSize;

		float r2RightEdge = this->getPosition().x + halfSize;
		float r2LeftEdge = this->getPosition().x - halfSize;
		float r2TopEdge = this->getPosition().y + halfSize;
		float r2BottomEdge = this->getPosition().y - halfSize;

		if (r1RightEdge >= r2LeftEdge &&
			r1LeftEdge <= r2RightEdge &&
			r1TopEdge >= r2BottomEdge &&
			r1BottomEdge <= r2TopEdge) {
			// std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
			float shiftSize;
			if (r1RightEdge >= r2LeftEdge) {
				shiftSize = r1RightEdge - r2LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.x += shiftSize;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				shiftSize = r2RightEdge - r1LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.x -= shiftSize;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				shiftSize = r1TopEdge - r2BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y += shiftSize;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				shiftSize = r2TopEdge - r1BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y -= shiftSize;
			}
		}
		p *= (float)this->getStats().getSpeed();
		p *= (float)elapsedTime;
		m_Pos += p;
	}
}

void Mob::checkBuildings(float elapsedTime, bool isNorth) {
	float shiftSizeNew = 0.f;
	const Player& player = Game::get().getPlayer(isNorth);
	Vec2 velocityVec;
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	for (Entity* building : player.getBuildings())
	{
		float buildingHalfSize = building->getStats().getSize() / 2;
		Vec2 buildingPos = building->getPosition();
		float r1RightEdge = buildingPos.x + buildingHalfSize;
		float r1LeftEdge = buildingPos.x - buildingHalfSize;
		float r1TopEdge = buildingPos.y + buildingHalfSize;
		float r1BottomEdge = buildingPos.y - buildingHalfSize;

		float thisSize = this->getStats().getSize();
		float halfSize = (float)thisSize / 2;
		float r2RightEdge = m_Pos.x + halfSize;
		float r2LeftEdge = m_Pos.x - halfSize;
		float r2TopEdge = m_Pos.y + halfSize;
		float r2BottomEdge = m_Pos.y - halfSize;

		if (r1RightEdge >= r2LeftEdge &&
			r1LeftEdge <= r2RightEdge &&
			r1TopEdge >= r2BottomEdge &&
			r1BottomEdge <= r2TopEdge) {
			// std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
			float shiftSize;
			Vec2 p = Vec2(this->getPosition().x - building->getPosition().x,
				this->getPosition().y - building->getPosition().y);
			p.normalize();
			if (r1RightEdge >= r2LeftEdge) {
				shiftSize = r1RightEdge - r2LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.x += shiftSize;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				shiftSize = r2RightEdge - r1LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.x -= shiftSize;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				shiftSize = r1TopEdge - r2BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y += shiftSize;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				shiftSize = r2TopEdge - r1BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y -= shiftSize;
			}

			p *= (float)this->getStats().getSpeed();
			p *= (float)elapsedTime;
			m_Pos += p;
		}
	}
}

Vec2 Mob::checkBuildingCollision(Vec2 nextPos, float deltaTSec, Vec2 vel) {
	float thisSize = this->getStats().getSize();
	float halfSize = (float)thisSize / 2;
	Vec2 velocityVec = Vec2(0, 0);
	float maxDist = m_Stats.getSpeed() * (float)deltaTSec;

	const Player& northPlayer = Game::get().getPlayer(true);
	for (Entity* building : northPlayer.getBuildings())
	{
		float buildingHalfSize = building->getStats().getSize() / 2;
		Vec2 buildingPos = building->getPosition();

		float r1RightEdge = buildingPos.x + buildingHalfSize;
		float r1LeftEdge = buildingPos.x - buildingHalfSize;
		float r1TopEdge = buildingPos.y + buildingHalfSize;
		float r1BottomEdge = buildingPos.y - buildingHalfSize;

		float r2RightEdge = nextPos.x + halfSize;
		float r2LeftEdge = nextPos.x - halfSize;
		float r2TopEdge = nextPos.y + halfSize;
		float r2BottomEdge = nextPos.y - halfSize;

		if (r1RightEdge >= r2LeftEdge &&
			r1LeftEdge <= r2RightEdge &&
			r1TopEdge >= r2BottomEdge &&
			r1BottomEdge <= r2TopEdge) {
			std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
			float shiftSize;
			if (r1RightEdge >= r2LeftEdge) {
				shiftSize = r1RightEdge - r2LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.x += shiftSize;
				velocityVec.x += shiftSize;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				shiftSize = r2RightEdge - r1LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.x -= shiftSize;
				velocityVec.x -= shiftSize;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				shiftSize = r1TopEdge - r2BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.y += shiftSize;
				velocityVec.y += shiftSize;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				shiftSize = r2TopEdge - r1BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.y -= shiftSize;
				velocityVec.y -= shiftSize;
			}
			// return nextPos;
		}
	}

	const Player& southPlayer = Game::get().getPlayer(false);
	for (Entity* building : southPlayer.getBuildings())
	{
		float buildingHalfSize = building->getStats().getSize() / 2;
		Vec2 buildingPos = building->getPosition();

		float r1RightEdge = buildingPos.x + buildingHalfSize;
		float r1LeftEdge = buildingPos.x - buildingHalfSize;
		float r1TopEdge = buildingPos.y + buildingHalfSize;
		float r1BottomEdge = buildingPos.y - buildingHalfSize;

		float r2RightEdge = nextPos.x + halfSize;
		float r2LeftEdge = nextPos.x - halfSize;
		float r2TopEdge = nextPos.y + halfSize;
		float r2BottomEdge = nextPos.y - halfSize;

		if (r1RightEdge >= r2LeftEdge &&
			r1LeftEdge <= r2RightEdge &&
			r1TopEdge >= r2BottomEdge &&
			r1BottomEdge <= r2TopEdge) {
			std::cout << this->getStats().getName() << std::string(" gon collide south") << std::endl;
			float shiftSize;
			if (r1RightEdge >= r2LeftEdge) {
				shiftSize = r1RightEdge - r2LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.x += shiftSize;
				velocityVec.x += shiftSize;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				shiftSize = r2RightEdge - r1LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.x -= shiftSize;
				velocityVec.x -= shiftSize;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				shiftSize = r1TopEdge - r2BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.y += shiftSize;
				velocityVec.y += shiftSize;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				shiftSize = r2TopEdge - r1BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				nextPos.y -= shiftSize;
				velocityVec.y -= shiftSize;
			}
			//   return nextPos;
		}
	}
	// return velocityVec;
	 //Vec2 resultVel = vel + velocityVec;
	 ////if (resultVel.length() > m_Stats.getSpeed()) {
	 ////    resultVel /= resultVel.length();
	 ////    resultVel *= m_Stats.getSpeed();
	 ////}
	 //resultVel *= deltaTSec;
	 //Vec2 resultPos = m_Pos + resultVel;
	 //return resultPos;

	 //Vec2 newDistance = velocityVec * deltaTSec;
	 //if (newDistance.length() != 0) {
	 //    return nextPos + newDistance;
	 //}
	return velocityVec;

	// notes. 
	// I'm moving more than I am allowed to, so it looks like the mob is moving quick. It is also moving inside the buildling
	// cuz of that. 
	//To avoid this, somehow try to make sure the cap/max is effectively working.
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

	const Player& southPlayer = Game::get().getPlayer(false);
	for (Entity* pOtherMob : southPlayer.getMobs())
	{
		if (this == pOtherMob)
		{
			continue;
		}
		bool collision = lineIntersectsMob(ahead, ahead2, pOtherMob);
		if (collision && (mostThreateningMob == nullptr || m_Pos.dist(pOtherMob->getPosition()) < m_Pos.dist(mostThreateningMob->getPosition()))) {
			mostThreateningMob = pOtherMob;
		}
	}
	//std::pair<Entity*, const Vec2> returnPair = std::make_pair(mostThreateningMob, ahead);
	//return returnPair;
	return mostThreateningMob;
}

float distance(Entity* mob1, Vec2 ahead) {
	return sqrt((mob1->getPosition().x - ahead.x) * (mob1->getPosition().x - ahead.x) +
		(mob1->getPosition().y - ahead.y) * (mob1->getPosition().y - ahead.y));
}

bool Mob::lineIntersectsMob(Vec2 ahead, Vec2 ahead2, Entity* mob) {
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
	float halfSize = this->getStats().getSize() / 2;
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
		float mobHalfSize = (float)pOtherMob->getStats().getSize() / 2;
		Vec2 mobPos = pOtherMob->getPosition();

		float r1RightEdge = mobPos.x + mobHalfSize;
		float r1LeftEdge = mobPos.x - mobHalfSize;
		float r1TopEdge = mobPos.y + mobHalfSize;
		float r1BottomEdge = mobPos.y - mobHalfSize;

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
		float mobHalfSize = (float)pOtherMob->getStats().getSize() / 2;
		Vec2 mobPos = pOtherMob->getPosition();

		float r1RightEdge = mobPos.x + mobHalfSize;
		float r1LeftEdge = mobPos.x - mobHalfSize;
		float r1TopEdge = mobPos.y + mobHalfSize;
		float r1BottomEdge = mobPos.y - mobHalfSize;

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

//void Mob::processCollision(Entity* otherMob, float deltaTSec, Vec2 moveVec) 
//{
//    float thisSize = this->getStats().getSize();
//    float halfSize = (float) thisSize / 2;
//    // TODO: maybe make both mobs shift and not only one (split the shift distance) so that the game feels more smooth.
//    // This might not be possible with how the Entity is set up rn because i can't change otherMob's position 
//
//    const Player& northPlayer = Game::get().getPlayer(true);
//    for (Entity* otherMob : northPlayer.getMobs())
//    {
//        float otherMobHalfSize = (float)otherMob->getStats().getSize() / 2;
//        Vec2 otherMobPos = otherMob->getPosition();
//
//        float r1RightEdge = otherMobPos.x + otherMobHalfSize;
//        float r1LeftEdge = otherMobPos.x - otherMobHalfSize;
//        float r1TopEdge = otherMobPos.y + otherMobHalfSize;
//        float r1BottomEdge = otherMobPos.y - otherMobHalfSize;
//
//        float r2RightEdge = this->getPosition().x + halfSize;
//        float r2LeftEdge = this->getPosition().x - halfSize;
//        float r2TopEdge = this->getPosition().y + halfSize;
//        float r2BottomEdge = this->getPosition().y - halfSize;
//
//        if (r1RightEdge >= r2LeftEdge &&
//            r1LeftEdge <= r2RightEdge &&
//            r1TopEdge >= r2BottomEdge &&
//            r1BottomEdge <= r2TopEdge) {
//            // std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
//            float shiftSize;
//            if (r1RightEdge >= r2LeftEdge) {
//                shiftSize = r1RightEdge - r2LeftEdge;
//                m_Pos.x += shiftSize;
//            }
//            else if (r1LeftEdge <= r2RightEdge) {
//                shiftSize = r2RightEdge - r1LeftEdge;
//                m_Pos.x -= shiftSize;
//            }
//            else if (r1TopEdge >= r2BottomEdge) {
//                shiftSize = r1TopEdge - r2BottomEdge;
//                m_Pos.y += shiftSize;
//            }
//            else if (r1BottomEdge <= r2TopEdge) {
//                shiftSize = r2TopEdge - r1BottomEdge;
//                m_Pos.y -= shiftSize;
//            }
//            // return nextPos;
//        }
//    }
//
//    const Player& southPlayer = Game::get().getPlayer(false);
//    for (Entity* otherMob : southPlayer.getMobs())
//    {
//        float otherMobHalfSize = (float) otherMob->getStats().getSize() / 2;
//        Vec2 otherMobPos = otherMob->getPosition();
//
//        float r1RightEdge = otherMobPos.x + otherMobHalfSize;
//        float r1LeftEdge = otherMobPos.x - otherMobHalfSize;
//        float r1TopEdge = otherMobPos.y + otherMobHalfSize;
//        float r1BottomEdge = otherMobPos.y - otherMobHalfSize;
//
//        float r2RightEdge = this->getPosition().x + halfSize;
//        float r2LeftEdge = this->getPosition().x - halfSize;
//        float r2TopEdge = this->getPosition().y + halfSize;
//        float r2BottomEdge = this->getPosition().y - halfSize;
//
//        if (r1RightEdge >= r2LeftEdge &&
//            r1LeftEdge <= r2RightEdge &&
//            r1TopEdge >= r2BottomEdge &&
//            r1BottomEdge <= r2TopEdge) {
//            // std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
//            float shiftSize;
//            if (r1RightEdge >= r2LeftEdge) {
//                shiftSize = r1RightEdge - r2LeftEdge;
//                m_Pos.x += shiftSize;
//            }
//            else if (r1LeftEdge <= r2RightEdge) {
//                shiftSize = r2RightEdge - r1LeftEdge;
//                m_Pos.x -= shiftSize;
//            }
//            else if (r1TopEdge >= r2BottomEdge) {
//                shiftSize = r1TopEdge - r2BottomEdge;
//                m_Pos.y += shiftSize;
//            }
//            else if (r1BottomEdge <= r2TopEdge) {
//                shiftSize = r2TopEdge - r1BottomEdge;
//                m_Pos.y -= shiftSize;
//            }
//            //   return nextPos;
//        }
//    }
//    // return nextPos;
//}

//Vec2 Mob::checkMobCollision(Vec2 nextPos)
//{
//    float thisSize = this->getStats().getSize();
//    float halfSize = (float) thisSize / 2;
//    // TODO: maybe make both mobs shift and not only one (split the shift distance) so that the game feels more smooth.
//    // This might not be possible with how the Entity is set up rn because i can't change otherMob's position 
//
//    const Player& northPlayer = Game::get().getPlayer(true);
//    for (Entity* otherMob : northPlayer.getMobs())
//    {
//        float otherMobHalfSize = (float)otherMob->getStats().getSize() / 2;
//        Vec2 otherMobPos = otherMob->getPosition();
//
//        float r1RightEdge = otherMobPos.x + otherMobHalfSize;
//        float r1LeftEdge = otherMobPos.x - otherMobHalfSize;
//        float r1TopEdge = otherMobPos.y + otherMobHalfSize;
//        float r1BottomEdge = otherMobPos.y - otherMobHalfSize;
//
//        float r2RightEdge = nextPos.x + halfSize;
//        float r2LeftEdge = nextPos.x - halfSize;
//        float r2TopEdge = nextPos.y + halfSize;
//        float r2BottomEdge = nextPos.y - halfSize;
//
//        if (r1RightEdge >= r2LeftEdge &&
//            r1LeftEdge <= r2RightEdge &&
//            r1TopEdge >= r2BottomEdge &&
//            r1BottomEdge <= r2TopEdge) {
//            // std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
//            float shiftSize;
//            if (r1RightEdge >= r2LeftEdge) {
//                shiftSize = r1RightEdge - r2LeftEdge;
//                nextPos.x += shiftSize;
//            }
//            else if (r1LeftEdge <= r2RightEdge) {
//                shiftSize = r2RightEdge - r1LeftEdge;
//                nextPos.x -= shiftSize;
//            }
//            else if (r1TopEdge >= r2BottomEdge) {
//                shiftSize = r1TopEdge - r2BottomEdge;
//                nextPos.y += shiftSize;
//            }
//            else if (r1BottomEdge <= r2TopEdge) {
//                shiftSize = r2TopEdge - r1BottomEdge;
//                nextPos.y -= shiftSize;
//            }
//            // return nextPos;
//        }
//    }
//
//    const Player& southPlayer = Game::get().getPlayer(false);
//    for (Entity* otherMob : southPlayer.getMobs())
//    {
//        float otherMobHalfSize = (float)otherMob->getStats().getSize() / 2;
//        Vec2 otherMobPos = otherMob->getPosition();
//
//        float r1RightEdge = otherMobPos.x + otherMobHalfSize;
//        float r1LeftEdge = otherMobPos.x - otherMobHalfSize;
//        float r1TopEdge = otherMobPos.y + otherMobHalfSize;
//        float r1BottomEdge = otherMobPos.y - otherMobHalfSize;
//
//        float r2RightEdge = nextPos.x + halfSize;
//        float r2LeftEdge = nextPos.x - halfSize;
//        float r2TopEdge = nextPos.y + halfSize;
//        float r2BottomEdge = nextPos.y - halfSize;
//
//        if (r1RightEdge >= r2LeftEdge &&
//            r1LeftEdge <= r2RightEdge &&
//            r1TopEdge >= r2BottomEdge &&
//            r1BottomEdge <= r2TopEdge) {
//            // std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
//            float shiftSize;
//            if (r1RightEdge >= r2LeftEdge) {
//                shiftSize = r1RightEdge - r2LeftEdge;
//                nextPos.x += shiftSize;
//            }
//            else if (r1LeftEdge <= r2RightEdge) {
//                shiftSize = r2RightEdge - r1LeftEdge;
//                nextPos.x -= shiftSize;
//            }
//            else if (r1TopEdge >= r2BottomEdge) {
//                shiftSize = r1TopEdge - r2BottomEdge;
//                nextPos.y += shiftSize;
//            }
//            else if (r1BottomEdge <= r2TopEdge) {
//                shiftSize = r2TopEdge - r1BottomEdge;
//                nextPos.y -= shiftSize;
//            }
//            //   return nextPos;
//        }
//    }
//     return nextPos;
//}