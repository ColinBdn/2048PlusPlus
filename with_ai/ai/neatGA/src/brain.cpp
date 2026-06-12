#include <string>
#include <sstream>
#include <filesystem>
#include <random>
#include <fstream>
#include <list>
#include <cmath>

#include "glHelper.h"
#include "brain.h"
#include "neat.h"


float Brain::relu(float x)
{
	if (x > 0) return x;
	else return 0;
}
float Brain::sigmoid(float x, float stepness)
{
	return 1.0f / (1.0f + std::exp(-x*stepness));
}

static std::mt19937 rng(std::random_device{}());
//static std::mt19937 rng(1);

float Brain::genRandomFloat(const float& a, const float& b)
{
	std::uniform_real_distribution<float> dist(a, b);
	return dist(rng);
}
int Brain::genRandomInt(int a, int b)
{
	std::uniform_int_distribution<int> dis(a, b);
	return dis(rng);
}
bool Brain::proba(float probability)
{
	std::uniform_real_distribution<float> dis(0.0, 1.0);
	return dis(rng) <= probability;
}


int partition(std::vector<int>& arr, int start, int end)
{
	int pivot = arr[start];
	int count = 0;
	for (int i = start + 1; i <= end; i++) {
		if (arr[i] <= pivot)
			count++;
	}
	int pivotIndex = start + count;
	std::swap(arr[pivotIndex], arr[start]);
	int i = start, j = end;
	while (i < pivotIndex && j > pivotIndex) {

		while (arr[i] <= pivot) {
			i++;
		}

		while (arr[j] > pivot) {
			j--;
		}

		if (i < pivotIndex && j > pivotIndex) {
			std::swap(arr[i++], arr[j--]);
		}
	}
	return pivotIndex;
}
void quickSort(std::vector<int>& arr, int start=-1, int end=-1)
{
	if (start == -1 && end == -1)
	{
		start = 0;
		end = arr.size()-1;
	}
	if (start >= end)
		return;
	int p = partition(arr, start, end);
	quickSort(arr, start, p - 1);
	quickSort(arr, p + 1, end);
}
void insertionSort(std::vector<int>& arr)
{
	size_t n = arr.size();
	size_t i, key, j;
	for (i = 1; i < n; i++) {
		key = arr[i];
		j = i - 1;
		while (j >= 0 && arr[j] > key) {
			arr[j + 1] = arr[j];
			j = j - 1;
		}
		arr[j + 1] = key;
	}
}


int Brain::global_connection_innovation_number = 0;
std::vector<Brain::Connection> Brain::global_connection_innovations_history;
int Brain::global_neuron_innovation_number=0;
std::vector<Brain::Neuron> Brain::global_neuron_innovations_history;


Brain::Brain(int nbInput, int nbOutput)
	:m_nbInput(nbInput), m_nbOutput(nbOutput), fitness(0), ajustedFitness(0), especeId(0),
	m_firstPrint(true), m_idDragged(-1), runSucessfull(false), selectionProbability(0)
{

	for (int i = 0; i < nbInput; i++)
	{
		Neuron neuron;
		neuron.type = Type::INPUT;
		neuron.neuron_id = i;
		neuron.overwrited_connection_id = -1;
		if (global_neuron_innovation_number == 0)
		{
			global_neuron_innovations_history.push_back(neuron);
		}

		m_neurons[neuron.neuron_id] = neuron;
		m_userOffsets[neuron.neuron_id] = ImVec2(0, 0);
		m_input_id.push_back(neuron.neuron_id);
	}

	for (int i = 0; i < nbOutput; i++)
	{
		Neuron neuron;
		neuron.type = Type::OUTPUT;
		neuron.neuron_id = i+nbInput;
		neuron.overwrited_connection_id = -1;
		if (global_neuron_innovation_number == 0)
		{
			global_neuron_innovations_history.push_back(neuron);
		}

		m_neurons[neuron.neuron_id] = neuron;
		m_userOffsets[neuron.neuron_id] = ImVec2(0,0);
		m_output_id.push_back(neuron.neuron_id);

		for (size_t j = 0; j < nbInput; j++)
		{
			addConnection(m_input_id[j], m_output_id[i], genRandomFloat(-1, 1));
		}
	}

	if (global_neuron_innovation_number == 0)
	{
		global_neuron_innovation_number += nbInput + nbOutput-1;
	}
}
Brain::Brain()
	: m_nbInput(0), m_nbOutput(0), fitness(0), ajustedFitness(0), especeId(0),
	m_firstPrint(true), m_idDragged(-1), runSucessfull(false), selectionProbability(0)
{}
Brain::Brain(const Brain& other)
	: fitness(other.fitness), ajustedFitness(other.ajustedFitness), especeId(other.especeId), m_nbInput(other.m_nbInput),
	m_nbOutput(other.m_nbOutput), m_input_id(other.m_input_id), m_output_id(other.m_output_id),
	m_neurons(other.m_neurons), m_connections(other.m_connections), m_userOffsets(other.m_userOffsets), m_firstPrint(true),
	m_idDragged(-1), runSucessfull(other.runSucessfull), selectionProbability(other.selectionProbability){}

Brain::~Brain()
{
	stopPrinting();
}



void Brain::recursiveOutputEvaluation(int baseId, float activation_stepness)
{
	Neuron* neuron = &m_neurons[baseId];
	Connection* connection;
	Neuron* inNeuron;

	float connectionWeight;
	float value=0;

	for (int id : neuron->in_connections_id)
	{
		connection = &m_connections[id];
		if (connection->enabled)
		{
			inNeuron = &m_neurons[connection->in_neuron_id];
			connectionWeight = connection->weight;

			if (inNeuron->has_value)
			{
				value += inNeuron->value * connectionWeight;
			}
			else
			{
				recursiveOutputEvaluation(inNeuron->neuron_id, activation_stepness);
				value += inNeuron->value * connectionWeight;
			}
		}
	}

	value += neuron->bias;
	value = sigmoid(value, activation_stepness);
	//value = tanh(value);
	neuron->value = value;
	neuron->has_value = true;
}
void Brain::forward(const std::vector<float>& inputs, float activation_stepness)
{
	for (std::pair<const int, Neuron>& neuron : m_neurons)
	{
		neuron.second.has_asked = false;
		neuron.second.has_value = false;
	}
	if (inputs.size() > m_nbInput)
	{
		std::cout << "forward error: too much inputs\n";
		exit(1);
	}
	Neuron* neuron;
	for (int i=0; i<inputs.size();i++)
	{
		neuron = &m_neurons[m_input_id[i]];
		if (neuron->type != Type::INPUT)
		{
			std::cout << "wrong input id\n";
			exit(1);
		}
		neuron->value = inputs[i];
		neuron->has_value = true;
	}

	for (size_t i = 0; i < m_nbOutput; i++)
	{
		recursiveOutputEvaluation(m_output_id[i], activation_stepness);
	}
}


