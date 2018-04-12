#include "ArbreAbstrait.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////
/*
SymboleValue* NoeudSeqInst::cherche(const Symbole& s, bool except){

  NoeudSeqInst* p = getSeq();
  
  SymboleValue* val = nullptr;
  TableSymboles* table = &getTable();
  if(!except){
    try{
      val = table->cherche(s);
      return val;
    }
    catch(...){}
  }
  else{
    val = table->cherche(s);
    return val;
  }
  while(p != nullptr){
    table = &(p->getTable());
    if(!except){
      try{
        val = table->cherche(s);
        return val;
      }
      catch(...){}
    }else{
      val = table->cherche(s);
      return val;
    }
    p = p->getSeq();
  }
  return nullptr;
}
SymboleValue* NoeudSeqInst::chercheAjoute(Symbole s){
  SymboleValue* v = cherche(s);
  if(!v){
    v = getTable().chercheAjoute(s);
  }
  return v;
}
*/
NoeudSeqInst::NoeudSeqInst(NoeudSeqInst* parent, NoeudFonction* func) : Noeud(parent, func), exec(true){}

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


NoeudInstReturn::NoeudInstReturn(NoeudSeqInst* parent, NoeudFonction* func, Noeud* returnValue) : Noeud(parent, func), m_return(returnValue){}
Value NoeudInstReturn::executer(){
  NoeudSeqInst* p = getSeq();
  while(p->getSeq()->getSeq() != nullptr){
    p->breakExec();
    p = p->getSeq();
  }
  p->breakExec();
  Value v = m_return->executer();
  getFunc()->m_globals->chercheAjoute(Symbole("__func_return_val__"))->setValeur(v);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstBreak
////////////////////////////////////////////////////////////////////////////////

NoeudInstBreak::NoeudInstBreak(NoeudSeqInst* parent, NoeudFonction* func) : Noeud(parent, func){}
Value NoeudInstBreak::executer(){
  NoeudSeqInst* p = getSeq();
  while(!dynamic_cast<NoeudBoucle*>(p)){
    p->breakExec();
    p = p->getSeq();
  }
  p->breakExec();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(NoeudSeqInst* parent, NoeudFonction* func, Noeud* variable, Noeud* expression)
: Noeud(parent, func), m_variable(variable), m_expression(expression){}

Value NoeudAffectation::executer() {
  Value valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(NoeudSeqInst* parent, NoeudFonction* func, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: Noeud(parent, func), m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit){}

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

NoeudFonction::NoeudFonction(NoeudSeqInst* parent, TableSymboles* globals)
: NoeudSeqInst(parent, nullptr)
, m_globals(globals)
, m_stack()
{
    m_stack.push(TableSymboles());
}

Symbole NoeudFonction::chercheAjoute(Symbole s){
    try{
        m_globals->cherche(s);
    }
    catch(...){
        vector<Symbole>::iterator i;
        i = m_variables.begin();
        while (i < m_variables.end() && (*i).getChaine() < s.getChaine()) i++;
        if (i == m_variables.end() || (*i).getChaine() != s.getChaine()) // si pas trouvé...
          i = m_variables.insert(i, s);
        return *i;
    }
    return s;
    
}

void NoeudFonction::ajoute(Symbole s, bool parametre){
  m_variables.push_back(s);
  if(parametre)
    m_parametres.push_back(&m_variables.back());
}
Value NoeudFonction::executer(){
  m_globals->chercheAjoute(Symbole("__func_return_val__"))->setValeur(0);
  for (unsigned int i = 0; i < m_instructions.size() && exec; i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  Value v = m_globals->cherche(Symbole("__func_return_val__"))->getValue();
  m_stack.pop();
  return v;
}


////////////////////////////////////////////////////////////////////////////////
// NoeudVariable
////////////////////////////////////////////////////////////////////////////////

Value NoeudVariable::executer(){
    SymboleValue* s = getFunc()->getTable().chercheAjoute(m_symbole);
    return s->executer();
}

////////////////////////////////////////////////////////////////////////////////
// NoeudCall
////////////////////////////////////////////////////////////////////////////////

void NoeudCall::ajoute(Noeud* arg){
  m_args.push_back(arg);
}
Value NoeudCall::executer(){
  NoeudFonction* f = (NoeudFonction*)m_target_func->executer();
  if(f->m_parametres.size() != m_args.size()){
    throw SyntaxeException("Nombre de paramètres incorrect");
  }
  std::vector<Value> vals; 
  for(unsigned int i = 0; i < f->m_parametres.size(); i++)
    vals.push_back(m_args[i]->executer());
  f->push();
  for(unsigned int i = 0; i < f->m_parametres.size(); i++)
    f->getTable().chercheAjoute(*f->m_parametres[i])->setValeur(vals[i]);
  
  return f->executer();
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(NoeudSeqInst* parent, NoeudFonction* func)
: Noeud(parent, func), state(true){}

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

NoeudInstRepeter::NoeudInstRepeter(NoeudSeqInst* parent, NoeudFonction* func)
: NoeudBoucle(parent, func){}
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

NoeudInstUntil::NoeudInstUntil(NoeudSeqInst* parent, NoeudFonction* func)
: NoeudBoucle(parent, func){}

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

NoeudInstTantQue::NoeudInstTantQue(NoeudSeqInst* parent, NoeudFonction* func)
: NoeudBoucle(parent, func){}

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

NoeudInstDoWhile::NoeudInstDoWhile(NoeudSeqInst* parent, NoeudFonction* func)
: NoeudBoucle(parent, func){}

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

NoeudInstPour::NoeudInstPour(NoeudSeqInst* parent, NoeudFonction* func)
: NoeudBoucle(parent, func){}


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

NoeudInstPrint::NoeudInstPrint(NoeudSeqInst* parent, NoeudFonction* func)
: Noeud(parent, func){}

void NoeudInstPrint::ajoute(Noeud* val){
  m_vals.push_back(val);
}

Value NoeudInstPrint::executer(){
  for(auto val: m_vals){
    std::cout << val->executer();
  }
  std::cout << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstScan
////////////////////////////////////////////////////////////////////////////////
/*
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
*/