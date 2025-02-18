#include "ExMath.h"

//Calculating the GCD using the Euclidean algorithm
int ExMath::GetGCD(int a, int b){
    int temp = 0;

    if(a < b)
    {
        temp = a;
        a = b;
        b = temp;
        temp = 0;
    }

    while(b != 0){
        temp = b;
        b = a % b;
        a = temp;
    }

    return a; // GCD
};