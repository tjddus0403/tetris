package ssu.btetris.singleuser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;
import android.util.Log;

import ssu.btetris.model.Matrix;

public class TetrisView extends View {
    private int dy, dx;   // view size in unit blocks
    private int by = 0, bx = 0;     // unit block size
    private int color = Color.BLACK;
    private int skip = 0;
    private Matrix screen = null;
    private Paint paint = new Paint();
    public int ondraw_calls = 0;

    public TetrisView(Context context, AttributeSet attrs, int defStyle) { super(context, attrs, defStyle); }
    public TetrisView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    public TetrisView(Context context) {
        super(context);
    }
    public void init(int y, int x, int w) {
        dy = y; dx = x;
        skip = w;
    }
    public void accept(Matrix m)  { screen = m; }
    public void onDraw(Canvas canvas) {
        int cy = 10;
        int cx = 10;
        ondraw_calls++;
        if (screen == null) return;
        int[][] array = screen.get_array();
        super.onDraw(canvas);
        paint.setStyle(Paint.Style.FILL);
        bx = (getWidth() - 20 - ((dx-1)*5))/dx;
        by = (getHeight() - 20 - ((dy-1)*5))/dy;
        //Log.d("TetrisView", "dy="+dy+",dx="+dx+",bx="+bx+",by="+by+",skip="+skip);
        Bitmap picture;
        for (int y = 0; y < dy; y++) {
            for (int x = skip; x < skip + dx; x++) {
                switch(array[y][x]) {
                    case 0: paint.setColor(Color.BLACK); break;
                    case 10:
                        picture= BitmapFactory.decodeResource(getResources(), R.drawable.red);
                        canvas.drawBitmap(picture,cx,cy,null);
                        picture.recycle();
                        break;
                    case 20:
                        picture= BitmapFactory.decodeResource(getResources(), R.drawable.yellow);
                        canvas.drawBitmap(picture,cx,cy,null);
                        picture.recycle();
                        break;
                    case 30:
                        picture= BitmapFactory.decodeResource(getResources(), R.drawable.green);
                        canvas.drawBitmap(picture,cx,cy,null);
                        picture.recycle();
                        break;
                    default : paint.setColor(Color.WHITE); break;
                }
                cx += (bx + 5);
            }
            cx = 10;
            cy += (by + 5);
        }
    }
}
