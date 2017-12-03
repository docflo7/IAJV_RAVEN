#ifndef GOAL_WANDER_TO_ITEM_H
#define GOAL_WANDER_TO_ITEM_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Wander.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Causes a bot to wander until terminated
//-----------------------------------------------------------------------------

#include "../raven/goals/Raven_Goal_Types.h"
#include "../raven/Raven_Bot.h"
#include "Goals/Goal_Composite.h"
#include "../raven/Raven_SteeringBehaviors.h"
#include "../raven/navigation/Raven_PathPlanner.h"
#include "Goals/Goal.h"
#include "debug/DebugConsole.h"
#include "misc/cgdi.h"
#include "../raven/goals/Goal_FollowPath.h"
#include "../raven/goals/Goal_DodgeSideToSide.h"
#include "../raven/armory/Raven_Projectile.h"
#include "../raven/Raven_Game.h"
#include "../raven/Goal_FleeProjectile.h"


class Goal_SeekItem : public Goal_Composite<Raven_Bot>
{
private:
	//a local copy of the path returned by the path planner
	std::list<PathEdge>  m_Path;

public:

	Goal_SeekItem(Raven_Bot* pBot, std::list<PathEdge> path);

	void Activate();

	int  Process();

	void Terminate();

	bool Goal_SeekItem::projectileWillHitBot(Raven_Projectile* proj);

	Vector2D    m_vStrafeTarget;

};

double distanceBetweenTwoPoint(Vector2D a, Vector2D b);
double clockwiseAngleBetweenVector(Vector2D a, Vector2D b);
#endif
