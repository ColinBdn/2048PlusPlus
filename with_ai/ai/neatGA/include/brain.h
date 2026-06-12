#pragma once
#include <vector>
#include <unordered_map>
#include <thread>
#include <iostream>

#include "glHelper.h"

class Brain
{
public:
	Brain(int nbInput, int nbOutput);
	void forward(const std::vector<float>& inputs, float activation_stepness=1);
	void addConnection(int in, int out, float weight);
	void addNode(int connectionId);
	void disableConnection(int connectionId);
	void enableConnection(int connectionId);

	void mutateNode(float probability);
	void mutateConnection(float probability);
	void mutateEnableConnection(float probability);
	void mutateDisableConnection(float probability);
	void mutateWeight(float probability, float power, float max, float probaReset);
	void mutateBias(float probability, float power);

	void printAsync();
	void print();
	void stopPrinting();

	void writeToFile(const std::string& filename);
	static Brain readFromFile(const std::string& filename);


	static float distance(const Brain& brain1, const Brain& brain2);

	bool operator<(const Brain& other) const {
		return fitness < other.fitness;
	}
	bool operator>(const Brain& other) const {
		return fitness > other.fitness;
	}
	bool operator==(const Brain& other) const {
		return fitness == other.fitness;
	}

	static Brain crossover(const Brain& parent1, const Brain& parent2);

	float fitness;
	float ajustedFitness;
	bool runSucessfull;
	int especeId;

	std::vector<float> getOutput();
	int getSize() const;
	
	static float genRandomFloat(const float& a, const float& b);
	static int genRandomInt(int a, int b);

	Brain(const Brain& other);
	~Brain();
	
private:
	Brain();

	enum Type { INPUT, OUTPUT, HIDDEN };
	struct Connection
	{
		int connection_id;
		int in_neuron_id;
		int out_neuron_id;
		float weight;
		bool enabled = true;
		bool recurrent = false;
		int numer_of_time_overwrited = 0;
	};
	struct Neuron
	{
		int neuron_id;
		Type type;
		int overwrited_connection_id = -1;
		int numer_of_time_connection_overwrited = -1;
		std::vector<int> in_connections_id;
		std::vector<int> out_connections_id;

		float value=0;
		float bias = 0;

		bool has_value = false;
		bool has_asked = false;
	
		ImVec2 pos;
	};



	void writeNeuronToFile(std::ofstream& outFile, const Neuron& neuron);
	void writeConnectionToFile(std::ofstream& outFile, const Connection& neuron);

	static Neuron readNeuronFromFile(std::ifstream& inFile);
	static Connection readConnectionFromFile(std::ifstream& inFile);


	int m_nbInput;
	int m_nbOutput;

	std::vector<int> m_input_id;
	std::vector<int> m_output_id;

	std::unordered_map<int, Neuron> m_neurons;
	std::unordered_map<int, Connection> m_connections;

	static int global_connection_innovation_number;
	static std::vector<Connection> global_connection_innovations_history;

	static int global_neuron_innovation_number;
	static std::vector<Neuron> global_neuron_innovations_history;

	float selectionProbability;

	int getConnectionInnovationNumber(const Connection& connection);
	int getNeuronInnovationNumber(const Neuron& neuron);

	void setConnectionInnovationNumber(Connection& connection);
	void setNeuronInnovationNumber(Neuron& neuron);

	void recursiveOutputEvaluation(int id, float activation_stepness = 1);
	float relu(float x);
	float sigmoid(float x, float stepness = 0);

	int findMaxDepth(int baseId, int n, bool first = true);
	int findMaxReverseDepth(int baseId, int n, bool first = true);
	bool isRecurrent(int in, int out, int n=0);

	void drawBrain(bool* p_open);

	static bool proba(float proba);

	template<typename T>
	static T vec1dMax(const std::vector<T>& vec)
	{
		T max = 0;
		for (T ele : vec)
		{
			if (ele > max) max = ele;
		}
		return max;
	}


	bool m_firstPrint;
	int m_idDragged;
	std::unordered_map<int, ImVec2> m_userOffsets;

	friend class Neat;
};

