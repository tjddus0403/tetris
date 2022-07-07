package ssu.btetris.singleuser;

import java.io.Serializable;

import ssu.btetris.model.Matrix;
import ssu.btetris.model.JTetris;

public class JTetrisModel implements Serializable {
    private JTetris board;
    public boolean isBlockIndex(char key) {
        int nBlks = setOfBlockArrays.length;
        int idx = key - '0';
        if (idx >= 0 && idx < nBlks)
            return true;
        else
            return false;
    }
    public Matrix getBlock(char type) { return board.setOfBlockObjects[type - '0'][0]; }
    public Matrix getScreen() { return board.get_oScreen(); }
    public JTetrisModel(int dy, int dx) throws Exception {
        JTetris.init(setOfBlockArrays);
        board = new JTetris(dy, dx);
    }
    public JTetris.JTetrisState accept(char ch) throws Exception { return board.accept(ch); }
    private int[][][][] setOfBlockArrays = { // [7][4][?][?]
            {
                    {
                            {0,10,0},
                            {0,10,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,10,0},
                            {0,10,0}
                    },
                    {
                            {0,10,0},
                            {0,20,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,10,0},
                            {0,10,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,10,0},
                            {0,10,0}
                    },
                    {
                            {0,10,0},
                            {0,30,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,10,0},
                            {0,20,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,10,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,20,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,10,0},
                            {0,20,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,10,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,30,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,10,0},
                            {0,30,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,10,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,20,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,10,0},
                            {0,30,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,10,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,30,0},
                            {0,10,0}
                    }
            },
            {
                    {
                            {0,20,0},
                            {0,20,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,20,0},
                            {0,20,0}
                    },
                    {
                            {0,20,0},
                            {0,30,0},
                            {0,20,0}
                    }
            },
            {
                    {
                            {0,20,0},
                            {0,30,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,20,0},
                            {0,30,0}
                    },
                    {
                            {0,30,0},
                            {0,30,0},
                            {0,20,0}
                    }
            }
    }; // end of arrayBlock
}
