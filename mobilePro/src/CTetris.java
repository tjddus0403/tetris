public class CTetris extends Tetris{
    private static Matrix[][] setOfCBlockObjects;
    private Matrix iCScreen;
    public Matrix oCScreen;
    private Matrix currCBlk;

    private static Matrix[][] createSetOfCBlocks(int[][][][] setOfArrays) throws Exception {
        int ntypes = setOfArrays.length;
        int ndegrees = setOfArrays[0].length;
        Matrix[][] setOfObjects = new Matrix[nBlockTypes][nBlockDegrees];
        for (int t = 0; t < ntypes; t++)
            for (int d = 0; d < ndegrees; d++)
                setOfObjects[t][d] = new Matrix(setOfArrays[t][d]);
        return setOfObjects;
    }
    public static void init(int[][][][] setOfBlockArrays) throws Exception {
        nBlockTypes = setOfBlockArrays.length;
        nBlockDegrees = setOfBlockArrays[0].length;
        setOfCBlockObjects=createSetOfCBlocks(setOfBlockArrays);
        Tetris.init(setOfBlockArrays);
    }

    public CTetris(int cy, int cx) throws Exception {
        super(cy, cx);
        iCScreen = new Matrix(iScreen);
        oCScreen = new Matrix(oScreen);
    }

    public TetrisState accept(char key) throws Exception {
        Matrix tempCBlk;
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