#define NOMINMAX
#include <Windows.h>
#include <cassert>
#include <iomanip> 
#include <sstream>
#include <future>
#include <filesystem>
#include <fstream>
#include <cmath>

#include "jeu.h"
#include "useful.h"
#include "keyboard.h"

/**
 * Counts the number of empty cells in the grid.
 *
 * @param g Reference to the grid.
 * @return The number of empty cells in the grid.
 */
int vides(const Grille& g)
{
    int res = 0;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 0; j < g.dim; j++)
        {
            if (g.table[i][j] == 0) res++;
        }
    }
    return res;
}

/**
 * Returns the proportion value of the grid.
 *
 * @param g Reference to the grid.
 * @return The proportion value of the grid.
 */
int proportion(const Grille& g){ return g.prop; }

/**
 * Returns the target value of the grid.
 *
 * @param g Reference to the grid.
 * @return The target value of the grid.
 */
int cible(const Grille& g) { return g.cible; }

/**
 * Returns the dimension of the grid.
 *
 * @param g Reference to the grid.
 * @return The dimension of the grid.
 */
int dimension(const Grille& g) { return g.dim; }

/**
 * Returns the score of the grid.
 *
 * @param g Reference to the grid.
 * @return The score of the grid.
 */
int score(const Grille& g) { return g.score; }

/**
 * Checks if the target value is present in the grid.
 *
 * @param g Reference to the grid.
 * @return True if the target value is present, otherwise false.
 */
bool succes(const Grille& g)
{ 
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 0; j < g.dim; j++)
        {
            if (g.table[i][j] == g.cible) return true;
        }
    }
    return false;
}

/**
 * Spawns a random tile at an empty cell in the grid.
 *
 * @param g Reference to the grid.
 * @return True if a tile is spawned successfully, otherwise false.
 */
int spawnRandom(Grille& g)
{
    int indexCible = place(g);
    int indexCurrent = 0;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 0; j < g.dim; j++)
        {
            if (g.table[i][j] == 0) indexCurrent++;
            if (indexCurrent == indexCible) { g.table[i][j] = nouvelle(g); return true; }
        }
    }

    return false;
}

/**
 * Initializes the grid with specified dimension, target value, and proportion.
 *
 * @param g Reference to the grid to initialize.
 * @param dimension Dimension of the grid.
 * @param cible Target value for the grid.
 * @param proportion Proportion value for the grid.
 * @return True if initialization is successful, otherwise false.
 */
bool init(Grille& g, int dimension, int cible, int proportion)
{
    if (dimension < 2 || (cible & (cible - 1)) != 0 && cible!=-1 || cible == 0 || cible == 1 || proportion<0 || proportion>10) {
        return false;
    }

    g.score = 0;
    g.dim = dimension;
    g.cible = cible;
    g.prop = proportion;
    
    for (size_t i = 0; i < dimension; i++)
    {
        g.table.push_back({});
        for (size_t j = 0; j < dimension; j++)
        {
            g.table[i].push_back(0);
        }
    }

    spawnRandom(g);
    spawnRandom(g);

    return true;
}

/**
 * Loads grid data from a 2D vector and initializes the grid with specified target value and proportion.
 *
 * @param g Reference to the grid to initialize.
 * @param v 2D vector representing the grid data.
 * @param cible Target value for the grid.
 * @param proportion Proportion value for the grid.
 * @return True if loading and initialization are successful, otherwise false.
 */
bool charge(Grille& g, std::vector<std::vector<int>>& v, int cible, int proportion) {
    int dim = v.size();

    if (dim < 2 || (cible & (cible - 1)) != 0 || cible == 0 || cible == 1 || proportion < 0 || proportion>10) {
        return false;
    }

    for (std::vector<int> ele : v)
    {
        if (ele.size() != dim) return false;
    }


    g.table = v;
    g.score = 0;
    g.dim = dim;
    g.cible = cible;
    g.prop = proportion;

    if (vides(g) == dim * dim) return false;

    return true;
}




