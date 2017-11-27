#include "Raven_WeaponSystem.h"
#include "armory/Weapon_GrenadeLauncher.h"
#include "armory/Weapon_RocketLauncher.h"
#include "armory/Weapon_RailGun.h"
#include "armory/Weapon_ShotGun.h"
#include "armory/Weapon_Blaster.h"
#include "Raven_Bot.h"
#include "misc/utils.h"
#include "misc/Stream_Utility_Functions.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_Game.h"
#include "Raven_UserOptions.h"
#include "2D/transformations.h"


//------------------------- ctor ----------------------------------------------
//-----------------------------------------------------------------------------
Raven_WeaponSystem::Raven_WeaponSystem(Raven_Bot* owner,
                                       double ReactionTime,
                                       double AimAccuracy,
                                       double AimPersistance):m_pOwner(owner),
                                                          m_dReactionTime(ReactionTime),
                                                          m_dAimAccuracy(AimAccuracy),
                                                          m_dAimPersistance(AimPersistance)
{
  Initialize();
  InitializeFuzzyAimingModule();
}

//------------------------- dtor ----------------------------------------------
//-----------------------------------------------------------------------------
Raven_WeaponSystem::~Raven_WeaponSystem()
{
  for (unsigned int w=0; w<m_WeaponMap.size(); ++w)
  {
    delete m_WeaponMap[w];
  }
}

//------------------------------ Initialize -----------------------------------
//
//  initializes the weapons
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::Initialize()
{
  //delete any existing weapons
  WeaponMap::iterator curW;
  for (curW = m_WeaponMap.begin(); curW != m_WeaponMap.end(); ++curW)
  {
    delete curW->second;
  }

  m_WeaponMap.clear();

  //set up the container
  m_pCurrentWeapon = new Blaster(m_pOwner);

  m_WeaponMap[type_blaster]         = m_pCurrentWeapon;
  m_WeaponMap[type_shotgun]         = 0;
  m_WeaponMap[type_rail_gun]        = 0;
  m_WeaponMap[type_rocket_launcher] = 0;
  m_WeaponMap[type_grenade_launcher] = 0;
}

//-------------------------  InitializeFuzzyAimingModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::InitializeFuzzyAimingModule()
{
	// NOTE: Each variable below has 3 sets. I could have done more,
	// but that already means having to write 27 rules (3^3)...

	FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

	FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close", 0, 25, 150);
	FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
	FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far", 150, 300, 600);


	FuzzyVariable& Velocity = m_FuzzyModule.CreateFLV("SpeedRatio");

	FzSet& Slow = Velocity.AddTriangularSet("Slow", 0, 0, 33);
	FzSet& Jogging = Velocity.AddTriangularSet("Jogging", 0, 33, 80);
	FzSet& Fast = Velocity.AddRightShoulderSet("Fast", 33, 80, 100);


	FuzzyVariable& TimeTargetVisible = m_FuzzyModule.CreateFLV("TimeTargetVisible");

	FzSet& Target_QuickGlance = TimeTargetVisible.AddLeftShoulderSet("QuickGlance", 0, 0, 1);
	FzSet& Target_GoodView = TimeTargetVisible.AddTriangularSet("GoodView", 0, 1, 5);
	FzSet& Target_DetailedView = TimeTargetVisible.AddRightShoulderSet("DetailedView", 1, 5, 100);

	FuzzyVariable& AimNoise = m_FuzzyModule.CreateFLV("AimNoise");

	// NOTE: This is called a 'word play'.
	FzSet& Whisper = AimNoise.AddRightShoulderSet("Whisper", 0, 5, 25);
	FzSet& JetTakeoff = AimNoise.AddTriangularSet("JetTake-off", 5, 25, 75);
	FzSet& BarMusic = AimNoise.AddLeftShoulderSet("BarMusic", 25, 75, 100);


	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Slow), Target_QuickGlance), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Slow), Target_GoodView), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Slow), Target_DetailedView), Whisper);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Jogging), Target_QuickGlance), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Jogging), Target_GoodView), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Jogging), Target_DetailedView), Whisper);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Fast), Target_QuickGlance), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Fast), Target_GoodView), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Close, Fast), Target_DetailedView), Whisper);



	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Slow), Target_QuickGlance), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Slow), Target_GoodView), Whisper);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Slow), Target_DetailedView), Whisper);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Jogging), Target_QuickGlance), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Jogging), Target_GoodView), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Jogging), Target_DetailedView), Whisper);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Fast), Target_QuickGlance), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Fast), Target_GoodView), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Medium, Fast), Target_DetailedView), JetTakeoff);



	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Slow), Target_QuickGlance), BarMusic);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Slow), Target_GoodView), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Slow), Target_DetailedView), JetTakeoff);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Jogging), Target_QuickGlance), BarMusic);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Jogging), Target_GoodView), JetTakeoff);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Jogging), Target_DetailedView), JetTakeoff);

	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Fast), Target_QuickGlance), BarMusic);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Fast), Target_GoodView), BarMusic);
	m_FuzzyModule.AddRule(FzAND(FzAND(Target_Far, Fast), Target_DetailedView), JetTakeoff);
}

