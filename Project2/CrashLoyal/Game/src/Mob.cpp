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
		ahead = m_Pos + moveVec;
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
			// avoidanceForce = avoidanceForce / avoidanceForce.length();
			avoidanceForce.normalize();
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
			if (xStop) {
				std::cout << std::string(" x blocked HERE") << std::endl;
				nextNewVelocity.x = 0;
			}
			if (yStop) {
				nextNewVelocity.y = 0;
			}

			Vec2 newOffsetVector = nextNewVelocity * deltaTSec;
			m_Pos += newOffsetVector;
		}
		else {
			// std::cout << std::string(" most threatening mob is null ") << std::endl;
			// Vec2 nextPos = m_Pos + (moveVec * moveDist);
			Vec2 velocity = velocityVector;
			if (xStop) {
				std::cout << std::string(" x blocked ") << std::endl;
				velocity.x = 0;
			}
			if (yStop) {
				velocity.y = 0;
			}
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
		 // if the destination was a waypoint, find the next one and continue movement
		if (m_pWaypoint)
		{
			m_pWaypoint = pickWaypoint();
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

bool Mob::checkMapEdgesCollides(Vec2 newPos) {
	if (newPos.x > GAME_GRID_WIDTH) {
		return true;
	}
	if (newPos.x < 0.0f) {
		return true;
	}
	if (newPos.y > GAME_GRID_HEIGHT) {
		return true;
	}
	if (newPos.y < 0) {
		return true;
	}
	return false;
}

bool Mob::checkRiverEdgesCollides(Vec2 newPos) {
	std::cout << std::string(" method called at least ") << std::endl;
	std::shared_ptr<Vec2>river;
	if (newPos.x >= RIVER_LEFT_X
		&& newPos.x <= RIVER_LEFT_X + (LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.0)
		&& newPos.y >= RIVER_TOP_Y
		&& newPos.y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2(RIVER_LEFT_X, RIVER_TOP_Y));
	}
	else if (newPos.x >= (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5)
		&& newPos.x <= (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5) + (SCREEN_WIDTH_PIXELS - RIGHT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.0)
		&& newPos.y >= RIVER_TOP_Y
		&& newPos.y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2((RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5), RIVER_TOP_Y));
	}
	else if (newPos.x >= (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5)
		&& newPos.x <= (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5) + (RIGHT_BRIDGE_CENTER_X - LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH)
		&& newPos.y >= RIVER_TOP_Y
		&& newPos.y <= RIVER_TOP_Y + (RIVER_BOT_Y - RIVER_TOP_Y)) {
		river = std::make_shared<Vec2>(Vec2((LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.0 - 0.5), RIVER_TOP_Y));
	}
	else {
		std::cout << std::string(" does not  river ") << std::endl;
		xStop = false;
		return false;
	}
	std::cout << std::string(" has river ") << std::endl;
	xStop = true;
	return true;
	//if (river->x == (BRIDGE_WIDTH + LEFT_BRIDGE_CENTER_X / 2.0) - 0.5) {
	//	if (newPos.x > river->x + ((RIGHT_BRIDGE_CENTER_X - LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH) / 2)) {
	//		xStop = true;
	//		return true;
	//	}
	//	else {
	//		xStop = true;
	//		return true;
	//	}
	//}
	//else if (river->x == RIVER_LEFT_X) {
	//	xStop = true;
	//	return true;
	//}
	//else {
	//	xStop = true;
	//	return true;
	//}

	//return false;
}

