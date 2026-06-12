#include <iostream>
#include <cmath>

#include "../../../base/include/useful.h"
#include "../include/bot.h"


using namespace SimpleGA;

int gaucheCustom(Grille& g)
{
	std::vector<std::vector<int>> new_grid = g.table;
	int has_moved = -1;
	for (size_t i = 0; i < g.dim; i++)
	{
		for (size_t j = 1; j < g.dim; j++)
		{
			if (g.table[i][j] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (g.table[i][j] == g.table[i][j - k - 1])
					{
						new_grid[i][j - k - 1] *= 2;
						new_grid[i][j] = 0;
						has_moved = 1;
						g.score += new_grid[i][j - k - 1];
						j++;
						break;
					}
					else if (g.table[i][j - k - 1] != 0)
					{
						break;
					}
				}
			}
		}

		for (size_t j = 0; j < g.dim; j++)
		{
			if (new_grid[i][j] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (new_grid[i][k] == 0)
					{
						new_grid[i][k] = new_grid[i][j];
						new_grid[i][j] = 0;
						has_moved = 1;
					}
				}
			}
		}
	}
	g.table = new_grid;

	if (has_moved == 1) { has_moved = vides(g); }
	return has_moved;
}
int droiteCustom(Grille& g)
{
	std::vector<std::vector<int>> new_grid = g.table;
	int has_moved = -1;
	for (size_t i = 0; i < g.dim; i++)
	{
		for (size_t j = 1; j < g.dim; j++)
		{
			if (g.table[i][g.dim - 1 - j] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (g.table[i][g.dim - 1 - j] == g.table[i][g.dim - 1 - (j - k - 1)])
					{
						new_grid[i][g.dim - 1 - (j - k - 1)] *= 2;
						new_grid[i][g.dim - 1 - j] = 0;
						has_moved = 1;
						g.score += new_grid[i][g.dim - 1 - (j - k - 1)];
						j++;
						break;
					}
					else if (g.table[i][g.dim - 1 - (j - k - 1)] != 0)
					{
						break;
					}
				}
			}
		}

		for (size_t j = 0; j < g.dim; j++)
		{
			if (new_grid[i][g.dim - 1 - j] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (new_grid[i][g.dim - 1 - k] == 0)
					{
						new_grid[i][g.dim - 1 - k] = new_grid[i][g.dim - 1 - j];
						new_grid[i][g.dim - 1 - j] = 0;
						has_moved = 1;
					}
				}
			}
		}
	}
	g.table = new_grid;
	if (has_moved == 1) { has_moved = vides(g); }
	return has_moved;
}
int hautCustom(Grille& g)
{
	std::vector<std::vector<int>> new_grid = g.table;
	int has_moved = -1;
	for (size_t i = 0; i < g.dim; i++)
	{
		for (size_t j = 1; j < g.dim; j++)
		{
			if (g.table[j][i] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (g.table[j][i] == g.table[j - k - 1][i])
					{
						new_grid[j - k - 1][i] *= 2;
						new_grid[j][i] = 0;
						has_moved = 1;
						g.score += new_grid[j - k - 1][i];
						j++;
						break;
					}
					else if (g.table[j - k - 1][i] != 0)
					{
						break;
					}
				}
			}
		}

		for (size_t j = 0; j < g.dim; j++)
		{
			if (new_grid[j][i] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (new_grid[k][i] == 0)
					{
						new_grid[k][i] = new_grid[j][i];
						new_grid[j][i] = 0;
						has_moved = 1;
					}
				}
			}
		}
	}
	g.table = new_grid;
	if (has_moved == 1) { has_moved = vides(g); }
	return has_moved;
}
int basCustom(Grille& g)
{
	std::vector<std::vector<int>> new_grid = g.table;
	int has_moved = -1;
	for (size_t i = 0; i < g.dim; i++)
	{
		for (size_t j = 1; j < g.dim; j++)
		{
			if (g.table[g.dim - 1 - j][i] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (g.table[g.dim - 1 - j][i] == g.table[g.dim - 1 - (j - k - 1)][i])
					{
						new_grid[g.dim - 1 - (j - k - 1)][i] *= 2;
						new_grid[g.dim - 1 - j][i] = 0;
						has_moved = 1;
						g.score += new_grid[g.dim - 1 - (j - k - 1)][i];
						j++;
						break;
					}
					else if (g.table[g.dim - 1 - (j - k - 1)][i] != 0)
					{
						break;
					}
				}
			}
		}

		for (size_t j = 0; j < g.dim; j++)
		{
			if (new_grid[g.dim - 1 - j][i] != 0)
			{
				for (size_t k = 0; k < j; k++)
				{
					if (new_grid[g.dim - 1 - k][i] == 0)
					{
						new_grid[g.dim - 1 - k][i] = new_grid[g.dim - 1 - j][i];
						new_grid[g.dim - 1 - j][i] = 0;
						has_moved = 1;
					}
				}
			}
		}
	}
	g.table = new_grid;
	if (has_moved == 1) { has_moved = vides(g); }
	return has_moved;
}