/**
 * Moves the tiles in the grid to the left.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int gauche(Grille& g)
{
    int has_moved = -1;
    for (int i = 0; i < g.dim; i++)
    {
        int toMoveIndex = 0;
        int toMergeIndex = 0;
        bool merge = false;
        int value = 0;
        for (int j = 0; j < g.dim; j++)
        {
            if (g.table[i][j] != 0 && !merge)
            {
                toMergeIndex = j;
                merge = true;
            }
            else if (g.table[i][j] == g.table[i][toMergeIndex] && merge)
            {
                value = g.table[i][toMergeIndex] * 2;
                g.table[i][toMergeIndex] = 0;
                g.table[i][j] = 0;
                g.score += value;
                g.table[i][toMoveIndex] = value;
                merge = false;
                toMoveIndex += 1;
                has_moved = 1;
            }
            else if (g.table[i][j] != 0)
            {
                if (toMergeIndex != toMoveIndex) has_moved = 1;
                value = g.table[i][toMergeIndex];
                g.table[i][toMergeIndex] = 0;
                g.table[i][toMoveIndex] = value;
                toMergeIndex = j;
                toMoveIndex += 1;
                merge = true;
            }
            if (j==g.dim-1 && merge && toMergeIndex != toMoveIndex)
            {
                value = g.table[i][toMergeIndex];
                g.table[i][toMergeIndex] = 0;
                g.table[i][toMoveIndex] = value;
                has_moved = 1;
            }
        }
    }
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}

/**
 * Moves the tiles in the grid to the right.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int droite(Grille& g)
{
    int has_moved = -1;
    for (int i = 0; i < g.dim; i++)
    {
        int toMoveIndex = g.dim-1;
        int toMergeIndex = g.dim-1;
        bool merge = false;
        int value = 0;
        for (int j = g.dim-1; j >= 0; j--)
        {
            if (g.table[i][j] != 0 && !merge)
            {
                toMergeIndex = j;
                merge = true;
            }
            else if (g.table[i][j] == g.table[i][toMergeIndex] && merge)
            {
                value = g.table[i][toMergeIndex] * 2;
                g.table[i][toMergeIndex] = 0;
                g.table[i][j] = 0;
                g.score += value;
                g.table[i][toMoveIndex] = value;
                merge = false;
                toMoveIndex -= 1;
                has_moved = 1;
            }
            else if (g.table[i][j] != 0)
            {
                if (toMergeIndex != toMoveIndex) has_moved = 1;
                value = g.table[i][toMergeIndex];
                g.table[i][toMergeIndex] = 0;
                g.table[i][toMoveIndex] = value;
                toMergeIndex = j;
                toMoveIndex -= 1;
                merge = true;
            }
            if (j == 0 && merge && toMergeIndex != toMoveIndex)
            {
                value = g.table[i][toMergeIndex];
                g.table[i][toMergeIndex] = 0;
                g.table[i][toMoveIndex] = value;
                has_moved = 1;
            }
        }
    }
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}

/**
 * Moves the tiles in the grid upwards.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int haut(Grille& g)
{
    int has_moved = -1;
    for (int i = 0; i < g.dim; i++)
    {
        int toMoveIndex = 0;
        int toMergeIndex = 0;
        bool merge = false;
        int value = 0;
        for (int j = 0; j < g.dim; j++)
        {
            if (g.table[j][i] != 0 && !merge)
            {
                toMergeIndex = j;
                merge = true;
            }
            else if (g.table[j][i] == g.table[toMergeIndex][i] && merge)
            {
                value = g.table[toMergeIndex][i] * 2;
                g.table[toMergeIndex][i] = 0;
                g.table[j][i] = 0;
                g.score += value;
                g.table[toMoveIndex][i] = value;
                merge = false;
                toMoveIndex += 1;
                has_moved = 1;
            }
            else if (g.table[j][i] != 0)
            {
                if (toMergeIndex != toMoveIndex) has_moved = 1;
                value = g.table[toMergeIndex][i];
                g.table[toMergeIndex][i] = 0;
                g.table[toMoveIndex][i] = value;
                toMergeIndex = j;
                toMoveIndex += 1;
                merge = true;
            }
            if (j == g.dim - 1 && merge && toMergeIndex != toMoveIndex)
            {
                value = g.table[toMergeIndex][i];
                g.table[toMergeIndex][i] = 0;
                g.table[toMoveIndex][i] = value;
                has_moved = 1;
            }
        }
    }
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}

/**
 * Moves the tiles in the grid downwards.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int bas(Grille& g)
{
    int has_moved = -1;
    for (int i = 0; i < g.dim; i++)
    {
        int toMoveIndex = g.dim - 1;
        int toMergeIndex = g.dim - 1;
        bool merge = false;
        int value = 0;
        for (int j = g.dim-1; j >= 0; j--)
        {
            if (g.table[j][i] != 0 && !merge)
            {
                toMergeIndex = j;
                merge = true;
            }
            else if (g.table[j][i] == g.table[toMergeIndex][i] && merge)
            {
                value = g.table[toMergeIndex][i] * 2;
                g.table[toMergeIndex][i] = 0;
                g.table[j][i] = 0;
                g.score += value;
                g.table[toMoveIndex][i] = value;
                merge = false;
                toMoveIndex -= 1;
                has_moved = 1;
            }
            else if (g.table[j][i] != 0)
            {
                if (toMergeIndex != toMoveIndex) has_moved = 1;
                value = g.table[toMergeIndex][i];
                g.table[toMergeIndex][i] = 0;
                g.table[toMoveIndex][i] = value;
                toMergeIndex = j;
                toMoveIndex -= 1;
                merge = true;
            }
            if (j == 0 && merge && toMergeIndex != toMoveIndex)
            {
                value = g.table[toMergeIndex][i];
                g.table[toMergeIndex][i] = 0;
                g.table[toMoveIndex][i] = value;
                has_moved = 1;
            }
        }
    }
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}

//first version I made, with a complexity bigger than n^2 and lesser then n^3. The newer version has a complexity of n^2.
int gauche_first_version(Grille& g)
{
    std::vector<std::vector<int>> new_grid = g.table;
    int has_moved = -1;
    int zeroIndex=0;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 1; j < g.dim; j++)
        {
            if (g.table[i][j] != 0)
            {
                for (size_t k = 0; k < j; k++) 
                {
                    if (g.table[i][j] == g.table[i][j - k - 1])
                    {
                        new_grid[i][j - k - 1] *= 2;
                        new_grid[i][j] = 0;
                        has_moved = 1;
                        g.score += new_grid[i][j - k - 1];
                        j++;
                        break; 
                    }
                    else if (g.table[i][j - k - 1] != 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                zeroIndex = j;
            }
        }
        for (size_t j = 0; j < g.dim; j++)
        {
            if (new_grid[i][j] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (new_grid[i][k] == 0)
                    {
                        new_grid[i][k] = new_grid[i][j];
                        new_grid[i][j] = 0;
                        has_moved = 1;
                    }
                }
            }
        }
    }
    g.table = new_grid;
    if (has_moved==1) { has_moved = vides(g); }
    return has_moved;
}
int droite_first_version(Grille& g)
{
    std::vector<std::vector<int>> new_grid = g.table;
    int has_moved = -1;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 1; j < g.dim; j++)
        {
            if (g.table[i][g.dim-1 - j] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (g.table[i][g.dim-1 - j] == g.table[i][g.dim-1 - (j - k - 1)])
                    {
                        new_grid[i][g.dim-1 - (j - k - 1)] *= 2;
                        new_grid[i][g.dim-1 - j] = 0;
                        has_moved = 1;
                        g.score += new_grid[i][g.dim-1 - (j - k - 1)];
                        j++;
                        break;
                    }
                    else if (g.table[i][g.dim-1 - (j - k - 1)] != 0)
                    {
                        break;
                    }
                }
            }
        }

        for (size_t j = 0; j < g.dim; j++)
        {
            if (new_grid[i][g.dim-1 - j] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (new_grid[i][g.dim-1 - k] == 0)
                    {
                        new_grid[i][g.dim-1 - k] = new_grid[i][g.dim-1 - j];
                        new_grid[i][g.dim-1 - j] = 0;
                        has_moved = 1;
                    }
                }
            }
        }
    }
    g.table = new_grid;
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}
int haut_first_version(Grille& g)
{
    std::vector<std::vector<int>> new_grid = g.table;
    int has_moved = -1;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 1; j < g.dim; j++)
        {
            if (g.table[j][i] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (g.table[j][i] == g.table[j - k - 1][i])
                    {
                        new_grid[j - k - 1][i] *= 2;
                        new_grid[j][i] = 0;
                        has_moved = 1;
                        g.score += new_grid[j - k - 1][i];
                        j++;
                        break;
                    }
                    else if (g.table[j - k - 1][i] != 0)
                    {
                        break;
                    }
                }
            }
        }

        for (size_t j = 0; j < g.dim; j++)
        {
            if (new_grid[j][i] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (new_grid[k][i] == 0)
                    {
                        new_grid[k][i] = new_grid[j][i];
                        new_grid[j][i] = 0;
                        has_moved = 1;
                    }
                }
            }
        }
    }
    g.table = new_grid;
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}
int bas_first_version(Grille& g)
{
    std::vector<std::vector<int>> new_grid = g.table;
    int has_moved = -1;
    for (size_t i = 0; i < g.dim; i++)
    {
        for (size_t j = 1; j < g.dim; j++)
        {
            if (g.table[g.dim-1 - j][i] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (g.table[g.dim-1 - j][i] == g.table[g.dim-1 - (j - k - 1)][i])
                    {
                        new_grid[g.dim-1 - (j - k - 1)][i] *= 2;
                        new_grid[g.dim-1 - j][i] = 0;
                        has_moved = 1;
                        g.score += new_grid[g.dim-1 - (j - k - 1)][i];
                        j++;
                        break;
                    }
                    else if (g.table[g.dim-1 - (j - k - 1)][i] != 0)
                    {
                        break;
                    }
                }
            }
        }

        for (size_t j = 0; j < g.dim; j++)
        {
            if (new_grid[g.dim-1 - j][i] != 0)
            {
                for (size_t k = 0; k < j; k++)
                {
                    if (new_grid[g.dim-1 - k][i] == 0)
                    {
                        new_grid[g.dim-1 - k][i] = new_grid[g.dim-1 - j][i];
                        new_grid[g.dim-1 - j][i] = 0;
                        has_moved = 1;
                    }
                }
            }
        }
    }
    g.table = new_grid;
    if (has_moved == 1) { has_moved = vides(g); }
    return has_moved;
}


/**
 * Saves the current state of the game grid to a file.
 * @param g The grid to save.
 * @param filename The name of the file to save to.
 */
