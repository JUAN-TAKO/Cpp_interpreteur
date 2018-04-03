/* 
 * File:   Exceptions.h
 * Author: martin
 *
 * Created on 7 décembre 2014, 19:08
 */

#ifndef EXCEPTIONS_H
#define	EXCEPTIONS_H
#include <exception>
#include <string>
using namespace std;

// Classe mère de toutes les exceptions de l'interpréteur
class InterpreteurException : public exception {
public:
    const char * what() const throw() {
        return "Exception Interpreteur";
    }
};

class FichierException : public InterpreteurException {
public:
    const char * what() const throw() {
        return "Ouverture Fichier Impossible";
    }
};

class SyntaxeException : public InterpreteurException {
public:
    SyntaxeException(const char * message = NULL) : m_message(message) {}
    const char * what() const throw() {
        return m_message;
    }
private:
    const char* m_message;
};


class IndefiniException : public InterpreteurException {
public:
    const char * what() const throw() {
        return "Valeur Indéfinie";
    }
};

class NonDeclareException : public InterpreteurException {
public:
    NonDeclareException(std::string m) : message(m){}
    const char * what() const throw() {
        return std::string(message + std::string(" non declare")).c_str();
    }
private:
    std::string message;
};

class DivParZeroException : public InterpreteurException {
public:
    const char * what() const throw() {
        return "Division par 0";
    }
};

class OperationInterditeException : public InterpreteurException {
public:
    OperationInterditeException(const char* message = nullptr) : m_message(message){}
    const char * what() const throw() {
        return m_message;
    }
private:
const char* m_message;
};

#endif	/* EXCEPTIONS_H */

