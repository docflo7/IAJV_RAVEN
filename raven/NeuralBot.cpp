#include "NeuralBot.h"
#include "goals/Goal_Think.h"


NeuralBot::NeuralBot(Raven_Game* world, Vector2D pos, const std::string& data_file)
	: Raven_Bot(world, pos)
{

}

void NeuralBot::Update()
{
	//process the currently active goal. Note this is required even if the bot
	//is under user control. This is because a goal is created whenever a user 
	//clicks on an area of the map that necessitates a path planning request.
	GetBrain()->Process();

	//Calculate the steering force and update the bot's velocity and position
	this->UpdateMovement();

	//if the bot is under AI control but not scripted
	if (!isPossessed())
	{
		//examine all the opponents in the bots sensory memory and select one
		//to be the current target
		if (isReadytoTarget())
		{
			GetTargetSys()->Update();
		}

		//appraise and arbitrate between all possible high level goals
		if (m_pGoalArbitrationRegulator->isReady())
		{
			m_pBrain->Arbitrate();
		}

		//update the sensory memory with any visual stimulus
		if (m_pVisionUpdateRegulator->isReady())
		{
			m_pSensoryMem->UpdateVision();
		}

		//select the appropriate weapon to use from the weapons currently in
		//the inventory
		if (m_pWeaponSelectionRegulator->isReady())
		{
			m_pWeaponSys->SelectWeapon();
		}

		//this method aims the bot's current weapon at the current target
		//and takes a shot if a shot is possible
		//A faire si le réseau neuronal renvoie 1
		m_pWeaponSys->TakeAimAndShoot();
	}
}


NeuralBot::~NeuralBot()
{
	Raven_Bot::~Raven_Bot();
}
