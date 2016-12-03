package com.amirgu.tarfx;

import android.app.Activity;
import android.graphics.Canvas;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

public class MainActivity extends Activity
{

    CanvasView canvas;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        canvas = (CanvasView)findViewById(R.id.drawing);
        AudioProcessor.initialize();
    }

    @Override
    protected void onResume() {
        super.onResume();

        AudioProcessor.start();
        canvas.start();
    }

    @Override
    protected void onPause() {
        super.onPause();

        canvas.stop();
        AudioProcessor.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        AudioProcessor.destroy();
    }



}
