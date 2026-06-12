#include <iostream>
#include <string>
#include <omp.h>
#include <thread>
#include <filesystem>

#include "../include/population.h"
#include "../../../base/include/useful.h"
#include "../../../base/include/keyboard.h"

#include "../../glHelper/glHelper.h"


using namespace SimpleGA;

Population::Population(int size)
{
	m_everyoneDead = false;
	m_generation = 0;
	m_size = size;

	m_bots = std::vector<Bot>(m_size, Bot());

	m_generationMeanScores = 0;
	m_generationBestMeanScore = 0;
	m_generationBestScore = 0;
	m_allTimeBestMeanScore = 0;
	m_allTimeBestScore = 0;


	m_generationPerBotScores = std::vector<std::vector<int>>(m_size, std::vector<int>(m_gameNumber, 0));
	m_generationPerBotMeanScores = std::vector<int>(m_size, 0);

	m_allMeanScores.push_back(0);
	m_allBestMeanScores.push_back(0);
	m_allBestScores.push_back(0);
}

void Population::runGen()
{
	for (int i = 0; i < m_gameNumber; i++)
	{
		while (!isEveryoneDead())
		{
			play();
		}
		for (int j = 0; j < m_bots.size(); ++j) {
			m_generationPerBotScores[j][i] = m_bots[j].getScore();
			m_bots[j].reset();
		}
		m_everyoneDead = false;
	}
	updateScore();
}


void Population::generateNewGen()
{
	Bot parent1;
	Bot parent2;
	getTwoBestChild(parent1, parent2);
	for (Bot& child : m_bots)
	{
		crossParents(parent1, parent2, child);
		child.mutateAdd(0.1);
	}
	if (m_size>1)
	{
		m_bots[0] = parent1;
		m_bots[1] = parent2;
	}
	if (m_size>5)
	{
		m_bots[2] = Bot();
		m_bots[3] = Bot();
		m_bots[4] = Bot();
	}
	m_generation++;
}

void Population::generateNewGen1()
{
	Bot parent1;
	getBestChild(parent1);
	for (Bot& child : m_bots)
	{
		child = parent1;
		child.mutateAdd(0.1);
	}
	m_bots[0] = parent1;
	m_generation++;
}

void Population::play()
{
	m_everyoneDead = true;
#pragma omp parallel for
	for (int i = 0; i < m_bots.size(); ++i) {
		Bot* bot = &m_bots[i];
		if (bot->m_alive) {
			m_everyoneDead = false;
			//bot->printGrid((i % 3) * 40 + 1, ((i) / 3) * 24 + 1);
			bot->play();
			//bot->printOutput((i % 3) * 40 + 1, ((i) / 3) * 24 + 1);
		}
	}
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


void Population::plotting(bool* p_open)
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	float ratio = glfwGetVideoMode(glfwGetPrimaryMonitor())->width/1920.f;
	ImGui::Begin("window with plot", nullptr, ImGuiWindowFlags_NoScrollbar);
	static bool first = true;
	if (first)
	{
		ImGui::SetWindowSize(ImVec2(800*ratio, 400*ratio));
		first = false;
	}
	
	if (m_generation==0)
		ImPlot::SetNextAxesLimits(0, 100, 0, 100, ImGuiCond_Always);
	else
		ImPlot::SetNextAxesLimits(0, m_generation, 0, m_allTimeBestScore * 1.1, ImGuiCond_Always);

	//ImVec2 size = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - ImGui::GetTextLineHeightWithSpacing() };
	if (ImPlot::BeginPlot("plot", ImVec2(-1,-1), ImPlotFlags_Crosshairs))
	{
		ImPlot::PlotLine("BestMeanScores", m_allBestMeanScores.data(), m_allBestMeanScores.size());
		ImPlot::PlotLine("MeanScores", m_allMeanScores.data(), m_allMeanScores.size());
		ImPlot::PlotLine("BestScores", m_allBestScores.data(), m_allBestScores.size());
		ImPlot::EndPlot();
	}
	ImGui::End();
	ImGui::PopStyleVar();
}


