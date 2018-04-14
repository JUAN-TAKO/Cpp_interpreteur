#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "Symbole.h"
#include "Exceptions.h"
#include <stack>

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst;
class Value;
class NoeudFonction;

class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
public:
  Noeud(NoeudSeqInst* parent, NoeudFonction* func) : m_parent(parent), m_func(func){}
  virtual Value executer() = 0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
  virtual std::ostream& convertir_python(std::ostream& out, int& indent) = 0;
  virtual void ajoute(Noeud* instruction) { throw OperationInterditeException("Ajout non supporté"); }
  virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
  NoeudSeqInst* getSeq(){return m_parent;}
  void setParent(NoeudSeqInst* parent){m_parent = parent;}
  NoeudFonction* getFunc(){return m_func;}
private:
  NoeudSeqInst* m_parent;
  NoeudFonction* m_func;
};

#include "TableSymboles.h"
#include "SymboleValue.h"

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
public:
     NoeudSeqInst(NoeudSeqInst* parent, NoeudFonction* func);   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    virtual Value executer();    // Exécute chaque instruction de la séquence
    virtual std::ostream& convertir_python(std::ostream& out, int& indent);
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence
    void breakExec();
protected:
    bool exec;
    //SymboleValue* m_func;
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};
class NoeudBoucle : public NoeudSeqInst{
public:
  NoeudBoucle(NoeudSeqInst* parent, NoeudFonction* func) : NoeudSeqInst(parent, func){}
  virtual Value  executer() = 0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
  void ajoute(Noeud* instruction) { m_instructions.push_back(instruction); }
  virtual ~NoeudBoucle() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
private:

};

class NoeudVariable : public Noeud{
public:
  NoeudVariable(NoeudSeqInst* parent, NoeudFonction* func, Symbole s) : Noeud(parent, func), m_symbole(s){}
  ~NoeudVariable(){}
  Value executer();
  std::ostream& convertir_python(std::ostream& out, int& indent);
  void setValue(Value v);
  Symbole getSymbole(){return m_symbole;}

private:
  Symbole m_symbole;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* variable, Noeud* expression); // construit une affectation
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable
    std::ostream& convertir_python(std::ostream& out, int& indent);
  private:
    NoeudVariable* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(NoeudSeqInst* parent, NoeudFonction* func, Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();            // Exécute (évalue) l'opération binaire)
    std::ostream& convertir_python(std::ostream& out, int& indent);
  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

class NoeudFonction : public NoeudSeqInst{
friend class NoeudCall;
friend class NoeudInstReturn;
friend class SymboleValue;
public:
  NoeudFonction(NoeudSeqInst* parent, TableSymboles* globals, Symbole self);
  ~NoeudFonction(){}
  Value executer();
  std::ostream& convertir_python(std::ostream& out, int& indent);
  void ajoute(Symbole s, bool parametre = false);
  Symbole chercheAjoute(Symbole s);
  TableSymboles& getTable(){return m_stack.top();}
  TableSymboles* getGlobals(){return m_globals;}
  void push() {m_stack.push(TableSymboles());}
private:
  SymboleValue* getReturn();
  std::stack<TableSymboles> m_stack;
  std::vector<Symbole> m_variables;
  unsigned int m_parametres;
  TableSymboles* m_globals;
  Noeud* returnValue;
  Symbole m_self;
};



class NoeudCall : public Noeud{
public:
  NoeudCall(NoeudSeqInst* parent, NoeudFonction* func, Noeud* target_func) : Noeud(parent, func), m_target_func(target_func){}
  ~NoeudCall(){}
  Value executer();
  std::ostream& convertir_python(std::ostream& out, int& indent);
  void ajoute(Noeud* n);
private:
  std::vector<Noeud*> m_args;
  Noeud* m_target_func;
};

class NoeudInstReturn : public Noeud{
public:
  NoeudInstReturn(NoeudSeqInst* parent, NoeudFonction* func, Noeud* returnValue);
  ~NoeudInstReturn(){}
  Value executer();
  std::ostream& convertir_python(std::ostream& out, int& indent);
private:
  Noeud* m_return;
};

class NoeudInstBreak : public Noeud{
public:
  NoeudInstBreak(NoeudSeqInst* parent, NoeudFonction* func);
  ~NoeudInstBreak(){}
  Value executer();
  std::ostream& convertir_python(std::ostream& out, int& indent);
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSiRiche : public Noeud{
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSiRiche(NoeudSeqInst* parent, NoeudFonction* func);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSiRiche() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void ajoute(Noeud* instruction);

  private:
    bool state;
    std::vector<Noeud*> m_conditions;
    std::vector<Noeud*> m_sequences;
};

class NoeudInstRepeter : public NoeudBoucle{
public:
    NoeudInstRepeter(NoeudSeqInst* parent, NoeudFonction* func);
    ~NoeudInstRepeter() {}
    Value executer();
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void init(Noeud* condition);
private:
    Noeud*  m_condition;
};

class NoeudInstTantQue : public NoeudBoucle{
// Classe pour représenter un noeud "instruction ta,t que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstTantQue(NoeudSeqInst* parent, NoeudFonction* func);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstTantQue() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void init(Noeud* condition);
  private:
    Noeud*  m_condition;
};

class NoeudInstPour : public NoeudBoucle{
// Classe pour représenter un noeud "instruction tant que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstPour(NoeudSeqInst* parent, NoeudFonction* func);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstPour() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void init(Noeud* initialisation, Noeud* condition, Noeud* iteration);
  private:
    Noeud*  m_initialisation;
    Noeud*  m_condition;
    Noeud*  m_iteration;
};

class NoeudInstDoWhile : public NoeudBoucle{
public:
    NoeudInstDoWhile(NoeudSeqInst* parent, NoeudFonction* func);
    ~NoeudInstDoWhile() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void init(Noeud* condition);
private:
    Noeud* m_condition;
};

class NoeudInstUntil : public NoeudBoucle{
public:
    NoeudInstUntil(NoeudSeqInst* parent, NoeudFonction* func);
    ~NoeudInstUntil() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void init(Noeud* condition);
private:
    Noeud* m_condition;
};
class NoeudInstPrint : public Noeud{
public:
    NoeudInstPrint(NoeudSeqInst* parent, NoeudFonction* func);
    ~NoeudInstPrint() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
    void ajoute(Noeud* val);
private:
    std::vector<Noeud*> m_vals;
};
class NoeudInstScanI : public Noeud{
public:
    NoeudInstScanI(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var);
    ~NoeudInstScanI() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
private:
    NoeudVariable* m_var;
    
};
class NoeudInstScanF : public Noeud{
public:
    NoeudInstScanF(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var);
    ~NoeudInstScanF() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
private:
    NoeudVariable* m_var;
};
class NoeudInstScanS : public Noeud{
public:
    NoeudInstScanS(NoeudSeqInst* parent, NoeudFonction* func, NoeudVariable* var);
    ~NoeudInstScanS() {} // A cause du destructeur virtuel de la classe Noeud
    Value executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    std::ostream& convertir_python(std::ostream& out, int& indent);
private:
    NoeudVariable* m_var;
};

#endif /* ARBREABSTRAIT_H */