//-------------------------------- SelectWeapon -------------------------------
//
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::SelectWeapon()
{ 
  //if a target is present use fuzzy logic to determine the most desirable 
  //weapon.
  if (m_pOwner->GetTargetSys()->isTargetPresent())
  {
    //calculate the distance to the target
    double DistToTarget = Vec2DDistance(m_pOwner->Pos(), m_pOwner->GetTargetSys()->GetTarget()->Pos());

    //for each weapon in the inventory calculate its desirability given the 
    //current situation. The most desirable weapon is selected
    double BestSoFar = MinDouble;

    WeaponMap::const_iterator curWeap;
    for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
    {
      //grab the desirability of this weapon (desirability is based upon
      //distance to target and ammo remaining)
      if (curWeap->second)
      {
        double score = curWeap->second->GetDesirability(DistToTarget);

        //if it is the most desirable so far select it
        if (score > BestSoFar)
        {
          BestSoFar = score;

          //place the weapon in the bot's hand.
          m_pCurrentWeapon = curWeap->second;
        }
      }
    }
  }

  else
  {
    m_pCurrentWeapon = m_WeaponMap[type_blaster];
  }
}

//--------------------  AddWeapon ------------------------------------------
//
//  this is called by a weapon affector and will add a weapon of the specified
//  type to the bot's inventory.
//
//  if the bot already has a weapon of this type then only the ammo is added
//-----------------------------------------------------------------------------
void  Raven_WeaponSystem::AddWeapon(unsigned int weapon_type)
{
  //create an instance of this weapon
  Raven_Weapon* w = 0;

  switch(weapon_type)
  {
  case type_rail_gun:

    w = new RailGun(m_pOwner); break;

  case type_shotgun:

    w = new ShotGun(m_pOwner); break;

  case type_rocket_launcher:

    w = new RocketLauncher(m_pOwner); break;

  case type_grenade_launcher:

	  w = new GrenadeLauncher(m_pOwner); break;

  }//end switch
  

  //if the bot already holds a weapon of this type, just add its ammo
  Raven_Weapon* present = GetWeaponFromInventory(weapon_type);

  if (present)
  {
    present->IncrementRounds(w->NumRoundsRemaining());

    delete w;
  }
  
  //if not already holding, add to inventory
  else
  {
    m_WeaponMap[weapon_type] = w;
  }
}


//------------------------- GetWeaponFromInventory -------------------------------
//
//  returns a pointer to any matching weapon.
//
//  returns a null pointer if the weapon is not present
//-----------------------------------------------------------------------------
Raven_Weapon* Raven_WeaponSystem::GetWeaponFromInventory(int weapon_type)
{
  return m_WeaponMap[weapon_type];
}

//----------------------- ChangeWeapon ----------------------------------------
void Raven_WeaponSystem::ChangeWeapon(unsigned int type)
{
  Raven_Weapon* w = GetWeaponFromInventory(type);

  if (w) m_pCurrentWeapon = w;
}

//--------------------------- TakeAimAndShoot ---------------------------------
//
//  this method aims the bots current weapon at the target (if there is a
//  target) and, if aimed correctly, fires a round
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::TakeAimAndShoot()
{
  //aim the weapon only if the current target is shootable or if it has only
  //very recently gone out of view (this latter condition is to ensure the 
  //weapon is aimed at the target even if it temporarily dodges behind a wall
  //or other cover)
  if (m_pOwner->GetTargetSys()->isTargetShootable() ||
      (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenOutOfView() < 
       m_dAimPersistance) )
  {
    //the position the weapon will be aimed at
    Vector2D AimingPos = m_pOwner->GetTargetBot()->Pos();
    
    //if the current weapon is not an instant hit type gun the target position
    //must be adjusted to take into account the predicted movement of the 
    //target
    if (GetCurrentWeapon()->GetType() == type_rocket_launcher ||
        GetCurrentWeapon()->GetType() == type_blaster ||
		GetCurrentWeapon()->GetType() == type_grenade_launcher)
    {
      AimingPos = PredictFuturePositionOfTarget();

      //if the weapon is aimed correctly, there is line of sight between the
      //bot and the aiming position and it has been in view for a period longer
      //than the bot's reaction time, shoot the weapon
      if ( m_pOwner->RotateFacingTowardPosition(AimingPos) &&
           (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible() >
            m_dReactionTime) &&
           m_pOwner->hasLOSto(AimingPos) )
      {
        AddNoiseToAim(AimingPos);

        GetCurrentWeapon()->ShootAt(AimingPos);
      }
    }

    //no need to predict movement, aim directly at target
    else
    {
      //if the weapon is aimed correctly and it has been in view for a period
      //longer than the bot's reaction time, shoot the weapon
      if ( m_pOwner->RotateFacingTowardPosition(AimingPos) &&
           (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible() >
            m_dReactionTime) )
      {
        AddNoiseToAim(AimingPos);
        
        GetCurrentWeapon()->ShootAt(AimingPos);
      }
    }

  }
  
  //no target to shoot at so rotate facing to be parallel with the bot's
  //heading direction
  else
  {
    m_pOwner->RotateFacingTowardPosition(m_pOwner->Pos()+ m_pOwner->Heading());
  }
}

