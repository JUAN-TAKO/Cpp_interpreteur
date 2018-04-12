#ifndef VALUE_H
#define VALUE_H
#include <string>
#include <math.h>
#include <float.h>
#include "ArbreAbstrait.h"
#include "Exceptions.h"

class NoeudFonction;
class Value{
private:
    struct S{
        int vi;
        float vf;
        std::string vs;
        NoeudFonction* vn;
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
        }else if(type == 2){
            data.vs = v.data.vs;
        }else{
            data.vn = v.data.vn;
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
    Value(NoeudFonction* v){
        type = 3;
        data.vn = v;
    }
    int getType() const{
        return type;
    }
    /*
    void scanI(){
        type = 0;
        std::cin >> S.vi;
    }
    void scanF(){
        type = 1;
        std::cin >> S.vf;
    }
    void scanS(){
        type = 2;
        std::cin >> S.vs;
    } */  
    operator int() const{
        if(type == 0){
            return data.vi;
        }
        else if(type == 1){
            return (int)data.vf;
        }
        else if(type == 2){
            throw OperationInterditeException("Invalid conversion from string to int");
        }
        else{
            throw OperationInterditeException("Invalid conversion from function to int");
        }
    }
    operator float() const{
        if(type == 1){
            return data.vf;
        }
        else if(type == 0){
            return (float)data.vi;
        }
        else if(type == 2){
            throw OperationInterditeException("Invalid conversion from string to float");
        }
        else{
            throw OperationInterditeException("Invalid conversion from function to float");
        }
    }
    operator std::string() const{
        if(type == 2){
            return data.vs;
        }
        else if(type == 0){
            return std::to_string(data.vi);
        }
        else if(type == 1){
            return std::to_string(data.vf);
        }
        else{
            throw OperationInterditeException("Invalid conversion from function to string");
        }
        
    }
    operator NoeudFonction*() const{
        if(type == 3){
            return data.vn;
        }
        else{
            throw OperationInterditeException("Invalid conversion to function");
        }
    }
    operator bool() const{
        if(type==0){
            return (bool)data.vi;
        }
        else{
            throw OperationInterditeException("Invalid conversion to bool");
        }
    }
    const Value& operator=(int v){
        type = 0;
        data.vi = v;
        data.vs = "";
        return *this;
    }
    const Value& operator=(float v){
        type = 1;
        data.vf = v;
        data.vs = "";
        return *this;
    }
    const Value& operator=(std::string v){
        type = 2;
        data.vs = v;
        return *this;
    }
    const Value& operator=(NoeudFonction* v){
        type = 3;
        data.vn = v;
        return *this;
    } 
    const Value& operator=(const Value& v){
        type = v.type;
        if(type == 0){
            data.vi = v.data.vi;
        }else if(type == 1){
            data.vf = v.data.vf;
        }else if(type == 2){
            data.vs = v.data.vs;
        }
        else{
            data.vn = v.data.vn;
        }
        return *this;
    }
    friend Value operator+(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation + on function");
        }
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
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation - on function");
        }
       int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi - v2.data.vi);
                break;
            case 1: // int float
                return Value((float)v1.data.vi - v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException("Invalid operation - on string");
                break;
            case 3: // float int
                return Value(v1.data.vf - (float)v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf - v2.data.vf);
                break;
            default:
                throw OperationInterditeException("Invalid operation - on string");
                break;
        }
    }
    friend Value operator*(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation * on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi * v2.data.vi);
                break;
            case 1: // int float
                return Value((float)v1.data.vi * v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException("Invalid operation * on string");
                break;
            case 3: // float int
                return Value(v1.data.vf * (float)v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf * v2.data.vf);
                break;
            default:
                throw OperationInterditeException("Invalid operation * on string");
                break;
        }
    }
    friend Value operator/(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation / on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi / v2.data.vi);
                break;
            case 1: // int float
                return Value(v1.data.vi / v2.data.vf);
                break;
            case 2: // int string
                throw OperationInterditeException("Invalid operation / on string");
                break;
            case 3: // float int
                return Value(v1.data.vf / v2.data.vi);
                break;
            case 4: // float float
                return Value(v1.data.vf / v2.data.vf);
                break;
            default:
                throw OperationInterditeException("Invalid operation / on string");
                break;
        }
    }
    friend Value operator%(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation % on function");
        }
        int types =v1.type*3 + v2.type;
        switch(types){
            case 0: // int int
                return Value(v1.data.vi % v2.data.vi);
                break;
            case 1: // int float
                return Value((float)fmod((float)v1.data.vi, v2.data.vf));
                break;
            case 2: // int string
                throw OperationInterditeException("Invalid operation % on string");
                break;
            case 3: // float int
                return Value((float)fmod(v1.data.vf, (float)v2.data.vi));
                break;
            case 4: // float float
                return Value((float)fmod(v1.data.vf, v2.data.vf));
                break;
            default:
                throw OperationInterditeException("Invalid operation % on string");
                break;
        }
        return 0;
    }
    friend Value operator==(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation == on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi == v2.data.vi); 
                break;
            case 1:
                return (int)(fabs(v1.data.vi - v2.data.vf) <= FLT_EPSILON);
                break;
            case 2:
                return 0;
                break;
            case 3:
                return (int)(fabs(v1.data.vf - v2.data.vi) <= FLT_EPSILON);
                break;
            case 4:
                return (int)(fabs(v1.data.vf - v2.data.vf) <= FLT_EPSILON);
                break;
            case 5:
                return 0;
                break;
            case 6:
                return 0;
                break;
            case 7:
                return 0;
                break;
            case 8:
                return (int)(v1.data.vs == v2.data.vs);
                break;
        }
        return 0;
    }
    friend Value operator!=(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation != on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi != v2.data.vi); 
                break;
            case 1:
                return (int)(fabs(v1.data.vi - v2.data.vf) > FLT_EPSILON);
                break;
            case 2:
                return 1;
                break;
            case 3:
                return (int)(fabs(v1.data.vf - v2.data.vi) > FLT_EPSILON);
                break;
            case 4:
                return (int)(fabs(v1.data.vf - v2.data.vf) > FLT_EPSILON);
                break;
            case 5:
                return 1;
                break;
            case 6:
                return 1;
                break;
            case 7:
                return 1;
                break;
            case 8:
                return (int)(v1.data.vs != v2.data.vs);
                break;
        }
        return 0;
    }
    friend Value operator<=(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation <= on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi <= v2.data.vi); 
                break;
            case 1:
                return (int)(v1.data.vi <= v2.data.vf);
                break;
            case 2:
                throw OperationInterditeException("Invalid comparison int <= string");
                break;
            case 3:
                return (int)(v1.data.vf <= v2.data.vi);
                break;
            case 4:
                return (int)(v1.data.vf <= v2.data.vf);
                break;
            case 5:
                throw OperationInterditeException("Invalid comparison float <= string");
                break;
            case 6:
                throw OperationInterditeException("Invalid comparison string <= int");
                break;
            case 7:
                throw OperationInterditeException("Invalid comparison string <= float");
                break;
            case 8:
                return (int)(v1.data.vs <= v2.data.vs);
                break;
        }
        return 0;
    }
    friend Value operator>=(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation >= on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi >= v2.data.vi); 
                break;
            case 1:
                return (int)(v1.data.vi >= v2.data.vf);
                break;
            case 2:
                throw OperationInterditeException("Invalid comparison float >= string");
                break;
            case 3:
                return (int)(v1.data.vf >= v2.data.vi);
                break;
            case 4:
                return (int)(v1.data.vf >= v2.data.vf);
                break;
            case 5:
                throw OperationInterditeException("Invalid comparison int >= string");
                break;
            case 6:
                throw OperationInterditeException("Invalid comparison string >= int");
                break;
            case 7:
                throw OperationInterditeException("Invalid comparison string >= float");
                break;
            case 8:
                return (int)(v1.data.vs >= v2.data.vs);
                break;
        }
        return 0;
    }
    friend Value operator<(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation < on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi < v2.data.vi); 
                break;
            case 1:
                return (int)(v1.data.vi < v2.data.vf);
                break;
            case 2:
                throw OperationInterditeException("Invalid comparison int < string");
                break;
            case 3:
                return (int)(v1.data.vf < v2.data.vi);
                break;
            case 4:
                return (int)(v1.data.vf < v2.data.vf);
                break;
            case 5:
                throw OperationInterditeException("Invalid comparison float < string");
                break;
            case 6:
                throw OperationInterditeException("Invalid comparison string < int");
                break;
            case 7:
                throw OperationInterditeException("Invalid comparison string < float");
                break;
            case 8:
                return (int)(v1.data.vs < v2.data.vs);
                break;
        }
        return 0;
    }
    friend Value operator>(const Value& v1, const Value& v2){
        if(v1.type == 3 || v2.type == 3){
            throw OperationInterditeException("Invalid operation > on function");
        }
        int types = v1.type*3 + v2.type;
        switch(types){
            case 0:
                return (int)(v1.data.vi > v2.data.vi); 
                break;
            case 1:
                return (int)(v1.data.vi > v2.data.vf);
                break;
            case 2:
                throw OperationInterditeException("Invalid comparison int > string");
                break;
            case 3:
                return (int)(v1.data.vf > v2.data.vi);
                break;
            case 4:
                return (int)(v1.data.vf > v2.data.vf);
                break;
            case 5:
                throw OperationInterditeException("Invalid comparison float > string");
                break;
            case 6:
                throw OperationInterditeException("Invalid comparison string > int");
                break;
            case 7:
                throw OperationInterditeException("Invalid comparison string > float");
                break;
            case 8:
                return (int)(v1.data.vs > v2.data.vs);
                break;
        }
        return 0;
    }
    friend std::ostream& operator<<(ostream& out, const Value& v){
        if (v.type == 0) out << (int)v;
        else if (v.type == 1) out << (float)v;
        else if (v.type == 2) out << (std::string)v;
        else out << (NoeudFonction*)v;
        return out;
    }
};
#endif