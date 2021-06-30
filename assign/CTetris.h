#pragma once
#include "Tetris.h"

class CTetris : public Tetris {
    public:
        static Matrix** setOfCBlockObjects;
        Matrix iCScreen;
        Matrix oCScreen; 
        Matrix currCBlk;
        CTetris():Tetris(){
            iCScreen=Matrix(iScreen);
            oCScreen=Matrix(iCScreen);
            };
        CTetris(int Dy, int Dx):Tetris(Dy,Dx){
            iCScreen=Matrix(iScreen);
            oCScreen=Matrix(iCScreen);
        };
        ~CTetris();
        static void init(int *setOfBlockArrays[], int MAX_BLK_TYPES, int MAX_BLK_DEGREES);
        TetrisState accept(char key);
        void deleteFullLines();
};
Matrix** CTetris::setOfCBlockObjects;