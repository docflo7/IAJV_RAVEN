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
		//fuzzify distance and hittable target count
		m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
		m_FuzzyModule.Fuzzify("HittableTargetCount", GetHittableTargetCount());

		m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
	}

	return m_dLastDesirabilityScore;
}

// Get all bots (except the owner) that can be hit with the blast.
int GrenadeLauncher::GetHittableTargetCount() {
	Raven_Game *world = m_pOwner->GetWorld();

	Raven_Bot *target = m_pOwner->GetTargetSys()->GetTarget();
	int result = 1; // At least the target

	// Beyond this value, we do not care how many more bots we can hit.
	// There will be... BLOOOOOD!
	int overkillCount = 10;

	double grenadeBlastRadius = script->GetDouble("Grenade_BlastRadius");

	std::list<Raven_Bot*>::const_iterator curBot = world->GetAllBots().begin();
	for (curBot; curBot != world->GetAllBots().end(); ++curBot)
	{
		Raven_Bot *bot = *curBot;

		if ((bot == m_pOwner) || (bot == target)) continue;

		if (Vec2DDistance(target->Pos(), (*curBot)->Pos()) < grenadeBlastRadius + (*curBot)->BRadius())
		{
			++result;

			if (result > overkillCount) break;
		}
	}

	return result;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void GrenadeLauncher::InitializeFuzzyModule()
{
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


	FuzzyVariable& HittableTargetCount = m_FuzzyModule.CreateFLV("HittableTargetCount");

	// TODO/FIXME: This should (maybe) use a ratio. 
	FzSet& Hit_Lots = HittableTargetCount.AddRightShoulderSet("Hit_Lots", 4, 5, 10);
	FzSet& Hit_Many = HittableTargetCount.AddTriangularSet("Hit_Many", 3, 4, 5);
	FzSet& Hit_Some = HittableTargetCount.AddTriangularSet("Hit_Some", 2, 3, 4);
	FzSet& Hit_Few = HittableTargetCount.AddTriangularSet("Hit_Few", 1, 2, 3);
	FzSet& Hit_OneOrTwo = HittableTargetCount.AddTriangularSet("Hit_OneOrTwo", 0, 1, 2);


	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Hit_Lots), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Hit_Many), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Hit_Some), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Hit_Few), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Hit_OneOrTwo), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Close, Hit_Lots), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Hit_Many), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Hit_Some), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Hit_Few), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Hit_OneOrTwo), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Medium, Hit_Lots), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Hit_Many), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Hit_Some), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Hit_Few), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Hit_OneOrTwo), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Far, Hit_Lots), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Hit_Many), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Hit_Some), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Hit_Few), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Hit_OneOrTwo), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Hit_Lots), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Hit_Many), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Hit_Some), QuiteDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Hit_Few), SomewhatDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_FarAway, Hit_OneOrTwo), Undesirable);
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