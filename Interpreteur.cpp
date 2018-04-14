#include "Interpreteur.h"

using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_valid(false){}

void Interpreteur::analyse() {
  m_valid = true;
  programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symbNoeudInstPour(init,cond,iter,seq)oles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

NoeudSeqInst* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    m_arbre = new NoeudSeqInst(nullptr, nullptr);
    while(m_lecteur.getSymbole() == "def"){
        m_arbre->ajoute(fonction(m_arbre));
        m_func = nullptr;
    }
    
    tester("<FINDEFICHIER>");
  return m_arbre;
}

Value Interpreteur::executer(){
    if(!m_valid)
      return -2;
    SymboleValue* main = m_globals.cherche(Symbole("main"));
    if(main){
        //NoeudFonction* nf = main->getValue();
        NoeudCall* mainCall = new NoeudCall(nullptr, nullptr, main);
        return mainCall->executer();
    }
    else{
        return -1;
    }
}

void Interpreteur::convertir_python(std::ostream& out){
  if(!m_valid){
    std::cout << "erreur de syntaxe, conversion impossible" << std::endl;
    return;
  }
  int indent = -1;
  out << std::setfill(' ');
  m_arbre->convertir_python(out, indent);
  out << "main()" << std::endl;
}

Noeud* Interpreteur::fonction(NoeudSeqInst* parent){
    testerEtAvancer("def");
    SymboleValue* funcV;
    if(m_lecteur.getSymbole() == "<VARIABLE>"){
        funcV = m_globals.chercheAjoute(m_lecteur.getSymbole());
    }
    testerEtAvancer("<VARIABLE>");
    NoeudFonction* func = new NoeudFonction(parent, &m_globals, funcV->getChaine());
    testerEtAvancer("(");
    if(m_lecteur.getSymbole() == "<VARIABLE>"){
      func->ajoute(m_lecteur.getSymbole(), true);
      m_lecteur.avancer();
      while(m_lecteur.getSymbole() == ","){
        m_lecteur.avancer();
        tester("<VARIABLE>");
        func->ajoute(m_lecteur.getSymbole(), true);
        m_lecteur.avancer();
      }
    }
    testerEtAvancer(")");
    m_func = func;
    
    seqInst(func);
    testerEtAvancer("end");
    funcV->setValeur(func);
    return func;
}

NoeudSeqInst* Interpreteur::seqInst(NoeudSeqInst* sequence) {
  // <seqInst> ::= <inst> { <inst> }
  do {
    try{

      sequence->ajoute(inst(sequence));
    }
    catch(SyntaxeException e){
      m_valid = false;
      std::cout << e.what() << std::endl;
    }
  
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "if"    ||
           m_lecteur.getSymbole() == "while"      || m_lecteur.getSymbole() == "repeat"||
           m_lecteur.getSymbole() == "for"        || m_lecteur.getSymbole() == "until" ||
           m_lecteur.getSymbole() == "do"         || m_lecteur.getSymbole() == "print" ||
           m_lecteur.getSymbole() == "return"     || m_lecteur.getSymbole() == "break" ||
           m_lecteur.getSymbole() == "scanI"      || m_lecteur.getSymbole() == "scanF" ||
           m_lecteur.getSymbole() == "scanS");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst(NoeudSeqInst* parent) {
  // <inst> ::= <affectation>  ; | <instSi>
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *aoc = variableOperation(parent);
    testerEtAvancer(";");
    return aoc;
  }
  else if (m_lecteur.getSymbole() == "if")
    return instSiRiche(parent);
  else if (m_lecteur.getSymbole() == "for")
    return instPour(parent);
  else if (m_lecteur.getSymbole() == "while")
    return instTantQue(parent);
  else if (m_lecteur.getSymbole() == "do")
    return instDoWhile(parent);
  else if (m_lecteur.getSymbole() == "until")
    return instUntil(parent);
  else if (m_lecteur.getSymbole() == "repeat")
    return instRepeter(parent);
  else if (m_lecteur.getSymbole() == "print")
    return instPrint(parent);
  else if (m_lecteur.getSymbole() == "return")
    return instReturn(parent);
  else if (m_lecteur.getSymbole() == "break")
    return instBreak(parent);
  else if (m_lecteur.getSymbole() == "scanI" || m_lecteur.getSymbole() == "scanF" || m_lecteur.getSymbole() == "scanS")
    return instScan(parent);
  // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
  else erreur("Instruction incorrecte");
  return nullptr;
}

