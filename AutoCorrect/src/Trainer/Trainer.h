#pragma once

namespace Backend {
	class Trainer
	{
		static const char* GetModelLoc() { return "Model.txt"; }
		static const char* GetTrainingDataDir() { return "Training_Data"; }
		static const char* GetModelDir() { return ""; }
	public:
		static void Init();
	private:
		static void TrainModel();
		void static LoadModel();
	};

}