void sauve(const Grille& g, std::string filename)
{
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        printCentered("Error: Couldn't open file for writing.");
        exit(1);
    }

    outFile << "score: " << g.score << std::endl;
    outFile << "dim: " << g.dim << std::endl;
    outFile << "cible: " << g.cible << std::endl;
    outFile << "prop: " << g.prop << std::endl;

    outFile << "table:" << std::endl;
    for (const auto& row : g.table) {
        for (int value : row) {
            outFile << value << " ";
        }
        outFile << std::endl;
    }

    outFile.close();
}

/**
 * Restores a previously saved state of the game grid from a file.
 * @param g The grid to restore into.
 * @param filename The name of the file to restore from.
 */
void restaure(Grille& g, std::string filename)
{
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        clearConsole(SYSTEM_CLEAR);
        printCentered("Error: Couldn't open file for reading.");
        exit(1);
    }

    std::string line;
    while (std::getline(inFile, line)) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }
        std::string field = line.substr(0, colonPos);
        if (field == "score") {
            g.score = std::stoi(line.substr(colonPos + 2));
        }
        else if (field == "dim") {
            g.dim = std::stoi(line.substr(colonPos + 2));
        }
        else if (field == "cible") {
            g.cible = std::stoi(line.substr(colonPos + 2));
        }
        else if (field == "prop") {
            g.prop = std::stoi(line.substr(colonPos + 2));
        }
        else if (field == "table") {
            g.table.clear();
            while (std::getline(inFile, line)) {
                if (line.empty()) {
                    break;
                }
                std::vector<int> row;
                std::istringstream iss(line);
                int value;
                while (iss >> value) {
                    row.push_back(value);
                }
                g.table.push_back(row);
            }
        }
    }

    inFile.close();
}

/**
 * Displays the game grid.
 * @param g The game grid to display.
 */
void affiche(const Grille& g) {
    int i, j, k;
    int max = dimension(g);
    std::cout << std::endl << " \t ";
    for (k = 0; k < (6 + 1) * max - 1; k = k + 1) { std::cout << "-"; }
    std::cout << std::endl;
    /* Verifier si on veut que la ligne du haut soit 0 ou max-1 */
    for (i = 0; i < max; i = i + 1) {
        std::cout << "\t|";
        for (j = 0; j < max; j = j + 1) {
            if (g.table.at(i).at(j) == 0) {
                std::cout << "      |";
            }
            else { std::cout << " " << std::setw(4) << g.table.at(i).at(j) << " |"; }
        }
        if (i != max - 1) {
            std::cout << std::endl << "\t|";
            for (k = 0; k < (6 + 1) * max - 1; k = k + 1) { std::cout << "-"; }
            std::cout << "|" << std::endl;
        }
        else {
            std::cout << std::endl << "\t ";
            for (k = 0; k < (6 + 1) * max - 1; k = k + 1) { std::cout << "-"; }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl << "Score: " << score(g) << ", Vides: " << vides(g) << "  \n";
}




// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------




/**
 * Returns the number of digits in a number.
 * @param i The number to count digits for.
 * @return The number of digits.
 */
UINT digits(UINT i)
{
    return i > 0 ? (int)log10((double)i) + 1 : 1;
}

/**
 * Retrieves the RGB color values corresponding to a number for the game display.
 * @param number The number for which to retrieve the color.
 * @param r The red component of the color .
 * @param g The green component of the color.
 * @param b The blue component of the color.
 */
void getGameColor(int number, int& r, int& g, int& b)
{
    if (number < 65)
    {
        r = 225;
        switch (number)
        {
        case 2:
            g = b = 255;
            break;
        case 4:
            g = b = 205;
            break;
        case 8:
            g = b = 165;
            break;
        case 16:
            g = b = 110;
            break;
        case 32:
            g = b = 75;
            break;
        case 64:
            g = b = 0;
            break;
        }
    }
    else if (number < 2049)
    {
        r = 225;
        g = 255;
        b = 128 - round((sqrt(number - 127)) * 2.75);
    }
    else
    {
        r = 0;
        g = 256 + round(pow(1.000037, (-number + 150000))) + 256;
        b = -pow(1.000037, (-number + 150000)) + 256;
    }
}






// ------------------------ init -------------------------
bool _colored;
bool _fun;
int _dim;
int _cible;
int _prop;
bool _interactive;
bool _safe;
std::string _saveFileName;

/**
 * Initializes the default settings for the game.
 * @remark you can change the value of "safe" to "true" if there's a problem with the display.
 */
void setDefaultSettings()
{
    _colored = true;
    _fun = false;
    _dim = 4;
    _cible = 2048;
    _prop = 9;
    _interactive = true;
    _safe = false; //read @remark
    _saveFileName = "saveFile.data";
}




//------------------- display function -------------------
/**
 * Displays the header of the game.
 * @param y The vertical offset for displaying the header.
 */
void header(int y=0)
{
    printCenteredX("--------------------------------------",0, y);
    printCenteredX("---------------- 2048 ----------------", 0, y + 1);
    printCenteredX("------------ made by Colin -----------", 0, y + 2);
    printCenteredX("--------------------------------------", 0, y + 3);
}

/**
 * Displays the safe version of the header of the game.
 * @param y The vertical offset for displaying the header.
 */
void headerSafe(int y = 0)
{
    std::cout << "--------------------------------------\n";
    std::cout << "---------------- 2048 ----------------\n";
    std::cout << "-------- made by Colin & Revan -------\n";
    std::cout << "--------------------------------------\n";
}


/**
 * Displays the customized game grid.
 * @param grid The game grid to display.
 * @param xoffset The horizontal offset for displaying the grid.
 * @param yoffset The vertical offset for displaying the grid.
 * @param centered Whether to center the grid on the screen.
 * @param root The rectangular region to display the grid within.
 */
void afficheCustom(const Grille& grid, int xoffset, int yoffset, bool centered, const SMALL_RECT& root)
{
    int number;
    int digit_nb;
    int r, g, b;

    int _xoffset = xoffset;
    int _yoffset;
    if (centered) _yoffset = - (4 * grid.dim + 1) / 2 + yoffset;
    else _yoffset = yoffset;


    if (centered) printCentered(std::string(grid.dim * 9, '-') + '-', _xoffset, _yoffset, root);
    else printAtPos(std::string(grid.dim * 9, '-')+'-', _xoffset, _yoffset, root);
    _yoffset++;

    std::ostringstream stream;
    for (size_t i = 0; i < grid.dim; i++)
    {
        stream << "|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
            {
                getGameColor(number, r, g, b);
                if (_colored)
                {
                    stream << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
                    stream << "        \x1b[0m|";
                }
                else
                {
                    stream << "########|";
                }
            }
            else
                stream << std::setw(8) << "" << "|";
        }
        if (centered) printCentered(stream.str(), _xoffset, _yoffset, root);
        else printAtPos(stream.str(), _xoffset, _yoffset, root);
        _yoffset++;
        stream.str("");

        stream << "|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
            {
                getGameColor(number, r, g, b);
                digit_nb = digits(number);

                if (_colored)
                {
                    stream << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
                    if (number < 8193) for (int k = 0; k < (8 - digit_nb) / 2 - 1; k++) stream << " ";
                    else for (int k = 0; k < (8 - digit_nb) / 2 - 0; k++) stream << " ";
                    stream << "\x1b[0m";
                }
                else
                {
                    if (number < 8193) for (int k = 0; k < (8 - digit_nb) / 2 - 1; k++) stream << "#";
                    else for (int k = 0; k < (8 - digit_nb) / 2 - 0; k++) stream << "#";
                }

                stream << " ";
                stream << grid.table[i][j];
                stream << " ";

                if (_colored)
                {
                    stream << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
                    for (int k = 0; k < (8 - digit_nb - 1) / 2; k++) stream << " ";
                    stream << "\x1b[0m|";
                }
                else
                {
                    for (int k = 0; k < (8 - digit_nb - 1) / 2; k++) stream << "#";
                    stream << "|";
                }
            }
            else
            {
                stream << std::setw(8) << "" << "|";
            }
        }
        if (centered) printCentered(stream.str(), _xoffset, _yoffset, root);
        else printAtPos(stream.str(), _xoffset, _yoffset, root);
        _yoffset++;
        stream.str("");

        stream << "|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
            {
                getGameColor(number, r, g, b);

                if (_colored)
                {
                    stream << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
                    stream << "        \x1b[0m|";
                }
                else
                {
                    stream << "########|";
                }
            }
            else
                stream << std::setw(8) << "" << "|";
        }
        if (centered) printCentered(stream.str(), _xoffset, _yoffset, root);
        else printAtPos(stream.str(), _xoffset, _yoffset, root);
        _yoffset++;

        stream.str("");

        if (centered) printCentered(std::string(grid.dim * 9, '-') + '-', _xoffset, _yoffset, root);
        else printAtPos(std::string(grid.dim * 9, '-') + '-', _xoffset, _yoffset, root);
        _yoffset++;
    }
    
    if (centered) _yoffset = -(4 * grid.dim + 1) / 2 + yoffset -1;
    else _yoffset = yoffset -1;

    if (centered) printCentered("   Vides: " + std::to_string(vides(grid)) + "   ", 10 + _xoffset, _yoffset, root);
    else printAtPos("Vides: " + std::to_string(vides(grid)) + "   ", _xoffset, _yoffset, root);

    if (centered) printCentered("   Score: " + std::to_string(score(grid)) + "   ", -10 + _xoffset, _yoffset, root);
    else printAtPos("Score: " + std::to_string(score(grid)),_xoffset+10, _yoffset, root);
}

