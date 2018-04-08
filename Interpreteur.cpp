#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
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

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud* sequence = new NoeudSeqInst(nullptr);
    while(m_lecteur.getSymbole() == "def")
        sequence->ajoute(fonction());
    tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::fonction(){
    testerEtAvancer("def");
    if(m_lecteur.getSymbole() == "<VARIABLE>"){
        m_arbre->chercheAjoute(m_lecteur.getSymbole());
    }
    testerEtAvancer("<VARIABLE>");

    std::vector<Symbole> parametres;
    testerEtAvancer("(");
    if(m_lecteur.getSymbole() == "<VARIABLE>"){
      parametres.push_back(m_lecteur.getSymbole());
      m_lecteur.avancer();
      while(m_lecteur.getSymbole() == ","){
        m_lecteur.avancer();
        tester("<VARIABLE>");
        parametres.push_back(m_lecteur.getSymbole());
        m_lecteur.avancer();
      }
    }
    testerEtAvancer(")");
    NoeudSeqInst* func = new NoeudFonction(m_arbre, parametres);
    seqInst(func);
    testerEtAvancer("end");
    return func;
}

Noeud* Interpreteur::seqInst(NoeudSeqInst* sequence) {
  // <seqInst> ::= <inst> { <inst> }
  do {
    sequence->ajoute(inst(sequence));
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "if"    ||
           m_lecteur.getSymbole() == "while"      || m_lecteur.getSymbole() == "repeat"||
           m_lecteur.getSymbole() == "for"        || m_lecteur.getSymbole() == "until" ||
           m_lecteur.getSymbole() == "do"         || m_lecteur.getSymbole() == "print" ||
           m_lecteur.getSymbole() == "return"     || m_lecteur.getSymbole() == "break" ||
           m_lecteur.getSymbole() == "scan");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst(NoeudSeqInst* parent) {
  // <inst> ::= <affectation>  ; | <instSi>
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation(parent);
    testerEtAvancer(";");
    return affect;
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
  /*else if (m_lecteur.getSymbole() == "scan")
    return instScan();*/

  // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
  else erreur("Instruction incorrecte");
  return nullptr;
}

Noeud* Interpreteur::affectation(NoeudSeqInst* parent) {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = parent->chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* ex = expression(parent);             // On mémorise l'expression trouvée
  return new NoeudAffectation(parent, var, ex); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression(NoeudSeqInst* parent) {
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* fact = facteur(parent);
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-"  ||
          m_lecteur.getSymbole() == "*"  || m_lecteur.getSymbole() == "/"  ||
          m_lecteur.getSymbole() == "%"  || m_lecteur.getSymbole() == "xor"||
          m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">=" ||
          m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "and" || m_lecteur.getSymbole() == "or"   ) {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = facteur(parent); // On mémorise l'opérande droit
    fact = new NoeudOperateurBinaire(parent, operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
  }
  return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur(NoeudSeqInst* parent) {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>" || m_lecteur.getSymbole() == "<REEL>" || m_lecteur.getSymbole() == "<CHAINE>") {
    fact = parent->chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(parent, Symbole("-"), parent->chercheAjoute(Symbole("0")), facteur(parent));
  } else if (m_lecteur.getSymbole() == "not") { // non <facteur>
    m_lecteur.avancer();
    fact = new NoeudOperateurBinaire(parent, Symbole("not"), parent->chercheAjoute(Symbole("0")), facteur(parent));
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expression(parent);
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::instPour(NoeudSeqInst* parent){
    testerEtAvancer("for");
    Noeud* init;
    Noeud* cond;
    Noeud* iter;
    NoeudInstPour* pour = new NoeudInstPour(parent);
    if(m_lecteur.getSymbole() == "("){
      testerEtAvancer("(");
      if(m_lecteur.getSymbole() == ";")
        init = pour->chercheAjoute(Symbole("0"));
      else
        init = affectation(pour);
        
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ";")
        cond = pour->chercheAjoute(Symbole("1"));
      else
        cond = expression(pour);
      
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ")")
        iter = pour->chercheAjoute(Symbole("0"));
      else
        iter = affectation(pour);
      testerEtAvancer(")");
    }
    else{
      tester("<VARIABLE>");
      Noeud* var;
      Noeud* i;
      var = pour->chercheAjoute(m_lecteur.getSymbole());
      m_lecteur.avancer();
      testerEtAvancer("in");
      Noeud* n1 = facteur(pour);
      Noeud* n2 = nullptr;
      if(m_lecteur.getSymbole() == ":"){
        testerEtAvancer(":");
        n2 = facteur(pour);
      }
      else{
        n2 = n1;
        n1 = pour->chercheAjoute(Symbole("0"));
      }
      if(m_lecteur.getSymbole() == "|"){
        testerEtAvancer("|");
        tester("<NUMBER>");
        i = pour->chercheAjoute(m_lecteur.getSymbole());
        m_lecteur.avancer();
      }
      else{
        i = pour->chercheAjoute(Symbole("1"));
      }
      init = new NoeudAffectation(pour, var, n1);
      if(i->executer() < Value(0))
        cond = new NoeudOperateurBinaire(pour, Symbole(">="), var, n2);
      else
        cond = new NoeudOperateurBinaire(pour, Symbole("<="), var, n2);
      iter = new NoeudAffectation(pour, var, new NoeudOperateurBinaire(pour, Symbole("+"), var, i));
    }
    pour->init(init, cond, iter);
    seqInst(pour);
    testerEtAvancer("end");
    return pour;
}

Noeud* Interpreteur::instTantQue(NoeudSeqInst* parent){
    NoeudInstTantQue* tq = new NoeudInstTantQue(parent);
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expression(tq);
    testerEtAvancer(")");
    tq->init(cond);
    seqInst(tq);
    testerEtAvancer("end");

    return tq;
}

Noeud* Interpreteur::instRepeter(NoeudSeqInst* parent){
    NoeudInstRepeter* rep = new NoeudInstRepeter(parent);
    testerEtAvancer("repeat");
    seqInst(rep);
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expression(rep);
    testerEtAvancer(")");
    rep->init(cond);
    return rep;
}

Noeud* Interpreteur::instDoWhile(NoeudSeqInst* parent){
    NoeudInstDoWhile* dw = new NoeudInstDoWhile(parent);
    testerEtAvancer("do");
    seqInst(dw);
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expression(dw);
    testerEtAvancer(")");
    dw->init(cond);
    return dw;
}

Noeud* Interpreteur::instUntil(NoeudSeqInst* parent){
    NoeudInstUntil* un = new NoeudInstUntil(parent);
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expression(un);
    testerEtAvancer(")");
    seqInst(un);
    testerEtAvancer("end");
    un->init(cond);
    return un;
}

Noeud* Interpreteur::instPrint(NoeudSeqInst* parent){
    Noeud* p = new NoeudInstPrint(parent);
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
/*
Noeud* Interpreteur::instScan(){
    testerEtAvancer("scan");
    testerEtAvancer("(");
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole());
    testerEtAvancer(")");
    testerEtAvancer(";");
    return new NoeudInstScan(var);
}
*/
Noeud* Interpreteur::instSiRiche(NoeudSeqInst* parent){
    NoeudInstSiRiche* si = new NoeudInstSiRiche(parent);
    NoeudSeqInst* seq = new NoeudSeqInst(parent);
    
    testerEtAvancer("if");
    testerEtAvancer("(");
    Noeud* cond = expression(seq);
    testerEtAvancer(")");
    si->ajoute(seq);
    si->ajoute(cond);

    while(m_lecteur.getSymbole() == "elseif"){
        seq = new NoeudSeqInst(parent);
        testerEtAvancer("elseif");
        testerEtAvancer("(");
        cond = expression(seq);
        testerEtAvancer(")");
        seqInst(seq);
        si->ajoute(seq);
        si->ajoute(cond);
    }
    if(m_lecteur.getSymbole() == "else"){
        seq = new NoeudSeqInst(parent);
        testerEtAvancer("else");
        seqInst(seq);
        si->ajoute(seq);
    }
    testerEtAvancer("end");
    
    return si;
}