Bot::Bot()
{
	m_alive = true;
	m_printDead = false;
	if (!(init(m_g, 4, -1, 9)))
	{
		std::cerr << "impossible d'initialiser la grille";
		exit(1);
	}

	m_ai.add(Layer(16, 16, Layer::RELU));
	m_ai.add(Layer(16, 16, Layer::RELU));
	m_ai.add(Layer(16, 8, Layer::RELU));
	m_ai.add(Layer(8, 4, Layer::SOFTMAX));



	m_ai.finalize();
}


void Bot::reset()
{
	m_g.table.clear();
	m_alive = true;
	m_printDead = false;
	if (!(init(m_g, 4, -1, 9)))
	{
		std::cerr << "impossible d'initialiser la grille";
		exit(1);
	}
}

int Bot::getScore1() const
{
	int score=0;
	int highestTile = vec2dMax(m_g.table);
	int nbVides=vides(m_g);
	score = highestTile + m_g.score;
	return score;
}



void Bot::play()
{
	int mov = 0;
	int has_moved = -1;

	std::vector<float> input_norm;

	std::vector<float> vec1d = Matrix::vec2dToVec1d(m_g.table);

	for (float ele : vec1d)
	{
		if (ele != 0)
			input_norm.push_back(log2f(ele));
		else
			input_norm.push_back(ele);
	}

	m_ai.forward(input_norm);

	for (int i = 0; i < m_ai.m_output->row(); i++)
	{
		if (m_ai.m_output->operator()(i, 0) > m_ai.m_output->operator()(mov, 0))
			mov = i;
	}

	std::vector<int> testScore = { 0,0,0,0 };
	Grille testGrille = m_g;
	gaucheCustom(testGrille);
	testScore[0] = score(testGrille);

	testGrille = m_g;
	droiteCustom(testGrille);
	testScore[1] = score(testGrille);

	testGrille = m_g;
	hautCustom(testGrille);
	testScore[2] = score(testGrille);

	testGrille = m_g;
	basCustom(testGrille);
	testScore[3] = score(testGrille);

	int bestTestScore = vec1dMax(testScore);
	int prevScore = score(m_g);

	switch (mov) {
	case 0:
		has_moved = gaucheCustom(m_g);
		break;
	case 1:
		has_moved = droiteCustom(m_g);
		break;
	case 2:
		has_moved = hautCustom(m_g);
		break;
	case 3:
		has_moved = basCustom(m_g);
		break;
	case 'q':
		break;
	default:
		break;
	}

	if (score(m_g) < bestTestScore)
	{
		if (score(m_g) - (bestTestScore - score(m_g))*2 >0)
			m_g.score -= (bestTestScore - score(m_g)) * 2;
	}
	else
	{
		m_g.score += (score(m_g)-prevScore);
	}

	if (vides(m_g) > 0 && has_moved != -1)
		spawnRandom(m_g);
	else
		m_alive = false;
}

void Bot::printGrid(int x, int y)
{
	afficheCustom(m_g, x, y, false);
}
void Bot::printOutput(int x, int y)
{
	if (!m_printDead)
	{
		printAtPos(setColor(0, 255, 0) + "output:" + resetColor(), x, y + 18);
		Matrix result = m_ai.getOutput();
		result.print(x, y + 19);
	}
	else
	{
		printAtPos(setColor(255, 0, 0) + "--------" + resetColor(), x, y + 18);
		printAtPos(setColor(255, 0, 0) + "| DEAD |" + resetColor(), x, y + 19);
		printAtPos(setColor(255, 0, 0) + "--------" + resetColor(), x, y + 20);
	}

	if (!m_alive) m_printDead = true;
}

void Bot::mutateFlip(float rate)
{
	for (Layer& layer : m_ai.m_layers)
	{
		for (int i = 0; i < layer.m_nbNeurons; i++)
		{
			for (int j = 0; j < layer.m_NbInput; j++)
			{
				if (genRandomInt(1, round(1.0 / rate)) == 1)
					layer.m_weight(i, j) = -layer.m_weight(i, j);
			}
		}
	}
}
void Bot::mutateAdd(float fac)
{
	for (Layer& layer : m_ai.m_layers)
	{
		for (int i = 0; i < layer.m_nbNeurons; i++)
		{
			for (int j = 0; j < layer.m_NbInput; j++)
			{
				layer.m_weight(i, j) += genRandomFloat(-fac / 2.0f, fac / 2.0f);
			}
		}
	}
}
void Bot::mutateReset(float rate)
{
	for (Layer& layer : m_ai.m_layers)
	{
		for (int i = 0; i < layer.m_nbNeurons; i++)
		{
			for (int j = 0; j < layer.m_NbInput; j++)
			{
				if (genRandomInt(1, round(1.0 / rate)) == 1)
					layer.m_weight(i, j) = genRandomFloat(-1,1);
			}
		}
	}
}

