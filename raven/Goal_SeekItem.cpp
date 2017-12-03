#include "Goal_SeekItem.h"

//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
Goal_SeekItem::
Goal_SeekItem(Raven_Bot*          pBot,
	std::list<PathEdge> path) :Goal_Composite<Raven_Bot>(pBot, goal_wander_to_item), m_Path(path)
{
}

//---------------------------- Initialize -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_SeekItem::Activate()
{
	m_iStatus = active;
	AddSubgoal(new Goal_FollowPath(m_pOwner, m_Path));

}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_SeekItem::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();
	//process the subgoals
	m_iStatus = ProcessSubgoals();

	std::list<Raven_Projectile*> projectileList = m_pOwner->GetWorld()->GetAllProjectils();
	//Create an iterator of std::list
	std::list<Raven_Projectile*>::iterator it;



	bool needToDodge = false;
	double distanceCurrentProjectile = 0;
	double distanceClosestProjectile = -1;
	Raven_Projectile* closestProjectile;

	// go thourght every projectile
	for (it = projectileList.begin(); it != projectileList.end(); it++)
	{
		//make sure we don't check against the shooter of the projectile
		if (m_pOwner->ID() != (*it)->getShooterID())
		{
			distanceCurrentProjectile = distanceBetweenTwoPoint((*it)->Pos(), m_pOwner->Pos());
			//arbitrary check to only considerate projectiles close to bot
			//then check if the projectile will hit the bot
			if (distanceCurrentProjectile < (m_pOwner->BRadius() * 10)  && projectileWillHitBot((*it)) ){
				//take the closest projectiles
				if (distanceClosestProjectile == -1 || (distanceClosestProjectile != -1 && distanceClosestProjectile > distanceCurrentProjectile)) {
					distanceClosestProjectile = distanceCurrentProjectile;
					closestProjectile = (*it);
					needToDodge = true;
				}
			}
		}
	}
	if (needToDodge) {
		RemoveAllSubgoals();
		AddSubgoal(new Goal_FleeProjectile(m_pOwner, closestProjectile, m_Path));
	}

	return m_iStatus;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_SeekItem::Terminate()
{

}

//------------------ isIntersectingBot --------------------------------

bool Goal_SeekItem::projectileWillHitBot(Raven_Projectile* proj)
{
	Raven_Bot* ClosestIntersectingBot = 0;
	double ClosestSoFar = MaxDouble;
	Vector2D    From = proj->getOrigin();
	Vector2D    To = proj->getImpactPoint();

	//make sure we don't check against the shooter of the projectile
	if (m_pOwner->ID() != proj->getShooterID())
	{
		//there is an intersection between the travel course of the bot and the projectile
		if (DistToLineSegment(From, To, m_pOwner->Pos()) < (m_pOwner->BRadius() * 2)) {
			debug_con << "y" << "";
			return true;
		}
	}
	debug_con << "n" << "";
	return false;
}



double distanceBetweenTwoPoint(Vector2D a, Vector2D b)
{
	double sqrt_diff_x = (b.x - a.x) * (b.x - a.x);
	double sqrt_diff_y = (b.y - b.y) * (b.y - b.y);
	double sum = sqrt_diff_x + sqrt_diff_y;
	return sqrt(sum);
}