/**
 * Displays the safe version of the customized game grid.
 * @param grid The game grid to display.
 */
void afficheCustomSafe(const Grille& grid)
{
    int number;
    int digit_nb;
    int r, g, b;

    std::cout << "\n";
    for (size_t j = 0; j < grid.dim; j++)
        std::cout << "---------";
    std::cout << "-\n";

    for (size_t i = 0; i < grid.dim; i++)
    {
        std::cout << "|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
                std::cout << "########|";
            else
                std::cout << std::setw(8) << "" << "|";
        }
        std::cout << "\n|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
            {
                digit_nb = digits(number);

                if (number < 8193) for (int k = 0; k < (8 - digit_nb) / 2 - 1; k++) std::cout << "#";
                else for (int k = 0; k < (8 - digit_nb) / 2 - 0; k++) std::cout << "#";

                std::cout << " ";
                std::cout << grid.table[i][j];
                std::cout << " ";

                for (int k = 0; k < (8 - digit_nb - 1) / 2; k++) std::cout << "#";
                std::cout << "|";
            }
            else
            {
                std::cout << std::setw(8) << "" << "|";
            }
        }
        std::cout << "\n|";
        for (size_t j = 0; j < grid.dim; j++)
        {
            number = grid.table[i][j];
            if (number != 0)
                std::cout << "########|";
            else
                std::cout << std::setw(8) << "" << "|";
        }

        std::cout << "\n";
        for (size_t j = 0; j < grid.dim; j++)
            std::cout << "---------";
        std::cout << "-\n";
    }

    std::cout << "\nScore: " << score(grid) << ", Vides: " << vides(grid) << "\n";
}



//-------------------- 2048 game loop --------------------

/**
 * Starts the game loop for the standard version of the game.
 */
