#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "ArbreAbstrait.h"
#include "TableSymboles.h"
#include "SymboleValue.h"
#include "Value.h"

class Interpreteur {
public:
	Interpreteur(ifstream & fichier);   // Construit un interpréteur pour interpreter
	                                    //  le programme dans  fichier 
                                      
	void analyse();                     // Si le contenu du fichier est conforme à la grammaire,
	                                    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
                                      //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
	                                    // Sinon, une exception sera levée

	inline NoeudSeqInst* getArbre () const { return m_arbre; }                    // accesseur
        Value executer();
        void convertir_python(std::ostream& out);
	
private:
    Lecteur        m_lecteur;  // Le lecteur de symboles utilisé pour analyser le fichier
    NoeudSeqInst*         m_arbre;    // L'arbre abstrait
    NoeudFonction* m_func;
    TableSymboles m_globals;
    bool m_valid;
    // Implémentation de la grammaire
    NoeudSeqInst*  programme();   //   <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    
    NoeudSeqInst*  seqInst(NoeudSeqInst* sequence);	   //     <seqInst> ::= <inst> { <inst> }
    Noeud*  inst(NoeudSeqInst* parent);	   //        <inst> ::= <affectation> ; | <instSi> | <instTantQue> 
    Noeud*  variableOperation(NoeudSeqInst* parent); // <affectation> ::= <variable> = <expression>
    Noeud*  affectation(NoeudSeqInst* parent, Symbole s);
    Noeud*  call(NoeudSeqInst* parent, Symbole s);
    Noeud*  expression(NoeudSeqInst* parent);  //  <expression> ::= <facteur> { <opBinaire> <facteur> }
    Noeud*  facteur(NoeudSeqInst* parent);     //     <facteur> ::= <entier>  |  <variable>  |  - <facteur>  | non <facteur> | ( <expression> )
    Noeud*  terme(NoeudSeqInst* parent);
    Noeud*  expBool(NoeudSeqInst* parent);
    Noeud*  relationEt(NoeudSeqInst* parent);
    Noeud*  relation(NoeudSeqInst* parent);
    Noeud*  opRel(NoeudSeqInst* parent);


    Noeud*  fonction(NoeudSeqInst* parent);
    Noeud*  instReturn(NoeudSeqInst* parent);
    Noeud*  instBreak(NoeudSeqInst* parent);
    Noeud*  instPour(NoeudSeqInst* parent);
    Noeud*  instTantQue(NoeudSeqInst* parent); // <instTantQue> ::= 
    Noeud*  instDoWhile(NoeudSeqInst* parent);
    Noeud*  instUntil(NoeudSeqInst* parent);
    Noeud*  instRepeter(NoeudSeqInst* parent);
    Noeud*  instSiRiche(NoeudSeqInst* parent);
    Noeud*  instPrint(NoeudSeqInst* parent);
    Noeud*  instScan(NoeudSeqInst* parent);
    // outils pour simplifier l'analyse syntaxique
    void tester (const string & symboleAttendu) const throw (SyntaxeException);   // Si symbole courant != symboleAttendu, on lève une exception
    void testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur (const string & mess) const throw (SyntaxeException);             // Lève une exception "contenant" le message mess
};

#endif /* INTERPRETEUR_H */
