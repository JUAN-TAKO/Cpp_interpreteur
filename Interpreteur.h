#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include "ArbreAbstrait.h"

class Interpreteur {
public:
	Interpreteur(ifstream & fichier);   // Construit un interpréteur pour interpreter
	                                    //  le programme dans  fichier 
                                      
	void analyse();                     // Si le contenu du fichier est conforme à la grammaire,
	                                    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
                                      //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
	                                    // Sinon, une exception sera levée

	inline Noeud* getArbre () const { return m_arbre; }                    // accesseur
	
private:
    Lecteur        m_lecteur;  // Le lecteur de symboles utilisé pour analyser le fichier
    NoeudSeqInst*         m_arbre;    // L'arbre abstrait
    //Noeud*         m_last;     // dernier noeud
    // Implémentation de la grammaire
    Noeud*  programme();   //   <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud*  seqInst(NoeudSeqInst* sequence);	   //     <seqInst> ::= <inst> { <inst> }
    Noeud*  inst(NoeudSeqInst* parent);	   //        <inst> ::= <affectation> ; | <instSi> | <instTantQue> 
    Noeud*  affectation(NoeudSeqInst* parent); // <affectation> ::= <variable> = <expression> 
    Noeud*  expression(NoeudSeqInst* parent);  //  <expression> ::= <facteur> { <opBinaire> <facteur> }
    Noeud*  facteur(NoeudSeqInst* parent);     //     <facteur> ::= <entier>  |  <variable>  |  - <facteur>  | non <facteur> | ( <expression> )
                           //   <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* fonction();
    
    Noeud*  instPour(NoeudSeqInst* parent);
    Noeud*  instTantQue(NoeudSeqInst* parent); // <instTantQue> ::= 
    Noeud*  instDoWhile(NoeudSeqInst* parent);
    Noeud*  instUntil(NoeudSeqInst* parent);
    Noeud*  instRepeter(NoeudSeqInst* parent);
    Noeud*  instSiRiche(NoeudSeqInst* parent);
    Noeud*  instPrint(NoeudSeqInst* parent);
    //Noeud*  instScan();
    // outils pour simplifier l'analyse syntaxique
    void tester (const string & symboleAttendu) const throw (SyntaxeException);   // Si symbole courant != symboleAttendu, on lève une exception
    void testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur (const string & mess) const throw (SyntaxeException);             // Lève une exception "contenant" le message mess
};

#endif /* INTERPRETEUR_H */
