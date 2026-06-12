#include <algorithm>
#define NOMINMAX
#include <windows.h>
#include <cmath>
#include <omp.h>
#include <iomanip>

#include "glHelper.h"
#include "neat.h"
#include "keyboard.h"


Neat::Neat(const int& popSize)
	: m_generation(0), m_populationSize(popSize), m_allTimeBestFitness(0), m_allTimeBiggestNumberOfSpecies(0),
	m_exitIfSuccessfull(false), speciesIdCounter(0), m_nbInput(0), m_nbOutput(0), m_finished(false), m_popMeanAjustedFitness(0)
{
	m_population.reserve(popSize);
}

Neat::~Neat()
{
	GlHelper::remove("neat plotting", this);
}

void Neat::init(const int& nbInput, const int& nbOutput)
{
	m_nbInput = nbInput;
	m_nbOutput = nbOutput;
	for (size_t i = 0; i < m_populationSize; i++)
	{
		m_population.push_back(Brain(nbInput, nbOutput));
	}
}

void Neat::plot(bool* p_open)
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	float ratio = glfwGetVideoMode(glfwGetPrimaryMonitor())->width / 1920.f;

	ImGui::Begin("species plotting", p_open, ImGuiWindowFlags_NoScrollbar);

	static bool first = true;
	if (first)
	{
		ImGui::SetWindowSize(ImVec2(800 * ratio, 600 * ratio));
		first = false;
	}

	if (ImPlot::BeginSubplots("Evolution of the best fitness and the number of species per generation", 2, 1, ImVec2(-1, -1)))
	{
		if (m_generation == 0)
			ImPlot::SetNextAxesLimits(0, 10, 0, 10, ImGuiCond_Always);
		else
			ImPlot::SetNextAxesLimits(0, m_generation, 0, m_allTimeBestFitness * 1.1, ImGuiCond_Always);
		if (ImPlot::BeginPlot("##fitness"), ImVec2(), ImPlotFlags_Crosshairs)
		{
			ImPlot::PlotLine("best fitness", m_bestFitnessRecord.data(), m_bestFitnessRecord.size());
			ImPlot::EndPlot();
		}

		if (m_generation == 0)
		{
			ImPlot::SetNextAxesLimits(0, 10, 0, 10, ImGuiCond_Always);
		}
		else
		{
			ImPlot::SetNextAxesLimits(0, m_generation, 0, m_allTimeBiggestNumberOfSpecies * 1.1, ImGuiCond_Always);
		}
		if (ImPlot::BeginPlot("##species"), ImVec2(), ImPlotFlags_Crosshairs)
		{
			ImPlot::PlotLine("number of species", m_numberOfSpeciesRecord.data(), m_numberOfSpeciesRecord.size());
			ImPlot::EndPlot();
		}

		ImPlot::EndSubplots();
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void Neat::train(int nbOfGeneration, void(*funcTraining)(Brain*), void(*funcValidate)(Brain*), bool exitIfSuccessfull)
{
	m_exitIfSuccessfull = exitIfSuccessfull;
	m_finished = false;

	m_sortedSpecies.reserve(settings.num_species_target * 3);
	m_newSpeciesId.reserve(settings.num_species_target * 2);
	m_killedSpeciesId_member.reserve(settings.num_species_target * 2);
	m_killedSpeciesId_offspring.reserve(settings.num_species_target * 2);
	m_killedSpeciesId_stagnation.reserve(settings.num_species_target * 2);

	std::cout << "Training started, press escape to exit training before the end...\n";
	for (;m_generation< nbOfGeneration; m_generation++)
	{

		initGeneration();
		runGeneration(funcTraining);
		speciate();
		calculateGenData();
		offspring_and_sort_and_sill();
		printInfo();

		//exit loop if finished
		if (m_finished)
		{
			break;
		}
		if (Keyboard::isKeyPressed('P'))
		{
			while (!Keyboard::isKeyPressed('P'))
			{
				Sleep(10);
			}
		}
		if (Keyboard::isKeyPressed('G'))
		{
			Brain tempBest = getBestWinner();
			tempBest.printAsync();
			funcValidate(&tempBest);
			tempBest.stopPrinting();
		}
		if (Keyboard::isKeyPressed(VK_ESCAPE))
		{
			break;
		}
		//crossover and mutatation
		std::vector<Brain> offspringPopulation;
		offspringPopulation.reserve(m_populationSize);

		if (m_generation < nbOfGeneration-1)
		{
			createNewGen();
		}
	}

	std::cout << "\n-----------------------------------------\n";
	std::cout <<   "---------- TRAINING FINISHED ! ----------\n";
	std::cout <<   "-----------------------------------------\n";
}

void Neat::initGeneration()
{
	m_newSpeciesId.clear();
	m_sortedSpecies.clear();
	m_killedSpeciesId_offspring.clear();
	m_killedSpeciesId_member.clear();
	m_killedSpeciesId_stagnation.clear();

	//ajust treshold to force species size to shrink/grow to the target size
	if (m_generation>1)
	{
		int size = m_species.size();
		if (size < settings.num_species_target - 2)
			settings.speciation_threshold -= settings.compat_mod;
		else if (size > settings.num_species_target + 2)
			settings.speciation_threshold += settings.compat_mod;
		if (settings.speciation_threshold < 0.3) settings.speciation_threshold = 0.3;
	}

	//reset species
	for (Species& species : m_species)
	{
		species.members.clear();
		species.previous_best_fitness = species.best_fitness;
		species.best_fitness = 0;
		species.lowestSize = -1;
		species.biggestSize = -1;
	}
}
void Neat::runGeneration(void(*func)(Brain*))
{
#pragma omp parallel for
	for (int j = 0; j < m_populationSize; j++)
	{
		func(&m_population[j]);
	}
}
void Neat::speciate()
{
	//speciate
	for (int j = 0; j < m_populationSize; j++)
	{
		if (m_population[j].runSucessfull && m_exitIfSuccessfull)
		{
			m_finished = true;
		}
		if (m_population[j].fitness > m_allTimeBestFitness)
		{
			m_allTimeBestFitness = m_population[j].fitness;
		}
		bool assigned = false;
		for (Species& species : m_species)
		{
			if (Brain::distance(m_population[j], species.reference_member) < settings.speciation_threshold)
			{
				species.members.push_back(&m_population[j]);
				int size = m_population[j].getSize();
				if (size > species.biggestSize || species.lowestSize == -1)
				{
					species.biggestSize = size;
				}
				if (size < species.lowestSize || species.lowestSize == -1)
				{
					species.lowestSize = size;
				}

				if (false)
				{
					if (species.biggestSize - species.reference_member.getSize() > 50 || species.reference_member.getSize() - species.lowestSize > 50)
					{
						const Brain* ref = &species.reference_member;
						const Brain* other = nullptr;

						std::cout << "\n\nl'ecart entre un membre et le ref est bcp trop grand !!!\n";
						for (Brain* member : species.members)
						{
							if (species.biggestSize - ref->getSize() > 50)
							{
								if (member->getSize() == species.biggestSize)
								{
									other = member;
									break;
								}
							}
							else
							{
								if (member->getSize() == species.lowestSize)
								{
									other = member;
									break;
								}
							}
						}
						if (other == nullptr)
						{
							std::cout << "le membre le plus grand/petit n'as pas ete trouve !!!";
							exit(1);
						}
						else
						{
							float distance = Brain::distance(*ref, *other);
							std::cout << "membres trouve, distance=" << distance;
							exit(1);
						}
					}
				}

				assigned = true;
				break;
			}
		}
		if (!assigned)
		{
			if (m_species.size() > 0)
				Brain::distance(m_population[j], m_species[0].reference_member);
			Species newSpecies;
			newSpecies.reference_member = m_population[j];
			newSpecies.members.push_back(&m_population[j]);
			newSpecies.biggestSize = m_population[j].getSize();
			newSpecies.lowestSize = m_population[j].getSize();
			newSpecies.speciesId = speciesIdCounter;
			speciesIdCounter++;
			m_newSpeciesId.push_back(newSpecies.speciesId);
			m_species.push_back(newSpecies);
		}
	}

}
void Neat::calculateGenData()
{
	//calculate ajusted fitness, mean fitness, best fitness etc...
	float bestPopulationFitness = 0;
	m_popMeanAjustedFitness = 0;
	for (Species& species : m_species)
	{
		species.totalFitness = 0.0f;
		float totalAjustedFitness = 0.0f;
		for (Brain* member : species.members)
		{
			if (member->fitness > species.best_fitness)
				species.best_fitness = member->fitness;

			if (member->fitness > bestPopulationFitness)
				bestPopulationFitness = member->fitness;

			species.totalFitness += member->fitness;

			member->ajustedFitness = (species.members.size() > 0) ? member->fitness / species.members.size() : 0;
			totalAjustedFitness += member->ajustedFitness;
			m_popMeanAjustedFitness += member->ajustedFitness;
		}
		for (Brain* member : species.members)
		{
			member->selectionProbability = (species.totalFitness > 0) ? member->fitness / species.totalFitness : 1;
		}
		species.mean_fitness = (species.members.size() > 0) ? species.totalFitness / species.members.size() : 0;
		species.meanAdjustedFitness = (species.members.size() > 0) ? totalAjustedFitness / species.members.size() : 0;

		if (species.best_fitness <= species.previous_best_fitness)
			species.stagnation++;
		else
			species.stagnation = 0;
	}
	m_popMeanAjustedFitness = m_popMeanAjustedFitness / m_population.size();
	m_bestFitnessRecord.push_back(bestPopulationFitness);
	m_numberOfSpeciesRecord.push_back(m_species.size());
	if (m_species.size() > m_allTimeBiggestNumberOfSpecies)
	{
		m_allTimeBiggestNumberOfSpecies = m_species.size();
	}
}
void Neat::offspring_and_sort_and_sill()
{
	//calculate number of offspring, sort member, and kill

	for (int i = m_species.size() - 1; i >= 0; i--)  //reverse to avoid erase indexing problem
	{
		Species& species = m_species[i];
		if (m_popMeanAjustedFitness != 0)
		{
			species.numOffspring = floor((species.meanAdjustedFitness / m_popMeanAjustedFitness) * species.members.size());
		}
		else
		{
			std::cout << "\nALL SPECIES HAVE A FITNESS OF 0 !!!!!!\n";
		}

		if (species.members.size() == 0)
		{
			m_killedSpeciesId_member.push_back(m_species[i].speciesId);
			m_species.erase(m_species.begin() + i);
		}
		else if (species.numOffspring == 0)
		{
			m_killedSpeciesId_offspring.push_back(m_species[i].speciesId);
			m_species.erase(m_species.begin() + i);
		}
		else
		{
			std::sort(species.members.begin(), species.members.end(), [](Brain* const& brain1, Brain* const& brain2) { return brain1->fitness > brain2->fitness; });
			//species.reference_member = *species.members[Brain::genRandomInt(0, species.members.size() - 1)];
			species.reference_member = *species.members.front();
			//species.reference_member = *species.members.back();
		}

	}

	//sort species so I can print them in the correct order
	std::transform(m_species.begin(), m_species.end(), std::back_inserter(m_sortedSpecies), [](Species& species) { return &species; });
	std::sort(m_sortedSpecies.begin(), m_sortedSpecies.end(), [](Species* const& species1, Species* const& species2)
		{
			if (species1->best_fitness == species2->best_fitness)
				return species1->mean_fitness > species2->mean_fitness;
			else
				return species1->best_fitness > species2->best_fitness;
		});


	//put member of the best species in the stagnating species
	bool resort = false;
	for (int i = m_species.size() - 1; i >= 0; i--)  //reverse to avoid erase indexing problem
	{
		Species& species = m_species[i];
		if (species.stagnation >= settings.stagnation_treshold && species.best_fitness < m_sortedSpecies[0]->best_fitness / 6.f
			|| species.stagnation >= settings.stagnation_treshold*3 && species.best_fitness < m_sortedSpecies[0]->best_fitness / 4.f
			|| species.stagnation >= settings.stagnation_treshold*6 && species.best_fitness < m_sortedSpecies[0]->best_fitness / 2.f)
		{
			if (m_species.size() > 1)
			{
				m_killedSpeciesId_stagnation.push_back(m_species[i].speciesId);
				m_species[i].members = m_sortedSpecies[0]->members;
				//m_species.erase(m_species.begin() + i);
				//resort = true;
				//m_sortedSpecies.erase(std::find(m_sortedSpecies.begin(), m_sortedSpecies.end(), &*(m_species.begin() + i)));
			}
			else
			{
				species.stagnation--;
			}
		}
	}
	
	//sort again
	if (resort)
	{
		m_sortedSpecies.clear();
		std::transform(m_species.begin(), m_species.end(), std::back_inserter(m_sortedSpecies), [](Species& species) { return &species; });
		std::sort(m_sortedSpecies.begin(), m_sortedSpecies.end(), [](Species* const& species1, Species* const& species2)
			{
				if (species1->best_fitness == species2->best_fitness)
					return species1->mean_fitness > species2->mean_fitness;
				else
					return species1->best_fitness > species2->best_fitness;
			});
	}
	//verify if no species with 0 members exist
	for (Species& species : m_species)
	{
		//species.reference_member.printAsync();
		if (species.members.size() == 0)
		{
			std::cout << "\nerror: no species can exist with 0 member\n";
			exit(1);
		}
	}
}
void Neat::printInfo()
{
	std::cout << "\n\n------------ " << "generation : " << m_generation << " ------------\n";
	std::cout << "best fitness : " << m_sortedSpecies[0]->best_fitness;
	std::cout << ", number of species : " << m_species.size();
	std::cout << ", population size : " << m_population.size();
	std::cout << ", specialisation treshold : " << settings.speciation_threshold;

	std::cout << "\nkilled species because of member:";
	for (int id : m_killedSpeciesId_member)
	{
		std::cout << " " << id;
	}
	std::cout << "\nkilled species because of offspring :";
	for (int id : m_killedSpeciesId_offspring)
	{
		std::cout << " " << id;
	}
	std::cout << "\nkilled because of stagnation :";
	for (int id : m_killedSpeciesId_stagnation)
	{
		std::cout << " " << id;
	}

	std::cout << "\nnew species id :";
	for (int id : m_newSpeciesId)
	{
		std::cout << " " << id;
	}

	for (int i = 0; i < m_sortedSpecies.size(); i++)
	{
		std::cout << "\n" << std::setw(5) << m_sortedSpecies[i]->speciesId
			<< " : size=" << std::setw(5) << m_sortedSpecies[i]->members.size()
			<< " | f best=" << std::setw(10) << m_sortedSpecies[i]->best_fitness
			<< " | f mean=" << std::setw(10) << m_sortedSpecies[i]->mean_fitness
			<< " | s ref=" << std::setw(3) << m_sortedSpecies[i]->reference_member.getSize()
			<< " | s lowest=" << std::setw(3) << m_sortedSpecies[i]->lowestSize
			<< " | s biggest=" << std::setw(3) << m_sortedSpecies[i]->biggestSize
			<< " | stagntion=" << std::setw(4) << m_sortedSpecies[i]->stagnation;
	}
	std::cout << "\n";
}
void Neat::createNewGen()
{
	std::vector<Brain> offspringPopulation;
	offspringPopulation.reserve(m_populationSize);
	for (Species& species : m_species)
	{
		for (size_t j = 0; j < species.numOffspring; ++j)
		{

			if (j == 0)
			{
				offspringPopulation.push_back(*species.members[0]);
				//species.reference_member = offspringPopulation.back();
			}
			else
			{
				offspringPopulation.push_back(rouletteWheel(species));
				//if (species.members.size() > 0)
				//{
				//	offspringPopulation.push_back(Brain::crossover(*species.members[Brain::genRandomInt(0, species.members.size() * 0.1)], *species.members[Brain::genRandomInt(0, species.members.size() * 0.1)]));
				//}
				//else
				//{
				//	offspringPopulation.push_back(*species.members[0]);
				//}
				offspringPopulation.back().mutateConnection(settings.mutate_Connection);
				offspringPopulation.back().mutateNode(settings.mutate_Node);
				offspringPopulation.back().mutateBias(settings.mutate_Bias, settings.mutate_Bias_power);
				offspringPopulation.back().mutateWeight(settings.mutate_Weight, settings.mutate_Weight_power, settings.mutate_Weight_max, settings.mutate_Weight_reset);
				offspringPopulation.back().mutateEnableConnection(settings.mutate_EnableConnection);
				offspringPopulation.back().mutateDisableConnection(settings.mutate_DisableConnection);
				//if (j == species.numOffspring - 1)
				//{
				//	species.reference_member = offspringPopulation.back();
				//}
			}
		}
	}
	int restToAdd = m_populationSize - offspringPopulation.size();
	for (int i = 0; i < restToAdd; i++)
	{
		offspringPopulation.push_back(Brain(m_nbInput, m_nbOutput));
	}
	m_population = std::move(offspringPopulation);
}


Brain Neat::rouletteWheel(Species& species)
{
	Brain* parent1 = nullptr;
	Brain* parent2 = nullptr;
	
	std::vector<Brain*> bestOfSpecies;
	if (species.members.size() > 10)
		bestOfSpecies = std::vector<Brain*>(species.members.begin(), species.members.begin() + species.members.size() * 0.3);
	else if (species.members.size() > 5)
		bestOfSpecies = std::vector<Brain*>(species.members.begin(), species.members.begin() + species.members.size() * 0.5);
	else
		return *species.members.front();


	float total=0;
	for (Brain* member : bestOfSpecies)
	{
		total += member->fitness;
	}

	float spin = Brain::genRandomFloat(0, total);
	float offset = 0;

	for (Brain* member : bestOfSpecies)
	{
		offset += member->fitness;
		if (spin <= offset) {
			parent1 = member;
			break;
		}
	}

	spin = Brain::genRandomFloat(0, total);
	offset = 0;
	for (Brain* member : bestOfSpecies)
	{
		offset += member->fitness;
		if (spin <= offset) {
			parent2 = member;
			break;
		}
	}

	if (parent1 == nullptr || parent2 == nullptr)
	{
		std::cout << "\nno parent or child found !\n";
		exit(1);
	}

	if (parent1 == parent2)
	{
		return *parent1;
	}
	else
	{
		return Brain::crossover(*parent1, *parent2);
	}
}


Brain Neat::getBestWinner()
{
	Brain* best = nullptr;
	for (int i = 0; i < m_population.size(); i++)
	{
		if (best == nullptr)
		{
			best = &m_population[i];
		}
		else
		{
			if (best->runSucessfull)
			{
				if (m_population[i].getSize() < best->getSize() && m_population[i].runSucessfull)
				{
					best = &m_population[i];
				}
			}
			else
			{
				if (m_population[i].runSucessfull)
				{
					best = &m_population[i];
				}
				else if (m_population[i].fitness>best->fitness)
				{
					best = &m_population[i];
				}
			}
		}
	}
	if (best == nullptr)
	{
		std::cout << "\n/!\\ no best brain found /!\\\n";
		return Brain(m_population[0].m_nbInput, m_population[0].m_nbOutput);
	}
	else
	{
		return *best;
	}
}

Brain Neat::getBrainWithBestFitness()
{
	Brain* best = nullptr;
	for (int i = 0; i < m_population.size(); i++)
	{
		if (best == nullptr)
		{
			best = &m_population[i];
		}
		else if (m_population[i].fitness > best->fitness)
		{
			best = &m_population[i];
		}
	}

	if (best == nullptr)
	{
		std::cout << "\n/!\\ error : no brain ? /!\\\n";
		return Brain(m_population[0].m_nbInput, m_population[0].m_nbOutput);
	}
	else
	{
		return *best;
	}
}


