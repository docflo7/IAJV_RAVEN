#include "Raven_UserOptions.h"


Raven_UserOptions* Raven_UserOptions::Instance()
{
	static Raven_UserOptions instance;
	return &instance;
}

Raven_UserOptions::Raven_UserOptions() :
	m_bWaitForUpdate(false),
	m_bMouse(true),
	m_bPlayer(false),
	m_bAzerty(true),
	m_bShowGraph(false),
	m_bShowPathOfSelectedBot(true),
	m_bSmoothPathsQuick(false),
	m_bSmoothPathsPrecise(false),
	m_bShowBotIDs(false),
	m_bShowBotHealth(true),
	m_bShowTargetOfSelectedBot(false),
	m_bOnlyShowBotsInTargetsFOV(false),
	m_bShowScore(false),
	m_bShowGoalsOfSelectedBot(true),
	m_bShowGoalAppraisals(true),
	m_bShowNodeIndices(false),
	m_bShowOpponentsSensedBySelectedBot(true),
	m_bShowWeaponAppraisals(false)
{}
