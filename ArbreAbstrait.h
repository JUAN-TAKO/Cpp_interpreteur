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
class NoeudSeqInst;

class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
public:
  virtual Value  executer() = 0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
  virtual void ajoute(Noeud* instruction) { throw OperationInterditeException("Ajout non supporté"); }
  virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
  virtual NoeudSeqInst* getSeq(){return m_parent;}
  void setParent(NoeudSeqInst* parent){m_parent = parent;}
protected:
  NoeudSeqInst* m_parent;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
public:
     NoeudSeqInst(NoeudSeqInst* parent);   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    virtual Value executer();    // Exécute chaque instruction de la séquence
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence
    void breakExec();
    SymboleValue* cherche(const Symbole& s);
    SymboleValue* chercheAjoute(const Symbole& s);
protected:
    bool exec;
    TableSymboles m_table;
    SymboleValue* m_func;
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};
class NoeudBoucle : public NoeudSeqInst{
public:
  virtual Value  executer() = 0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
  void ajoute(Noeud* instruction) { m_instructions.push_back(instruction); }
  virtual ~NoeudBoucle() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
private:

};
////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(NoeudSeqInst* parent, Noeud* variable, Noeud* expression); // construit une affectation
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
    NoeudOperateurBinaire(NoeudSeqInst* parent, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();            // Exécute (évalue) l'opération binaire)

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

class NoeudFonction : public NoeudSeqInst{
public:
  NoeudFonction(NoeudSeqInst* parent, const std::vector<Symbole>& parametres);
  ~NoeudFonction(){}
  Value executer();
private:
  std::vector<Noeud*> m_parametres;
  Noeud* returnValue;
};

class NoeudInstReturn : public Noeud{
public:
  NoeudInstReturn(Noeud* returnValue);
  ~NoeudInstReturn(){}
  Value executer();
private:
  Noeud* m_return;
};

class NoeudInstBreak : public Noeud{
public:
  NoeudInstBreak();
  ~NoeudInstBreak(){}
  Value executer();
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSiRiche : public Noeud{
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSiRiche(NoeudSeqInst* parent);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSiRiche() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajoute(Noeud* instruction);

  private:
    bool state;
    std::vector<Noeud*> m_conditions;
    std::vector<Noeud*> m_sequences;
};

class NoeudInstRepeter : public NoeudBoucle{
public:
    NoeudInstRepeter(NoeudSeqInst* parent);
    ~NoeudInstRepeter() {}
    Value executer();
    void init(Noeud* condition);
private:
    Noeud*  m_condition;
};

class NoeudInstTantQue : public NoeudBoucle{
// Classe pour représenter un noeud "instruction ta,t que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstTantQue(NoeudSeqInst* parent);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstTantQue() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void init(Noeud* condition);
  private:
    Noeud*  m_condition;
};

class NoeudInstPour : public NoeudBoucle{
// Classe pour représenter un noeud "instruction tant que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstPour(NoeudSeqInst* parent);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstPour() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void init(Noeud* initialisation, Noeud* condition, Noeud* iteration);
  private:
    Noeud*  m_initialisation;
    Noeud*  m_condition;
    Noeud*  m_iteration;
};

class NoeudInstDoWhile : public NoeudBoucle{
public:
    NoeudInstDoWhile(NoeudSeqInst* parent);
    ~NoeudInstDoWhile() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void init(Noeud* condition);
private:
    Noeud* m_condition;
};

class NoeudInstUntil : public NoeudBoucle{
public:
    NoeudInstUntil(NoeudSeqInst* parent);
    ~NoeudInstUntil() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void init(Noeud* condition);
private:
    Noeud* m_condition;
};
class NoeudInstPrint : public Noeud{
public:
    NoeudInstPrint(NoeudSeqInst* parent);
    ~NoeudInstPrint() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajoute(Noeud* val);
private:
    std::vector<Noeud*> m_vals;
};
class NoeudInstScanI{
public:
    NoeudInstScanI();
    ~NoeudInstScanI() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajouter(Noeud* var);
private:
    Noeud* m_affect;
    
};
class NoeudInstScanF{
public:
    NoeudInstScanF();
    ~NoeudInstScanF() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajouter(Noeud* var);
private:
    Noeud* m_affect;
};
class NoeudInstScanS{
public:
    NoeudInstScanS();
    ~NoeudInstScanS() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void ajouter(Noeud* var);
private:
    Noeud* m_affect;
};

#endif /* ARBREABSTRAIT_H */
