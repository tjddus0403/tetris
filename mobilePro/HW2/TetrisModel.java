package ssu.btetris.singleuser;

import ssu.btetris.model.Matrix;
import ssu.btetris.model.Tetris;
import ssu.btetris.model.CTetris;

public class TetrisModel { // derived from TestMain.java in Lecture 5
    private CTetris board;
    public Matrix getBlock(char type) { return board.setOfCBlockObjects[type - '0'][0]; }
    public Matrix getScreen() { return board.get_oScreen(); }
    public TetrisModel(int dy, int dx) throws Exception {
        CTetris.init(setOfBlockArrays);
        board = new CTetris(dy, dx);
    }
    public CTetris.TetrisState accept(char ch) throws Exception { return board.accept(ch); }
    private int[][][][] setOfBlockArrays = { // [7][4][?][?]
            {
                    {
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                    },
                    {
                            { 0, 0, 0, 0 },
                            { 10, 10, 10, 10 },
                            { 0, 0, 0, 0 },
                            { 0, 0, 0, 0 }
                    },
                    {
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                            { 0, 0, 10, 0 },
                    },
                    {
                            { 0, 0, 0, 0 },
                            { 10, 10, 10, 10 },
                            { 0, 0, 0, 0 },
                            { 0, 0, 0, 0 }
                    }
            },
            {
                    {
                            { 20, 0, 0 },
                            { 20, 20, 20 },
                            { 0, 0, 0 }
                    },
                    {
                            { 0, 20, 20 },
                            { 0, 20, 0 },
                            { 0, 20, 0 }
                    },
                    {
                            { 0, 0, 0 },
                            { 20, 20, 20 },
                            { 0, 0, 20 }
                    },
                    {
                            { 0, 20, 0 },
                            { 0, 20, 0 },
                            { 20, 20, 0 }
                    }
            },
            {
                    {
                            { 0, 0, 30 },
                            { 30, 30, 30 },
                            { 0, 0, 0 }
                    },
                    {
                            { 0, 30, 0 },
                            { 0, 30, 0 },
                            { 0, 30, 30 }
                    },
                    {
                            { 0, 0, 0 },
                            { 30, 30, 30 },
                            { 30, 0, 0 }
                    },
                    {
                            { 30, 30, 0 },
                            { 0, 30, 0 },
                            { 0, 30, 0 }
                    }
            },
            {
                    {
                            { 0, 40, 0 },
                            { 40, 40, 40 },
                            { 0, 0, 0 }
                    },
                    {
                            { 0, 40, 0 },
                            { 0, 40, 40 },
                            { 0, 40, 0 }
                    },
                    {
                            { 0, 0, 0 },
                            { 40, 40, 40 },
                            { 0, 40, 0 }
                    },
                    {
                            { 0, 40, 0 },
                            { 40, 40, 0 },
                            { 0, 40, 0 }
                    }
            },
            {
                    {
                            { 0, 50, 0 },
                            { 50, 50, 0 },
                            { 50, 0, 0 }
                    },
                    {
                            { 50, 50, 0 },
                            { 0, 50, 50 },
                            { 0, 0, 0 }
                    },
                    {
                            { 0, 50, 0 },
                            { 50, 50, 0 },
                            { 50, 0, 0 }
                    },
                    {
                            { 50, 50, 0 },
                            { 0, 50, 50 },
                            { 0, 0, 0 }
                    }
            },
            {
                    {
                            { 0, 60, 0 },
                            { 0, 60, 60 },
                            { 0, 0, 60 }
                    },
                    {
                            { 0, 60, 60 },
                            { 60, 60, 0 },
                            { 0, 0, 0 }
                    },
                    {
                            { 0, 60, 0 },
                            { 0, 60, 60 },
                            { 0, 0, 60 }
                    },
                    {
                            { 0, 60, 60 },
                            { 60, 60, 0 },
                            { 0, 0, 0 }
                    }
            },
            {
                    {
                            { 70, 70 },
                            { 70, 70 }
                    },
                    {
                            { 70, 70 },
                            { 70, 70 }
                    },
                    {
                            { 70, 70 },
                            { 70, 70 }
                    },
                    {
                            { 70, 70 },
                            { 70, 70 }
                    }
            }
    }; // end of arrayBlock
}