void Population::drawNetwork(bool* p_open)
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);

	float ratio = glfwGetVideoMode(glfwGetPrimaryMonitor())->width / 1920.f;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("neural network representation");
	static bool first = true;
	if (first)
	{
		ImGui::SetWindowSize(ImVec2(800 * ratio, 500 * ratio));
		first = false;
	}
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImVec2 center = { topLeft.x + ImGui::GetWindowWidth() / 2.f , topLeft.y + (ImGui::GetWindowHeight()+ImGui::GetTextLineHeightWithSpacing()) / 2.f  };

	//draw_list->AddCircle(center, 20, IM_COL32(255, 0, 0, 255));

	std::vector<int> networkTopologie;
	networkTopologie.push_back(m_bots[0].m_ai.m_layers[0].getInputNumber());
	for (size_t i = 0; i < m_bots[0].m_ai.m_layers.size(); i++)
	{
		networkTopologie.push_back(m_bots[0].m_ai.m_layers[i].getNeuronsNumber());
	}
	int nbOfLayer = networkTopologie.size();

	float radius = 10;

	float gapSizeX = 100;
	float gapSizeY = 25;
	float netWorkLeftPos = center.x - (nbOfLayer-1) / 2.f * gapSizeX;

	float currentLayerX = netWorkLeftPos;
	float currentLayerY = 0;
	for (size_t i = 0; i < nbOfLayer; i++)
	{
		currentLayerY = center.y - networkTopologie[i] / 2.f * gapSizeY;
		for (size_t j = 0; j < networkTopologie[i]; j++)
		{
			if (i == 0)
				draw_list->AddCircle({ currentLayerX, currentLayerY }, radius, IM_COL32(255, 0, 0, 255));
			else if (i == nbOfLayer-1)
				draw_list->AddCircle({ currentLayerX, currentLayerY }, radius, IM_COL32(0, 0, 255, 255));
			else
				draw_list->AddCircle({ currentLayerX, currentLayerY }, radius, IM_COL32(0, 255, 0, 255));
			currentLayerY += gapSizeY;
		}
		currentLayerX += gapSizeX;
	}

	ImVec2 currentConnectionStart = { 0,0 };
	ImVec2 currentConnectionEnd = { 0,0 };
	for (size_t i = 0; i < nbOfLayer-1; i++)
	{
		for (size_t j = 0; j < networkTopologie[i]; j++)
		{
			currentConnectionStart.x = netWorkLeftPos + gapSizeX * i + radius;
			currentConnectionStart.y = (center.y - networkTopologie[i] / 2.f * gapSizeY) + (gapSizeY * j);
			for (size_t k = 0; k < networkTopologie[i+1]; k++)
			{
				currentConnectionEnd.x = netWorkLeftPos + gapSizeX * (i+1) - radius;
				currentConnectionEnd.y = (center.y - networkTopologie[i+1] / 2.f * gapSizeY) + (gapSizeY * k);

				int r = 0;
				int g = 0;
				int b = 0;
				int a = m_bots[0].m_ai.m_layers[i].m_weight(k, k) * 255;
				if (m_bots[0].m_ai.m_layers[i].m_weight(k, k) > 0)
				{
					r = 255;
				}
				else
				{
					b = 255;
				}
				draw_list->AddLine(currentConnectionStart, currentConnectionEnd, IM_COL32(r, g, b, a));
			}
		}
	}
	
	ImGui::End();
	ImGui::PopStyleVar();
}



