/**
 * @file Data.h
 * @brief Defines the Dataclass.
 */
#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>
/**
 * @class Data
 * @brief Stores system Data
 */

class Data{
    public:
        /** 
        *@brief Constructs a Data object with an empty value.
        */
        Data();

        /**
        *@brief Constructs a Data object with an initial value.
        */
        Data(std::string);

        /**
        *@brief Retrieves the stored value.
        *@return The current string value.
        */
        std::string getValue() const;

        /**
        *@brief Updates the stored value.
        */
        void setValue(std::string);

    private:
        /**
        * @brief The string value stored by this object.
        */
        std::string value;
};

#endif
