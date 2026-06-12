#include <iostream>

#include "../include/model.h"
#include "../../../base/include/useful.h"

using namespace SimpleGA;

Model::Model() : m_output(nullptr) {};

Model::Model(const Model& m)
{
	m_output = new Matrix(m.m_layers[m.m_layers.size() - 1].getNeuronsNumber(), 1, false);
	m_layers = m.m_layers;
}

Model& Model::operator=(const Model& m)
{
	if (this != &m)
	{
		m_output = new Matrix(m.m_layers[m.m_layers.size() - 1].getNeuronsNumber(), 1, false);
		m_layers = m.m_layers;
	}
	return *this;
}



void Model::finalize()
{
	m_output = new Matrix(m_layers[m_layers.size()-1].getNeuronsNumber(), 1, false);
}	

void Model::forward(const std::vector<float>& input)
{
	size_t nbLayer = m_layers.size();	
	for (size_t i = 0; i < nbLayer; i++)
	{
		if (i == 0)
		{
			Matrix test = Matrix::vec1dToMat(input);
			m_layers[i].forward(test);
		}
		else
		{
			m_layers[i].forward(m_layers[i - 1].m_output);
		}

		if (i == nbLayer - 1)
		{
			*m_output = m_layers[i].m_output;
		}
	}
}

Model::~Model()
{
	delete m_output;
}

