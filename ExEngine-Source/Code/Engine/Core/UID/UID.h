#pragma once

class UID{
private: 
    static int lastUID;
public:
    static int GenerateUId();
};

//Needs to preserve session information