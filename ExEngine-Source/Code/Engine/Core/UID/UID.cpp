#include "UID.h"

int UID::lastUID = 0;

int UID::GenerateUId(){
    return lastUID++;
};