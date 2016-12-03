package com.amirgu.tarfx;

import android.os.Handler;
import android.util.Log;
import android.util.Pair;
import android.view.View;
import android.content.Context;
import android.util.AttributeSet;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.MotionEvent;

import java.util.ArrayList;
import java.util.Arrays;

public class CanvasView extends View {

    //drawing path
    private Path drawPath, signalPath;
    //drawing and canvas paint
    private Paint drawPaint, canvasPaint, signalPaint;
    //initial color
    private int paintColor = 0xFF000000;
    //canvas
    private Canvas drawCanvas;
    //canvas bitmap
    private Bitmap canvasBitmap;

    private Handler mHandler;

    float width;
    float height;

    boolean stopHandler;


    private ArrayList<Pair<Float, Float>> points;
    int curPointIdx;

    public CanvasView(Context context, AttributeSet attrs){
        super(context, attrs);

        drawPath = new Path();
        drawPaint = new Paint();

        signalPath = new Path();
        signalPaint = new Paint();

        drawPaint.setColor(paintColor);
        drawPaint.setAntiAlias(true);
        drawPaint.setStrokeWidth(10);
        drawPaint.setStyle(Paint.Style.STROKE);
        drawPaint.setStrokeJoin(Paint.Join.ROUND);
        drawPaint.setStrokeCap(Paint.Cap.ROUND);

        canvasPaint = new Paint(Paint.DITHER_FLAG);

        signalPaint.setColor(0xFF00ddff);
        signalPaint.setAntiAlias(true);
        signalPaint.setStrokeWidth(10);
        signalPaint.setStyle(Paint.Style.STROKE);
        signalPaint.setStrokeJoin(Paint.Join.ROUND);
        signalPaint.setStrokeCap(Paint.Cap.ROUND);

        points = new ArrayList<Pair<Float, Float>>();

        stopHandler = true;
        mHandler = new Handler();
    }


    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        canvasBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        drawCanvas = new Canvas(canvasBitmap);

        width = w;
        height = h;

        points.add(new Pair<Float, Float>(new Float(0), new Float(h/2)));
        points.add(new Pair<Float, Float>(new Float(w), new Float(h/2)));

        drawPoints();
        start();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.drawPath(drawPath, drawPaint);
        canvas.drawPath(signalPath, signalPaint);
    }

    private void drawPoints() {
        drawPath.reset();
        drawPath.moveTo(points.get(0).first, points.get(0).second);
        for (int index = 1; index < points.size(); index++) {
            Pair<Float, Float> point = points.get(index);
            drawPath.lineTo(point.first, point.second);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float touchX = event.getX();
        float touchY = event.getY();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                for (int index = 0; index < points.size(); index++) {
                    Pair<Float, Float> point = points.get(index);
                    if (touchX - 100 < point.first && point.first < touchX + 100) {
                        curPointIdx = index;
                        break;
                    }
                    else if (point.first > touchX) {
                        curPointIdx = index;
                        points.add(index, new Pair<Float, Float>(touchX, touchY));
                        break;
                    }
                }
                break;
            case MotionEvent.ACTION_MOVE:
                Pair<Float, Float> curPoint = points.get(curPointIdx);
                points.set(curPointIdx, new Pair<Float, Float>(curPoint.first, touchY));
                drawPoints();
                break;
//            case MotionEvent.ACTION_UP:
//                drawCanvas.drawPath(drawPath, drawPaint);
//                drawPath.reset();
//                break;
            default:
                return false;
        }

        invalidate();
        return true;
    }

    void start() {
        stopHandler = false;
        mHandler.post(mUpdate);
    }

    void stop() {
        stopHandler = true;
    }


    private Runnable mUpdate = new Runnable() {
        public void run() {
            if (true) return;

            short[] test = AudioProcessor.readBuffer();
            int size = test.length/2 + 2;
            int i = 0;

            signalPath.reset();

            for (short val : test) {
                float value = Math.min((float)val, 100000);
                value = value/100000 * height;
                if (i == 0) {
                    signalPath.moveTo(0, height - value);
                } else {
                    signalPath.lineTo(width/size * i, height - value);
                }

                if (i == size) break;
                i += 1;
            }
            invalidate();

            mHandler.postDelayed(this, 20);
        }
    };


}
