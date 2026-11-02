#include "Data.h"
using namespace std;

Data::Data(): value(""){
}

Data::Data(std::string value) : value(value){
}

string Data::getValue() const{
    return value;
}

void Data::setValue(string value){
    this->value = value;
}
