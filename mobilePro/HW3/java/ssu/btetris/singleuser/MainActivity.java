package ssu.btetris.singleuser;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

import java.util.Random;

import ssu.btetris.model.JTetris;
import ssu.btetris.model.Matrix;
import ssu.btetris.model.JTetris.JTetrisState;

public class MainActivity extends AppCompatActivity {
    private TetrisView myTetView, peerTetView;
    private BlockView myBlkView, peerBlkView;
    private Button upArrowBtn, leftArrowBtn, rightArrowBtn, downArrowBtn, dropBtn, topLeftBtn, topRightBtn;
    private Button startBtn, pauseBtn, settingBtn, modeBtn, reservedBtn;
    private AlertDialog quitBtn;
    //private boolean gameStarted = false;
    private int dy = 25, dx = 15; // screen size

    private final int reqCode4SettingActivity = 0;
    private String servHostName = "255.255.255.255";
    private int servPortNo = 9999;

    private JTetrisModel myTetModel;
    private Random random;
    //private TetrisState state;
    private char currBlk, nextBlk;

    private GameState gameState = GameState.Initial;
    private GameState savedState;
    private enum GameState {
        Error(-1), Initial(0), Running(1), Paused(2);
        private final int value;
        private GameState(int value) { this.value = value; }
        public int value() { return value; }
        public static GameState stateFromInteger(int value) {
            switch (value) {
                case -1: return Error;
                case 0: return Initial;
                case 1: return Running;
                case 2: return Paused;
                default: return null;
            }
        }
    }
    private enum GameCommand {
        NOP(-1), Quit(0), Start(1), Pause(2), Resume(3), Update(4), Recover(5);
        private final int value;
        private GameCommand(int value) { this.value = value; }
        public int value() { return value; }
    }
    int stateTransMatrix[][] = { // stateTransMatrix[currGameState][GameCommand] --> nextGameState
            { -1, 1, -1, -1, -1, 2 },  // [Initial][Start] --> Running
            // [Initial][Recover] --> Paused
            { 0, -1, 2, -1, 1, -1 },    // [Running][Quit] --> Initial,
            // [Running][Pause] --> Paused,
            // [Running][Update] --> Running,
            { 0, 1, -1, 1, 2, -1 },     // [Paused][Quit] --> Initial
            // [Paused][Start,Resume] --> Running
            // [Paused][Update] --> Paused
    };
    private void executeCommand(GameCommand cmd, char key) {
        GameState newState = GameState.stateFromInteger(stateTransMatrix[gameState.value()][cmd.value()]);
        //if (newState == GameState.Error)
        Log.d("MainActivity", "newState="+newState+" <- (gameState="+gameState+",cmd="+cmd+")");
        gameState = newState;
        switch (cmd.value()) {
            case 0: quitGame(); break;
            case 1: startGame(); launchTimer(); break;
            case 2: pauseGame(); break;
            case 3: resumeGame(); launchTimer(); break;
            case 4: updateModel(key); break;
            case 5: recoverGame(); break;
            default: Log.d("MainActivity", "unknown command!!!"); break;
        }
    }
    private View.OnClickListener OnClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            char key;
            int id = v.getId();
            GameCommand cmd = GameCommand.NOP;
            switch (id) {
                case R.id.startBtn: key = 'N';
                    if (gameState == GameState.Initial) cmd = GameCommand.Start;
                    else if (gameState == GameState.Running) {
                        // cmd = GameCommand.Quit;
                        savedState = gameState;
                        executeCommand(GameCommand.Pause, 'P');
                        quitBtn.show();
                        return;
                    }
                    else if (gameState == GameState.Paused) {
                        // cmd = GameCommand.Quit;
                        savedState = gameState;
                        quitBtn.show();
                        return;
                    }
                    break;
                case R.id.pauseBtn: key = 'P';
                    if (gameState == GameState.Running) cmd = GameCommand.Pause;
                    else if (gameState == GameState.Paused) cmd = GameCommand.Resume;
                    break;
                case R.id.settingBtn: key = 'S';
                    if (gameState == GameState.Initial) startSettingActivity(reqCode4SettingActivity);
                    return;
                case R.id.upArrowBtn: key = 'w'; cmd = GameCommand.Update; break;
                case R.id.leftArrowBtn: key = 'a'; cmd = GameCommand.Update; break;
                case R.id.rightArrowBtn: key = 'd'; cmd = GameCommand.Update; break;
                case R.id.downArrowBtn: key = 's'; cmd = GameCommand.Update; break;
                case R.id.dropBtn: key = ' '; cmd = GameCommand.Update; break;
                default: return;
            }
            executeCommand(cmd, key);
        }
    };
    private  void logMatrix(Matrix m) {
        int dy = m.get_dy();
        int dx = m.get_dx();
        int array[][] = m.get_array();
        for (int y=0; y < dy; y++) {
            for (int x=0; x < dx; x++) {
                if (array[y][x] == 0) System.out.print("□ ");
                else if (array[y][x] == 1) System.out.print("■ ");
                else System.out.print("X ");
            }
            System.out.println();
        }
    }
    private void recoverGame() {
        setButtonsState(); // no argument of flag
        startBtn.setText("Q"); // 'Q' means Quit.
        if (savedState == GameState.Running)
            pauseBtn.setText("P"); // 'P' means Pause.
        else if (savedState == GameState.Paused)
            pauseBtn.setText("R"); // 'R' means Resume.
        //[BEGIN] recover random, TetrisView, and BlockView.
        random = new Random();
        myTetView.init(dy, dx, JTetris.iScreenDw);
        myBlkView.init(JTetris.iScreenDw);
        //logMatrix(myTetModel.getScreen());
        myTetView.accept(myTetModel.getScreen());
        myBlkView.accept(myTetModel.getBlock(nextBlk));
        myTetView.invalidate();
        myBlkView.invalidate();
        //[END] recover random, TetrisView, and BlockView.
        Toast.makeText(MainActivity.this, "Game Recovered!", Toast.LENGTH_SHORT).show();
    }
    private void updateModel(char key) {
        try {
            JTetrisState state; // model state
            state = myTetModel.accept(key);
            myTetView.accept(myTetModel.getScreen());
            if (state == JTetrisState.NewBlock){
                currBlk = nextBlk;
                nextBlk = (char) ('0' + random.nextInt(JTetris.nBlockTypes));
                state = myTetModel.accept(currBlk);
                myTetView.accept(myTetModel.getScreen());
                myBlkView.accept(myTetModel.getBlock(nextBlk));
                myBlkView.invalidate();
                if (state == JTetrisState.Finished)
                    executeCommand(GameCommand.Quit, 'Q');
            }
            myTetView.invalidate();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void startGame() {
        setButtonsState(); // no argument of flag
        startBtn.setText("Q"); // 'Q' means Quit.
        pauseBtn.setText("P"); // 'P' means Pause.
        Toast.makeText(MainActivity.this, "Game Started!", Toast.LENGTH_SHORT).show();
        try {
            JTetrisState state; // model state
            random = new Random();
            myTetModel = new JTetrisModel(dy, dx);
            myTetView.init(dy, dx, JTetris.iScreenDw);
            myBlkView.init(JTetris.iScreenDw);
            currBlk = (char) ('0' + random.nextInt(JTetris.nBlockTypes));
            nextBlk = (char) ('0' + random.nextInt(JTetris.nBlockTypes));
            state = myTetModel.accept(currBlk);
            myTetView.accept(myTetModel.getScreen());
            myBlkView.accept(myTetModel.getBlock(nextBlk));
            myTetView.invalidate();
            myBlkView.invalidate();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void quitGame() {
        setButtonsState(); // no argument of flag
        startBtn.setText("N"); // 'N' means New Game.
        Toast.makeText(MainActivity.this, "Game Over!", Toast.LENGTH_SHORT).show();
    }
    private void pauseGame() {
        setButtonsState();
        pauseBtn.setText("R");
        Toast.makeText(MainActivity.this, "Game Paused!", Toast.LENGTH_SHORT).show();
    }
    private void resumeGame() {
        setButtonsState();
        pauseBtn.setText("P");
        Toast.makeText(MainActivity.this, "Game Resumed!", Toast.LENGTH_SHORT).show();
    }
    private final Handler handler = new Handler(Looper.getMainLooper());
    private Runnable runnableTimer = new Runnable() {
        @Override
        public void run() {
            if (gameState == GameState.Running) {
                updateModel('s');
                Log.d("MainActivity", "ondraws="+myTetView.ondraw_calls);
                handler.postDelayed(this, 1000);
            }
        }
    };
    private void launchTimer() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) // P means 'Android Pie', aka Android 9.0
            handler.postDelayed(runnableTimer, 0, 1000);
    }
    @Override
    protected void onPause() {
        super.onPause();
        savedState = gameState;
        Log.d("MainActivity", "onPause(gameState="+gameState+",savedState="+savedState+")");
        if (gameState == GameState.Running)
            executeCommand(GameCommand.Pause, 'P');
    }
    @Override
    protected void onResume() {
        super.onResume();
        Log.d("MainActivity", "onResume(gameState="+gameState+",savedState="+savedState+")");
        if (savedState == GameState.Running) {
            savedState = GameState.Paused;
            executeCommand(GameCommand.Resume, 'R');
        }
    }
    boolean buttonState[][] = { // buttonState[currGameState][btnID] --> enable/disable
            { true, false, false }, // [Initial][startBtn] --> enabled
            { true, true, true },   // [Running][anyBtn] --> enabled
            { true, true, false },  // [Paused][startBtn,resumeBtn] --> enabled
    };
    private void setButtonsState() {
        boolean startFlag = buttonState[gameState.value()][0];
        boolean pauseFlag = buttonState[gameState.value()][1];
        boolean defaultFlag = buttonState[gameState.value()][2];
        startBtn.setEnabled(startFlag);
        pauseBtn.setEnabled(pauseFlag);
        if (gameState == GameState.Initial) settingBtn.setEnabled(true);
        else settingBtn.setEnabled(false);
        modeBtn.setEnabled(false);
        reservedBtn.setEnabled(false);

        upArrowBtn.setEnabled(defaultFlag);
        leftArrowBtn.setEnabled(defaultFlag);
        rightArrowBtn.setEnabled(defaultFlag);
        downArrowBtn.setEnabled(defaultFlag);
        dropBtn.setEnabled(defaultFlag);
        topLeftBtn.setEnabled(false); // always disabled
        topRightBtn.setEnabled(false); // always disabled
    }
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.d("MainActivity", "onSave(gameState="+gameState+",savedState="+savedState+")");
        outState.putSerializable("myTetModel", myTetModel);
        outState.putInt("savedState", savedState.value());
        outState.putChar("currBlk", currBlk);
        outState.putChar("nextBlk", nextBlk);
    }
    @Override
    protected void onRestoreInstanceState(Bundle inState) {
        super.onRestoreInstanceState(inState);
        savedState = GameState.stateFromInteger(inState.getInt("savedState"));
        Log.d("MainActivity", "onRestore(gameState="+gameState+",savedState="+savedState+")");
        if (savedState != GameState.Initial) {
            myTetModel = (JTetrisModel) inState.getSerializable("myTetModel");
            currBlk = inState.getChar("currBlk");
            nextBlk = inState.getChar("nextBlk");
            executeCommand(GameCommand.Recover, 'C');
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        WindowManager wm = getWindowManager();
        if (wm == null) return;
        int rotation = wm.getDefaultDisplay().getRotation();
        if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) {
            Log.d("MainActivity", "onCreate(portrait mode)");
            setContentView(R.layout.activity_main);
        }
        else { // rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270
            Log.d("MainActivity", "onCreate(landscape mode)");
            setContentView(R.layout.activity_main_landscape);
        }
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
        quitBtn = (AlertDialog) AlertDialogBtnCreate();

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

        setButtonsState(); // no argument of flag
    }
    private AlertDialog AlertDialogBtnCreate() {
        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
        builder.setTitle("JTetris: ")
                .setMessage("Do you want to quit?")
                .setCancelable(false)
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        executeCommand(GameCommand.Quit, 'Q');
                    }
                })
                .setNegativeButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        if (savedState == GameState.Running)
                            executeCommand(GameCommand.Resume, 'R');
                    }
                });
        return builder.create();
    }
    private void startSettingActivity(int reqCode) {
        Intent reqmsg = new Intent(MainActivity.this, SettingActivity.class);
        reqmsg.putExtra("servHostName", servHostName);
        reqmsg.putExtra("servPortNo", String.valueOf(servPortNo));
        startActivityResult.launch(reqmsg);
    }
    ActivityResultLauncher<Intent> startActivityResult = registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(),
            new ActivityResultCallback<ActivityResult>() {
                @Override
                public void onActivityResult(ActivityResult result) {
                    int rescode = result.getResultCode();
                    if (rescode == Activity.RESULT_OK) {
                        Intent res = result.getData();
                        servHostName = res.getStringExtra("servHostName");
                        String s = res.getStringExtra("servPortNo");
                        servPortNo = Integer.parseInt(s);
                        Log.d("MainActivity", "response=("+servHostName+","+servPortNo+")");
                    }
                    else if (rescode == Activity.RESULT_CANCELED)
                        Log.d("MainActivity", "response=(null)");
                }
            });
}