void Brain::addConnection(int in, int out, float weight)
{
	if (in == out)
	{
		std::cout << "add connection error: 'in' can't be equal to 'out'\n";
		exit(1);
	}
	else if (m_neurons.count(in)==0 || m_neurons.count(out) == 0)
	{
		std::cout << "add connection error: 'out' or 'in' neurons do not exist in the genome\n";
		exit(1);
	}
	else if (m_neurons[out].type == Type::INPUT)
	{
		std::cout << "add connection error: 'out' can't be an input neuron\n";
		exit(1);
	}

	for (int connectionID : m_neurons[out].in_connections_id)
	{
		if (m_connections[connectionID].in_neuron_id == in)
		{
			std::cout << "add connection error: connection allready exist\n";
			exit(1);
		}
	}

	Connection connection;
	connection.in_neuron_id = in;
	connection.out_neuron_id = out;
	connection.weight = weight;
	setConnectionInnovationNumber(connection);

	connection.recurrent = isRecurrent(in, out);

	m_connections[connection.connection_id] = connection;
	m_neurons[out].in_connections_id.push_back(connection.connection_id);
	m_neurons[in].out_connections_id.push_back(connection.connection_id);

}
void Brain::addNode(int connectionID)
{
	if (m_connections.find(connectionID) == m_connections.end())
	{
		std::cout << "add node error: 'connectionID' is out of bound\n";
		exit(1);
	}
	else if (m_connections[connectionID].enabled == false)
	{
		std::cout << "add node error: can't add a node on a disabled connection (id:"<< connectionID <<")\n";
		exit(1);
	}

	Connection newConnection1;
	Connection newConnection2;
	Neuron newNeuron;

	newNeuron.type = Type::HIDDEN;
	newNeuron.overwrited_connection_id = connectionID;
	newNeuron.numer_of_time_connection_overwrited = m_connections[connectionID].numer_of_time_overwrited;
	m_connections[connectionID].numer_of_time_overwrited++;
	setNeuronInnovationNumber(newNeuron);
	m_userOffsets[newNeuron.neuron_id] = ImVec2(0,0);
	m_neurons[newNeuron.neuron_id] = newNeuron;

	addConnection(m_connections[connectionID].in_neuron_id,
		newNeuron.neuron_id,
		1);
	
	addConnection(newNeuron.neuron_id,
		m_connections[connectionID].out_neuron_id,
		m_connections[connectionID].weight);
	
	disableConnection(connectionID);
}
void Brain::disableConnection(int connectionID)
{
	if (m_connections[connectionID].enabled == false)
	{
		std::cout << "disable connection error : connection already disabled";
		exit(1);
	}
	m_connections[connectionID].enabled = false;
	std::vector<int>* in_connections_of_out_neuron = &m_neurons[m_connections[connectionID].out_neuron_id].in_connections_id;
	for (int i=0; i< in_connections_of_out_neuron->size(); i++)
	{
		if (in_connections_of_out_neuron->operator[](i) == connectionID)
		{
			in_connections_of_out_neuron->erase(in_connections_of_out_neuron->begin()+i);
		}
	}
	std::vector<int>* out_connections_of_in_neuron = &m_neurons[m_connections[connectionID].in_neuron_id].out_connections_id;
	for (int i = 0; i < out_connections_of_in_neuron->size(); i++)
	{
		if (out_connections_of_in_neuron->operator[](i) == connectionID)
		{
			out_connections_of_in_neuron->erase(out_connections_of_in_neuron->begin() + i);
		}
	}
}
void Brain::enableConnection(int connectionID)
{
	if (m_connections[connectionID].enabled == true)
	{
		std::cout << "enable connection error : connection already enabled";
		exit(1);
	}
	m_connections[connectionID].enabled = true;
	std::vector<int>* in_connections_of_out_neuron = &m_neurons[m_connections[connectionID].out_neuron_id].in_connections_id;
	in_connections_of_out_neuron->push_back(connectionID);
	std::vector<int>* out_connections_of_in_neuron = &m_neurons[m_connections[connectionID].in_neuron_id].out_connections_id;
	out_connections_of_in_neuron->push_back(connectionID);
}