void Population::train()
{
	bool isTraining = true;
	bool paused = true;

	clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
	std::cout << "\x1b[?25l";
	SMALL_RECT root = drawRectangle(70, 20);
	printCentered("generation: " + std::to_string(getGeneration()), 0, -4, root);
	printCentered("   generation best scores (mean of multiple game): " + std::to_string(m_generationBestMeanScore) + "   ", 0, -2, root);
	printCentered("all time best score (mean of multiple game): " + std::to_string(m_allTimeBestMeanScore), 0, -1, root);
	printCentered("press 'enter' to pause/resume training", 0, 1, root);
	printCentered("press 'key down' to test the current generation best bot", 0, 1, root);
	printCentered("press 'escape' to quit training", 0, 2, root);

	if (paused)
		printCentered("paused : true  (press 'enter' to change)", 0, 4, root);
	else
		printCentered("paused : false (press 'enter' to change)", 0, 4, root);


	GlHelper::add(&Population::drawNetwork, this, "drawing");
	GlHelper::add(&Population::plotting, this, "plotting");

	while (isTraining)
	{
		if (Keyboard::isKeyPressed(VK_RETURN)) {
			paused = !paused;
			if (paused)
				printCentered("paused : true  (press 'enter' to change)", 0, 4, root);
			else
				printCentered("paused : false (press 'enter' to change)", 0, 4, root);
		}
		if (Keyboard::isKeyPressed(VK_ESCAPE)) {
			isTraining = false;
		}
		if (Keyboard::isKeyPressed(VK_DOWN)) {
			Bot best;
			bool stop = false;
			getBestChild(best);
			best.reset();
			while (!stop)
			{
				clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
				std::cout << "\x1b[?25l";
				best.printGrid(1, 1);
				best.play();
				best.printOutput(1, 1);
				while (true)
				{
					if (Keyboard::isKeyPressed(VK_RETURN)) {
						break;
					}
					if (Keyboard::isKeyPressed(VK_ESCAPE)) {
						stop = true;
						break;
					}
				}
			}
			clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
			std::cout << "\x1b[?25l";
			drawRectangle(70, 20);
			printCentered("generation: " + std::to_string(getGeneration()), 0, -4, root);
			printCentered("   generation best scores (mean of multiple game): " + std::to_string(m_generationBestMeanScore) + "   ", 0, -2, root);
			printCentered("all time best score (mean of multiple game): " + std::to_string(m_allTimeBestMeanScore), 0, -1, root);
			printCentered("press 'enter' to pause/resume training", 0, 1, root);
			printCentered("press 'key down' to test the current generation best bot", 0, 1, root);
			printCentered("press 'escape' to quit training", 0, 2, root);
			if (paused)
				printCentered("paused : true  ", 0, 4, root);
			else
				printCentered("paused : false", 0, 4, root);
		}

		if (!paused)
		{
			printCentered("generation: " + std::to_string(getGeneration()), 0, -4, root);
			printCentered("   generation best scores (mean of multiple game): " + std::to_string(m_generationBestMeanScore) + "   ", 0, -2, root);
			printCentered("all time best score (mean of multiple game): " + std::to_string(m_allTimeBestMeanScore), 0, -1, root);

			runGen();
			generateNewGen();
		}
		else
		{
			Sleep(50);
		}
	}

	GlHelper::remove("plotting", this);
	GlHelper::remove("drawing", this);
}



void Population::updateScore()
{
	m_generationMeanScores = 0;
	m_generationBestMeanScore = 0;
	m_generationBestScore = 0;

	int max = 0;
	for (int i = 0; i < m_size; i++) {
		m_generationPerBotMeanScores[i] = vecMean(m_generationPerBotScores[i]);
		if (m_generationPerBotMeanScores[i] > m_generationBestMeanScore) m_generationBestMeanScore = m_generationPerBotMeanScores[i];

		max = vec1dMax(m_generationPerBotScores[i]);
		if (max > m_generationBestScore) m_generationBestScore = max;
	}
	m_generationMeanScores = vecMean(m_generationPerBotMeanScores);

	if (m_generationBestMeanScore > m_allTimeBestMeanScore) m_allTimeBestMeanScore = m_generationBestMeanScore;
	if (m_generationBestScore > m_allTimeBestScore)			m_allTimeBestScore = m_generationBestScore;

	m_allMeanScores.push_back(m_generationMeanScores);
	m_allBestMeanScores.push_back(m_generationBestMeanScore);
	m_allBestScores.push_back(m_generationBestScore);
}