Noeud* Interpreteur::instReturn(NoeudSeqInst* parent){
  testerEtAvancer("return");
  Noeud* ex = expression(parent);
  testerEtAvancer(";");
  return new NoeudInstReturn(parent, m_func, ex);
}
Noeud* Interpreteur::instBreak(NoeudSeqInst* parent){
  m_lecteur.avancer();
  testerEtAvancer(";");
  return new NoeudInstBreak(parent, m_func);
}
Noeud* Interpreteur::affectation(NoeudSeqInst* parent, Symbole s){
    if(!(s == "<VARIABLE>"))
      erreur("Variable incrorrecte");
    NoeudVariable* var = new NoeudVariable(parent, m_func, m_func->chercheAjoute(s));
    testerEtAvancer("=");
    Noeud* ex = expression(parent);             // On mémorise l'expression trouvée  
    return new NoeudAffectation(parent, m_func, var, ex); // On renvoie un noeud affectation
}

Noeud* Interpreteur::call(NoeudSeqInst* parent, Symbole s){
    NoeudVariable* var = new NoeudVariable(parent, m_func, s);
    NoeudCall* c = new NoeudCall(parent, m_func, var);
    testerEtAvancer("(");
    if(!(m_lecteur.getSymbole() == ")")){
      c->ajoute(expression(parent));
      while(m_lecteur.getSymbole() == ","){
        m_lecteur.avancer();
        c->ajoute(expression(parent));
      }
    }
    testerEtAvancer(")");
    return c;
}
Noeud* Interpreteur::variableOperation(NoeudSeqInst* parent) {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Symbole s = m_lecteur.getSymbole();
  m_lecteur.avancer();
  NoeudVariable* var = new NoeudVariable(parent, m_func, s);
  if(m_lecteur.getSymbole() == "("){
    return call(parent, s);
  }
  else if(m_lecteur.getSymbole() == "="){
    return affectation(parent, s);
  }
  else if(m_lecteur.getSymbole() == "++"){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("+"), var, m_globals.chercheAjoute(Symbole("1"))));
  }
  else if(m_lecteur.getSymbole() == "--"){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("-"), var, m_globals.chercheAjoute(Symbole("1"))));
  }
  else if(m_lecteur.getSymbole() == "+="){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("+"), var, expression(parent)));  
  }
  else if(m_lecteur.getSymbole() == "-="){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("-"), var, expression(parent)));
  }
  else if(m_lecteur.getSymbole() == "*="){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("*"), var, expression(parent)));
  }
  else if(m_lecteur.getSymbole() == "/="){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("/"), var, expression(parent)));
  }
  else if(m_lecteur.getSymbole() == "%="){
    m_lecteur.avancer();
    return new NoeudAffectation(parent, m_func, var, new NoeudOperateurBinaire(parent, m_func, Symbole("%"), var, expression(parent)));
  }
  else{
    return var;
  }

}


Noeud* Interpreteur::expression(NoeudSeqInst* parent) {

  Noeud* t = terme(parent);
  while(m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-"){
    Symbole operateur = m_lecteur.getSymbole();
    m_lecteur.avancer();
    Noeud* termeDroit = terme(parent);
    t = new NoeudOperateurBinaire(parent, m_func, operateur, t, termeDroit);
  }
  return t;
}

Noeud* Interpreteur::terme(NoeudSeqInst* parent){
  Noeud* fact = facteur(parent);
  while(m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" || m_lecteur.getSymbole() == "%"){
    Symbole operateur = m_lecteur.getSymbole();
    m_lecteur.avancer();
    Noeud* facteurDroit = facteur(parent);
    fact = new NoeudOperateurBinaire(parent, m_func, operateur, fact, facteurDroit);
  }
  return fact;
}