Brain Brain::crossover(const Brain& parent1, const Brain& parent2)
{
	Brain child;
	const Brain* fittestParent;
	const Brain* otherParent;
	bool equal = false;

	child.m_nbInput = parent1.m_nbInput;
	child.m_nbOutput = parent1.m_nbOutput;

	child.m_input_id = parent1.m_input_id;
	child.m_output_id = parent1.m_output_id;

	if (parent1.fitness > parent2.fitness)
	{
		fittestParent = &parent1;
		otherParent = &parent2;
	}
	else
	{
		fittestParent = &parent2;
		otherParent = &parent1;
	}
	if (parent1.fitness == parent2.fitness)
	{
		equal = true;
	}

	if (!equal || true)
	{
		for (const std::pair<int, Connection>& fittestParentConnection : fittestParent->m_connections)
		{
			std::unordered_map<int, Connection>::const_iterator p_otherParentConnection = otherParent->m_connections.find(fittestParentConnection.first);

			if (p_otherParentConnection == otherParent->m_connections.end())
			{
				child.m_connections.insert(fittestParentConnection);

				std::unordered_map<int, Neuron>::iterator it;
				int in_neuron_id = fittestParentConnection.second.in_neuron_id;
				it = child.m_neurons.find(in_neuron_id);
				if (it == child.m_neurons.end())
				{
					const Neuron* parentNeuron = &fittestParent->m_neurons.at(in_neuron_id);
					Neuron newNeuron = *parentNeuron;
					newNeuron.in_connections_id.clear();
					newNeuron.out_connections_id.clear();
					newNeuron.out_connections_id.push_back(fittestParentConnection.second.connection_id);
					child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
					child.m_neurons.insert({ in_neuron_id, newNeuron });
				}
				else
				{
					it->second.out_connections_id.push_back(fittestParentConnection.second.connection_id);
				}

				int out_neuron_id = fittestParentConnection.second.out_neuron_id;
				it = child.m_neurons.find(out_neuron_id);
				if (it == child.m_neurons.end())
				{
					const Neuron* parentNeuron = &fittestParent->m_neurons.at(out_neuron_id);
					Neuron newNeuron = *parentNeuron;
					newNeuron.in_connections_id.clear();
					newNeuron.out_connections_id.clear();
					newNeuron.in_connections_id.push_back(fittestParentConnection.second.connection_id);
					child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
					child.m_neurons.insert({ out_neuron_id, newNeuron });
				}
				else
				{
					it->second.in_connections_id.push_back(fittestParentConnection.second.connection_id);
				}
			}
			else
			{
				if (proba(0.5))
				{
					child.m_connections.insert(fittestParentConnection);
					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = fittestParentConnection.second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &fittestParent->m_neurons.at(in_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron });
					}
					else
					{
						it->second.out_connections_id.push_back(fittestParentConnection.second.connection_id);
					}

					int out_neuron_id = fittestParentConnection.second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &fittestParent->m_neurons.at(out_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(fittestParentConnection.second.connection_id);
					}
				}
				else
				{
					child.m_connections.insert(*p_otherParentConnection);
					child.m_connections[p_otherParentConnection->first].enabled = fittestParentConnection.second.enabled;

					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = p_otherParentConnection->second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &otherParent->m_neurons.at(in_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(p_otherParentConnection->second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron });
					}
					else
					{
						it->second.out_connections_id.push_back(p_otherParentConnection->second.connection_id);
					}

					int out_neuron_id = p_otherParentConnection->second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &otherParent->m_neurons.at(out_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(p_otherParentConnection->second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(p_otherParentConnection->second.connection_id);
					}
				}
			}
		}
	}
	else
	{
		for (const std::pair<int, Connection>& fittestParentConnection : fittestParent->m_connections)
		{
			std::unordered_map<int, Connection>::const_iterator p_otherParentConnection = otherParent->m_connections.find(fittestParentConnection.first);

			//si la connection existe pas chez l'autre parent
			if (p_otherParentConnection == otherParent->m_connections.end())
			{
				if (proba(0.5))
				{
					child.m_connections.insert(fittestParentConnection);

					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = fittestParentConnection.second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					//si neuron in de la connection existe pas
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = fittestParent->m_neurons.at(in_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron});
					}
					else
					{
						it->second.out_connections_id.push_back(fittestParentConnection.second.connection_id);
					}

					int out_neuron_id = fittestParentConnection.second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = fittestParent->m_neurons.at(out_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(fittestParentConnection.second.connection_id);
					}
				}
			}
			else
			{
				if (proba(0.5))
				{
					child.m_connections.insert(fittestParentConnection);

					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = fittestParentConnection.second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = fittestParent->m_neurons.at(in_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron });
					}
					else
					{
						it->second.out_connections_id.push_back(fittestParentConnection.second.connection_id);
					}

					int out_neuron_id = fittestParentConnection.second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = fittestParent->m_neurons.at(out_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(fittestParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(fittestParentConnection.second.connection_id);
					}
				}
				else
				{
					child.m_connections.insert(*p_otherParentConnection);

					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = p_otherParentConnection->second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = otherParent->m_neurons.at(in_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(p_otherParentConnection->second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron });
					}
					else
					{
						it->second.out_connections_id.push_back(p_otherParentConnection->second.connection_id);
					}

					int out_neuron_id = p_otherParentConnection->second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						Neuron newNeuron = otherParent->m_neurons.at(out_neuron_id);
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(p_otherParentConnection->second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(p_otherParentConnection->second.connection_id);
					}
				}
			}
		}
		for (const std::pair<int, Connection>& otherParentConnection : otherParent->m_connections)
		{
			std::unordered_map<int, Connection>::const_iterator p_fittestParentConnection = fittestParent->m_connections.find(otherParentConnection.first);

			if (p_fittestParentConnection == fittestParent->m_connections.end())
			{
				if (proba(0.5))
				{
					child.m_connections.insert(otherParentConnection);

					std::unordered_map<int, Neuron>::iterator it;
					int in_neuron_id = otherParentConnection.second.in_neuron_id;
					it = child.m_neurons.find(in_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &otherParent->m_neurons.at(in_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.out_connections_id.push_back(otherParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ in_neuron_id, newNeuron });
					}
					else
					{
						it->second.out_connections_id.push_back(otherParentConnection.second.connection_id);
					}

					int out_neuron_id = otherParentConnection.second.out_neuron_id;
					it = child.m_neurons.find(out_neuron_id);
					if (it == child.m_neurons.end())
					{
						const Neuron* parentNeuron = &otherParent->m_neurons.at(out_neuron_id);
						Neuron newNeuron = *parentNeuron;
						newNeuron.in_connections_id.clear();
						newNeuron.out_connections_id.clear();
						newNeuron.in_connections_id.push_back(otherParentConnection.second.connection_id);
						child.m_userOffsets[newNeuron.neuron_id] = ImVec2(0, 0);
						child.m_neurons.insert({ out_neuron_id, newNeuron });
					}
					else
					{
						it->second.in_connections_id.push_back(otherParentConnection.second.connection_id);
					}
				}
			}
		}
	}

	for (const std::pair<int, Neuron>& neuron : child.m_neurons)
	{
		if (neuron.second.type == HIDDEN)
		{
			if (neuron.second.numer_of_time_connection_overwrited + 1 > child.m_connections[neuron.second.overwrited_connection_id].numer_of_time_overwrited)
			{
				child.m_connections[neuron.second.overwrited_connection_id].numer_of_time_overwrited = neuron.second.numer_of_time_connection_overwrited + 1;
			}
		}
	}


	if (child.getSize() == 1)
	{
		exit(1);
	}

	return child;
}
float Brain::distance(const Brain& brain1, const Brain& brain2)
{
	const float c1 = 2.0f;
	const float c2 = 2.0f;
	const float c3 = 1.0f;

	size_t N = std::max(brain1.getSize(), brain2.getSize());
	//if (N<10)
	//{
	//	N = 1;
	//}

	size_t matchingGenes = 0;
	size_t disjointGenes = 0;
	size_t excessGenes = 0;
	float weightDifference = 0.0f; 

	std::unordered_map<int, Brain::Connection>::const_iterator it1 = brain1.m_connections.begin();
	std::unordered_map<int, Brain::Connection>::const_iterator it2 = brain2.m_connections.begin();

	while (it1 != brain1.m_connections.end() && it2 != brain2.m_connections.end())
	{
		if (it1->second.enabled == false)
		{
			++it1;
		}
		else if (it2->second.enabled == false)
		{
			++it2;
		}
		else if (it1->first == it2->first)
		{
			matchingGenes++;
			weightDifference += std::abs(it1->second.weight - it2->second.weight);
			++it1;
			++it2;
		}
		else if (it1->first < it2->first)
		{
			disjointGenes++;
			++it1;
		}
		else
		{
			disjointGenes++;
			++it2;
		}
	}

	while (it1 != brain1.m_connections.end())
	{
		if (it1->second.enabled == true)
		{
			excessGenes++;
		}
		++it1;
	}

	while (it2 != brain2.m_connections.end())
	{
		if (it2->second.enabled == true)
		{
			excessGenes++;
		}
		++it2;
	}

	float avgWeightDifference = (matchingGenes > 0) ? (weightDifference / matchingGenes) : 0.0f;
	float distance = (c1 * (float)excessGenes / (float)N) + (c2 * (float)disjointGenes / (float)N) + (c3 * avgWeightDifference);

	return distance;
}


