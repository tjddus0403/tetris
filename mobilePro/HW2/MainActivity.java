package ssu.btetris.singleuser;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.util.Random;

import ssu.btetris.model.Tetris;
import ssu.btetris.model.Tetris.TetrisState;

public class MainActivity extends AppCompatActivity {
    private TetrisView myTetView, peerTetView;
    private BlockView myBlkView, peerBlkView;
    private Button upArrowBtn, leftArrowBtn, rightArrowBtn, downArrowBtn, dropBtn, topLeftBtn, topRightBtn;
    private Button startBtn, pauseBtn, settingBtn, modeBtn, reservedBtn;
    private boolean gameStarted = false;
    private int dy = 25, dx = 15; // screen size

    private TetrisModel myTetModel;
    private Random random;
    private TetrisState state;
    private char currBlk, nextBlk;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private int timerCalls = 0;
    private Runnable runnableTimer = new Runnable() {
        @Override
        public void run() {
            if (gameStarted == true) {
                timerCalls++;
                Toast.makeText(MainActivity.this, "timerCalls="+timerCalls, Toast.LENGTH_SHORT).show();
                try {
                    state = myTetModel.accept('s');
                    myTetView.accept(myTetModel.getScreen());
                    if (state == TetrisState.NewBlock){
                        currBlk = nextBlk;
                        nextBlk = (char) ('0' + random.nextInt(Tetris.nBlockTypes));
                        state = myTetModel.accept(currBlk);
                        myTetView.accept(myTetModel.getScreen());
                        myBlkView.accept(myTetModel.getBlock(nextBlk));
                        myBlkView.invalidate();
                        if (state == TetrisState.Finished) {
                            gameStarted = false;
                            setButtonsState(false);
                            startBtn.setText("N");
                            Toast.makeText(MainActivity.this, "Game Over!", Toast.LENGTH_SHORT).show();
                        }
                    }
                    myTetView.invalidate();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                handler.removeCallbacks(runnableTimer);
                handler.postDelayed(this, 1000);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        myTetView = (TetrisView) findViewById(R.id.myTetrisView);
        peerTetView = (TetrisView) findViewById(R.id.peerTetrisView);
        myBlkView = (BlockView) findViewById(R.id.myBlockView);
        peerBlkView = (BlockView) findViewById(R.id.peerBlockView);
        startBtn = (Button) findViewById(R.id.startBtn);
        pauseBtn = (Button) findViewById(R.id.pauseBtn);
        settingBtn = (Button) findViewById(R.id.settingBtn);
        modeBtn = (Button) findViewById(R.id.modeBtn);
        reservedBtn = (Button) findViewById(R.id.reservedBtn);
        upArrowBtn = (Button) findViewById(R.id.upArrowBtn);
        leftArrowBtn = (Button) findViewById(R.id.leftArrowBtn);
        rightArrowBtn = (Button) findViewById(R.id.rightArrowBtn);
        downArrowBtn = (Button) findViewById(R.id.downArrowBtn);
        dropBtn = (Button) findViewById(R.id.dropBtn);
        topLeftBtn = (Button) findViewById(R.id.topLeftBtn);
        topRightBtn = (Button) findViewById(R.id.topRightBtn);

        startBtn.setOnClickListener(OnClickListener);
        pauseBtn.setOnClickListener(OnClickListener);
        settingBtn.setOnClickListener(OnClickListener);
        modeBtn.setOnClickListener(OnClickListener);
        reservedBtn.setOnClickListener(OnClickListener);

        upArrowBtn.setOnClickListener(OnClickListener);
        leftArrowBtn.setOnClickListener(OnClickListener);
        rightArrowBtn.setOnClickListener(OnClickListener);
        downArrowBtn.setOnClickListener(OnClickListener);
        dropBtn.setOnClickListener(OnClickListener);

        setButtonsState(false);
    }
    private void setButtonsState(boolean flag) {
        pauseBtn.setEnabled(flag);
        settingBtn.setEnabled(false);
        modeBtn.setEnabled(false);
        reservedBtn.setEnabled(false);

        upArrowBtn.setEnabled(flag);
        leftArrowBtn.setEnabled(flag);
        rightArrowBtn.setEnabled(flag);
        downArrowBtn.setEnabled(flag);
        dropBtn.setEnabled(flag);
        topLeftBtn.setEnabled(false); // always disabled
        topRightBtn.setEnabled(false); // always disabled
    }
    private View.OnClickListener OnClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            char key;
            int id = v.getId();
            switch (id) {
                case R.id.startBtn: key = 'N';
                    if (gameStarted == false) {
                        gameStarted = true;
                        setButtonsState(true);
                        startBtn.setText("Q"); // 'Q' means Quit.
                        Toast.makeText(MainActivity.this, "Game Started!", Toast.LENGTH_SHORT).show();
                        try {
                            random = new Random();
                            myTetModel = new TetrisModel(dy, dx);
                            myTetView.init(dy, dx, Tetris.iScreenDw);
                            myBlkView.init(Tetris.iScreenDw);
                            currBlk = (char) ('0' + random.nextInt(Tetris.nBlockTypes));
                            nextBlk = (char) ('0' + random.nextInt(Tetris.nBlockTypes));
                            state = myTetModel.accept(currBlk);
                            myTetView.accept(myTetModel.getScreen());
                            myBlkView.accept(myTetModel.getBlock(nextBlk));
                            myTetView.invalidate();
                            myBlkView.invalidate();
                        }
                        catch (Exception e) {
                            e.printStackTrace();
                        }

                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
                            handler.postDelayed(runnableTimer, 0, 1000);
                    }
                    else {
                        timerCalls = 0;
                        gameStarted = false;
                        setButtonsState(false);
                        startBtn.setText("N"); // 'N' means New Game.
                        Toast.makeText(MainActivity.this, "Game Over!", Toast.LENGTH_SHORT).show();
                    }
                    return;
                case R.id.pauseBtn: key = 'P'; break;
                case R.id.upArrowBtn: key = 'w'; break;
                case R.id.leftArrowBtn: key = 'a'; break;
                case R.id.rightArrowBtn: key = 'd'; break;
                case R.id.downArrowBtn: key = 's'; break;
                case R.id.dropBtn: key = ' '; break;
                default: return;
            }
            try {
                state = myTetModel.accept(key);
                myTetView.accept(myTetModel.getScreen());
                if (state == TetrisState.NewBlock){
                    currBlk = nextBlk;
                    nextBlk = (char) ('0' + random.nextInt(Tetris.nBlockTypes));
                    state = myTetModel.accept(currBlk);
                    myTetView.accept(myTetModel.getScreen());
                    myBlkView.accept(myTetModel.getBlock(nextBlk));
                    myBlkView.invalidate();
                    if (state == TetrisState.Finished) {
                        gameStarted = false;
                        setButtonsState(false);
                        startBtn.setText("N");
                        Toast.makeText(MainActivity.this, "Game Over!", Toast.LENGTH_SHORT).show();
                    }
                }
                myTetView.invalidate();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };
}