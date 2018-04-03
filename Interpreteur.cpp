#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
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
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
  testerEtAvancer("def");
  testerEtAvancer("main");
  testerEtAvancer("(");
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("end");
  tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> { <inst> }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "if" ||
           m_lecteur.getSymbole() == "while"      || m_lecteur.getSymbole() == "repeat"||
           m_lecteur.getSymbole() == "for"        || m_lecteur.getSymbole() == "until" ||
           m_lecteur.getSymbole() == "do"         || m_lecteur.getSymbole() == "print" ||
           m_lecteur.getSymbole() == "scan");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi>
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation();
    testerEtAvancer(";");
    return affect;
  }
  else if (m_lecteur.getSymbole() == "if")
    return instSiRiche();
  else if (m_lecteur.getSymbole() == "for")
    return instPour();
  else if (m_lecteur.getSymbole() == "while")
    return instTantQue();
  else if (m_lecteur.getSymbole() == "do")
    return instDoWhile();
  else if (m_lecteur.getSymbole() == "until")
    return instUntil();
  else if (m_lecteur.getSymbole() == "repeat")
    return instRepeter();
  else if (m_lecteur.getSymbole() == "print")
    return instPrint();
  /*else if (m_lecteur.getSymbole() == "scan")
    return instScan();*/

  // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
  else erreur("Instruction incorrecte");
  return nullptr;
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* ex = expression();             // On mémorise l'expression trouvée
  return new NoeudAffectation(var, ex); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* fact = facteur();
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-"  ||
          m_lecteur.getSymbole() == "*"  || m_lecteur.getSymbole() == "/"  ||
          m_lecteur.getSymbole() == "%"  || m_lecteur.getSymbole() == "xor"||
          m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">=" ||
          m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "and" || m_lecteur.getSymbole() == "or"   ) {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = facteur(); // On mémorise l'opérande droit
    fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
  }
  return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>" || m_lecteur.getSymbole() == "<REEL>" || m_lecteur.getSymbole() == "<CHAINE>") {
    fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
  } else if (m_lecteur.getSymbole() == "not") { // non <facteur>
    m_lecteur.avancer();
    fact = new NoeudOperateurBinaire(Symbole("not"), m_table.chercheAjoute(Symbole("0")), facteur());
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expression();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::instPour(){
    testerEtAvancer("for");
    Noeud* init;
    Noeud* cond;
    Noeud* iter;
    if(m_lecteur.getSymbole() == "("){
      testerEtAvancer("(");
      if(m_lecteur.getSymbole() == ";")
        init = m_table.chercheAjoute(Symbole("0"));
      else
        init = affectation();
        
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ";")
        cond = m_table.chercheAjoute(Symbole("1"));
      else
        cond = expression();
      
      testerEtAvancer(";");
      
      if(m_lecteur.getSymbole() == ")")
        iter = m_table.chercheAjoute(Symbole("0"));
      else
        iter = affectation();
      testerEtAvancer(")");
    }
    else{
      tester("<VARIABLE>");
      Noeud* var;
      Noeud* i;
      var = m_table.chercheAjoute(m_lecteur.getSymbole());
      m_lecteur.avancer();
      testerEtAvancer("in");
      Noeud* n1 = facteur();
      Noeud* n2 = nullptr;
      if(m_lecteur.getSymbole() == ":"){
        testerEtAvancer(":");
        n2 = facteur();
      }
      else{
        n2 = n1;
        n1 = m_table.chercheAjoute(Symbole("0"));
      }
      if(m_lecteur.getSymbole() == "|"){
        testerEtAvancer("|");
        tester("<NUMBER>");
        i = m_table.chercheAjoute(m_lecteur.getSymbole());
        m_lecteur.avancer();
      }
      else{
        i = m_table.chercheAjoute(Symbole("1"));
      }
      init = new NoeudAffectation(var, n1);
      if(i->executer() < Value(0))
        cond = new NoeudOperateurBinaire(Symbole(">="), var, n2);
      else
        cond = new NoeudOperateurBinaire(Symbole("<="), var, n2);
      iter = new NoeudAffectation(var, new NoeudOperateurBinaire(Symbole("+"), var, i));
    }
    Noeud* seq = seqInst();
    testerEtAvancer("end");
    return new NoeudInstPour(init,cond,iter,seq);
}

Noeud* Interpreteur::instTantQue(){
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    Noeud* seq = seqInst();
    testerEtAvancer("end");
    return new NoeudInstTantQue(seq, cond);
}

Noeud* Interpreteur::instRepeter(){
    testerEtAvancer("repeat");
    Noeud* seq = seqInst();
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    return new NoeudInstTantQue(seq, cond);
}

Noeud* Interpreteur::instDoWhile(){
    testerEtAvancer("do");
    Noeud* seq = seqInst();
    testerEtAvancer("while");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    return new NoeudInstDoWhile(seq, cond);
}

Noeud* Interpreteur::instUntil(){
    testerEtAvancer("until");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    Noeud* seq = seqInst();
    testerEtAvancer("end");
    return new NoeudInstUntil(cond,seq);
}

Noeud* Interpreteur::instPrint(){
    Noeud* p = new NoeudInstPrint();
    testerEtAvancer("print");
    testerEtAvancer("(");
    p->ajoute(expression());
    while(m_lecteur.getSymbole() == ","){
      m_lecteur.avancer();
      p->ajoute(expression());
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
Noeud* Interpreteur::instSiRiche(){
    testerEtAvancer("if");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    Noeud* seq = seqInst();
    Noeud* si = new NoeudInstSiRiche(cond, seq);
    while(m_lecteur.getSymbole() == "elseif"){
        testerEtAvancer("elseif");
        testerEtAvancer("(");
        cond = expression();
        testerEtAvancer(")");
        seq = seqInst();
        si->ajoute(seq);
        si->ajoute(cond);
    }
    if(m_lecteur.getSymbole() == "else"){
        testerEtAvancer("else");
        seq = seqInst();
        si->ajoute(seq);
    }
    testerEtAvancer("end");
    
    return si;
}