std::vector<float> Brain::getOutput()
{
	std::vector<float> res(m_nbOutput);

	for (size_t i = 0; i < m_nbOutput; i++)
	{
		res[i] = m_neurons[i + m_nbInput].value;
	}
	return res;
}
int Brain::getSize() const
{
	int size = 0;
	for (const std::pair<int, Connection>& connection : m_connections)
	{
		if (connection.second.enabled)
		{
			size++;
		}
	}
	return size;
}


int Brain::getConnectionInnovationNumber(const Connection& connection)
{
	for (size_t i=0; i<global_connection_innovations_history.size(); i++)
	{
		if (global_connection_innovations_history[i].in_neuron_id == connection.in_neuron_id
			&& global_connection_innovations_history[i].out_neuron_id == connection.out_neuron_id)
		{
			return i;
		}
	}
	return -1;
}
void Brain::setConnectionInnovationNumber(Connection& connection)
{
	if (getConnectionInnovationNumber(connection) != -1)
	{
		connection.connection_id = getConnectionInnovationNumber(connection);
	}
	else
	{
		if (global_connection_innovations_history.size() != 0)
		{
			global_connection_innovation_number++;
		}
		connection.connection_id = global_connection_innovation_number;
		global_connection_innovations_history.push_back(connection);
	}
}

int Brain::getNeuronInnovationNumber(const Neuron& neuron)
{
	for (size_t i = 0; i < global_neuron_innovations_history.size(); i++)
	{
		if (global_neuron_innovations_history[i].overwrited_connection_id == neuron.overwrited_connection_id
			&& global_neuron_innovations_history[i].numer_of_time_connection_overwrited == neuron.numer_of_time_connection_overwrited
			&& neuron.type!=Type::INPUT && neuron.type != Type::OUTPUT)
		{
			return i;
		}
	}
	return -1;
}
void Brain::setNeuronInnovationNumber(Neuron& neuron)
{
	if (getNeuronInnovationNumber(neuron) != -1)
	{
		neuron.neuron_id = getNeuronInnovationNumber(neuron);
	}
	else
	{
		if (global_neuron_innovations_history.size() != 0)
		{
			global_neuron_innovation_number++;
		}
		neuron.neuron_id = global_neuron_innovation_number;
		global_neuron_innovations_history.push_back(neuron);
	}
}


void Brain::mutateNode(float probability)
{
	if (proba(probability))
	{
		bool success = false;
		std::vector<int> enabled_connections_ids;
		enabled_connections_ids.reserve(m_connections.size());

		for (const std::pair<int, Connection>& connection : m_connections)
		{
			if (connection.second.enabled == true)
			{
				enabled_connections_ids.push_back(connection.first);
			}
		}

		if (enabled_connections_ids.size() > 0)
		{
			addNode(enabled_connections_ids[genRandomInt(0, enabled_connections_ids.size() - 1)]);
		}
	}
}
void Brain::mutateConnection(float probability)
{
	if (proba(probability))
	{
		std::vector<int> keys_in;
		keys_in.reserve(m_neurons.size());
		for (const auto& pair : m_neurons) {
			if (pair.second.type != OUTPUT)
			{
				keys_in.push_back(pair.first);
			}
		}

		std::vector<int> keys_out_original;
		keys_out_original.reserve(m_neurons.size());
		for (const auto& pair : m_neurons) {
			if (pair.second.type != INPUT)
			{
				keys_out_original.push_back(pair.first);
			}
		}


		while (keys_in.size() != 0)
		{
			int inIndex = genRandomInt(0, keys_in.size() - 1);
			int in_id = keys_in[inIndex];

			std::vector<int> keys_out = keys_out_original;
			bool success = false;
			while (keys_out.size()!=0)
			{
				success = true;
				int outIndex = genRandomInt(0, keys_out.size() - 1);
				int out_id = keys_out[outIndex];
				if (in_id == out_id || isRecurrent(in_id, out_id) || m_neurons[out_id].type == INPUT)
				{
					success=false;
				}
				else
				{
					for (const std::pair<int, Connection>& connection : m_connections)
					{
						if (connection.second.in_neuron_id == in_id && connection.second.out_neuron_id==out_id)
						{
							success = false;
							break;
						}
					}
				}

				if (success == true)
				{
					addConnection(in_id, out_id, genRandomFloat(-1, 1));
					return;
				}
				else
				{
					keys_out.erase(keys_out.begin() + outIndex);
				}
			}
			keys_in.erase(keys_in.begin() + inIndex);
		}
		//std::cout << "can't add more connection\n";
	}
}
void Brain::mutateEnableConnection(float probability)
{
	if (proba(probability))
	{
		std::vector<int> disabled_connections_ids;
		disabled_connections_ids.reserve(m_connections.size());

		for (const std::pair<int, Connection>& connection : m_connections)
		{
			if (connection.second.enabled == false)
			{
				disabled_connections_ids.push_back(connection.first);
			}
		}
		if (disabled_connections_ids.size()>0)
		{
			enableConnection(disabled_connections_ids[genRandomInt(0, disabled_connections_ids.size() - 1)]);
		}
	}
}
void Brain::mutateDisableConnection(float probability)
{
	if (proba(probability))
	{
		std::vector<int> enabled_connections_ids;
		enabled_connections_ids.reserve(m_connections.size());

		for (const std::pair<int, Connection>& connection : m_connections)
		{
			if (connection.second.enabled == true)
			{
				enabled_connections_ids.push_back(connection.first);
			}
		}
		if (enabled_connections_ids.size() > m_nbInput*m_nbOutput)
		{
			disableConnection(enabled_connections_ids[genRandomInt(0, enabled_connections_ids.size() - 1)]);
		}
	}
}

