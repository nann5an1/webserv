#ifndef SCHEMA_HPP
#define SCHEMA_HPP

//call by reference to each of the classes under the schema according to the request category
class Schema{
    public:
        Schema();
        ~Schema();
        Schema(const Schema &other);
        Schema& operator=(const Schema &other);
        
};

#endif