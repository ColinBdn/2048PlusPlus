#pragma once

#include <vector>

#include "matrix.h"
#include "jeu.h"


namespace SimpleGA
{
	class Layer
	{
	public:
		enum Function{SIGMOID, RELU, SOFTMAX, TANH};
		Layer(const int nb_input, const int nb_neurons, Function func);

		void forward(const Matrix& inputs);

		inline Matrix getOutput() const
		{
			return m_output;
		}
		inline Matrix getWeight() const
		{
			return m_weight;
		}
		inline int getNeuronsNumber() const
		{
			return m_nbNeurons;
		}
		inline int getInputNumber() const
		{
			return m_NbInput;
		}

	private:
		int m_NbInput;
		int m_nbNeurons;
		Matrix m_weight;
		Matrix m_bias;
		Matrix m_output;
		Function m_func;

		float relu(float x);
		float sigmoid(float x);

		friend class Model;
		friend class Population;
		friend class Bot;
	};
}