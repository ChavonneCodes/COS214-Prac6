#include "Data.h"
using namespace std;

Data::Data(){
    value = "";
}

Data::Data(string value) : value(value){
}


string Data::getValue(){
    return value;
}

void Data::setValue(string value){
    this->value = value;
}