#include "Goal_FleeProjectile.h"
#include "Goals/Goal.h"
#include "debug/DebugConsole.h"
#include "misc/cgdi.h"
#include "../raven/goals/Goal_FollowPath.h"
#include "../raven/goals/Goal_DodgeSideToSide.h"
#include "../raven/armory/Raven_Projectile.h"
#include "../raven/Raven_Game.h"


//---------------------------- Initialize -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_FleeProjectile::Activate()
{
	//find the best methode start to dodge the nearest projectil
	Vector2D botDeplamentVector = (m_Path.front().Destination() - m_Path.front().Source());
	botDeplamentVector.Normalize();

	Vector2D projectileDeplamentVector = (m_objectToDodge->getImpactPoint() - m_objectToDodge->Pos());
	projectileDeplamentVector.Normalize();

	double accuracy = 20;
	//angle between the bot and the projectile
	bool isNearlyParrallele = clockwiseAngleBetweenVector(projectileDeplamentVector, botDeplamentVector) < accuracy ||
							  clockwiseAngleBetweenVector(projectileDeplamentVector, botDeplamentVector) > (180 - accuracy);
	//chose the best startegie to dodge the projectil
	if (isNearlyParrallele) {
		// dodge to side if parallele
		if ((m_pOwner->canStepRight(m_vStrafeTarget)) || (m_pOwner->canStepLeft(m_vStrafeTarget))) {
			RemoveAllSubgoals();
			m_pOwner->GetSteering()->SetTarget(m_vStrafeTarget);
			m_pOwner->GetSteering()->SeekOn();
		}
	}
	else {
		//flee
		m_pOwner->GetSteering()->SetTarget(m_objectToDodge->Pos());
		m_pOwner->GetSteering()->FleeOn();
	}

}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_FleeProjectile::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();
	if(m_objectToDodge->isDead())
	{
		m_iStatus = completed;
	}
	
	return m_iStatus;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_FleeProjectile::Terminate()
{
	m_iStatus = completed;
	m_pOwner->GetSteering()->FleeOff();
	m_pOwner->GetSteering()->SeekOff();
}

double clockwiseAngleBetweenVector(Vector2D a, Vector2D b) {
	double dot = a.x*b.y + a.y*b.y;
	double det = a.x*b.y - b.x*a.y;
	return abs(atan2(dot, det)* 57.2958);
}
