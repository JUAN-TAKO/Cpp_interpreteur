#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"



SymboleValue* Noeud::cherche(const Symbole& s){
  Noeud* p = m_parent;
  SymboleValue* val = nullptr;
  TableSymboles& table = m_table;
  
  val = table.cherche(s);
    if(val)
      return val;
  while(p != nullptr){
    table = p->m_table;
    val = table.cherche(s);
    if(val)
      return val;
    p = p->m_parent;
  }
  
}
SymboleValue* Noeud::chercheAjoute(const Symbole& s){
  SymboleValue* v = cherche(s);
  if(!v){
    v = m_table.chercheAjoute(s);
  }
  return v;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst(Noeud* parent) : m_instructions(){
  setParent(parent);
}

Value NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr){
    instruction->setParent(this);
    m_instructions.push_back(instruction);
  }
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* parent, Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression){
  setParent(parent);
}

Value NoeudAffectation::executer() {
  Value valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Noeud* parent, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit){
  setParent(parent);
}

Value NoeudOperateurBinaire::executer() {
  Value og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit

  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "and") valeur = (og && od);
  else if (this->m_operateur == "or") valeur = (og || od);
  else if (this->m_operateur == "not") valeur = (!og);
  else if (this->m_operateur == "xor") valeur = (og != od);
  else if (this->m_operateur == "%") {
    if (od == Value(0)) throw DivParZeroException();
    valeur = og % od;
  }
  else if (this->m_operateur == "/") {
    if (od == Value(0)) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudFonction
////////////////////////////////////////////////////////////////////////////////

NoeudFonction::NoeudFonction(Noeud* parent, Noeud* sequence, const std::vector<Symbole>& parametres)
: m_sequence(sequence)
{
  for(auto p : parametres){
    m_parametres.push_back((Noeud*)m_table.chercheAjoute(p));
  }
}

Value NoeudFonction::executer(){

}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(Noeud* parent, Noeud* condition, Noeud* sequence)
: state(true){
  m_conditions.push_back(condition);
  m_sequences.push_back(sequence);
}

Value NoeudInstSiRiche::executer() {
    //parcours de toutes les séquence jusquà : soit executer la premiere instructions valide, 
    //soit si aucune conditions est validé on execute l'instuction else en detectant si il y a plus de sequence que de condition.
    unsigned int i;
    for(i = 0; i < m_conditions.size() && !m_conditions[i]->executer(); i++);
    if(i < m_conditions.size() || m_conditions.size() < m_sequences.size())
      m_sequences[i]->executer();
    return 0;
}

void NoeudInstSiRiche::ajoute(Noeud* instruction){
    if(state)
        m_sequences.push_back(instruction);
    else
        m_conditions.push_back(instruction);
    state = !state;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* parent, Noeud* sequence, Noeud* condition)
: m_sequence(sequence)
, m_condition(condition)
{}
Value NoeudInstRepeter::executer(){
    do{
        m_sequence->executer();
    }
    while(!m_condition->executer());
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstUntil
////////////////////////////////////////////////////////////////////////////////

NoeudInstUntil::NoeudInstUntil(Noeud* parent, Noeud* sequence, Noeud* condition)
: m_sequence(sequence), m_condition(condition){
}

Value NoeudInstUntil::executer(){
  while(!m_condition->executer()) m_sequence->executer();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* parent, Noeud* sequence, Noeud* condition) 
: m_sequence(sequence), m_condition(condition) {
}

Value NoeudInstTantQue::executer() {
  while (m_condition->executer()) m_sequence->executer();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstDoWhile
////////////////////////////////////////////////////////////////////////////////

NoeudInstDoWhile::NoeudInstDoWhile(Noeud* parent, Noeud* condition, Noeud* sequence)
: m_sequence(sequence), m_condition(condition){
}

Value NoeudInstDoWhile::executer(){
  do{
    m_sequence->executer();
  }while(m_condition->executer());
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* parent, Noeud* initialisation, Noeud* condition, Noeud* iteration, Noeud* sequence) 
: m_initialisation(initialisation), m_condition(condition), m_iteration(iteration), m_sequence(sequence) {
}


Value NoeudInstPour::executer() {
  for (m_initialisation->executer();m_condition->executer();m_iteration->executer()){
      m_sequence->executer();
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstPrint
////////////////////////////////////////////////////////////////////////////////

NoeudInstPrint::NoeudInstPrint(Noeud* parent){}

void NoeudInstPrint::ajoute(Noeud* val){
  m_vals.push_back(val);
}

Value NoeudInstPrint::executer(){
  for(auto val: m_vals){
    std::cout << val->executer();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstScanI
////////////////////////////////////////////////////////////////////////////////

NoeudInstScanI::NoeudInstScanI(Noeud* var){
    Value v;
    v.scanI();
    m_affect = new NoeudAffectation(var,v);
}

Value NoeudInstScanI::executer(){
  return m_affect->executer();
}

NoeudInstScanI::NoeudInstScanF(Noeud* var){
    Value v;
    v.scanF();
    m_affect = new NoeudAffectation(var,v);
}

Value NoeudInstScanF::executer(){
  return m_affect->executer();
}

NoeudInstScanI::NoeudInstScanS(Noeud* var){
    Value v;
    v.scanS();
    m_affect = new NoeudAffectation(var,v);
}

Value NoeudInstScanS::executer(){
  return m_affect->executer();
}
