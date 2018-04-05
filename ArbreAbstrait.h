#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

#include "Symbole.h"
#include "Exceptions.h"
#include "TableSymboles.h"
class Value;

////////////////////////////////////////////////////////////////////////////////

class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
public:
  virtual Value  executer() = 0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
  virtual void ajoute(Noeud* instruction) { throw OperationInterditeException("Ajout non supporté"); }
  virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
  TableSymboles& getTable() {return m_table;}
  void setParent(Noeud* parent){m_parent = parent;}
  void addChild(Noeud* child) {child->setParent(this);}
  SymboleValue* cherche(const Symbole& s);
  SymboleValue* chercheAjoute(const Symbole& s);
protected:
  Noeud* m_parent;
  TableSymboles m_table;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
  public:
     NoeudSeqInst(Noeud* parent);   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();    // Exécute chaque instruction de la séquence
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence

  private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(Noeud* parent, Noeud* variable, Noeud* expression); // construit une affectation
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable

  private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(Noeud* parent, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();            // Exécute (évalue) l'opération binaire)

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

class NoeudFonction : public Noeud{
public:
  NoeudFonction(Noeud* parent, Noeud* sequence, const std::vector<Symbole>& parametres);
  ~NoeudFonction(){}
  Value executer();
private:
  Noeud* m_sequence;
  std::vector<Noeud*> m_parametres;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSiRiche : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSiRiche(Noeud* parent, Noeud* condition, Noeud* sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSiRiche() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajoute(Noeud* instruction);

  private:
    bool state;
    std::vector<Noeud*> m_conditions;
    std::vector<Noeud*> m_sequences;
};

class NoeudInstRepeter : public Noeud{
public:
    NoeudInstRepeter(Noeud* parent, Noeud* sequence, Noeud* condition);
    ~NoeudInstRepeter() {}
    Value executer();
private:
    Noeud*  m_sequence;
    Noeud*  m_condition;
};

class NoeudInstTantQue : public Noeud {
// Classe pour représenter un noeud "instruction ta,t que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstTantQue(Noeud* parent, Noeud* sequence, Noeud* condition);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstTantQue() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence

  private:
    Noeud*  m_sequence;
    Noeud*  m_condition;
};

class NoeudInstPour : public Noeud {
// Classe pour représenter un noeud "instruction ta,t que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstPour(Noeud* parent, Noeud* initialisation, Noeud* condition,Noeud* iteration, Noeud* sequence);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstPour() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence

  private:
    Noeud*  m_initialisation;
    Noeud*  m_condition;
    Noeud*  m_iteration;
    Noeud*  m_sequence;
};

class NoeudInstDoWhile : public Noeud{
public:
    NoeudInstDoWhile(Noeud* parent, Noeud* sequence, Noeud* cond);
    ~NoeudInstDoWhile() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
private:
    Noeud* m_sequence;
    Noeud* m_condition;
};

class NoeudInstUntil : public Noeud{
public:
    NoeudInstUntil(Noeud* parent, Noeud* sequence, Noeud* cond);
    ~NoeudInstUntil() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
private:
    Noeud* m_sequence;
    Noeud* m_condition;
};
class NoeudInstPrint : public Noeud{
public:
    NoeudInstPrint(Noeud* parent);
    ~NoeudInstPrint() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajoute(Noeud* val);
private:
    std::vector<Noeud*> m_vals;
};/*
class NoeudInstScan{
public:
    NoeudInstScan();
    ~NoeudInstScan() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajouter(Noeud* var);
private:
    std::vector<Noeud*> m_vars;
};*/

#endif /* ARBREABSTRAIT_H */
