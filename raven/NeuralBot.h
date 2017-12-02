#pragma once
#include "Raven_Bot.h"
#include "misc/utils.h"
#include <string>

class NeuralBot :
	private Raven_Bot
{
public:
	NeuralBot(Raven_Game* world, Vector2D pos, const std::string& data_file);
	void Update();
	~NeuralBot();
};