Noeud* Interpreteur::facteur(NoeudSeqInst* parent) {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<LITTERAL>") {
    fact = m_globals.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if(m_lecteur.getSymbole() == "<VARIABLE>"){
    fact = variableOperation(parent);
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(parent, m_func, Symbole("-"), m_globals.chercheAjoute(Symbole("0")), expBool(parent));
  } else if (m_lecteur.getSymbole() == "not") { // non <facteur>
    m_lecteur.avancer();
    fact = new NoeudOperateurBinaire(parent, m_func, Symbole("not"), m_globals.chercheAjoute(Symbole("0")), expBool(parent));
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expBool(parent);
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::expBool(NoeudSeqInst* parent){
  Noeud* rel = relationEt(parent);
  while(m_lecteur.getSymbole() == "or"){
    m_lecteur.avancer();
    Noeud* relationDroite = relationEt(parent);
    rel = new NoeudOperateurBinaire(parent, m_func, Symbole("or"), rel, relationDroite);
  }
  return rel;
}

Noeud* Interpreteur::relationEt(NoeudSeqInst* parent){
  Noeud* rel = relation(parent);
  while(m_lecteur.getSymbole() == "and"){
    m_lecteur.avancer();
    Noeud* relationDroite = relation(parent);
    rel = new NoeudOperateurBinaire(parent, m_func, Symbole("and"), rel, relationDroite);
  }
  return rel;
}

Noeud* Interpreteur::relation(NoeudSeqInst* parent){
  Noeud* expr = expression(parent);
  while(m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
        m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
        m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">="){
          Symbole operateur = m_lecteur.getSymbole();
          m_lecteur.avancer();
          Noeud* expressionDroite = expression(parent);
          expr = new NoeudOperateurBinaire(parent, m_func, operateur, expr, expressionDroite);
        }
  return expr;
}


Noeud* Interpreteur::instPour(NoeudSeqInst* parent){
    testerEtAvancer("for");
    Noeud* init;
    Noeud* cond;
    Noeud* iter;
    NoeudInstPour* pour = new NoeudInstPour(parent, m_func);
    if(m_lecteur.getSymbole() == "("){
      testerEtAvancer("(");
      if(m_lecteur.getSymbole() == ";")
        init = m_globals.chercheAjoute(Symbole("0"));
      else{
        Symbole s = m_lecteur.getSymbole();
        m_lecteur.avancer();
        init = affectation(pour, s);
      }
        
        
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ";")
        cond = m_globals.chercheAjoute(Symbole("1"));
      else
        cond = expBool(pour);
      
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ")")
        iter = m_globals.chercheAjoute(Symbole("0"));
      else
        iter = variableOperation(pour);
        
      testerEtAvancer(")");
    }
    else{
      tester("<VARIABLE>");
      NoeudVariable* var;
      Noeud* i;
      var = new NoeudVariable(parent, m_func, m_func->chercheAjoute(m_lecteur.getSymbole()));
      m_lecteur.avancer();
      testerEtAvancer("in");
      Noeud* n1 = facteur(pour);
      Noeud* n2 = nullptr;
      if(m_lecteur.getSymbole() == ":"){
        testerEtAvancer(":");
        n2 = expression(pour);
      }
      else{
        n2 = n1;
        n1 = m_globals.chercheAjoute(Symbole("0"));
      }
      if(m_lecteur.getSymbole() == "|"){
        testerEtAvancer("|");
        i = expression(parent);
        //m_lecteur.avancer();
      }
      else{
        i = m_globals.chercheAjoute(Symbole("1"));
      }
      init = new NoeudAffectation(pour, m_func, var, n1);
      if(i->executer() < Value(0))
        cond = new NoeudOperateurBinaire(pour, m_func, Symbole(">="), var, n2);
      else
        cond = new NoeudOperateurBinaire(pour, m_func, Symbole("<="), var, n2);
      iter = new NoeudAffectation(pour, m_func, var, new NoeudOperateurBinaire(pour, m_func, Symbole("+"), var, i));
    }
    pour->init(init, cond, iter);
    seqInst(pour);
    testerEtAvancer("end");
    return pour;
}

