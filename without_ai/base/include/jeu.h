#include <cstdlib> // pour srand, exit
#include <time.h>  // pour time
#include <iostream>
#include <vector>
#define NOMINMAX
#include <Windows.h>

//using namespace std;

/* les lignes
 * #ifndef  _JEU_2024
 * #define  _JEU_2024 1
 * ...
 * #endif
 * qui encadrent les definitions utiles servent à eviter que le compilateur
 * definisse deux fois les memes identificateurs (ce qui provoquerait une
 * erreur) au cas ou vous feriez deux fois #include "jeu.h"
 * Le compilateur ne traite les lignes que si la macro _JEU_2048 n'est
 * pas definie, ce qu'il s'empresse de faire quand il voit ce code pour la
 * premiere fois.
 */
#ifndef _JEU_2048
#define _JEU_2048 1

struct Grille {
	int score;
	int dim;
	int cible;
	int prop;
	std::vector<std::vector<int>> table;
};

/* déclarations de fonctions. Tout fichier qui fait un include de jeu.h
 * connaîtra donc les en-têtes de ces fonctions.
 */

 /**
  * Counts the number of empty cells in the grid.
  *
  * @param g Reference to the grid.
  * @return The number of empty cells in the grid.
  */
int vides(const Grille& g);

/**
 * Returns the proportion value of the grid.
 *
 * @param g Reference to the grid.
 * @return The proportion value of the grid.
 */
int proportion(const Grille& g);

/**
 * Returns the target value of the grid.
 *
 * @param g Reference to the grid.
 * @return The target value of the grid.
 */
int cible(const Grille& g);

/**
 * Returns the dimension of the grid.
 *
 * @param g Reference to the grid.
 * @return The dimension of the grid.
 */
int dimension(const Grille& g);

/**
 * Returns the score of the grid.
 *
 * @param g Reference to the grid.
 * @return The score of the grid.
 */
int score(const Grille& g);

/**
 * Checks if the target value is present in the grid.
 *
 * @param g Reference to the grid.
 * @return True if the target value is present, otherwise false.
 */
bool succes(const Grille& g);


/**
 * Initializes the grid with specified dimension, target value, and proportion.
 *
 * @param g Reference to the grid to initialize.
 * @param dimension Dimension of the grid.
 * @param cible Target value for the grid.
 * @param proportion Proportion value for the grid.
 * @return True if initialization is successful, otherwise false.
 */
bool init(Grille& g, int dimension, int cible, int proportion);

/**
 * Loads grid data from a 2D vector and initializes the grid with specified target value and proportion.
 *
 * @param g Reference to the grid to initialize.
 * @param v 2D vector representing the grid data.
 * @param cible Target value for the grid.
 * @param proportion Proportion value for the grid.
 * @return True if loading and initialization are successful, otherwise false.
 */
bool charge(Grille& g, std::vector<std::vector<int>>& v, int cible, int proportion);

/**
 * Displays the game grid.
 * @param g The game grid to display.
 */
void affiche(const Grille& g);

/**
 * Moves the tiles in the grid to the left.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int gauche(Grille& g);

/**
 * Moves the tiles in the grid to the right.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int droite(Grille& g);

/**
 * Moves the tiles in the grid upwards.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int haut(Grille& g);

/**
 * Moves the tiles in the grid downwards.
 * @param g The grid to manipulate.
 * @return 1 if the tiles were moved, -1 otherwise.
 */
int bas(Grille& g);



/**
 * Spawns a random tile at an empty cell in the grid.
 *
 * @param g Reference to the grid.
 * @return True if a tile is spawned successfully, otherwise false.
 */
int spawnRandom(Grille& g);


/**
 * Launches the application and manages the flow of control between different menu interfaces.
 *
 * This function sets default settings, activates virtual terminal processing,
 * and manages the flow of control between the main menu, game mode, settings menu,
 * and AI mode based on user input.
 *
 * @remark Depending on the `_safe` flag, it selects either the normal menu or the safe one (non-interactive without any graphical enhancements)
 */
void launch();


/**
 * Displays the customized game grid.
 * @param grid The game grid to display.
 * @param xoffset The horizontal offset for displaying the grid.
 * @param yoffset The vertical offset for displaying the grid.
 * @param centered Whether to center the grid on the screen.
 * @param root The rectangular region to display the grid within.
 */
void afficheCustom(const Grille& grid, int xoffset = 0, int yoffset = 0, bool centered = true, const SMALL_RECT& root = { -1 });

// ------------------------------------
// ------------------------------------
// ------------------------------------


/**
 * Restores a previously saved state of the game grid from a file.
 * @param g The grid to restore into.
 * @param filename The name of the file to restore from.
 */
void restaure(Grille& g, std::string filename);

/**
 * Saves the current state of the game grid to a file.
 * @param g The grid to save.
 * @param filename The name of the file to save to.
 */
void sauve(const Grille& g, std::string filename);

// pour les fonctions mises à disposition
int nouvelle(const Grille& g), place(const Grille& g);
void resetRand(bool mode);

#endif