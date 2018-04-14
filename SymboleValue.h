#ifndef SYMBOLEVALUE_H
#define SYMBOLEVALUE_H

#include <string>
#include <iostream>
#include "Symbole.h"
#include "Value.h"
#include "ArbreAbstrait.h"




class SymboleValue : public Symbole, public Noeud{  //  et c'est aussi une feuille de l'arbre abstrait
public:
	  SymboleValue(const Symbole & s); // Construit un symbole valué à partir d'un symbole existant s
	  ~SymboleValue( ) {}
	  Value executer();         // exécute le SymboleValue (revoie sa valeur !)
	  std::ostream& convertir_python(std::ostream& out, int& indent);
	  inline void setValeur(Value valeur)    { this->m_valeur=valeur; m_defini=true;  } // accesseur
	  inline bool estDefini()              { return m_defini;                       } // accesseur
          inline int getType() const{
              return m_valeur.getType();
          }
          inline Value getValue() const{
              return m_valeur;
          }
	  friend ostream & operator << (ostream & cout, const SymboleValue & symbole); // affiche un symbole value sur cout

private:
	  bool m_defini;	// indique si la valeur du symbole est définie
	  Value m_valeur;	// valeur du symbole si elle est définie, zéro sinon

};

#endif /* SYMBOLEVALUE_H */
