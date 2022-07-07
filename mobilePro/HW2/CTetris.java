package ssu.btetris.model;

import ssu.btetris.model.Matrix;
import ssu.btetris.model.Tetris;

public class CTetris extends Tetris {
    public static Matrix[][] setOfCBlockObjects;
    private Matrix iCScreen;
    public Matrix oCScreen;
    private Matrix currCBlk;
    private Matrix tmpScreen;

    private static void changeToTetris(int[][][][] setOfArrays) throws Exception {
        int ntypes = setOfArrays.length;
        int ndegrees = setOfArrays[0].length;
        for (int t = 0; t < ntypes; t++) {
            for (int d = 0; d < ndegrees; d++) {
                for(int i=0; i<setOfArrays[t][d].length; i++){
                    for(int j=0; j<setOfArrays[t][d].length; j++){
                        if(setOfArrays[t][d][i][j]!=0)
                            setOfArrays[t][d][i][j]=1;
                        else
                            setOfArrays[t][d][i][j]=0;
                    }
                }
            }
        }
    }

    public static void init(int[][][][] setOfBlockArrays) throws Exception {
        nBlockTypes = setOfBlockArrays.length;
        nBlockDegrees = setOfBlockArrays[0].length;
        setOfCBlockObjects=createSetOfBlocks(setOfBlockArrays);
        changeToTetris(setOfBlockArrays);
        Tetris.init(setOfBlockArrays);
    }

    public CTetris(int cy, int cx) throws Exception {
        super(cy, cx);
        iCScreen = new Matrix(iScreen);
        oCScreen = new Matrix(oScreen);
        tmpScreen=new Matrix(oScreen);
    }

    public TetrisState accept(char key) throws Exception {
        Matrix tempCBlk;
        oScreen=new Matrix(tmpScreen);
        if (state == TetrisState.NewBlock) {
            oCScreen=deleteFullLines(oCScreen, currCBlk, top, iScreenDy, iScreenDx, iScreenDw);
            iCScreen = new Matrix(oCScreen);
        }
        state = super.accept(key);
        currCBlk = setOfCBlockObjects[idxBlockType][idxBlockDegree];
        tempCBlk = iCScreen.clip(top, left, top + currCBlk.get_dy(), left + currCBlk.get_dx());
        tempCBlk = tempCBlk.add(currCBlk);
        oCScreen = new Matrix(iCScreen);
        oCScreen.paste(tempCBlk, top, left); //System.out.println();
        tmpScreen=new Matrix(oScreen);
        oScreen=new Matrix(oCScreen);
        return state;
    }
    private Matrix deleteFullLines(Matrix screen, Matrix blk, int top, int dy, int dx, int dw) throws Exception {
        Matrix line, zero, temp, ctmp;
        if (blk == null) return screen; // called right after the game starts!!
        int cy, y, nDeleted = 0,nScanned = blk.get_dy();
        if (top + blk.get_dy() - 1 >= dy)
            nScanned -= (top + blk.get_dy() - dy);
        zero = new Matrix(1, dx - 2*dw);
        for (y = nScanned - 1; y >= 0 ; y--) {
            cy = top + y + nDeleted;
            line = oScreen.clip(cy, 0, cy + 1, screen.get_dx());
            if (line.sum() == oScreen.get_dx()) {
                temp = oScreen.clip(0, 0, cy, screen.get_dx());
                ctmp=screen.clip(0, 0, cy, screen.get_dx());
                oScreen.paste(temp, 1, 0);
                screen.paste(ctmp, 1, 0);
                oScreen.paste(zero, 0, dw);
                screen.paste(zero, 0, dw);

                nDeleted++;
            }
        }
        return screen;
    }
}