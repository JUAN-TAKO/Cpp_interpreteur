#include <iostream>
using namespace std;
#include "Interpreteur.h"
#include "Exceptions.h"

int main(int argc, char* argv[]) {
  string nomFich;
  if (argc != 2) {
    cout << "Usage : " << argv[0] << " nom_fichier_source" << endl << endl;
    cout << "Entrez le nom du fichier que voulez-vous interpréter : ";
    getline(cin, nomFich);
  } else
    nomFich = argv[1];
  ifstream fichier(nomFich.c_str());
  try {
    Interpreteur interpreteur(fichier);
    interpreteur.analyse();
    // Si pas d'exception levée, l'analyse syntaxique a réussi
    cout << endl << "================ Syntaxe Correcte" << endl;
    // On affiche le contenu de la table des symboles avant d'exécuter le programme
    //cout << endl << "================ Table des symboles avant exécution : " << interpreteur.getArbre();
    cout << endl << "================ Execution de l'arbre" << endl;
    // On exécute le programme si l'arbre n'est pas vide
    Value ret = interpreteur.executer();
    if((int)ret == 0){
        std::cout << "Succes" << std::endl;
    }
    else{
        std::cout << "Erreur" << std::endl;
    }
    cout << endl << "================ Conversion en python" << endl;
    interpreteur.convertir_python(cout);

    // Et on vérifie qu'il a fonctionné en regardant comment il a modifié la table des symboles
    //cout << endl << "================ Table des symboles apres exécution : " << interpreteur->getArbre();
  } catch (InterpreteurException & e) {
    cout << "Erreur : " << e.what() << endl;
  }
  return 0;
}
