#pragma once

#include <vector>

#include "matrix.h"
#include "jeu.h"
#include "bot.h"

namespace SimpleGA
{
    class Population
    {
    public:
        Population(int size);


        void runGen();
        void generateNewGen();
        void generateNewGen1();

        void play();

        void train();

        inline bool isEveryoneDead()
        {
            return m_everyoneDead;
        }
        inline int getGeneration()
        {
            return m_generation;
        }

        int getNumberAlive();

        void printGrid(int nbOfBotPerRow);
        void printOutput(int nbOfBotPerRow);


        void getBestChild(Bot& child);



        //per generation
        std::vector<std::vector<int>> m_generationPerBotScores;
        std::vector<int> m_generationPerBotMeanScores;

        int m_generationMeanScores;
        int m_generationBestMeanScore;
        int m_generationBestScore;

        //all times
        std::vector<int> m_allMeanScores;
        std::vector<int> m_allBestMeanScores;
        std::vector<int> m_allBestScores;

        int m_allTimeBestMeanScore;
        int m_allTimeBestScore;


    private:
        int m_size;
        std::vector<Bot> m_bots;
        int m_gameNumber = 20;

        bool m_everyoneDead;
        int m_generation;

        void updateScore();
        void plotting(bool* p_open);
        void drawNetwork(bool* p_open);

        void getTwoBestChild(Bot& child1, Bot& child2);
        Bot crossParents(Bot& parrent1, Bot& parrent2, Bot& child);
    };
}