//---------------------------- AddNoiseToAim ----------------------------------
//
//  adds a fuzzy random deviation to the firing angle not greater than m_dAimAccuracy 
//  rads
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::AddNoiseToAim(Vector2D& AimingPos)
{
  Vector2D toPos = AimingPos - m_pOwner->Pos();

  //fuzzify distance, speed and target visible time
  m_FuzzyModule.Fuzzify("DistToTarget", toPos.Length());
  m_FuzzyModule.Fuzzify("SpeedRatio", 100.0 * (m_pOwner->Speed() / m_pOwner->MaxSpeed()));
  m_FuzzyModule.Fuzzify("TargetVisibleTime", m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible());

  double aimNoiseMagnitude = m_FuzzyModule.DeFuzzify("AimNoise", FuzzyModule::max_av) / 100.0;

  Vec2DRotateAroundOrigin(toPos, aimNoiseMagnitude * RandInRange(-m_dAimAccuracy, m_dAimAccuracy));

  AimingPos = toPos + m_pOwner->Pos();
}

//-------------------------- PredictFuturePositionOfTarget --------------------
//
//  predicts where the target will be located in the time it takes for a
//  projectile to reach it. This uses a similar logic to the Pursuit steering
//  behavior.
//-----------------------------------------------------------------------------
Vector2D Raven_WeaponSystem::PredictFuturePositionOfTarget()const
{
  double MaxSpeed = GetCurrentWeapon()->GetMaxProjectileSpeed();
  
  //if the target is ahead and facing the agent shoot at its current pos
  Vector2D ToEnemy = m_pOwner->GetTargetBot()->Pos() - m_pOwner->Pos();
 
  //the lookahead time is proportional to the distance between the enemy
  //and the pursuer; and is inversely proportional to the sum of the
  //agent's velocities
  double LookAheadTime = ToEnemy.Length() / 
                        (MaxSpeed + m_pOwner->GetTargetBot()->MaxSpeed());
  
  //return the predicted future position of the enemy
  return m_pOwner->GetTargetBot()->Pos() + 
         m_pOwner->GetTargetBot()->Velocity() * LookAheadTime;
}


//------------------ GetAmmoRemainingForWeapon --------------------------------
//
//  returns the amount of ammo remaining for the specified weapon. Return zero
//  if the weapon is not present
//-----------------------------------------------------------------------------
int Raven_WeaponSystem::GetAmmoRemainingForWeapon(unsigned int weapon_type)
{
  if (m_WeaponMap[weapon_type])
  {
    return m_WeaponMap[weapon_type]->NumRoundsRemaining();
  }

  return 0;
}

//---------------------------- ShootAt ----------------------------------------
//
//  shoots the current weapon at the given position
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::ShootAt(Vector2D pos)const
{
  GetCurrentWeapon()->ShootAt(pos);
}

//-------------------------- RenderCurrentWeapon ------------------------------
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::RenderCurrentWeapon()const
{
  GetCurrentWeapon()->Render();
}

void Raven_WeaponSystem::RenderDesirabilities()const
{
  Vector2D p = m_pOwner->Pos();

  int num = 0;
  
  WeaponMap::const_iterator curWeap;
  for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
  {
    if (curWeap->second) num++;
  }

  int offset = 15 * num;

    for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
    {
      if (curWeap->second)
      {
        double score = curWeap->second->GetLastDesirabilityScore();
        std::string type = GetNameOfType(curWeap->second->GetType());

        gdi->TextAtPos(p.x+10.0, p.y-offset, ttos(score) + " " + type);

        offset+=15;
      }
    }
}