void startGame()
{
    Grille g;
    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
    if (std::filesystem::exists(_saveFileName))
    {
        if (_interactive)
        {
            printCentered("do you want to restore previous state ? (press y for yes or n for no)");
            while (true)
            {
                if (Keyboard::isKeyPressed('Y'))
                {
                    restaure(g, _saveFileName);
                    break;
                }
                if (Keyboard::isKeyPressed('N'))
                {
                    if (!(init(g, _dim, _cible, _prop)))
                    {
                        std::cerr << "impossible d'initialiser la grille";
                        exit(1);
                    }
                    break;
                }
                if (isConsoleResized())
                {
                    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                    printCentered("do you want to restore previous state ? (press y for yes or n for no)");
                }
            }
        }
        else
        {
            bool inputReceived = false;
            char choice;
            bool valid = true;
            bool first = true;
            std::thread asyncInput;
            do
            {
                choice = -1;
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                printCentered("do you want to restore previous state ? (y for yes or n for no)");
                if (!valid && inputReceived)
                    printCentered(setColor(255, 0, 0) + "/!\\ pls enter a correct choice /!\\" + resetColor(), 0,1);

                if (inputReceived || first)
                {
                    inputReceived = false;
                    first = false;
                    asyncInput = std::thread([&choice, &inputReceived] {
                        choice = std::cin.get();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        inputReceived = true;
                        });
                }

                while (true)
                {
                    Sleep(100);
                    if (isConsoleResized() || inputReceived)
                    {
                        break;
                    }
                }

                if (inputReceived)
                    asyncInput.join();

                if ((choice == 'y' || choice == 'n') && !std::cin.fail())
                {
                    valid = true;
                }
                else
                {
                    valid = false;
                    std::cin.clear();
                }

            } while (!valid);


            if (choice=='y')
            {
                restaure(g, _saveFileName);
            }
            else if (choice == 'n')
            {
                if (!(init(g, _dim, _cible, _prop)))
                {
                    std::cerr << "impossible d'initialiser la grille";
                    exit(1);
                }
            }
        }
    }
    else
    {
        if (!(init(g, _dim, _cible, _prop)))
        {
            std::cerr << "impossible d'initialiser la grille";
            exit(1);
        }
    }

    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
    SMALL_RECT root;
    if (g.dim<7)
    {
        if (_colored) root = drawRectangle(70, 30);
        else root = drawRectangle(70, 30, 255, 255, 255);
    }
    else
    {
        if (_colored) root = drawRectangle(70 * g.dim / 6, 30 * g.dim / 6);
        else root = drawRectangle(70 + 10*(g.dim-7), 30 + 5 * (g.dim - 7), 255, 255, 255);
    }
    afficheCustom(g,0,0, true, root);

    int pressed = false;
    int has_moved = false;
    bool valid = true;
    bool blockSpawn = false;
    char mov;

    while (true)
    {
        sauve(g, _saveFileName);

        if (_interactive)
        {
            setCursorInvisible();
            has_moved = -1;
            while (has_moved == -1){
                Sleep(10);
                if (Keyboard::isKeyPressed(VK_LEFT)) {
                    has_moved = gauche(g);
                }

                if (Keyboard::isKeyPressed(VK_RIGHT)) {
                    has_moved = droite(g);
                }

                if (Keyboard::isKeyPressed(VK_UP)) {
                    has_moved = haut(g);
                }

                if (Keyboard::isKeyPressed(VK_DOWN)) {
                    has_moved = bas(g);
                }

                if (Keyboard::isKeyPressed(VK_ESCAPE)) {
                    return;
                }

                if (isConsoleResized())
                {
                    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                    if (g.dim < 7)
                    {
                        if (_colored) root = drawRectangle(70, 30);
                        else root = drawRectangle(70, 30, 255, 255, 255);
                    }
                    else
                    {
                        if (_colored) root = drawRectangle(70 * g.dim / 6, 30 * g.dim / 6);
                        else root = drawRectangle(70 + 12 * (g.dim - 7), 30 + 6 * (g.dim - 7), 255, 255, 255);
                    }
                    blockSpawn = true;
                    setCursorInvisible();
                    break;
                }
            }
        }
        else
        {
            setCursorVisible();
            has_moved = -1;
            valid = true;
            do
            {
                if (!valid) printCenteredX(setColor(255, 0, 0) + "/!\\ pls enter a correct direction /!\\" + resetColor(), 0, root.Bottom - 2);
                else printCenteredX(setColor(255, 0, 0) + "                                       " + resetColor(), 0, root.Bottom - 2);
                printCenteredX("   enter mov :   ", -1, root.Bottom - 3);
                setCursorPos(getRectCenter(root).X + std::string("enter mov : ").length() / 2 + 1, root.Bottom - 3);

                input(mov);
                switch (mov) {
                case 'g':
                    valid = true;
                    has_moved = gauche(g);
                    break;
                case 'd':
                    valid = true;
                    has_moved = droite(g);
                    break;
                case 'h':
                    valid = true;
                    has_moved = haut(g);
                    break;
                case 'b':
                    valid = true;
                    has_moved = bas(g);
                    break;
                case 'q':
                    valid = true;
                    return;
                default:
                    valid = false;
                }
            } while (!valid || has_moved == -1);
        }


        if (succes(g))
        {
            valid = true;
            char value;
            if (_interactive)
            {
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);

                if (g.dim < 7)
                {
                    if (_colored) root = drawRectangle(70, 30);
                    else root = drawRectangle(70, 30, 255, 255, 255);
                }
                else
                {
                    if (_colored) root = drawRectangle(70 * g.dim / 6, 30 * g.dim / 6);
                    else root = drawRectangle(70 + 10 * (g.dim - 7), 30 + 5 * (g.dim - 7), 255, 255, 255);
                }
                afficheCustom(g, 0, -3, true, root);

                printCenteredX("----------------------------------", 0, root.Bottom - 6);
                printCenteredX("----- WELL DONE, YOU WON !!! -----", 0, root.Bottom - 5);
                printCenteredX("----------------------------------", 0, root.Bottom - 4);

                printCenteredX("press enter to return to main menu...", 0, root.Bottom - 3);

                while (!Keyboard::isKeyPressed(VK_RETURN));
            }
            else
            {
                do
                {
                    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);

                    if (_colored) root = drawRectangle(70, 30);
                    else root = drawRectangle(70, 30, 255, 255, 255);
                    afficheCustom(g, 0, -4, true, root);
                    printCenteredX("----------------------------------", 0, root.Bottom - 6);
                    printCenteredX("----- WELL DONE, YOU WON !!! -----", 0, root.Bottom - 5);
                    printCenteredX("----------------------------------", 0, root.Bottom - 4);

                    printCenteredX("write q to return to main menu : ", -1, root.Bottom - 3);
                    if (!valid) printCenteredX(setColor(255, 0, 0) + "/!\\ pls enter a correct value /!\\" + resetColor(), 0, root.Bottom - 2);

                    setCursorPos(getRectCenter(root).X + std::string("write q to return to main menu : ").length() / 2 + 1, root.Bottom - 3);
                    bool fail = input(value);
                    if (value == 'q' && !fail) valid = true;
                    else valid = false;
                } while (!valid);
            }
            return;
        }

        if (vides(g) > 0 && !blockSpawn)
            spawnRandom(g);
        else
            blockSpawn = false;

        
        if (vides(g) == 0)
        {
            Grille tempGrid = g;
            if (gauche(tempGrid) == -1 && droite(tempGrid) == -1 && haut(tempGrid) == -1 && bas(tempGrid) == -1)
            {

                valid = true;
                char value;
                if (_interactive)
                {
                    clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);

                    if (_colored) root = drawRectangle(70, 30);
                    else root = drawRectangle(70, 30, 255, 255, 255);
                    afficheCustom(g, 0, -3, true, root);

                    printCenteredX("----------------------------------", 0, root.Bottom - 6);
                    printCenteredX("------ too bad, you lost... ------", 0, root.Bottom - 5);
                    printCenteredX("----------------------------------", 0, root.Bottom - 4);

                    printCenteredX("press enter to return to main menu...", 0, root.Bottom - 3);

                    while (!Keyboard::isKeyPressed(VK_RETURN));
                }
                else
                {
                    do
                    {
                        clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);

                        if (_colored) root = drawRectangle(70, 30);
                        else root = drawRectangle(70, 30, 255, 255, 255);
                        afficheCustom(g, 0, -4, true, root);

                        printCenteredX("----------------------------------", 0, root.Bottom-6);
                        printCenteredX("------ too bad, you lost... ------", 0, root.Bottom - 5);
                        printCenteredX("----------------------------------", 0, root.Bottom - 4);

                        printCenteredX("write q to return to main menu : ", -1, root.Bottom - 3);
                        if (!valid) printCenteredX(setColor(255,0,0)+"/!\\ pls enter a correct value /!\\"+resetColor(), 0, root.Bottom - 2);

                        setCursorPos(getRectCenter(root).X+std::string("write q to return to main menu : ").length()/2+1, root.Bottom - 3);
                        bool fail = input(value);
                        if (value == 'q' && !fail) valid = true;
                        else valid = false;
                    } while (!valid);
                }

                return;
            }
        }

        clearConsole(RESET_CURSOR);
        afficheCustom(g, 0, 0, true, root);
    }
}

/**
 * Starts the game loop for the safe version of the game.
 */