void Brain::mutateWeight(float probability, float power, float max, float probaReset)
{
	for (auto& pair : m_connections)
	{
		if (proba(probability))
		{
			if (proba(probaReset))
			{
				pair.second.weight = genRandomFloat(-1, 1);
			}
			else
			{
				pair.second.weight += genRandomFloat(-power, power);
				if (pair.second.weight > max)
				{
					pair.second.weight = max;
				}
				else if (pair.second.weight < -max)
				{
					pair.second.weight = -max;
				}
			}
		}
	}
}
void Brain::mutateBias(float probability, float power)
{
	for (auto& pair : m_neurons)
	{
		if (proba(probability))
		{
			pair.second.bias += genRandomFloat(-power, power);
			if (pair.second.bias > 8)
			{
				pair.second.bias = 8;
			}
			else if (pair.second.bias < -8)
			{
				pair.second.bias = -8;
			}
		}
	}
}


void Brain::printAsync()
{
	GlHelper::add(&Brain::drawBrain, this, "drawBrain");
}
void Brain::print()
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);

	float ratio = glfwGetVideoMode(glfwGetPrimaryMonitor())->width / 1920.f;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(("brain phenotype##" + std::to_string(reinterpret_cast<uintptr_t>(this))).c_str());
	ImGui::Text("hold right click on a neuron to drag it");
	if (m_firstPrint)
	{
		ImGui::SetWindowSize(ImVec2(800 * ratio, 400 * ratio));
		m_firstPrint = false;
	}
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImVec2 center = { topLeft.x + ImGui::GetWindowWidth() / 2.f , topLeft.y + (ImGui::GetWindowHeight() + ImGui::GetTextLineHeightWithSpacing()) / 2.f };


	std::vector<int> neuron_ordered_ids;
	neuron_ordered_ids.reserve(m_neurons.size());
	for (std::pair<const int, Neuron>& it : m_neurons) {
		if (it.second.type == HIDDEN)
		{
			neuron_ordered_ids.push_back(it.first);
		}
	}
	//insertionSort(neuron_ordered_ids);
	std::sort(neuron_ordered_ids.begin(), neuron_ordered_ids.end());


	//int maxDepth = 1;
	int maxDepth = findMaxDepth(m_output_id[0], 0);
	int gapX = 100;
	int gapY = 80;
	int radius = 15;
	int x = center.x;
	int y = center.y;

	float xLeft = center.x - maxDepth * gapX / 2.f;
	float xRight = center.x + maxDepth * gapX / 2.f;

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		for (std::unordered_map<int, Neuron>::iterator it = m_neurons.begin(); it != m_neurons.end(); it++)
		{
			float dx = ImGui::GetMousePos().x - (it->second.pos.x + m_userOffsets[it->first].x);
			float dy = ImGui::GetMousePos().y - (it->second.pos.y + m_userOffsets[it->first].y);
			float distance = sqrt(dx * dx + dy * dy);
			if (distance <= radius && (m_idDragged == it->first || m_idDragged == -1))
			{
				m_idDragged = it->first;
				m_userOffsets[it->first].x = ImGui::GetMousePos().x - it->second.pos.x;
				m_userOffsets[it->first].y = ImGui::GetMousePos().y - it->second.pos.y;
			}
			else if (m_idDragged == it->first)
			{
				m_idDragged = -1;
			}
		}
	}

	for (int i = 0; i < m_nbInput; i++) {
		m_neurons[m_input_id[i]].pos = { xLeft, center.y - gapY * (m_nbInput / 2.f - i) };

		draw_list->AddCircle(
			{ m_neurons[m_input_id[i]].pos.x + m_userOffsets.at(m_input_id[i]).x ,
			m_neurons[m_input_id[i]].pos.y + m_userOffsets.at(m_input_id[i]).y },
			radius, IM_COL32(220, 220, 220, 255), 32, 3);

		draw_list->AddText(
			{ m_neurons[m_input_id[i]].pos.x - radius / 4.f + m_userOffsets.at(m_input_id[i]).x,
			m_neurons[m_input_id[i]].pos.y - radius / 2.5f + m_userOffsets.at(m_input_id[i]).y },
			IM_COL32(220, 220, 220, 255), std::to_string(m_input_id[i]).data());
	}
	for (int i = 0; i < m_nbOutput; i++) {
		m_neurons[m_output_id[i]].pos = { xRight, center.y - gapY * (m_nbOutput / 2.f - i) };

		draw_list->AddCircle(
			{ m_neurons[m_output_id[i]].pos.x + m_userOffsets.at(m_output_id[i]).x ,
			m_neurons[m_output_id[i]].pos.y + m_userOffsets.at(m_output_id[i]).y },
			radius, IM_COL32(220, 220, 220, 255), 32, 3);

		draw_list->AddText(
			{ m_neurons[m_output_id[i]].pos.x - radius / 4.f + m_userOffsets.at(m_output_id[i]).x,
			m_neurons[m_output_id[i]].pos.y - radius / 2.5f + m_userOffsets.at(m_output_id[i]).y },
			IM_COL32(220, 220, 220, 255), std::to_string(m_output_id[i]).data());
	}

	for (int id : neuron_ordered_ids)
	{
		if (m_neurons[id].type == HIDDEN)
		{

			ImVec2* pos1 = &m_neurons[m_connections[m_neurons[id].overwrited_connection_id].in_neuron_id].pos;
			ImVec2* pos2 = &m_neurons[m_connections[m_neurons[id].overwrited_connection_id].out_neuron_id].pos;
			float offset = findMaxDepth(id, 0) - findMaxReverseDepth(id, 0);
			//float offset = 1;
			if (offset >= 0)
				m_neurons[id].pos.x = xLeft + ((abs(offset) + 1) / (abs(offset) + 2) * (xRight - xLeft));
			else
				m_neurons[id].pos.x = xLeft + (1 / (abs(offset) + 2) * (xRight - xLeft));

			if (pos1->y < pos2->y)
				m_neurons[id].pos.y = pos1->y + ((abs(offset) + 1) / (abs(offset) + 2) * (pos2->y - pos1->y));
			else
				m_neurons[id].pos.y = pos1->y + (1 / (abs(offset) + 2) * (pos2->y - pos1->y));

			draw_list->AddCircle(
				{ m_neurons[id].pos.x + m_userOffsets.at(id).x ,
				m_neurons[id].pos.y + m_userOffsets.at(id).y },
				radius, IM_COL32(220, 220, 220, 255), 32, 3);

			draw_list->AddText(
				{ m_neurons[id].pos.x - radius / 4.f + m_userOffsets.at(id).x,
				m_neurons[id].pos.y - radius / 2.5f + m_userOffsets.at(id).y },
				IM_COL32(220, 220, 220, 255), std::to_string(id).data());
		}
	}

	for (std::pair<const int, Connection>& it : m_connections)
	{
		if (it.second.enabled)
		{
			ImVec2 start = m_neurons[it.second.in_neuron_id].pos;
			start.x += m_userOffsets.at(it.second.in_neuron_id).x;
			start.y += m_userOffsets.at(it.second.in_neuron_id).y;
			ImVec2 end = m_neurons[it.second.out_neuron_id].pos;
			end.x += m_userOffsets.at(it.second.out_neuron_id).x;
			end.y += m_userOffsets.at(it.second.out_neuron_id).y;

			float adj = end.x - start.x;
			float opp = end.y - start.y;

			if (it.second.recurrent)
			{
				start.x += (radius - 1.0) * cos(atan2(opp, adj) + 0.2);
				start.y += (radius - 1.0) * sin(atan2(opp, adj) + 0.2);
				end.x -= (radius - 1.0) * cos(atan2(opp, adj) - 0.2);
				end.y -= (radius - 1.0) * sin(atan2(opp, adj) - 0.2);
			}
			else
			{
				start.x += (radius - 1.0) * adj / sqrt(adj * adj + opp * opp);
				start.y += (radius - 1.0) * opp / sqrt(adj * adj + opp * opp);
				end.x -= (radius - 1.0) * adj / sqrt(adj * adj + opp * opp);
				end.y -= (radius - 1.0) * opp / sqrt(adj * adj + opp * opp);
			}


			int r = 0;
			int g = 0;
			int b = 0;
			//int a = abs(it.second.weight) * 255;
			int a = 255;
			int thickness = abs(it.second.weight) * 1.5 + 0.75;
			if (it.second.weight > 0)
			{
				r = 255;
			}
			else
			{
				b = 255;
			}


			if (it.second.recurrent)
			{
				draw_list->AddLine(start, end, IM_COL32(r, g, b, a), thickness);
				draw_list->AddCircleFilled(end, 3.5f, IM_COL32(r, g, b, a));
			}
			else
			{
				draw_list->AddLine(start, end, IM_COL32(r, g, b, a), thickness);
				draw_list->AddCircleFilled(end, 3.5f, IM_COL32(r, g, b, a));
			}
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}
void Brain::stopPrinting()
{
	GlHelper::remove("drawBrain", this);
}
void Brain::drawBrain(bool* p_open)
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);

	float ratio = glfwGetVideoMode(glfwGetPrimaryMonitor())->width / 1920.f;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(("brain phenotype##" + std::to_string(reinterpret_cast<uintptr_t>(this))).c_str(), p_open);
	ImGui::Text("hold right click on a neuron to drag it");
	if (m_firstPrint)
	{
		ImGui::SetWindowSize(ImVec2(800 * ratio, 400 * ratio));
		m_firstPrint = false;
	}
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImVec2 center = { topLeft.x + ImGui::GetWindowWidth() / 2.f , topLeft.y + (ImGui::GetWindowHeight() + ImGui::GetTextLineHeightWithSpacing()) / 2.f };


	std::vector<int> neuron_ordered_ids;
	neuron_ordered_ids.reserve(m_neurons.size());
	for (std::pair<const int, Neuron>& it : m_neurons) {
		if (it.second.type == HIDDEN)
		{
			neuron_ordered_ids.push_back(it.first);
		}
	}
	//insertionSort(neuron_ordered_ids);
	std::sort(neuron_ordered_ids.begin(), neuron_ordered_ids.end());


	//int maxDepth = 1;
	int maxDepth = findMaxDepth(m_output_id[0], 0);
	int gapX = 100;
	int gapY = 80;
	int radius = 15;
	int x = center.x;
	int y = center.y;

	float xLeft = center.x - maxDepth * gapX / 2.f;
	float xRight = center.x + maxDepth * gapX / 2.f;

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		for (std::unordered_map<int, Neuron>::iterator it = m_neurons.begin(); it != m_neurons.end(); it++)
		{
			float dx = ImGui::GetMousePos().x - (it->second.pos.x + m_userOffsets[it->first].x);
			float dy = ImGui::GetMousePos().y - (it->second.pos.y + m_userOffsets[it->first].y);
			float distance = sqrt(dx * dx + dy * dy);
			if (distance <= radius && (m_idDragged == it->first || m_idDragged == -1))
			{
				m_idDragged = it->first;
				m_userOffsets[it->first].x = ImGui::GetMousePos().x - it->second.pos.x;
				m_userOffsets[it->first].y = ImGui::GetMousePos().y - it->second.pos.y;
			}
			else if (m_idDragged == it->first)
			{
				m_idDragged = -1;
			}
		}
	}

	for (int i = 0; i < m_nbInput; i++) {
		m_neurons[m_input_id[i]].pos = { xLeft, center.y - gapY * (m_nbInput / 2.f - i) };

		draw_list->AddCircle(
			{ m_neurons[m_input_id[i]].pos.x + m_userOffsets.at(m_input_id[i]).x ,
			m_neurons[m_input_id[i]].pos.y + m_userOffsets.at(m_input_id[i]).y },
			radius, IM_COL32(220, 220, 220, 255), 32, 3);

		draw_list->AddText(
			{ m_neurons[m_input_id[i]].pos.x - radius / 4.f + m_userOffsets.at(m_input_id[i]).x,
			m_neurons[m_input_id[i]].pos.y - radius / 2.5f + m_userOffsets.at(m_input_id[i]).y },
			IM_COL32(220, 220, 220, 255), std::to_string(m_input_id[i]).data());
	}
	for (int i = 0; i < m_nbOutput; i++) {
		m_neurons[m_output_id[i]].pos = { xRight, center.y - gapY * (m_nbOutput / 2.f - i) };

		draw_list->AddCircle(
			{ m_neurons[m_output_id[i]].pos.x + m_userOffsets.at(m_output_id[i]).x ,
			m_neurons[m_output_id[i]].pos.y + m_userOffsets.at(m_output_id[i]).y },
			radius, IM_COL32(220, 220, 220, 255), 32, 3);

		draw_list->AddText(
			{ m_neurons[m_output_id[i]].pos.x - radius / 4.f + m_userOffsets.at(m_output_id[i]).x,
			m_neurons[m_output_id[i]].pos.y - radius / 2.5f + m_userOffsets.at(m_output_id[i]).y },
			IM_COL32(220, 220, 220, 255), std::to_string(m_output_id[i]).data());
	}

	for (int id: neuron_ordered_ids)
	{
		if (m_neurons[id].type == HIDDEN)
		{
			
			ImVec2* pos1 = &m_neurons[m_connections[m_neurons[id].overwrited_connection_id].in_neuron_id].pos;
			ImVec2* pos2 = &m_neurons[m_connections[m_neurons[id].overwrited_connection_id].out_neuron_id].pos;
			float offset = findMaxDepth(id, 0) - findMaxReverseDepth(id, 0);
			//float offset = 1;
			if (offset >= 0)
				m_neurons[id].pos.x = xLeft + ((abs(offset) + 1) / (abs(offset) + 2) * (xRight - xLeft)) ;
			else
				m_neurons[id].pos.x = xLeft + (1 / (abs(offset) + 2) * (xRight - xLeft));

			if (pos1->y < pos2->y)
				m_neurons[id].pos.y = pos1->y + ((abs(offset) + 1) / (abs(offset) + 2) * (pos2->y - pos1->y));
			else
				m_neurons[id].pos.y = pos1->y + (1 / (abs(offset) + 2) * (pos2->y - pos1->y));

			draw_list->AddCircle(
				{ m_neurons[id].pos.x + m_userOffsets.at(id).x ,
				m_neurons[id].pos.y + m_userOffsets.at(id).y },
				radius, IM_COL32(220, 220, 220, 255), 32, 3);

			draw_list->AddText(
				{ m_neurons[id].pos.x - radius / 4.f + m_userOffsets.at(id).x,
				m_neurons[id].pos.y - radius / 2.5f + m_userOffsets.at(id).y },
				IM_COL32(220, 220, 220, 255), std::to_string(id).data());
		}
	}

	for (std::pair<const int, Connection>& it : m_connections)
	{
		if (it.second.enabled)
		{
			ImVec2 start = m_neurons[it.second.in_neuron_id].pos;
			start.x += m_userOffsets.at(it.second.in_neuron_id).x;
			start.y += m_userOffsets.at(it.second.in_neuron_id).y;
			ImVec2 end = m_neurons[it.second.out_neuron_id].pos;
			end.x += m_userOffsets.at(it.second.out_neuron_id).x;
			end.y += m_userOffsets.at(it.second.out_neuron_id).y;

			float adj = end.x - start.x;
			float opp = end.y - start.y;

			if (it.second.recurrent)
			{
				start.x += (radius - 1.0) * cos(atan2(opp, adj) + 0.2);
				start.y += (radius - 1.0) * sin(atan2(opp, adj) + 0.2);
				end.x -= (radius - 1.0) * cos(atan2(opp, adj) - 0.2);
				end.y -= (radius - 1.0) * sin(atan2(opp, adj) - 0.2);
			}
			else
			{
				start.x += (radius - 1.0) * adj / sqrt(adj * adj + opp * opp);
				start.y += (radius - 1.0) * opp / sqrt(adj * adj + opp * opp);
				end.x -= (radius - 1.0) * adj / sqrt(adj * adj + opp * opp);
				end.y -= (radius - 1.0) * opp / sqrt(adj * adj + opp * opp);
			}


			int r = 0;
			int g = 0;
			int b = 0;
			//int a = abs(it.second.weight) * 255;
			int a = 255;
			int thickness = abs(it.second.weight)*1.5 + 0.75;
			if (it.second.weight > 0)
			{
				r = 255;
			}
			else
			{
				b = 255;
			}


			if (it.second.recurrent)
			{
				draw_list->AddLine(start, end, IM_COL32(r, g, b, a), thickness);
				draw_list->AddCircleFilled(end, 3.5f, IM_COL32(r, g, b, a));
			}
			else
			{
				draw_list->AddLine(start, end, IM_COL32(r, g, b, a), thickness);
				draw_list->AddCircleFilled(end, 3.5f, IM_COL32(r, g, b, a));
			}
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

int Brain::findMaxDepth(int baseId, int n, bool first)
{
	Neuron* neuron = &m_neurons[baseId];
	std::vector<int> depths;
	for (int i = 0; i < neuron->in_connections_id.size(); i++)
	{
		if (m_connections.at(neuron->in_connections_id[i]).recurrent==false)
		{
			depths.push_back(findMaxDepth(m_connections[neuron->in_connections_id[i]].in_neuron_id, n + 1, false));
		}
	}
	if (depths.size() == 0)
		return n;
	else
		return vec1dMax(depths);
}
int Brain::findMaxReverseDepth(int baseId, int n, bool first)
{
	Neuron* neuron = &m_neurons[baseId];
	std::vector<int> depths;
	for (int i = 0; i < neuron->out_connections_id.size(); i++)
	{
		if (m_connections.at(neuron->out_connections_id[i]).recurrent == false)
		{
			depths.push_back(findMaxReverseDepth(m_connections.at(neuron->out_connections_id[i]).out_neuron_id, n + 1, false));
		}
	}
	if (depths.size() == 0)
		return n;
	else
		return vec1dMax(depths);
}

bool Brain::isRecurrent(int in, int out, int n)
{
	Neuron* neuron = &m_neurons[in];

	if (n > 1000)
	{
		std::cout << "\nIS RECCURENT ERROR n=" << n << "\n";
		//exit(1);
	}


	for (int i = 0; i < neuron->in_connections_id.size(); i++)
	{
		if (m_connections[neuron->in_connections_id[i]].in_neuron_id == out)
		{
			return true;
		}
	}

	for (int i = 0; i < neuron->in_connections_id.size(); i++)
	{
		if (isRecurrent(m_connections[neuron->in_connections_id[i]].in_neuron_id, out, n + 1))
		{
			return true;
		}
	}

	return false;
}



void Brain::writeToFile(const std::string& filename)
{
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cout << "Error: Couldn't open file for writing.\n";
		exit(1);
	}

	outFile << "m_nbInput: " << m_nbInput << "\n";
	outFile << "m_nbOutput: " << m_nbOutput << "\n";

	outFile << "m_neurons_size: " << m_neurons.size() << "\n";
	outFile << "m_connections_size: " << m_connections.size() << "\n";

	outFile << "input id: ";
	for (int id : m_input_id)
	{
		outFile << id << " ";
	}
	outFile << "\n";

	outFile << "output id: ";
	for (int id : m_output_id)
	{
		outFile << id << " ";
	}
	outFile << "\n";

	for (const auto& pair : m_neurons) {
		writeNeuronToFile(outFile, pair.second);
	}

	for (const auto& pair : m_connections) {
		writeConnectionToFile(outFile, pair.second);
	}

	outFile << "m_userOffsets: " << "\n";
	for (const auto& pair : m_userOffsets) {
		outFile << pair.first << ": " << pair.second.x << ", " << pair.second.y << "\n";
	}

	outFile.close();
}
Brain Brain::readFromFile(const std::string& filename) {
	Brain brain;
	std::ifstream inFile(filename);
	std::ifstream inFile2(filename);
	if (!inFile.is_open() || !inFile2.is_open()) {
		std::cout << "Error: Couldn't open file for reading.\n";
		exit(1);
	}

	std::string line;
	while (std::getline(inFile, line)) {
		size_t colonPos = line.find(':');
		std::string field = line.substr(0, colonPos);
		if (field == "m_nbInput") {
			brain.m_nbInput = std::stoi(line.substr(colonPos+2));
		}
		else if (line.find("m_nbOutput") == 0) {
			brain.m_nbOutput = std::stoi(line.substr(colonPos+2));
		}
		else if (field == "input id") {
			std::istringstream iss(line.substr(colonPos + 2));
			int id;
			while (iss >> id) {
				brain.m_input_id.push_back(id);
			}
		}
		else if (field == "output id") {
			std::istringstream iss(line.substr(colonPos + 2));
			int id;
			while (iss >> id) {
				brain.m_output_id.push_back(id);
			}
		}
		else if (field == "m_neurons_size") {
			size_t numNeurons = std::stoi(line.substr(colonPos + 2));

			for (size_t i = 0; i < numNeurons; ++i) {
				Neuron neuron = readNeuronFromFile(inFile2);
				brain.m_neurons[neuron.neuron_id] = neuron;
			}
		}
		else if (field == "m_connections_size") {
			size_t numConnections = std::stoi(line.substr(colonPos + 2));
			for (size_t i = 0; i < numConnections; ++i) {
				Connection connection = readConnectionFromFile(inFile2);
				brain.m_connections[connection.connection_id] = connection;
			}
		}
		else if (field == "m_userOffsets") {
			while (std::getline(inFile, line)) {
				if (line.empty()) break;
				size_t colonPos = line.find(':');
				int key = std::stoi(line.substr(0, colonPos));
				size_t commaPos = line.find(',', colonPos);
				float x = std::stof(line.substr(colonPos + 2, commaPos - colonPos - 2));
				float y = std::stof(line.substr(commaPos + 2));
				brain.m_userOffsets[key] = ImVec2(x, y);
			}
		}
	}
	inFile.close();

	return brain;
}

void Brain::writeNeuronToFile(std::ofstream& outFile, const Neuron& neuron)
{
	outFile << "Neuron: " << "\n";
	outFile << "neuron_id: " << neuron.neuron_id << "\n";
	outFile << "type: " << neuron.type << "\n";
	outFile << "overwrited_connection_id: " << neuron.overwrited_connection_id << "\n";
	outFile << "numer_of_time_connection_overwrited: " << neuron.numer_of_time_connection_overwrited << "\n";
	outFile << "in_connections_id: ";
	for (const auto& id : neuron.in_connections_id) {
		outFile << id << " ";
	}
	outFile << "\n";
	outFile << "out_connections_id: ";
	for (const auto& id : neuron.out_connections_id) {
		outFile << id << " ";
	}
	outFile << "\n";
	outFile << "value: " << neuron.value << "\n";
	outFile << "bias: " << neuron.bias << "\n";
	outFile << "has_value: " << neuron.has_value << "\n";
	outFile << "has_asked: " << neuron.has_asked << "\n";
	outFile << "pos: " << neuron.pos.x << ", " << neuron.pos.y << "\n";
}
Brain::Neuron Brain::readNeuronFromFile(std::ifstream& inFile)
{
	Neuron neuron;
	std::string line;
	while (std::getline(inFile, line)) {
		if (line.empty()) break;
		size_t colonPos = line.find(':');
		std::string field = line.substr(0, colonPos);
		if (field == "neuron_id") {
			neuron.neuron_id = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "type") {
			int type = std::stoi(line.substr(colonPos + 2));
			if (type == 0)
				neuron.type = INPUT;
			else if (type == 1)
				neuron.type = OUTPUT;
			else if (type == 2)
				neuron.type = HIDDEN;
			else
			{
				std::cout << "\nerror while reading neuron type\n";
				exit(1);
			}
		}
		else if (field == "overwrited_connection_id") {
			neuron.overwrited_connection_id = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "numer_of_time_connection_overwrited") {
			neuron.numer_of_time_connection_overwrited = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "in_connections_id") {
			std::istringstream iss(line.substr(colonPos + 2));
			int id;
			while (iss >> id) {
				neuron.in_connections_id.push_back(id);
			}
		}
		else if (field == "out_connections_id") {
			std::istringstream iss(line.substr(colonPos + 2));
			int id;
			while (iss >> id) {
				neuron.out_connections_id.push_back(id);
			}
		}
		else if (field == "value") {
			neuron.value = std::stof(line.substr(colonPos + 2));
		}
		else if (field == "bias") {
			neuron.bias = std::stof(line.substr(colonPos + 2));
		}
		else if (field == "has_value") {
			neuron.has_value = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "has_asked") {
			neuron.has_asked = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "pos") {
			size_t commaPos = line.find(',', colonPos);
			neuron.pos.x = std::stof(line.substr(colonPos + 2, commaPos - colonPos - 2));
			neuron.pos.y = std::stof(line.substr(commaPos + 2));
			break;
		}
	}
	return neuron;
}

void Brain::writeConnectionToFile(std::ofstream& outFile, const Connection& connection)
{
	outFile << "Connection: " << "\n";
	outFile << "connection_id: " << connection.connection_id << "\n";
	outFile << "in_neuron_id: " << connection.in_neuron_id << "\n";
	outFile << "out_neuron_id: " << connection.out_neuron_id << "\n";
	outFile << "weight: " << connection.weight << "\n";
	outFile << "enabled: " << connection.enabled << "\n";
	outFile << "recurrent: " << connection.recurrent << "\n";
	outFile << "numer_of_time_overwrited: " << connection.numer_of_time_overwrited << "\n";
}
Brain::Connection Brain::readConnectionFromFile(std::ifstream& inFile) {
	Connection connection;
	std::string line;
	while (std::getline(inFile, line)) {
		if (line.empty()) break;
		size_t colonPos = line.find(':');
		std::string field = line.substr(0, colonPos);
		if (field == "connection_id") {
			connection.connection_id = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "in_neuron_id") {
			connection.in_neuron_id = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "out_neuron_id") {
			connection.out_neuron_id = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "weight") {
			connection.weight = std::stof(line.substr(colonPos + 2));
		}
		else if (field == "enabled") {
			connection.enabled = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "recurrent") {
			connection.recurrent = std::stoi(line.substr(colonPos + 2));
		}
		else if (field == "numer_of_time_overwrited") {
			connection.numer_of_time_overwrited = std::stoi(line.substr(colonPos + 2));
			break;
		}
	}
	return connection;
}