Noeud* Interpreteur::instTantQue(NoeudSeqInst* parent){
    NoeudInstTantQue* tq = new NoeudInstTantQue(parent, m_func);
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expBool(tq);
    testerEtAvancer(")");
    tq->init(cond);
    seqInst(tq);
    testerEtAvancer("end");

    return tq;
}

Noeud* Interpreteur::instRepeter(NoeudSeqInst* parent){
    NoeudInstRepeter* rep = new NoeudInstRepeter(parent, m_func);
    testerEtAvancer("repeat");
    seqInst(rep);
    testerEtAvancer("end");
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expBool(rep);
    testerEtAvancer(")");
    rep->init(cond);
    return rep;
}

Noeud* Interpreteur::instDoWhile(NoeudSeqInst* parent){
    NoeudInstDoWhile* dw = new NoeudInstDoWhile(parent, m_func);
    testerEtAvancer("do");
    seqInst(dw);
    testerEtAvancer("end");
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expBool(dw);
    testerEtAvancer(")");
    dw->init(cond);
    return dw;
}

Noeud* Interpreteur::instUntil(NoeudSeqInst* parent){
    NoeudInstUntil* un = new NoeudInstUntil(parent, m_func);
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expBool(un);
    testerEtAvancer(")");
    seqInst(un);
    testerEtAvancer("end");
    un->init(cond);
    return un;
}

Noeud* Interpreteur::instPrint(NoeudSeqInst* parent){
    Noeud* p = new NoeudInstPrint(parent, m_func);
    testerEtAvancer("print");
    testerEtAvancer("(");
    p->ajoute(expression(parent));
    while(m_lecteur.getSymbole() == ","){
      m_lecteur.avancer();
      p->ajoute(expression(parent));
    }
    
    testerEtAvancer(")");
    testerEtAvancer(";");
    return p;
}

Noeud* Interpreteur::instScan(NoeudSeqInst* parent){
    Symbole s = m_lecteur.getSymbole();
    m_lecteur.avancer();
    testerEtAvancer("(");
    tester("<VARIABLE>");
    Symbole var = m_func->chercheAjoute(m_lecteur.getSymbole());
    m_lecteur.avancer();
    testerEtAvancer(")");
    testerEtAvancer(";");
    if(s == "scanI")
      return new NoeudInstScanI(parent, m_func, new NoeudVariable(parent, m_func, var));
    else if(s == "scanF")
      return new NoeudInstScanF(parent, m_func, new NoeudVariable(parent, m_func, var));
    else if(s == "scanS")
      return new NoeudInstScanS(parent, m_func, new NoeudVariable(parent, m_func, var));
    else
      erreur("instruction scan attendue");
      return nullptr;
    
}
Noeud* Interpreteur::instSiRiche(NoeudSeqInst* parent){
    NoeudInstSiRiche* si = new NoeudInstSiRiche(parent, m_func);
    NoeudSeqInst* seq = new NoeudSeqInst(parent, m_func);
    
    testerEtAvancer("if");
    testerEtAvancer("(");
    Noeud* cond = expBool(seq);
    testerEtAvancer(")");
    seqInst(seq);
    si->ajoute(seq);
    si->ajoute(cond);

    while(m_lecteur.getSymbole() == "elseif"){
        seq = new NoeudSeqInst(parent, m_func);
        testerEtAvancer("elseif");
        testerEtAvancer("(");
        cond = expBool(seq);
        testerEtAvancer(")");
        seqInst(seq);
        si->ajoute(seq);
        si->ajoute(cond);
    }
    if(m_lecteur.getSymbole() == "else"){
        seq = new NoeudSeqInst(parent, m_func);
        testerEtAvancer("else");
        seqInst(seq);
        si->ajoute(seq);
    }
    testerEtAvancer("end");
    
    return si;
}
