#pragma once

#include <limits>
#define NOMINMAX
#include <Windows.h>
#include <vector>
#include <iostream>
/**
 * Generates a random floating-point number within the specified range [a, b].
 *
 * @param a The lower bound of the range.
 * @param b The upper bound of the range.
 * @return A random floating-point number within the specified range.
 */
float genRandomFloat(const float& a, const float& b);

/**
 * Generates a random integer within the specified range [a, b].
 *
 * @param a The lower bound of the range.
 * @param b The upper bound of the range.
 * @return A random integer within the specified range.
 */
int genRandomInt(int a, int b);


/**
 * Calculates the mean value of a vector.
 *
 * @param vec The vector for which to calculate the mean.
 * @return The mean value of the vector.
 */
template<typename T>
float vecMean(const std::vector<T>& vec)
{
    double mean = 0;
    int size = vec.size();
    for (int i = 0; i < size; ++i)
        mean += vec[i];
    return mean / size;
}

/**
 * Finds the maximum value in a 1D vector.
 *
 * @param vec The vector for which to find the maximum value.
 * @return The maximum value in the vector.
 */
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

/**
 * Finds the maximum value in a 2D vector.
 *
 * @param vec The vector for which to find the maximum value.
 * @return The maximum value in the vector.
 */
template<typename T>

T vec2dMax(const std::vector<std::vector<T>>& vec)
{
    T max= 0;
    for (std::vector<T> row : vec)
    {
        for (T ele : row)
        {
            if (ele > max) max = ele;
        }
    }
    return max;
}

//----------------- console manipulation -----------------
/**
 * Retrieves information about the console window.
 *
 * @param show Whether to display the console information.
 * @return Information about the console window.
 */
CONSOLE_SCREEN_BUFFER_INFO getConsoleInfo(const bool& show=false);

/**
 * Sets the console window to full screen.
 */
void setConsoleToFullScreen();

/**
 * Activates virtual terminal processing for the console.
 */
void activateVP();

/**
 * Sets the cursor position in the console window.
 *
 * @param x The X-coordinate of the cursor position.
 * @param y The Y-coordinate of the cursor position.
 */
void setCursorPos(const int& x, const int& y);

/**
 * Sets the cursor position in the console window using COORD structure.
 *
 * @param pos The position to set the cursor to.
 */
void setCursorPos(COORD pos);

/**
 * Sets the size of the console window.
 *
 * @param width The width of the console window.
 * @param height The height of the console window.
 */
void setConsoleWindowSize(const int& width, const int& height);

/**
 * Gets the center position of the console window.
 *
 * @return The center position of the console window.
 */
COORD getConsoleCenter();

/**
 * Checks if the console window has been resized.
 *
 * @return True if the console window has been resized, otherwise false.
 */
bool isConsoleResized();

/**
 * Clears the console window.
 *
 * @param x The mode of clearing the console window.
 */
enum clearMode { RESET_CURSOR, RESET_ALL_CELL, RESET_CURSOR_AND_FILL_WITH_SPACE, SYSTEM_CLEAR };
void clearConsole(const clearMode& x);
void clearConsoleInputBuffer();

/**
 * Clears the console input buffer.
 */
void clearConsoleInputBuffer();

/**
 * Sets the console to the alternate screen buffer.
 */
void setAlternateBuffer();

/**
 * Sets the console to the main screen buffer.
 */
void setMainBuffer();

/**
 * Sets the cursor to invisible.
 */
void setCursorInvisible();

/**
 * Sets the cursor to visible.
 */
void setCursorVisible();

/**
 * Inputs a value of type T from the standard input stream.
 * Clears the input buffer before reading and resets the stream state after reading.
 *
 * @param var The variable to store the input value.
 * @return True if input operation fails, otherwise false.
 */
template<typename T> bool input(T& var)
{
    clearConsoleInputBuffer();
    std::cin >> var;
    bool fail = std::cin.fail();
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return fail;
}

/**
 * Prints the elements of a 1D vector to the standard output stream.
 *
 * @param data The vector containing the elements to print.
 */
template<typename T> void printVec1d(std::vector<T>data)
{
    for (T& ele : data)
    {
        std::cout << ele;
        if (&ele != &data.end()) std::cout << " ";
    }
}


//------------------- display function -------------------
/**
 * Calculates the visible length of a string considering escape sequences.
 *
 * @param text The input string.
 * @return The visible length of the string.
 */
int getVisibleStringLenght(const std::string& text);

/**
 * Sets the color for subsequent text output.
 *
 * @param r The red component of the color.
 * @param g The green component of the color.
 * @param b The blue component of the color.
 * @return The color escape sequence.
 */
std::string setColor(const int& r, const int& g, const int b);

/**
 * Resets the color to default.
 *
 * @return The reset color escape sequence.
 */
std::string resetColor();

/**
 * Prints text centered on the console window.
 *
 * @param text The text to print.
 * @param xoffset The X-offset for centering.
 * @param yoffset The Y-offset for centering.
 * @param root The root rectangle for alignment.
 */
void printCentered(const std::string& text, const int& xoffset = 0, const int& yoffset = 0, const SMALL_RECT& root = { -1 });

/**
 * Prints text centered horizontally on the console window.
 *
 * @param text The text to print.
 * @param xoffset The X-offset for centering.
 * @param y The Y-coordinate for printing.
 * @param root The root rectangle for alignment.
 */
void printCenteredX(const std::string& text, const int& xoffset, const int& y, const SMALL_RECT& root = { -1 });

/**
 * Prints text centered vertically on the console window.
 *
 * @param text The text to print.
 * @param x The X-coordinate for printing.
 * @param yoffset The Y-offset for centering.
 * @param root The root rectangle for alignment.
 */
void printCenteredY(const std::string& text, const int& x, const int& yoffset, const SMALL_RECT& root = { -1 });

/**
 * Prints text at a specific position on the console window.
 *
 * @param text The text to print.
 * @param x The X-coordinate for printing.
 * @param y The Y-coordinate for printing.
 * @param root The root rectangle for alignment.
 */
void printAtPos(const std::string& text, const int& x, const int& y, const SMALL_RECT& root = { -1 });

/**
 * Prints a character at a specific position on the console window.
 *
 * @param text The character to print.
 * @param x The X-coordinate for printing.
 * @param y The Y-coordinate for printing.
 * @param root The root rectangle for alignment.
 */
void printAtPos(const char& text, const int& x, const int& y, const SMALL_RECT& root = { -1 });

/**
 * Draws a rectangle on the console window.
 *
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param r The red component of the color.
 * @param g The green component of the color.
 * @param b The blue component of the color.
 * @return The rectangle area.
 */
SMALL_RECT drawRectangle(const int& width, const int& height, const int& r = 0, const int& g = 0, const int& b = 255);

/**
 * Gets the center position of a rectangle.
 *
 * @param rect The rectangle area.
 * @return The center position of the rectangle.
 */
COORD getRectCenter(const SMALL_RECT& rect);



