#ifndef SYMBOLEVALUE_H
#define SYMBOLEVALUE_H

#include <string>
#include <iostream>
#include "Symbole.h"
#include "ArbreAbstrait.h"
#include <math.h>
class Value{
private:
    struct S{
        int vi;
        float vf;
        std::string vs;
        
        S() : vi(0){}
    } data;
    int type;
public:
    Value(){
        type = 0;
        data.vi = 0;
    }
    Value(const Value& v){
        type = v.type;
        if(type == 0){
            data.vi = v.data.vi;
        }else if(type == 1){
            data.vf = v.data.vf;
        }else{
            data.vs = v.data.vs;
        }
    }
    Value(int v){
        type = 0;
        data.vi = v;
    }
    Value(float v){
        type = 1;
        data.vf = v;
    }
    Value(std::string v){
        type = 2;
        data.vs = v;
    }
    
    int getType() const{
        return type;
    }
    operator int() const{
        if(type == 0){
            return data.vi;
        }
        else if(type == 1){
            return (int)data.vf;
        }
        else{
            throw OperationInterditeException();
        }
    }
    operator float() const{
        if(type == 1){
            return data.vf;
        }
        else if(type == 0){
            return (float)data.vi;
        }
        else{
            throw OperationInterditeException();
        }
    }
    operator std::string() const{
        if(type == 2){
            return data.vs;
        }
        else if(type == 0){
            return std::to_string(data.vi);
        }
        else{
            return std::to_string(data.vf);
        }
        
    }
    operator bool() const{
        if(type==0){
            return (bool)data.vi;
        }
        else{
            throw OperationInterditeException();
        }
    }
    int operator=(int v){
        type = 0;
        data.vi = v;
        data.vs = "";
        return data.vi;
    }
    float operator=(float v){
        type = 1;
        data.vf = v;
        data.vs = "";
        return data.vf;
    }
    const std::string& operator=(string v){
        type = 2;
        data.vs = v;
        return data.vs;
    }
    friend Value operator+(const Value& v1, const Value& v2){
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi + v2.data.vi);
                break;
            case 1: // int float
                return Value((float)v1.data.vi + v2.data.vf);
                break;
            case 2: // int string
                return Value(std::to_string(v1.data.vi) + v2.data.vs);
                break;
            case 3: // float int
                return Value(v1.data.vf + (float)v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf + v2.data.vf);
                break;
            case 5: // float string
                return Value(std::to_string(v1.data.vf) + v2.data.vs);
                break;
            case 6: // string int
                return Value(v1.data.vs + std::to_string(v2.data.vi));
                break;
            case 7: // string float
                return Value(v1.data.vs + std::to_string(v2.data.vf));
                break;
            case 8: // string string
                return Value(v1.data.vs + v2.data.vs);
                break;
        }
        return Value(0);
    }
    friend Value operator-(const Value& v1, const Value& v2){
       int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi - v2.data.vi);
                break;
            case 1: // int float
                return Value((float)v1.data.vi - v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException();
                break;
            case 3: // float int
                return Value(v1.data.vf - (float)v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf - v2.data.vf);
                break;
            default:
                throw OperationInterditeException();
                break;
        }
    }
    friend Value operator*(const Value& v1, const Value& v2){
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi * v2.data.vi);
                break;
            case 1: // int float
                return Value((float)v1.data.vi * v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException();
                break;
            case 3: // float int
                return Value(v1.data.vf * (float)v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf * v2.data.vf);
                break;
            default:
                throw OperationInterditeException();
                break;
        }
    }
    friend Value operator/(const Value& v1, const Value& v2){
        int types =v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi / v2.data.vi);
                break;
            case 1: // int float
                return Value(v1.data.vi / v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException();
                break;
            case 3: // float int
                return Value(v1.data.vf / v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf / v2.data.vf);
                break;
            default:
                throw OperationInterditeException();
                break;
        }
    }
    friend Value operator%(const Value& v1, const Value& v2){
        int types =v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi % v2.data.vi);
                break;
            case 1: // int float
                return Value(fmod((float)v1.data.vi, v2.data.vf));
                break;
            case 2: // int string
                throw OperationInterditeException();
                break;
            case 3: // float int
                return Value(fmod(v1.data.vf, (float)v2.data.vi));
                break;
            case 4: // float float
                return Value(fmod(v1.data.vf, v2.data.vf));
                break;
            default:
                throw OperationInterditeException();
                break;
        }
    }
    friend Value operator==(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi == v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf == v2.data.vf));
            }else{
                return Value((int)(v1.data.vs == v2.data.vs));
            }
        }
        else{
            return 0;
        }
    }
    friend Value operator!=(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi != v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf != v2.data.vf));
            }else{
                return Value((int)(v1.data.vs != v2.data.vs));
            }
        }
        else{
            return 1;
        }
    }
    friend Value operator<=(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi <= v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf <= v2.data.vf));
            }else{
                return Value((int)(v1.data.vs <= v2.data.vs));
            }
        }
        else{
            throw OperationInterditeException();
        }
    }
    friend Value operator>=(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi >= v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf >= v2.data.vf));
            }else{
                return Value((int)(v1.data.vs >= v2.data.vs));
            }
        }
        else{
            throw OperationInterditeException();
        }
    }
    friend Value operator<(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi < v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf < v2.data.vf));
            }else{
                return Value((int)(v1.data.vs < v2.data.vs));
            }
        }
        else{
            throw OperationInterditeException();
        }
    }
    friend Value operator>(const Value& v1, const Value& v2){
        if(v1.type == v2.type){
            if(v1.type == 0){
                return Value((int)(v1.data.vi > v2.data.vi));
            }else if(v1.type == 1){
                return Value((int)(v1.data.vf > v2.data.vf));
            }else{
                return Value((int)(v1.data.vs > v2.data.vs));
            }
        }
        else{
            throw OperationInterditeException();
        }
    }
    friend std::ostream& operator<<(ostream& out, const Value& v){
        if (v.type == 0) out << (int)v;
        else if (v.type == 1) out << (float)v;
        else out << (string)v;
        return out;
    }
};


class SymboleValue : public Symbole,  // Un symbole valué est un symbole qui a une valeur (définie ou pas)
                     public Noeud  {  //  et c'est aussi une feuille de l'arbre abstrait
public:
	  SymboleValue(const Symbole & s); // Construit un symbole valué à partir d'un symbole existant s
	  ~SymboleValue( ) {}
	  Value executer();         // exécute le SymboleValue (revoie sa valeur !)
	  inline void setValeur(Value valeur)    { this->m_valeur=valeur; m_defini=true;  } // accesseur
	  inline bool estDefini()              { return m_defini;                       } // accesseur
          inline int getType() const{
              return m_valeur.getType();
          }
          inline Value getValue() const{
              return m_valeur;
          }
	  friend ostream & operator << (ostream & cout, const SymboleValue & symbole); // affiche un symbole value sur cout

private:
	  bool m_defini;	// indique si la valeur du symbole est définie
	  Value m_valeur;	// valeur du symbole si elle est définie, zéro sinon

};

#endif /* SYMBOLEVALUE_H */
