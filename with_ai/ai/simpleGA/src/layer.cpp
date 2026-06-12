#include <iostream>
#include <cmath>

#include "../include/layer.h"
#include "../../../base/include/useful.h"


using namespace SimpleGA;

float Layer::relu(float x)
{
	if (x > 0) return x;
	else return 0;
}
float Layer::sigmoid(float x)
{
	return 1.0f / (1.0f + std::exp(-x));
}

Layer::Layer(const int nb_input, const int nb_neurons, Function func):
	m_NbInput(nb_input),
	m_nbNeurons(nb_neurons),
	m_weight(nb_neurons, nb_input, true),
	m_bias(nb_neurons, 1, true),
	m_output(nb_neurons, 1, false),
	m_func(func)
{}

void Layer::forward(const Matrix& inputs)
{
	Matrix::dot(m_weight, inputs, m_output);

	for (size_t i = 0; i < m_nbNeurons; i++)
		m_output(i, 0) += m_bias(i, 0);

	switch (m_func)
	{
	case Layer::SIGMOID:
		for (size_t i = 0; i < m_nbNeurons; i++)
			m_output(i, 0) = sigmoid(m_output(i, 0));
		break;
	case Layer::RELU:
		for (size_t i = 0; i < m_nbNeurons; i++)
			m_output(i, 0) = relu(m_output(i, 0));
		break;
	case Layer::TANH:
		for (size_t i = 0; i < m_nbNeurons; i++)
			m_output(i, 0) = tanh(m_output(i, 0));
		break;
	case Layer::SOFTMAX:
	{
		float sum = 0;
		for (size_t i = 0; i < m_nbNeurons; i++)
		{
			m_output(i, 0) = std::exp(m_output(i, 0));
			sum += m_output(i, 0);
		}
		for (size_t i = 0; i < m_nbNeurons; i++)
		{
			m_output(i, 0) /= sum;
		}
		break;
	}
	default:
		break;
	}


			
}