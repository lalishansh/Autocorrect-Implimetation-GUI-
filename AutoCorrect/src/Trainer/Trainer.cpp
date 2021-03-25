#include "Trainer.h"
#include <fstream>
#include <iostream>
#include <filesystem>



namespace Backend {
	void Trainer::Init()
	{
		std::ifstream file;
		file.open(GetModelLoc());
		if (file) {
			file.close();
			LoadModel();
		}
		else {
			TrainModel();
		}
	}
	
	void Trainer::TrainModel()
	{
		// Hard-code Book locations		
	}

	void Trainer::LoadModel()
	{
	}
}