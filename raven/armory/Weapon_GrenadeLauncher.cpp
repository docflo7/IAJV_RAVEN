#include "Weapon_GrenadeLauncher.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
GrenadeLauncher::GrenadeLauncher(Raven_Bot*   owner) :

	Raven_Weapon(type_grenade_launcher,
		script->GetInt("Grenade_DefaultRounds"),
		script->GetInt("Grenade_MaxRoundsCarried"),
		script->GetDouble("Grenade_FiringFreq"),
		script->GetDouble("Grenade_IdealRange"),
		script->GetDouble("Grenade_MaxSpeed"),
		owner)
{
	//setup the vertex buffer
	const int NumWeaponVerts = 8;
	const Vector2D weapon[NumWeaponVerts] = {
		Vector2D(0, -5),
		Vector2D(5, -5),
		Vector2D(5, 5),
		Vector2D(0, 5),
	};
	for (int vtx = 0; vtx<NumWeaponVerts; ++vtx)
	{
		m_vecWeaponVB.push_back(weapon[vtx]);
	}

	//setup the fuzzy module
	InitializeFuzzyModule();

}


//------------------------------ ShootAt --------------------------------------
//-----------------------------------------------------------------------------
inline void GrenadeLauncher::ShootAt(Vector2D pos)
{
	if (NumRoundsRemaining() > 0 && isReadyForNextShot())
	{
		m_pOwner->GetWorld()->AddGrenade(m_pOwner, pos);

		m_iNumRoundsLeft--;

		UpdateTimeWeaponIsNextAvailable();
	}
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double GrenadeLauncher::GetDesirability(double DistToTarget)
{
	if (m_iNumRoundsLeft == 0)
	{
		m_dLastDesirabilityScore = 0;
	}
	else
	{
		// TODO: Maybe add the number of enemies around the target?
		//fuzzify distance and amount of ammo
		m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
		m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

		m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
	}

	return m_dLastDesirabilityScore;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void GrenadeLauncher::InitializeFuzzyModule()
{
	// TODO: Change this values.
	// Use sets of another weapon so there are less rules to write.
	FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

	FzSet& Target_VeryClose = DistToTarget.AddLeftShoulderSet("Target_VeryClose", 0, 10, 25);
	FzSet& Target_Close = DistToTarget.AddTriangularSet("Target_Close", 10, 25, 150);
	FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
	FzSet& Target_Far = DistToTarget.AddTriangularSet("Target_Far", 150, 300, 600);
	FzSet& Target_FarAway = DistToTarget.AddRightShoulderSet("Target_FarAway", 300, 600, 1000);


	FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");

	FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 65, 80, 95);
	FzSet& QuiteDesirable = Desirability.AddTriangularSet("QuiteDesirable", 50, 65, 80);
	FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 35, 50, 65);
	FzSet& SomewhatDesirable = Desirability.AddTriangularSet("SomewhatDesirable", 20, 35, 50);
	FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 20, 35);


	FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");

	FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 50, 80, 100);
	FzSet& Ammo_Plenty = AmmoStatus.AddTriangularSet("Ammo_Plenty", 15, 50, 80);
	FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 5, 15, 50);
	FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 5, 15);
	FzSet& Ammo_NearlyOut = AmmoStatus.AddTriangularSet("Ammo_NearlyOut", 0, 0, 5);


	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Loads), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Plenty), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Okay), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Low), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_NearlyOut), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Plenty), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_NearlyOut), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Plenty), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_NearlyOut), Desirable);

	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Plenty), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_NearlyOut), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Ammo_Loads), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Ammo_Plenty), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Ammo_Okay), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Ammo_Low), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Ammo_NearlyOut), Undesirable);
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void GrenadeLauncher::Render()
{
	m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
		m_pOwner->Pos(),
		m_pOwner->Facing(),
		m_pOwner->Facing().Perp(),
		m_pOwner->Scale());

	gdi->DarkGreenBrush();

	gdi->ClosedShape(m_vecWeaponVBTrans);
}