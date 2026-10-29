#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>

class Data{
    public:
        /** 
        *@brief Constructs a Data object with an empty value.
        */
        Data();

        /**
        *@brief Constructs a Data object with an initial value.
        *@param value The initial string value to store.
        */
        Data(std::string);

        /**
        *@brief Retrieves the stored value.
        *@return The current string value.
        */
        std::string getValue() const;

        /**
        *@brief Updates the stored value.
        *@param value The new string value to store.
        */
        void setValue(std::string);

    private:
        /**
        * @brief The string value stored by this object.
        */
        std::string value;
};

#endif
