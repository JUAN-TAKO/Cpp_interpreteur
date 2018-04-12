#include "SymboleValue.h"
#include "Exceptions.h"
#include <stdlib.h>
#include <string>
SymboleValue::SymboleValue(const Symbole & s) : Noeud(nullptr)
, Symbole(s.getChaine()) {
  if (s == "<ENTIER>") {
    m_valeur = atoi(s.getChaine().c_str()); // c_str convertit une string en char*
    m_defini = true;
  }else if(s == "<REEL>"){
      m_defini = true;
      m_valeur = std::stof(s.getChaine().c_str());
  }else if(s == "<CHAINE>"){
      m_defini = true;
      m_valeur = s.getChaine();
      m_valeur = ((std::string)m_valeur).substr(1, ((std::string)m_valeur).size() - 2);
  }else{
    m_defini = false;
  }
}

Value SymboleValue::executer() {
  if (!m_defini){
    std::cout << "chaine : " << getChaine() << std::endl;
    std::cout << getSeq()->getTable() << std::endl;
    throw IndefiniException(); // on lève une exception si valeur non définie
  }
  return m_valeur;
}

ostream & operator<<(ostream & cout, const SymboleValue & symbole) {
  cout << (Symbole) symbole << "\t\t - Valeur=";
  
  if (symbole.m_defini) cout << symbole.getValue() << " ";
  else cout << "indefinie ";
  return cout;
}