int Population::getNumberAlive()
{
	int alive = 0;
	for (Bot& bot : m_bots)
		if (bot.m_alive)
			alive++;
	return alive;
}

void Population::printGrid(int nbOfBotPerRow)
{
	for (int i = 0; i < m_size; i++)
		m_bots[i].printGrid((i % nbOfBotPerRow) * 40 + 1, ((i) / nbOfBotPerRow) * 24 + 1);
}
void Population::printOutput(int nbOfBotPerRow)
{
	for (int i = 0; i < m_size; i++)
		m_bots[i].printOutput((i % nbOfBotPerRow) * 40 + 1, ((i) / nbOfBotPerRow) * 24 + 1);
}


void Population::getBestChild(Bot& child)
{
	int bestScore = 0;
	int index = 0;
	for (size_t i = 0; i < m_size; i++)
	{
		if (m_generationPerBotMeanScores[i] > bestScore)
		{
			bestScore = m_generationPerBotMeanScores[i];
			index = i;
		}
	}
	child = m_bots[index];
}
void Population::getTwoBestChild(Bot& child1, Bot& child2)
{
	int bestScore = 0;
	int index1 = 0;
	int index2 = 0;
	for (size_t i = 0; i < m_size; i++)
	{
		if (m_generationPerBotMeanScores[i] > bestScore)
		{
			bestScore = m_generationPerBotMeanScores[i];
			index1 = i;
		}
	}

	bestScore = 0;
	for (size_t i = 0; i < m_size; i++)
	{
		if (m_generationPerBotMeanScores[i] > bestScore && i != index1)
		{
			bestScore = m_generationPerBotMeanScores[i];
			index2 = i;
		}
	}
	child1 = m_bots[index1];
	child2 = m_bots[index2];
}

Bot Population::crossParents(Bot& parrent1, Bot& parrent2, Bot& child)
{
	for (int i = 0; i < child.m_ai.m_layers.size(); i++)
	{
		Layer* childLayer = &child.m_ai.m_layers[i];
		Layer* parent1Layer = &parrent1.m_ai.m_layers[i];
		Layer* parent2Layer = &parrent2.m_ai.m_layers[i];

		for (int j = 0; j < childLayer->m_nbNeurons; j++)
		{
			int crossPoint = genRandomInt(0, childLayer->m_NbInput - 1);
			//crossPoint = childLayer->m_NbInput - 1;
			if (crossPoint == 0)
			{
				childLayer->m_weight(j) = parent2Layer->m_weight(j);
				childLayer->m_bias(j) = parent2Layer->m_bias(j);

			}
			else if (crossPoint == childLayer->m_NbInput - 1)
			{
				childLayer->m_weight(j) = parent1Layer->m_weight(j);
				childLayer->m_bias(j) = parent1Layer->m_bias(j);

			}
			else
			{
				std::vector<float>::iterator it = childLayer->m_weight(j).begin();
				it = std::copy(parent1Layer->m_weight(j).begin(), parent1Layer->m_weight(j).begin() + crossPoint, it);
				std::copy(parent2Layer->m_weight(j).begin() + crossPoint, parent2Layer->m_weight(j).end(), it);
				if (crossPoint < childLayer->m_NbInput / 2)
				{
					childLayer->m_bias(j) = parent2Layer->m_bias(j);
				}
				else
				{
					childLayer->m_bias(j) = parent1Layer->m_bias(j);
				}
			}
		}
	}
	return child;
}
