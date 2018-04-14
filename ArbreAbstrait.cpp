#include "ArbreAbstrait.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

std::string tab(int n){
  if(n < 0)
    n = 0;
  return std::string(n, '\t');
}
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

std::ostream& NoeudSeqInst::convertir_python(std::ostream& out, int& indent){
  out << tab(++indent);
  for (unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl << tab(indent);
  }
  return out << std::endl << tab(--indent);
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstReturn
////////////////////////////////////////////////////////////////////////////////


NoeudInstReturn::NoeudInstReturn(NoeudSeqInst* parent, NoeudFonction* func, Noeud* returnValue) : Noeud(parent, func), m_return(returnValue){}
Value NoeudInstReturn::executer(){
  Value v = m_return->executer();
  getFunc()->getReturn()->setValeur(v);
  NoeudSeqInst* p = getSeq();
  while(p->getSeq()->getSeq() != nullptr){
    p->breakExec();
    p = p->getSeq();
  }
  p->breakExec();
  return 0;
}
std::ostream& NoeudInstReturn::convertir_python(std::ostream& out, int& indent){
  out << "return ";
  return m_return->convertir_python(out, indent);
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
std::ostream& NoeudInstBreak::convertir_python(std::ostream& out, int& indent){
  return out << "break";
}
////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* variable, Noeud* expression)
: Noeud(parent, func), m_variable(variable), m_expression(expression){}

Value NoeudAffectation::executer() {
  Value valeur = m_expression->executer(); // On exécute (évalue) l'expression
  m_variable->setValue(valeur); // On affecte la variable
  return valeur; // La valeur renvoyée ne représente rien !
}

std::ostream& NoeudAffectation::convertir_python(std::ostream& out, int& indent){
  out << m_variable->getSymbole().getChaine() << " = ";
  return m_expression->convertir_python(out, indent);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(NoeudSeqInst* parent, NoeudFonction* func, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: Noeud(parent, func), m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit){}

Value NoeudOperateurBinaire::executer() {
  Value og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr)
    od = m_operandeDroit->executer(); // On évalue l'opérande droit

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

std::ostream& NoeudOperateurBinaire::convertir_python(std::ostream& out, int& indent){
  m_operandeGauche->convertir_python(out, indent);
  out << " " << m_operateur.getChaine() << " ";
  return m_operandeDroit->convertir_python(out, indent);
}
////////////////////////////////////////////////////////////////////////////////
// NoeudFonction
////////////////////////////////////////////////////////////////////////////////

NoeudFonction::NoeudFonction(NoeudSeqInst* parent, TableSymboles* globals, Symbole self)
: NoeudSeqInst(parent, nullptr)
, m_globals(globals)
, m_stack()
, m_parametres(0)
, m_self(self)
{
    //m_stack.push_back(TableSymboles());
}

Symbole NoeudFonction::chercheAjoute(Symbole s){
    try{
        m_globals->cherche(s);
    }
    catch(...){
        vector<Symbole>::iterator i;
        i = m_variables.begin() + m_parametres;
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
    m_parametres++;
}

SymboleValue* NoeudFonction::getReturn(){
    return getTable().chercheAjoute(Symbole("__func_return_val__"));
}
Value NoeudFonction::executer(){
  
  for (unsigned int i = 0; i < m_instructions.size() && exec; i++){
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  }
  Value v;
  try{
    v = getTable().cherche(Symbole("__func_return_val__"))->getValue();
  }catch(...){}
  m_stack.pop();
  return v;
}

std::ostream& NoeudFonction::convertir_python(std::ostream& out, int& indent){
  out << "def " << m_self.getChaine() << "(";
  for(int i = 0; i < m_parametres; i++){
    out << m_variables[i].getChaine();
    if(i < m_parametres-1)
      out << ", ";
  }
  out << "):" << std::endl << tab(++indent);
  for (unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << endl << tab(indent);
  }

  out << std::endl << tab(--indent);
  return out;// << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
// NoeudVariable
////////////////////////////////////////////////////////////////////////////////

Value NoeudVariable::executer(){
    SymboleValue* s;
    try{
      s = getFunc()->getGlobals()->cherche(m_symbole);
    }
    catch(...){
      s = getFunc()->getTable().chercheAjoute(m_symbole);
    }
    
    return s->executer();
}
void NoeudVariable::setValue(Value v){
    SymboleValue* s;
    try{
      s = getFunc()->getGlobals()->cherche(m_symbole);
    }
    catch(...){
      s = getFunc()->getTable().chercheAjoute(m_symbole);
    }
    s->setValeur(v);
}

std::ostream& NoeudVariable::convertir_python(std::ostream& out, int& indent){
  return out << m_symbole.getChaine();
}
////////////////////////////////////////////////////////////////////////////////
// NoeudCall
////////////////////////////////////////////////////////////////////////////////

void NoeudCall::ajoute(Noeud* arg){
  m_args.push_back(arg);
}
Value NoeudCall::executer(){
  NoeudFonction* f = m_target_func->executer();
  if(f->m_parametres != m_args.size()){
    throw SyntaxeException("Nombre de paramètres incorrect");
  }
  std::vector<Value> vals; 
  for(unsigned int i = 0; i < f->m_parametres; i++)
    vals.push_back(m_args[i]->executer());
  f->push();
  for(unsigned int i = 0; i < f->m_parametres; i++)
    f->getTable().chercheAjoute(f->m_variables[i])->setValeur(vals[i]);
  

  return f->executer();
}
std::ostream& NoeudCall::convertir_python(std::ostream& out, int& indent){
  m_target_func->convertir_python(out, indent);
  out << "(";
  for(unsigned int i = 0; i < m_args.size(); i++){
    m_args[i]->convertir_python(out, indent);
    if(i < m_args.size()-1)
      out << ", ";
  }
  return out << ")";
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

std::ostream& NoeudInstSiRiche::convertir_python(std::ostream& out, int& indent){
  out << "if ";
  m_conditions[0]->convertir_python(out, indent);
  out << ":" << std::endl;
  m_sequences[0]->convertir_python(out, indent);
  for(unsigned int i = 1; i < m_conditions.size(); i++){
    out << "elif ";
    m_conditions[i]->convertir_python(out, indent);
    out << ":" << std::endl;
    m_sequences[i]->convertir_python(out, indent);
  }
  //out << std::endl << tab(indent);
  if(m_conditions.size() < m_sequences.size()){
    out << "else:" << std::endl;
    m_sequences.back()->convertir_python(out, indent);
  }
  return out;
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

std::ostream& NoeudInstRepeter::convertir_python(std::ostream& out, int& indent){
  out << "while True:" << std::endl << tab(++indent);
  for(unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl;
  }
  out << "if ";
  m_condition->convertir_python(out, indent);
  out << std::endl << tab(++indent) << "break" << std::endl;
  indent -= 2;
  return out;
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

std::ostream& NoeudInstUntil::convertir_python(std::ostream& out, int& indent){
  out << "while Not(";
  m_condition->convertir_python(out,indent);
  out << "):" << std::endl << tab(++indent);
  for(unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl << tab(indent);
  }
  --indent;
  return out;
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

std::ostream& NoeudInstTantQue::convertir_python(std::ostream& out, int& indent){
  out << "while ";
  m_condition->convertir_python(out,indent);
  out << ":" << std::endl << tab(++indent);
  for(unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl << tab(indent);
  }
  --indent;
  return out;
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

std::ostream& NoeudInstDoWhile::convertir_python(std::ostream& out, int& indent){
  out << "while True:" << std::endl;
  ++indent;
  for(unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl << tab(indent);
  }
  out << "if Not(";
  m_condition->convertir_python(out, indent);
  out << ")" << std::endl << tab(++indent) << "break" << std::endl;
  indent -=2 ;
  return out;
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

std::ostream& NoeudInstPour::convertir_python(std::ostream& out, int& indent){
  m_initialisation->convertir_python(out, indent);
  out << std::endl << tab(indent) << "while ";
  m_condition->convertir_python(out,indent);
  out << ":" << std::endl << tab(++indent);
  for(unsigned int i = 0; i < m_instructions.size(); i++){
    m_instructions[i]->convertir_python(out, indent);
    out << std::endl << tab(indent);
  }
  m_iteration->convertir_python(out, indent);
  return out << std::endl << tab(--indent);
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

std::ostream& NoeudInstPrint::convertir_python(std::ostream& out, int& indent){
  out << "print(";
  for(unsigned int i = 0; i < m_vals.size(); i++){
    m_vals[i]->convertir_python(out, indent);
    if(i < m_vals.size()-1)
      out << ", ";
  }
  return out << ")";
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstScan
////////////////////////////////////////////////////////////////////////////////

NoeudInstScanI::NoeudInstScanI(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var) : Noeud(parent, func), m_var(var){}

Value NoeudInstScanI::executer(){
  int v;
  cin >> v;
  m_var->setValue(v);
  return v;
}
std::ostream& NoeudInstScanI::convertir_python(std::ostream& out, int& indent){
  m_var->convertir_python(out, indent);
  return out << " = input(\"\")";
}

NoeudInstScanF::NoeudInstScanF(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var) : Noeud(parent, func), m_var(var){}

Value NoeudInstScanF::executer(){
  float v;
  cin >> v;
  m_var->setValue(v);
  return v;
}

std::ostream& NoeudInstScanF::convertir_python(std::ostream& out, int& indent){
  m_var->convertir_python(out, indent);
  return out << " = input(\"\")";
}

NoeudInstScanS::NoeudInstScanS(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var) : Noeud(parent, func), m_var(var){}

Value NoeudInstScanS::executer(){
  std::string v;
  cin >> v;
  m_var->setValue(v);
  return v;
}

std::ostream& NoeudInstScanS::convertir_python(std::ostream& out, int& indent){
  m_var->convertir_python(out, indent);
  return out << " = raw_input(\"\")";
}