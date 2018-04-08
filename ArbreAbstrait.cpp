#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"


////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

SymboleValue* NoeudSeqInst::cherche(const Symbole& s){
  NoeudSeqInst* p = m_parent;
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
SymboleValue* NoeudSeqInst::chercheAjoute(const Symbole& s){
  SymboleValue* v = cherche(s);
  if(!v){
    v = m_table.chercheAjoute(s);
  }
  return v;
}

NoeudSeqInst::NoeudSeqInst(NoeudSeqInst* parent) : m_instructions(){
  setParent(parent);
}

Value NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0;
}
void NoeudSeqInst::breakExec(){
  exec = false;
}
void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr){
    instruction->setParent(this);
    m_instructions.push_back(instruction);
  }
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstReturn
////////////////////////////////////////////////////////////////////////////////


NoeudInstReturn::NoeudInstReturn(Noeud* returnValue) : m_return(returnValue){}
Value NoeudInstReturn::executer(){
  NoeudSeqInst* p = m_parent;
  while(p->getSeq()->getSeq() != nullptr){
    p->breakExec();
    p = p->getSeq();
  }
  p->breakExec();
  p->chercheAjoute(Symbole("__func_return_val__"))->setValeur(m_return->executer());
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstBreak
////////////////////////////////////////////////////////////////////////////////

NoeudInstBreak::NoeudInstBreak(){}
Value NoeudInstBreak::executer(){
  NoeudSeqInst* p = m_parent;
  while(!dynamic_cast<NoeudBoucle*>(p)){
    p->breakExec();
    p = p->getSeq();
  }
  p->breakExec();
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(NoeudSeqInst* parent, Noeud* variable, Noeud* expression)
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

NoeudOperateurBinaire::NoeudOperateurBinaire(NoeudSeqInst* parent, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
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

NoeudFonction::NoeudFonction(NoeudSeqInst* parent, const std::vector<Symbole>& parametres) : NoeudSeqInst(parent)
{
  for(auto p : parametres){
    m_parametres.push_back((Noeud*)m_table.chercheAjoute(p));
  }
}
Value NoeudFonction::executer(){
  for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return m_table.cherche(Symbole("__func_return_val__"))->getValue();
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(NoeudSeqInst* parent)
: state(true)
{
  setParent(parent);
}

Value NoeudInstSiRiche::executer() {
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

NoeudInstRepeter::NoeudInstRepeter(NoeudSeqInst* parent)
{
  setParent(parent);
}
Value NoeudInstRepeter::executer(){
    do{
        for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
          m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    }
    while(exec && !m_condition->executer());
    return 0;
}

void NoeudInstRepeter::init(Noeud* condition){
  m_condition = condition;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstUntil
////////////////////////////////////////////////////////////////////////////////

NoeudInstUntil::NoeudInstUntil(NoeudSeqInst* parent)
{
  setParent(parent);
}

Value NoeudInstUntil::executer(){
  while(exec && !m_condition->executer())
    for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
      m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0;
}
void NoeudInstUntil::init(Noeud* condition){
  m_condition = condition;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(NoeudSeqInst* parent) 
{
  setParent(parent);
}

Value NoeudInstTantQue::executer() {
  while (exec && m_condition->executer())
    for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
      m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0;
}
void NoeudInstTantQue::init(Noeud* condition){
  m_condition = condition;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstDoWhile
////////////////////////////////////////////////////////////////////////////////

NoeudInstDoWhile::NoeudInstDoWhile(NoeudSeqInst* parent)
{
  setParent(parent);
}

void NoeudInstDoWhile::init(Noeud* condition){
  m_condition = condition;
}

Value NoeudInstDoWhile::executer(){
  do{
    for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
      m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  }while(exec && m_condition->executer());
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(NoeudSeqInst* parent)
{
  setParent(parent);
}


Value NoeudInstPour::executer() {
  for (m_initialisation->executer(); exec && m_condition->executer();m_iteration->executer())
      for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0;
}

void NoeudInstPour::init(Noeud* initialisation, Noeud* condition, Noeud* iteration){
  m_initialisation = initialisation;
  m_condition = condition;
  m_iteration = iteration;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPrint
////////////////////////////////////////////////////////////////////////////////

NoeudInstPrint::NoeudInstPrint(NoeudSeqInst* parent){
  setParent(parent);
}

void NoeudInstPrint::ajoute(Noeud* val){
  m_vals.push_back(val);
}

Value NoeudInstPrint::executer(){
  for(auto val : m_vals){
    std::cout << val->executer();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstScan
////////////////////////////////////////////////////////////////////////////////
/*
NoeudInstScan::NoeudInstScan(){}

void NoeudInstScan::ajouter(Noeud* var){
  m_vars.push_back(var);
}

Value NoeudInstScan::executer(){
  for(auto var : m_vars){
    Value v;
    std::cin >> v;
  }
}*/