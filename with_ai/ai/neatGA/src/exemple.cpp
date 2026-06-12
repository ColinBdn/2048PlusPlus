#include <cmath>
#include "../include/exemple.h"
#include "../../neatGA/include/brain.h"
#include "../../singlePoleBalancing/singlePoleBalancing.h"
#include "../../singlePoleBalancing/keyboard.h"

template<typename T>
float vecMean(const std::vector<T>& vec)
{
    double mean = 0;
    int size = vec.size();
    for (int i = 0; i < size; ++i)
        mean += vec[i];
    return mean / size;
}
template<typename T>
T vec1dMax(const std::vector<T>& vec)
{
    T max = 0;
    for (T ele : vec)
    {
        if (ele > max) max = ele;
    }
    return max;
}
template<typename T>
T vec1dMin(const std::vector<T>& vec)
{
    if (vec.size() == 0) return 0;
    T min = vec[0];
    for (T ele : vec)
    {
        if (ele < min) min = ele;
    }
    return min;
}

void xorTraining(Brain* brain)
{

    std::vector<std::vector<float>> xor_inputs = { {0, 0}, {0, 1 }, {1, 0}, {1, 1} };
    std::vector<float> xor_outputs = { 0, 1, 1, 0 };

    brain->fitness = 4;

    float tempSuccess = true;
    for (size_t i = 0; i < xor_inputs.size(); i++)
    {
        brain->forward(xor_inputs[i], 4);
        float output = brain->getOutput()[0];
        brain->fitness -= (output - xor_outputs[i]) * (output - xor_outputs[i]);

        if (output > 0.5 && xor_outputs[i] == 0)
        {
            tempSuccess = false;
        }
        else if (output <= 0.5 && xor_outputs[i] == 1)
        {
            tempSuccess = false;
        }
    }
    brain->runSucessfull = tempSuccess;
}
void xorValidate(Brain* brain)
{
    std::vector<std::vector<float>> xor_inputs = { {0, 0}, {0, 1 }, {1, 0}, {1, 1} };
    std::vector<float> xor_outputs = { 0, 1, 1, 0 };

    std::cout << "\n---------- VALIDATION STAGE ----------\n";
    brain->fitness = 4;
    for (size_t i = 0; i < xor_inputs.size(); i++)
    {
        brain->forward(xor_inputs[i], 4);
        float output = brain->getOutput()[0];
        float outputClamped = 0;
        if (output > 0.5)
        {
            outputClamped = 1;
        }
        std::cout << "\ninput : " << xor_inputs[i][0] << ", " << xor_inputs[i][1] << " expected output : " << xor_outputs[i] << "-----------\n";
        std::cout << "brain output : " << outputClamped << " (exact output : " << output << ")" << "\n";
    }
}

void spbTraining(Brain* brain)
{
    Spb spb;
    std::vector<float> inputs(4);
    spb.reset();
    spb.m_pole.angle = 3.14159265358979323846;

    bool finished = false;
    bool isHorizontal = false;

    while (!finished)
    {
        spb.m_frame++;

        inputs[0] = spb.m_cart.pos.x;
        inputs[1] = spb.m_cart.velocity.x;
        inputs[2] = spb.m_pole.angle;
        inputs[3] = spb.m_pole.velocity;

        brain->forward(inputs);
        float output = brain->getOutput()[0];

        spb.m_cart.is_moving = false;
        if (output < 0.5)
        {
            if (spb.left(200) == 1)
            {
                finished = true;
            }
        }
        else if (output > 0.5)
        {
            if (spb.right(200) == 1)
            {
                finished = true;
            }
        }

        spb.updatePhysics();
        spb.updateScore();

        if (spb.m_score>15000)
        {
            finished = true;
        }

        if (spb.m_isVertical && (spb.m_pole.angle > (3.14 / 4) && spb.m_pole.angle < (7 * 3.14 / 4)) || spb.m_frame > 100 && !spb.m_isVertical || spb.m_frame > 10000)
        {
            finished = true;
        }
    }

    brain->fitness = spb.m_score;

    if (brain->fitness > 15000)
    {
        brain->runSucessfull = true;
    }
}
void spbValidate(Brain* brain)
{
    std::chrono::nanoseconds targetUpdateDuration = std::chrono::nanoseconds((int)round(1000000000.0 / 60.0));
    std::chrono::high_resolution_clock::time_point startTime, endTime, currentTime, nextUpdateTime;
    std::chrono::high_resolution_clock::duration updateDuration, remainingTime, elapsedTime;

    Spb spb;
    //spb.m_pole.angle = Brain::genRandomFloat(3.05, 3.25);
    spb.m_pole.angle = 3.14159265358979323846;

    spb.startDrawing();

    std::cout << "\npress enter to start validation...\n";
    while (!Keyboard::isKeyPressed(VK_RETURN))
    {
        Sleep(10);
    }

    std::cout << "\npress enter to exit validation...\n";
    bool finished = false;
    while (!finished)
    {
        if (Keyboard::isKeyPressed(VK_RETURN))
        {
            return;
        }
        startTime = std::chrono::high_resolution_clock::now();
        spb.m_frame++;

        std::vector<float> inputs = { spb.m_cart.pos.x, spb.m_cart.velocity.x, spb.m_pole.angle, spb.m_pole.velocity };
        brain->forward(inputs);
        float output = brain->getOutput()[0];

        spb.m_cart.is_moving = false;
        if (output < 0.5)
        {
            if (spb.left(200) == 1)
            {
                finished = true;
            }
        }
        else if (output > 0.5)
        {
            if (spb.right(200) == 1)
            {
                finished = true;
            }
        }

        spb.updatePhysics();


        spb.updateScore();
        brain->fitness = spb.m_score;


        do
        {
            endTime = std::chrono::high_resolution_clock::now();
            updateDuration = endTime - startTime;
            remainingTime = targetUpdateDuration - updateDuration;
        } while (remainingTime >= std::chrono::nanoseconds(0));
        if (updateDuration > targetUpdateDuration * 1.25)
        {
            std::cout << "Warning: Update took much longer than target duration.\n";
        }
        endTime = std::chrono::high_resolution_clock::now();
        spb.m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    }
    while (!Keyboard::isKeyPressed(VK_RETURN))
    {
        Sleep(10);
    }
}

