#include <cassert>
#include <string>
#include <chrono>
#include <iomanip>

#include "../include/jeu.h"
#include "../include/keyboard.h"

void teste3() {
    Grille g;
    std::vector<std::vector<int>> v =
    { { 8, 4, 4, 4 }, { 4, 0, 4, 4 }, { 4, 4, 4, 4 }, { 4, 0, 0, 4} };
    std::cout << "Debut du test3\n";
    assert(charge(g, v, 32, 7)); // charge et teste le resultat
    affiche(g);

    gauche(g); affiche(g);
    droite(g); affiche(g);
    haut(g);   affiche(g);
    gauche(g); affiche(g);
    haut(g);   affiche(g);
    bas(g);    affiche(g);
    assert(score(g) == 128 and succes(g));
    std::cout << "Teste3 OK" << std::endl;
}

void teste3bis() {
    Grille g;
    std::vector<std::vector<int>> v =
    { { 2, 4, 0, 0, 4 }, { 4, 0, 4, 8, 8 }, { 0, 0, 4, 0, 4 }, { 2, 4, 0, 0, 4},
      { 2, 4, 0, 4, 2 }
    };
    std::cout << "Debut du teste3bis" << std::endl;
    assert(charge(g, v, 64, 10)); // avec que des tuiles de valeur 2
    affiche(g);

    gauche(g); affiche(g);
    assert(score(g) == 56 and gauche(g) == -1);
    std::cout << "teste3bis-I: OK" << std::endl;
    // On repart de la meme configuration initiale mais on essaye
    // un autre deplacement
    Grille g2;
    assert(charge(g2, v, 8, 0));
    affiche(g2);

    haut(g2);   affiche(g2);
    bas(g2);    affiche(g2);
    droite(g2); affiche(g2);
    assert(score(g2) == 68 and vides(g2) == 16);
    std::cout << "teste3bis-II: OK" << std::endl;
    std::cout << "Fin de teste3bis" << std::endl;
}

void teste4() {
    Grille g;
    std::vector<std::vector<int>> v =
        // une grille bloqu�e.
    { { 128, 8, 2, 4 }, { 4, 16, 4, 8 }, { 8, 2, 8, 4}, { 2, 4, 16, 32} };
    std::cout << "Debut du test4" << std::endl;
    assert(charge(g, v, 2048, 7));
    affiche(g);

    assert(gauche(g) == -1 and droite(g) == -1 and
        haut(g) == -1 and droite(g) == -1);
    affiche(g);
    std::cout << "Teste4 OK" << std::endl;
}

void teste5() {
    Grille g;
    std::cout << "Debut du test5" << std::endl;
    assert(init(g, 5, 8, 0)); // on ne cree que des tuiles de valeur 4
    affiche(g);

    haut(g);   affiche(g);
    droite(g); affiche(g);
    assert(score(g) == 8 and succes(g));
    std::cout << "Teste5 OK" << std::endl;
}

void testFV() {
    teste3();
    teste3bis();
    teste4();
    teste5();
}



int main()
{
    resetRand(true);
    //testFV();
    launch();
    return 0;
}