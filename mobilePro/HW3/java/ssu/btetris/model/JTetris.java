package ssu.btetris.model;

import java.io.Serializable;

public class JTetris implements Serializable {
    public enum JTetrisState { // need to be defined as inner class within Tetris, otherwise requires a separate Java file.
        Running(0), NewBlock(1), Finished(2);
        private final int value;
        private JTetrisState(int value) { this.value = value; }
        public int value() { return value; }
    }
    public static int iScreenDw;		// large enough to cover the largest block
    public static int nBlockTypes;		// number of block types (typically 8)
    public static int nBlockDegrees;	// number of block degrees (typically 3)
    public static Matrix[][] setOfBlockObjects;	// Matrix object arrays of all blocks
    private static Matrix[][] createSetOfBlocks(int[][][][] setOfArrays) throws Exception {
        int ntypes = setOfArrays.length;
        int ndegrees = setOfArrays[0].length;
        Matrix[][] setOfObjects = new Matrix[nBlockTypes][nBlockDegrees];
        for (int t = 0; t < ntypes; t++)
            for (int d = 0; d < ndegrees; d++)
                setOfObjects[t][d] = new Matrix(setOfArrays[t][d]);
        return setOfObjects;
    }
    private static int max(int a, int b) { return (a > b ? a : b); }
    private static int findLargestBlockSize(int[][][][] setOfArrays) {
        int size, max_size = 0;
        for (int t = 0; t < nBlockTypes; t++) {
            for (int d = 0; d < nBlockDegrees; d++) {
                size = setOfArrays[t][d].length;
                max_size = max(max_size, size);
            }
        }
        //System.out.println("max_size = "+max_size);
        return max_size;
    }
    private Matrix changeToBinary(Matrix temp) throws Exception {
        int dx = temp.get_dx();
        int dy = temp.get_dy();
        Matrix result=new Matrix(dx, dy);
        int[][] resultArray=result.get_array();
        int[][] tempArray=temp.get_array();
        for (int x = 0; x < dx; x++) {
            for (int y = 0; y < dy; y++) {
                if(tempArray[x][y]!=0)
                    resultArray[x][y]=1;
                else
                    resultArray[x][y]=0;
            }
        }
        result=new Matrix(resultArray);
        return result;
    }
    private boolean anyCGreaterThan(Matrix temp, int size) throws Exception{
        Matrix check=new Matrix(changeToBinary(temp));
        return check.anyGreaterThan(size);
    }
    public static void init(int[][][][] setOfBlockArrays) throws Exception { // initialize static variables
        nBlockTypes = setOfBlockArrays.length;
        nBlockDegrees = setOfBlockArrays[0].length;
        setOfBlockObjects = createSetOfBlocks(setOfBlockArrays);
        iScreenDw = findLargestBlockSize(setOfBlockArrays);
    }
    private int iScreenDy;	// height of the background screen (excluding walls)
    private int iScreenDx;  // width of the background screen (excluding walls)
    private JTetris.JTetrisState state;		// game state
    private int top;		// y of the top left corner of the current block
    private int left;		// x of the top left corner of the current block
    private Matrix iScreen;	// input screen (as background)
    private Matrix oScreen;	// output screen

