#pragma once

#include <vector>

#include "brain.h"

#include "GLFW/glfw3.h"


class Neat
{
public:
	Neat(const int& popSize);
	~Neat();
	void init(const int& nbInput, const int& nbOutput);
	void train(int nbOfGeneration, void(*f)(Brain*), void(*funcValidate)(Brain*), bool exitIfSuccessfull);
	Brain getBestWinner();
	Brain getBrainWithBestFitness();
	void plot(bool* p_open);

	struct Settings
	{
		float mutate_Bias = 0.1;
		float mutate_Bias_power = 0.003;

		float mutate_Weight = 0.8;
		float mutate_Weight_power = 0.8;
		float mutate_Weight_reset = 0.01;
		float mutate_Weight_max = 8;

		float mutate_Node = 0.001;
		float mutate_Connection = 0.003;

		float mutate_EnableConnection = 0.001;
		float mutate_DisableConnection = 0.001;

		int num_species_target = 40;
		float compat_mod = 0.1;
		int stagnation_treshold = 10;
		float speciation_threshold = 3;
	};
	struct Species
	{
		std::vector<Brain*> members;
		Brain reference_member;
		float mean_fitness = 0;
		float best_fitness = 0;
		float previous_best_fitness = 0;
		bool alive = true;
		float meanAdjustedFitness = 0;
		int numOffspring = 0;
		int speciesId = 0;
		int biggestSize = -1;
		int lowestSize = -1;
		int stagnation = 0;
		float totalFitness = 0;
	};

	Settings settings;
	std::vector<Species> m_species;
	std::vector<Brain> m_population;

private:
	void createNewGen();
	Brain rouletteWheel(Species& species);
	void printInfo();

	void runGeneration(void(*func)(Brain*));

	void initGeneration();
	void calculateGenData();

	void speciate();
	void offspring_and_sort_and_sill();

	int m_nbInput;
	int m_nbOutput;
	int m_generation;
	int m_populationSize;
	int speciesIdCounter;
	std::vector<float> m_bestFitnessRecord;
	std::vector<int> m_numberOfSpeciesRecord;
	float m_allTimeBestFitness;
	float m_allTimeBiggestNumberOfSpecies;

	float m_popMeanAjustedFitness;



	std::vector<Species*> m_sortedSpecies;
	std::vector<int> m_newSpeciesId;
	std::vector<int> m_killedSpeciesId_member;
	std::vector<int> m_killedSpeciesId_offspring;
	std::vector<int> m_killedSpeciesId_stagnation;
	
	bool m_finished;
	bool m_exitIfSuccessfull;
};


