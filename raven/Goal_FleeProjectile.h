#ifndef GOAL_DODGE_OBJECT_H
#define GOAL_DODGE_OBJECT_H
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
#include "../raven/armory/Raven_Projectile.h"

class Goal_FleeProjectile : public Goal_Composite<Raven_Bot>
{
private:
	//a local copy of the path returned by the path planner
	std::list<PathEdge>  m_Path;
	Raven_Projectile* m_objectToDodge;
	Vector2D    m_vStrafeTarget;
	

public:

	Goal_FleeProjectile(Raven_Bot* pBot, Raven_Projectile* objectToDodge, std::list<PathEdge>  PathBot) :Goal_Composite<Raven_Bot>(pBot, goal_dodge_element)
	{
		m_objectToDodge = objectToDodge;
		m_Path = PathBot;
	}
	void Activate();

	int  Process();

	void Terminate();
};

double clockwiseAngleBetweenVector(Vector2D a, Vector2D b);

#endif