    public Matrix get_oScreen() {
        return oScreen;
    }
    private Matrix currBlk;	// current block
    private Matrix currbBlk;
    private int idxBlockType;	// index for the current block type
    private int idxBlockDegree; // index for the current block degree
    private int[][] createArrayScreen(int dy, int dx, int dw) {
        int y, x;
        int[][] array = new int[dy + dw][dx + 2*dw];
        for (y = 0; y < array.length; y++)
            for (x = 0; x < dw; x++)
                array[y][x] = 1;
        for (y = 0; y < array.length; y++)
            for (x = dw + dx; x < array[0].length; x++)
                array[y][x] = 1;
        for (y = dy; y < array.length; y++)
            for (x = 0; x < array[0].length; x++)
                array[y][x] = 1;
        return array;
    }
    public JTetris(int cy, int cx) throws Exception { // initialize dynamic variables
        if (cy < iScreenDw || cx < iScreenDw)
            throw new JTetris.JTetrisException("too small screen");
        iScreenDy = cy;
        iScreenDx = cx;
        int[][] arrayScreen = createArrayScreen(iScreenDy, iScreenDx, iScreenDw);
        state = JTetris.JTetrisState.NewBlock;	// The game should start with a new block needed!
        iScreen = new Matrix(arrayScreen);
        oScreen = new Matrix(iScreen);
    }
    public JTetris.JTetrisState accept(char key) throws Exception {
        Matrix tempBlk;
        Matrix tempbBlk;
        if (state == JTetris.JTetrisState.NewBlock) {
            //oScreen = deleteFullLines(oScreen, currBlk, top, iScreenDy, iScreenDx, iScreenDw);
            oScreen=deleteJ(oScreen, currBlk, iScreenDy, iScreenDx, iScreenDw);
            iScreen.paste(oScreen, 0, 0);
            state = JTetris.JTetrisState.Running;
            idxBlockType = key - '0'; // copied from key
            idxBlockDegree = 0;
            currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
            currbBlk=changeToBinary(currBlk);
            top = 0;
            left = iScreenDw + iScreenDx / 2 - (currBlk.get_dx()+1) / 2;
            tempBlk = iScreen.clip(top, left, top+currBlk.get_dy(), left+currBlk.get_dx());
            tempbBlk=changeToBinary(tempBlk);
            tempBlk = tempBlk.add(currBlk);
            tempbBlk=tempbBlk.add(currbBlk);
            oScreen.paste(iScreen, 0, 0);
            oScreen.paste(tempBlk, top, left); System.out.println();
            if (tempbBlk.anyGreaterThan(1)) {
                state = JTetris.JTetrisState.Finished;	// System.out.println("Game Over!");
                return state;	// System.exit(0);
            }
            return state;		// should require a key input
        }

        switch(key) {
            case 'a': left--; break; // move left
            case 'd': left++; break; // move right
            case 's': top++; break; // move down
            case 'w': // rotateCW
                idxBlockDegree = (idxBlockDegree+1)%nBlockDegrees;
                currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
                currbBlk=changeToBinary(currBlk);
                break;
            case ' ': // drop the block
                do {
                    top++;
                    tempBlk = iScreen.clip(top, left, top+currBlk.get_dy(), left+currBlk.get_dx());
                    tempbBlk=changeToBinary(tempBlk);
                    tempBlk = tempBlk.add(currBlk);
                    tempbBlk=tempbBlk.add(currbBlk);
                } while (tempbBlk.anyGreaterThan(1) == false);
                break;
            default: System.out.println("unknown key!");
        }
        tempBlk = iScreen.clip(top, left, top+currBlk.get_dy(), left+currBlk.get_dx());
        tempbBlk=changeToBinary(tempBlk);
        tempBlk = tempBlk.add(currBlk);
        tempbBlk=tempbBlk.add(currbBlk);
        if (tempbBlk.anyGreaterThan(1)) {
            switch(key) {
                case 'a': left++; break; // undo: move right
                case 'd': left--; break; // undo: move left
                case 's': top--; state = JTetris.JTetrisState.NewBlock; break; // undo: move up
                case 'w': // undo: rotateCCW
                    idxBlockDegree = (idxBlockDegree+nBlockDegrees-1)%nBlockDegrees;
                    currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
                    currbBlk=changeToBinary(currBlk);
                    break;
                case ' ': top--; state = JTetris.JTetrisState.NewBlock; break; // undo: move up
            }
            tempBlk = iScreen.clip(top, left, top+currBlk.get_dy(), left+currBlk.get_dx());
            tempbBlk=changeToBinary(tempBlk);
            tempBlk = tempBlk.add(currBlk);
            tempbBlk=tempbBlk.add(currbBlk);
        }
        oScreen.paste(iScreen, 0, 0);
        oScreen.paste(tempBlk, top, left);

        return state;
    }
    private Matrix deleteJ(Matrix screen, Matrix blk, int dy, int dx, int dw) throws Exception{
        if(blk==null) return screen;
        int[][] screen_arr=screen.get_array();
        int[] delJewx=new int[200];
        int[] delJewy=new int[200];
        int delnum;
        int numOfJew=1;
        int lastJew=1;
        while(true) {
            //가로 삭제 확인
            screen_arr=screen.get_array();
            delnum=0;
            for (int y = 0; y < dy; y++) {
                for (int x = dw; x < dw + dx + 1; x++) {
                    if (lastJew != 0 && lastJew == screen_arr[y][x]) {
                        numOfJew++;
                    } else {
                        if (numOfJew >= 3) {
                            for (int i = 0; i < numOfJew; i++) {
                                delJewx[delnum] = x - 1 - i;
                                delJewy[delnum] = y;
                                delnum++;
                            }
                        }
                        numOfJew = 1;
                        lastJew = screen_arr[y][x];
                    }
                }
            }
            //세로 삭제 확인
            for (int x = dw; x < dw + dx; x++) {
                for (int y = 0; y < dy + 1; y++) {
                    if (lastJew != 0 && lastJew == screen_arr[y][x]) {
                        numOfJew++;
                    } else {
                        if (numOfJew >= 3) {
                            for (int i = 0; i < numOfJew; i++) {
                                delJewy[delnum] = y - 1 - i;
                                delJewx[delnum] = x;
                                delnum++;
                            }
                        }
                        numOfJew = 1;
                        lastJew = screen_arr[y][x];
                    }
                }
            }
            if(delnum==0){
                break;
            }
            for (int i = 0; i < delnum; i++) { //delete 되어야 하는 칸 = -1
                int x = delJewx[i];
                int y = delJewy[i];
                screen_arr[y][x] = -1;
            }
            screen = new Matrix(screen_arr);
            Matrix temp;
            Matrix zero = new Matrix(1, 1);
            for (int y = 0; y < dy; y++) {
                for (int x = dw; x < dw + dx; x++) {
                    if (screen_arr[y][x] == -1) {
                        temp = screen.clip(0, x, y, x + 1);
                        screen.paste(temp, 1, x);
                        screen.paste(zero, 0, x);
                    }
                }
            }
        }
        return screen;
    }
    class JTetrisException extends Exception {
        public JTetrisException() { super("JTetris Exception"); }
        public JTetrisException(String msg) { super(msg); }
    }
}
