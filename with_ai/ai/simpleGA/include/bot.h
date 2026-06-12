#pragma once

#include <vector>

#include "matrix.h"
#include "jeu.h"
#include "model.h"

namespace SimpleGA
{
    class Bot
    {
    public:
        Bot();
        //Bot(const Bot& b);

        void reset();

        void play();

        void printGrid(int x, int y);
        void printOutput(int x, int y);

        void mutateFlip(float rate);
        void mutateReset(float rate);
        void mutateAdd(float fac);

        inline int getScore() const
        {
            return score(m_g);
        }
        int getScore1() const;

    private:
        bool m_alive;
        bool m_printDead;
        Grille m_g;
        Model m_ai;

        friend class Population;
    };
}