bool Mob::checkMapEdges(float elapsedTime) {
	float shiftSize = this->getStats().getSize();
	float mobSize = this->getStats().getSize();
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	Vec2 p = Vec2(this->m_Pos.x - targetPos.x, this->m_Pos.y - targetPos.y);
	p.normalize();
	bool adjusted = false;
	if (this->getPosition().x + (mobSize / 2) > GAME_GRID_WIDTH) {
		std::cout << std::string("going off screen to right") << std::endl;
		shiftSize = (this->getPosition().x + (mobSize / 2)) - GAME_GRID_WIDTH;
		p.x -= shiftSize;
		adjusted = true;
		xStop = true;
	}
	else {
		xStop = false;
	}
	if (this->getPosition().x - (mobSize / 2) < 0.0f) {
		std::cout << std::string("going off screen to left") << std::endl;
		shiftSize = std::abs(this->getPosition().x -(mobSize / 2));
		p.x += shiftSize;
		adjusted = true;
		xStop = true;
	}
	else {
		xStop = false;
	}
	if (this->getPosition().y + (mobSize / 2) > GAME_GRID_HEIGHT) {
		shiftSize = (this->getPosition().y + (mobSize / 2)) - GAME_GRID_HEIGHT;
		p.y -= shiftSize ;
		adjusted = true;
		yStop = true;
	}
	else {
		yStop = false;
	}
	if (this->getPosition().y - (mobSize / 2) < 0) {
		shiftSize = std::abs(this->getPosition().y - (mobSize / 2));
		p.y += shiftSize;
		adjusted = true;
		yStop = true;
	}
	else {
		yStop = false;
	}
	if (adjusted) {
		p *= (float)this->getStats().getSpeed();
		p *= (float)elapsedTime;
		m_Pos += p;
		return true;
	}
	return false;

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
	bool pushedBack = false;
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
	if (this->getStats().getMass() > otherMob->getStats().getMass()) {
		pushedBack = true;
	}

	if (pushedBack) {
		p *= (float)otherMob->getStats().getSpeed();
		p *= (float)elapsedTime;

		Vec2 tempPos = otherMob->m_Pos - p;
		// otherMob->m_Pos -= p;
		// std::cout << std::string(" gets here??") << std::endl;
		// checkRiverEdgesCollides(tempPos);
		if (!checkMapEdgesCollides(tempPos) && !checkRiverEdgesCollides(tempPos)
			&& !checkBuildingsCollides(tempPos, true)
			&& !checkBuildingsCollides(tempPos, false)) {
			otherMob->m_Pos -= p;
		}
	}
	else {
		p *= (float)this->getStats().getSpeed();
		p *= (float)elapsedTime;
		Vec2 tempPos = m_Pos - p;
		std::cout << std::string(" 2 gets here??") << std::endl;
		// checkRiverEdgesCollides(tempPos);
		// m_Pos += p;
		if (!checkMapEdgesCollides(tempPos) && !checkRiverEdgesCollides(tempPos)
			&& !checkBuildingsCollides(tempPos, true)
			&& !checkBuildingsCollides(tempPos, false)) {
			m_Pos += p;
		}
	}
}

bool Mob::checkBuildingsCollides(Vec2 newPos, bool isNorth) {
	const Player& player = Game::get().getPlayer(isNorth);
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
		float r2RightEdge = newPos.x + halfSize;
		float r2LeftEdge = newPos.x - halfSize;
		float r2TopEdge = newPos.y + halfSize;
		float r2BottomEdge = newPos.y - halfSize;

		if (r1RightEdge >= r2LeftEdge &&
			r1LeftEdge <= r2RightEdge &&
			r1TopEdge >= r2BottomEdge &&
			r1BottomEdge <= r2TopEdge) {
			// std::cout << this->getStats().getName() << std::string(" gon collide north") << std::endl;
			// float shiftSize;
			Vec2 p = Vec2(newPos.x - building->getPosition().x,
				newPos.y - building->getPosition().y);
			p.normalize();
			if (r1RightEdge >= r2LeftEdge) {
				xStop = true;
				return true;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				xStop = true;
				return true;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				yStop = true;
				return true;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				yStop = true;
				return true;
			}
		}
	}
	yStop = false;
	xStop = false;
	return false;
}

void Mob::checkBuildings(float elapsedTime, bool isNorth) {
	float shiftSizeNew = 0.f;
	const Player& player = Game::get().getPlayer(isNorth);
	Vec2 velocityVec;
	float maxDist = m_Stats.getSpeed() * elapsedTime;
	bool collides = false;
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
				// xStop = true;
				collides = true;
			}
			else if (r1LeftEdge <= r2RightEdge) {
				shiftSize = r2RightEdge - r1LeftEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.x -= shiftSize;
				// xStop = true;
				collides = true;
			}
			else if (r1TopEdge >= r2BottomEdge) {
				shiftSize = r1TopEdge - r2BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y += shiftSize;
				// yStop = true;
				collides = true;
			}
			else if (r1BottomEdge <= r2TopEdge) {
				shiftSize = r2TopEdge - r1BottomEdge;
				shiftSize = std::min(maxDist, shiftSize);
				p.y -= shiftSize;
				// yStop = true;
				collides = true;
			}

			p *= (float)this->getStats().getSpeed();
			p *= (float)elapsedTime;
			m_Pos += p;
		}
	}
	//if (!collides) {
	//	yStop = false;
	//	xStop = false;
	//}
	// do smth when they attacking
}

Entity* Mob::getMostThreateningMob(Vec2 ahead, Vec2 ahead2) {
	Entity* mostThreateningMob = nullptr;
	const Player& northPlayer = Game::get().getPlayer(true);
	for (Entity* pOtherMob : northPlayer.getMobs())
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
	return mostThreateningMob;
}

float distance(Entity* mob1, Vec2 ahead) {
	return sqrt((mob1->getPosition().x - ahead.x) * (mob1->getPosition().x - ahead.x) +
		(mob1->getPosition().y - ahead.y) * (mob1->getPosition().y - ahead.y));
}

bool Mob::lineIntersectsMob(Vec2 ahead, Vec2 ahead2, Entity* mob) {
	float mobRadius = ((float)sqrt(2) * mob->getStats().getSize()) / 2;

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
		}
	}

	return collidingMobs;
}