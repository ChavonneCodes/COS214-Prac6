#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>

class Data{
    public:
        Data();
        Data(std::string);
        std::string getValue();
        void setValue(std::string);

    private:
        std::string value;
};

#endif