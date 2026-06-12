#pragma once

#include <vector>

#include "matrix.h"
#include "jeu.h"
#include "layer.h"

namespace SimpleGA
{
	class Model
	{
	public:

		Model();
		Model(const Model& m);
		Model& operator=(const Model& m);
		~Model();

		inline void add(Layer a)
		{
			m_layers.push_back(a);
		}

		void finalize();

		void forward(const std::vector<float>& input);

		inline Matrix getOutput() const
		{
			return *m_output;
		}


	private:
		std::vector<Layer> m_layers;
		Matrix* m_output;

		friend class Bot;
		friend class Population;
	};
}
