#pragma once
#include "Matrix.h"

enum TetrisState { Running, NewBlock, Finished };

class Tetris {
    /*private:
        static int nBlockTypes;
        static int nBlockDegrees;
        static Matrix** setOfBlockObjects;
        Matrix iScreen;
        int iScreenDx;
        int iScreenDy;
        int* arrayScreen;
        bool justStarted;
        int idxBlockDegree;
        TetrisState state;
        int top;
        int left;
        Matrix currBlk;*/
    public:
        static int nBlockTypes;
        static int nBlockDegrees;
        static Matrix** setOfBlockObjects;
        Matrix iScreen;
        Matrix oScreen; 
        int iScreenDx;
        int iScreenDy;
        int* arrayScreen;
        bool justStarted;
        int idxBlockDegree;
        TetrisState state;
        int top;
        int left;
        Matrix currBlk;
        ////////////////////////////////
        int idxBlockType;
        static int iScreenDw;
        Tetris();
        Tetris(int Dy, int Dx);
        ~Tetris();
        static void init(int *setOfBlockArrays[], int MAX_BLK_TYPES, int MAX_BLK_DEGREES);
        void createArrayScreen(int* arrayScreen);
        TetrisState accept(char key);
        void deleteFullLines();
};
int Tetris::iScreenDw=0;
Matrix** Tetris::setOfBlockObjects;
int Tetris::nBlockTypes=0;
int Tetris::nBlockDegrees=0;