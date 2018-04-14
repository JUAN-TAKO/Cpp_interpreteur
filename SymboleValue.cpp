#include "SymboleValue.h"
#include "Exceptions.h"
#include <stdlib.h>
#include <string>
SymboleValue::SymboleValue(const Symbole & s)
: Symbole(s.getChaine())
, Noeud(nullptr, nullptr){
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
    throw IndefiniException(); // on lève une exception si valeur non définie
  }
  return m_valeur;
}

std::ostream& SymboleValue::convertir_python(std::ostream& out, int& indent){
  if(m_valeur.getType() == 2)
    out << "\"" << (std::string)m_valeur << "\"";
  else if(m_valeur.getType() == 3)
    out << ((NoeudFonction*)m_valeur)->m_self.getChaine();
  else
    out << m_valeur;
  return out;
}
ostream & operator<<(ostream & cout, const SymboleValue & symbole) {
  cout << (Symbole) symbole << "\t\t - Valeur=";
  
  if (symbole.m_defini) cout << symbole.getValue() << " ";
  else cout << "indefinie ";
  return cout;
}


