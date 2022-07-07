package ssu.btetris.singleuser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import ssu.btetris.model.Matrix;

/**
 * TODO: document your custom view class.
 */
public class BlockView extends View {
    public BlockView(Context context, AttributeSet attrs, int defStyle) { super(context, attrs, defStyle); }
    public BlockView(Context context, AttributeSet attrs) { super(context, attrs); }
    public BlockView(Context context) { super(context); }
    private int cy, cx; // current drawing position
    private int by, bx; // unit block size
    private int blkWidth; // width of the largest block
    private Matrix block = null;
    private Paint paint = new Paint();

    public void init(int w) { blkWidth = w; }
    public void accept(Matrix m) { block = m; }
    public void onDraw(Canvas canvas) {
        if (block == null) return;
        int[][] array = block.get_array();
        super.onDraw(canvas);
        by = (getHeight() - 20 - 5*(blkWidth-1)) / blkWidth;   // 20: Margin(up-10, down-10), block gap(5) * (blkWidth - 1)
        bx = (getWidth() - 20 - 5*(blkWidth-1)) / blkWidth;    // 20: Margin(up-10, down-10), block gap(5) * (blkWidth - 1)
        paint.setStyle(Paint.Style.FILL);

        cy = 10 + (blkWidth - array.length)*(by+5)/2;
        cx = 10 + (blkWidth - array.length)*(bx+5)/2;
        Bitmap picture;
        for (int y = 0; y < array.length; y++) {
            for (int x = 0; x < array[0].length; x++) {
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
            cx = 10 + (blkWidth - array.length)*(bx+5)/2;
            cy += (by + 5);
        }
    }
}