void startGameSafe()
{
    Grille g;
    if (!(init(g, _dim, _cible, _prop)))
    {
        std::cerr << "impossible d'initialiser la grille";
        exit(1);
    }
    clearConsole(SYSTEM_CLEAR);
    afficheCustomSafe(g);


    int pressed = false;
    int has_moved = false;
    bool valid = true;
    char mov;

    while (true)
    {
        has_moved = -1;
        do
        {
            valid = true;
            if (!valid) std::cout << "\n/!\\ pls enter a correct setting /!\\\n";
            std::cout << "enter mov : ";
            std::cin >> mov;
            switch (mov) {
            case 'g':
                valid = true;
                has_moved = gauche(g);
                break;
            case 'd':
                valid = true;
                has_moved = droite(g);
                break;
            case 'h':
                valid = true;
                has_moved = haut(g);
                break;
            case 'b':
                valid = true;
                has_moved = bas(g);
                break;
            case 'q':
                valid = true;
                exit(1);
                break;
            default:
                valid = false;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } while (!valid || has_moved == -1);
         

        if (succes(g))
        {
            valid = true;
            char value;
            do
            {
                clearConsole(SYSTEM_CLEAR);
                afficheCustomSafe(g);
                std::cout << "----------------------------------\n";
                std::cout << "----- WELL DONE, YOU WON !!! -----\n";
                std::cout << "----------------------------------\n";

                std::cout << "\nwrite q to return to main menu :\n";
                if (!valid) std::cout << "\n/!\\ pls enter a correct value /!\\\n";

                std::cin >> value;
                if (value == 'q' && !std::cin.fail()) valid = true;
                else valid = false;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (!valid);

            return;
        }

        if (vides(g) > 0)
            spawnRandom(g);

        if (vides(g) == 0)
        {
            Grille tempGrid = g;
            if (gauche(tempGrid) == -1 && droite(tempGrid) == -1 && haut(tempGrid) == -1 && bas(tempGrid) == -1)
            {

                valid = true;
                char value;
                do
                {
                    clearConsole(SYSTEM_CLEAR);
                    afficheCustomSafe(g);
                    std::cout << "----------------------------------\n";
                    std::cout << "------ too bad, you lost... ------\n";
                    std::cout << "----------------------------------\n";

                    std::cout << "\nwrite q to return to main menu :\n";
                    if (!valid) std::cout << "\n/!\\ pls enter a correct value /!\\\n";

                    std::cin >> value;
                    if (value == 'q' && !std::cin.fail()) valid = true;
                    else valid = false;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                } while (!valid);

                return;
            }
        }

        clearConsole(SYSTEM_CLEAR);
        afficheCustomSafe(g);
    }
}


//------------------------- menu -------------------------
/**
 * Implementation of the settings menu interface for interactive and non-interactive control.
 *
 * This function allows users to configure various settings such as dimension, goal, proportion,
 * colored mode, fun mode (Windows only), and interactive mode (Windows only). Users can also reset
 * settings to default or return to the main menu.
 *
 * @remark For interactive mode (Windows only), the user can navigate the settings using arrow keys and
 * confirm their choice by pressing Enter. For non-interactive mode, the user must input the choice directly.
 * If an invalid choice is entered, the user is prompted to enter a correct one.
 */
void settingsMenu()
{
    SMALL_RECT root;
    bool exitSettings = false;
    bool valid = true;
    char choice;
    int index = 0;
    while (!exitSettings)
    {

        if (_interactive)
        {
            setCursorInvisible();
            bool has_chosed = false;
            int previousIndex = index;
            while (!has_chosed)
            {
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                if (_colored) root = drawRectangle(70, 30);
                else root = drawRectangle(70, 30, 255, 255, 255);
                header(root.Top + 5);
                int posIndexY = 0;
                int xOffset = round((root.Right-root.Left+1) / 2.0 - 43/ 2.0);
                printCentered("------ SETTINGS ------", 0, posIndexY - 2, root); posIndexY++;

                if (_colored) std::cout << "\x1b[38;2;255;0;0m";
                printCentered("->", -25, posIndexY + index, root);
                printCentered("<-", 25, posIndexY + index, root);
                if (_colored) std::cout << "\x1b[0m";

                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- dimension (>=2) :" << _dim).str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- goal (-1 = infinite mode) :" << _cible).str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- proportion (>=0 && <=10) :" << _prop).str(), xOffset, posIndexY, root); posIndexY++;

                std::ostringstream tempStream;
                tempStream << std::left << std::setw(39) << "- colored mode : ";
                if (_colored) { tempStream << std::left << "on"; }
                else { tempStream << std::left << "off"; }
                printCenteredY(tempStream.str(), xOffset, posIndexY, root); posIndexY++;
                tempStream.str("");

                tempStream << std::left << std::setw(39) << "- interactive mode : ";
                if (_interactive) { tempStream << "on"; }
                else { tempStream << std::left << "off"; }
                printCenteredY(tempStream.str(), xOffset, posIndexY, root); posIndexY++;
                tempStream.str("");

                printCenteredY((std::ostringstream{} << "- reset settings").str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << "- back to main menu").str(), xOffset, posIndexY, root); posIndexY++;

                previousIndex = index;
                while (previousIndex == index && !has_chosed)
                {
                    Sleep(10);

                    if (isConsoleResized()){
                        setCursorInvisible();
                        break;
                    }

                    if (Keyboard::isKeyPressed(VK_UP) && index > 0) {
                        index--;
                    }

                    if (Keyboard::isKeyPressed(VK_DOWN) && index < 6) {
                        index++;
                    }

                    if (Keyboard::isKeyPressed(VK_ESCAPE) && index < 6) {
                        choice = 'b';
                        has_chosed = true;
                    }

                    if (Keyboard::isKeyPressed(VK_RETURN) && index >= 0 && index <= 6) {
                        choice = index;
                        has_chosed = true;
                    }
                }
            }
        }
        else
        {
            setCursorVisible();
            bool inputReceived = false;
            bool first = true;
            std::thread asyncInput;
            do
            {
                
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                if (_colored) root = drawRectangle(100, 30);
                else root = drawRectangle(100, 30, 255, 255, 255);
                header(root.Top+5);
                int posIndexY = 0;
                int xOffset = round(100/2.0 - 70/2.0);
                printCentered("------ SETTINGS ------", 0, posIndexY-2, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- dimension (>=2) :" << std::setw(9) << _dim << "(enter d to configure)").str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- goal (-1 = infinite mode) :" << std::setw(9) << _cible << "(enter g to configure)").str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(39) << "- proportion (>=0 && <=10) :" << std::setw(9) << _prop << "(enter p to configure)").str(), xOffset, posIndexY, root); posIndexY++;

                std::ostringstream tempStream;
                tempStream << std::left << std::setw(39) << "- colored mode : ";
                if (_colored) { tempStream << std::left << std::setw(9) << "on"; }
                else { tempStream << std::left << std::setw(9) << "off"; }
                tempStream << std::left << "(enter c to change)";
                printCenteredY(tempStream.str(), xOffset, posIndexY, root); posIndexY++;
                tempStream.str("");

                tempStream << std::left << std::setw(39) << "- interactive mode : ";
                if (_interactive) { tempStream << std::setw(9) << "on"; }
                else { tempStream << std::left << std::setw(9) << "off"; }
                tempStream << std::left << "(enter i to change)";
                printCenteredY(tempStream.str(), xOffset, posIndexY, root); posIndexY++;
                tempStream.str("");

                printCenteredY((std::ostringstream{} << std::left << std::setw(48) << "- reset settings" << "(enter r to reset)").str(), xOffset, posIndexY, root); posIndexY++;
                printCenteredY((std::ostringstream{} << std::left << std::setw(48) << "- back to main menu" << "(enter b to go back)").str(), xOffset, posIndexY, root); posIndexY++;

                posIndexY++;
                printCentered("Enter your choice : ", 0, posIndexY, root);
                
                if (!valid && inputReceived)
                    printCentered(setColor(255,0,0)+"/!\\ pls enter a correct setting /!\\"+resetColor(), 0, posIndexY + 1, root);

                if (inputReceived || first)
                {
                    inputReceived = false;
                    first = false;
                    asyncInput = std::thread([&choice, &inputReceived, &root] {
                        std::string test;
                        clearConsoleInputBuffer();
                        setCursorPos(getRectCenter(root).X + std::string("Enter your choice :").length() / 2.0 + 2, getRectCenter(root).Y + 11);
                        std::getline(std::cin, test);
                        choice = test[0];
                        inputReceived = true;
                        });
                }

                while (true)
                {
                    Sleep(50);
                    if (isConsoleResized() || inputReceived)
                    {
                        break;
                    }
                }

                if (inputReceived)
                    asyncInput.join();

                if ((choice == 'd' || choice == 'g' || choice == 'p' || choice == 'c' || choice == 'i' || choice == 'r' || choice == 'b') && !std::cin.fail())
                {
                    valid = true;
                }
                else
                {
                    valid = false;
                    std::cin.clear();
                }

            } while (!valid);
        }

        valid = true;
        int value=0;
        switch (choice)
        {
        case 0:
        case 'd':
            setCursorVisible();
            do
            {
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                if (_colored) root = drawRectangle(70, 30);
                else root = drawRectangle(70, 30, 255, 255, 255);
                header(root.Top + 5);
                printCentered("Enter a new dimension (>=2) : ", 0, 0, root);
                if (!valid)
                {
                    printCentered(setColor(255, 0, 0) + "/!\\ pls enter a correct dimension /!\\"+resetColor(), 0, 1, root);
                    setCursorPos(getRectCenter(root).X + std::string("Enter a new dimension (>=2) : ").length() / 2.0 + 1, getRectCenter(root).Y+1);
                }
                bool failed = input(value);
                if (value > 1 && !failed) { _dim = value; valid = true; }
                else valid = false;

            } while (!valid);
            break;
        case 1:
        case 'g':
            setCursorVisible();
            do
            {
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                if (_colored) root = drawRectangle(70, 30);
                else root = drawRectangle(70, 30, 255, 255, 255);
                header(root.Top + 5);
                printCentered("Enter a new objective (must be a power of 2 bigger than or equal to 2) : ", 0, 0, root);
                if (!valid)
                {
                    printCentered(setColor(255, 0, 0) + "/!\\ pls enter a correct dimension /!\\"+resetColor(), 0, 1, root);
                    setCursorPos(getRectCenter(root).X + std::string("Enter a new objective (must be a power of 2 bigger than or equal to 2) : ").length() / 2.0 + 1, getRectCenter(root).Y+1);
                }
                bool failed = input(value);
                if (!((value & (value - 1)) != 0 || value == 0 || value == 1) && !std::cin.fail()) { _cible = value; valid = true; }
                else valid = false;
            } while (!valid);
            break;

        case 2:
        case 'p':
            setCursorVisible();
            do
            {
                clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
                if (_colored) root = drawRectangle(70, 30);
                else root = drawRectangle(70, 30, 255, 255, 255);
                header(root.Top + 5);
                printCentered("Enter a new proportion (>= 0 and <=10) : ", 0, 0, root);
                if (!valid) 
                {
                    printCentered(setColor(255, 0, 0) + "/!\\ pls enter a correct dimension /!\\" + resetColor(), 0, 1, root);
                    setCursorPos(getRectCenter(root).X + std::string("Enter a new proportion (>= 0 and <=10) : ").length() / 2.0 + 1, getRectCenter(root).Y+1);
                }
                bool failed = input(value);
                if (value >= 0 && value <= 10 && !std::cin.fail()) { _prop = value; valid = true; }
                else valid = false;
            } while (!valid);
            break;

        case 3:
        case 'c':
            _colored = !_colored;
            break;

        case 4:
        case 'i':
            _interactive = !_interactive;
            break;

        case 5:
        case 'r':
            setDefaultSettings();
            break;

        case 6:
        case 'b':
            exitSettings = true;
            break;

        default:
            break;
        }
    }
}

/**
 * Safe version of the settings menu interface for non-interactive control.
 *
 * This function provides a version of the settings menu for non-interactive mode and without any graphical enhancements,
 * where the user directly inputs their choice. It ensures that the user's input
 * is validated before proceeding with the selected action.
 */
void settingsMenuSafe()
{
    SMALL_RECT root;
    bool exitSettings = false;
    bool valid;
    char choice;
    while (!exitSettings)
    {
        valid = true;
        do
        {
            clearConsole(SYSTEM_CLEAR);
            headerSafe();
            std::cout << "\n\n\n";
            int posIndexY = 0;
            int xOffset = 100 / 2 - 70 / 2;
            std::cout << "------ SETTINGS ------\n";
            std::cout << std::left << std::setw(39) << "- dimension (>=2) :" << std::setw(9) << _dim << "(enter d to configure)\n";
            std::cout << std::left << std::setw(39) << "- goal (-1 = infinite mode) :" << std::setw(9) << _cible << "(enter g to configure)\n";
            std::cout << std::left << std::setw(39) << "- proportion (>=0 && <=10) :" << std::setw(9) << _prop << "(enter p to configure)\n\n";

            std::cout << std::left << std::setw(39) << "- colored mode : ";
            if (_colored) { std::cout << std::left << std::setw(9) << "on"; }
            else { std::cout << std::left << std::setw(9) << "off"; }
            std::cout << std::left << "(enter c to change)\n";

            std::cout << std::left << std::setw(39) << "- interactive mode : ";
            if (_interactive) { std::cout << std::setw(9) << "on"; }
            else { std::cout << std::left << std::setw(9) << "off"; }
            std::cout << std::left << "(enter i to change)\n";

            std::cout << std::left << std::setw(48) << "- reset settings" << "(enter r to reset)\n";
            std::cout << std::left << std::setw(48) << "- back to main menu" << "(enter b to go back)\n";

            if (!valid && choice != -1) std::cout << "\n/!\\ pls enter a correct setting /!\\\n";

            std::cin >> choice;

            if ((choice == 'd' || choice == 'g' || choice == 'p' || choice == 'c' || choice == 'f' || choice == 'i' || choice == 'r' || choice == 'b') && !std::cin.fail())
                valid = true;
            else
                valid = false;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        } while (!valid);



        valid = true;
        int value;
        switch (choice)
        {
        case 'd':
            do
            {
                clearConsole(SYSTEM_CLEAR);
                header();
                std::cout << "\n\nEnter a new dimension (>=2) :\n";
                if (!valid) std::cout << "\n/!\\ pls enter a correct dimension /!\\\n";
                std::cin >> value;
                if (value > 1 && !std::cin.fail()) { _dim = value; valid = true; }
                else valid = false;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (!valid);
            break;

        case 'g':
            do
            {
                clearConsole(SYSTEM_CLEAR);
                header();
                std::cout << "\n\nEnter a new objective (must be a power of 2 bigger than or equal to 2) :\n";
                if (!valid) std::cout << "\n/!\\ pls enter a correct dimension /!\\\n";
                std::cin >> value;
                if (!((value & (value - 1)) != 0 || value == 0 || value == 1) && !std::cin.fail()) { _cible = value; valid = true; }
                else valid = false;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (!valid);
            break;

        case 'p':
            do
            {
                clearConsole(SYSTEM_CLEAR);
                header();
                std::cout << "\n\nEnter a new proportion (>= 0 and <=10) :\n";
                if (!valid) std::cout << "\n/!\\ pls enter a correct dimension /!\\\n";
                std::cin >> value;
                if (value >= 0 && value <= 10 && !std::cin.fail()) { _prop = value; valid = true; }
                else valid = false;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (!valid);
            break;

        case 'c':
            _colored = !_colored;
            break;

        case 'i':
            _interactive = !_interactive;
            break;

        case 'r':
            setDefaultSettings();
            break;

        case 'b':
            exitSettings = true;
            break;

        default:
            break;
        }
    }
}


/**
 * Displays the main menu interface for interactive and non-interactive control.
 *
 * This function allows users to choose between starting the game, accessing settings,
 * starting the AI mode, or quitting the application. The choice can be made interactively
 * using arrow keys and Enter for interactive mode (Windows only) or by direct input for
 * non-interactive mode.
 */
int mainMenu()
{
    char choice;
    SMALL_RECT root;
    if (_interactive)
    {
        bool has_chosed = false;
        int index = 0;
        int previousIndex = index;

        while (!has_chosed)
        {
            clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);
            setCursorInvisible();

            if (_colored) root = drawRectangle(70, 30);
            else root = drawRectangle(70, 30, 255, 255, 255);
            int posIndexY = 3;
            header(getRectCenter(root).Y - 9);

            printCentered("------ MAIN MENU ------", 0, posIndexY - 1, root); posIndexY++;

            if (_colored) std::cout << "\x1b[38;2;255;0;0m";
            printCentered("->", -11, posIndexY+index, root);
            printCentered("<-", 11, posIndexY+index, root);
            if (_colored) std::cout << "\x1b[0m";

            printCentered("- Start game -", 0, posIndexY, root); posIndexY++;
            printCentered("- Settings -", 0, posIndexY, root); posIndexY++;
            printCentered("- Quit -", 0, posIndexY, root);
            
            previousIndex = index;
            while (previousIndex == index && !has_chosed)
            {
                Sleep(10);

                if (isConsoleResized()){
                    break;
                }

                if (Keyboard::isKeyPressed(VK_UP) && index > 0) {
                    index--;
                }

                if (Keyboard::isKeyPressed(VK_DOWN) && index < 2) {
                    index++;
                }

                if (Keyboard::isKeyPressed(VK_ESCAPE)) {
                    choice = 'q';
                    has_chosed = true;
                }

                if (Keyboard::isKeyPressed(VK_RETURN) && index >= 0 && index <= 2) {
                    if (index == 0) choice = '1';
                    else if (index == 1) choice = '2';
                    else choice = 'q';
                    has_chosed = true;
                }
            }
        }
    }
    else
    {
        setCursorVisible();
        bool inputReceived = false;
        bool valid = true;
        bool first = true;
        std::thread asyncInput;
        do
        {
            choice = -1;
            clearConsole(RESET_CURSOR_AND_FILL_WITH_SPACE);

            if (_colored) root = drawRectangle(70, 30);
            else root = drawRectangle(70, 30, 255, 255, 255);
            int posIndexY = 3;
            header(getRectCenter(root).Y-9);

            printCentered("------ MAIN MENU ------", 0, posIndexY-1, root); posIndexY++;
            printCentered("- Start game (1) -", 0, posIndexY, root); posIndexY++;
            printCentered("- Settings (2) -", 0, posIndexY, root); posIndexY++;
            printCentered("- Ai (3) -", 0, posIndexY, root); posIndexY++;
            printCentered("- Quit (q) -", 0, posIndexY, root); posIndexY++;

            posIndexY++;
            printCentered("Enter your choice : ", 0, posIndexY, root);

            if (!valid && inputReceived)
                printCentered(setColor(255, 0, 0)+"/!\\ pls enter a correct setting /!\\"+resetColor(), 0, posIndexY + 1, root);
            
            setCursorPos(getRectCenter(root).X + std::string("Enter your choice :").length() / 2.0 + 2, getRectCenter(root).Y + posIndexY + 1);

            if (inputReceived || first)
            {
                inputReceived = false;
                first = false;
                asyncInput = std::thread([&choice, &inputReceived] {
                    choice = std::cin.get();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    inputReceived = true;
                });
            }


            while (true)
            {
                Sleep(100);
                if (isConsoleResized() || inputReceived)
                {
                    break;
                }
            }

            if (inputReceived)
                asyncInput.join();

            if ((choice == '1' || choice == '2' || choice == '3' || choice == 'q') && !std::cin.fail())
            {
                valid = true;
            }
            else
            {
                valid = false;
                std::cin.clear();
            }

        } while (!valid);
    }

    return choice;
}

/**
 * Safe version of the main menu interface for non-interactive control.
 *
 * This function provides a version of the main menu for non-interactive mode and without any graphical enhancements, where
 * the user directly inputs their choice. It ensures that the user's input is validated
 * before proceeding with the selected action.
 */
int mainMenuSafe()
{
    bool valid = true;
    char choice;
    do
    {
        choice;
        clearConsole(SYSTEM_CLEAR);

        headerSafe();
        std::cout << "\n\n\n";
        std::cout << "------ MAIN MENU ------\n";
        std::cout << "- Start game (1) -\n";
        std::cout << "- Settings (2) -\n";
        std::cout << "- Quit (q) -\n";
        if (!valid)
            std::cout << "\n/!\\ pls enter a correct choice /!\\\n";


        std::cin >> choice;

        if ((choice == '1' || choice == '2' || choice == 'q') && !std::cin.fail())
            valid = true;
        else
            valid = false;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    } while (!valid);

    return choice;
}


//------------------------- game main --------------------
/**
 * Launches the application and manages the flow of control between different menu interfaces.
 *
 * This function sets default settings, activates virtual terminal processing,
 * and manages the flow of control between the main menu, game mode, settings menu,
 * and AI mode based on user input.
 *
 * @remark Depending on the `_safe` flag, it selects either the normal menu or the safe one (non-interactive without any graphical enhancements)
 */
void launch()
{
    //setConsoleToFullScreen();
    setDefaultSettings();
    activateVP();
    SetConsoleOutputCP(65001);
    setAlternateBuffer();
    std::cout << "\x1b]2;2048\x07";


    char choice;

    if (!_safe)
    {
        while (true)
        {
            choice = mainMenu();
            if (choice == '1') startGame();
            else if (choice == '2') settingsMenu();
            else if (choice == 'q') break;
            else std::cout << "error";
        }
    }
    else
    {
        while (true)
        {
            choice = mainMenuSafe();
            if (choice == '1') startGameSafe();
            else if (choice == '2') settingsMenuSafe();
            else if (choice == 'q') break;
            else std::cout << "error";
        }
    }

    clearConsoleInputBuffer();

    setMainBuffer();
    return;
}
