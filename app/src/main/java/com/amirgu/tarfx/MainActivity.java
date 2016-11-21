package com.amirgu.tarfx;

import android.os.Bundle;
import com.facebook.react.ReactActivity;

public class MainActivity extends ReactActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        AudioProcessor.initialize();
    }

    @Override
    protected void onResume() {
        super.onResume();

        AudioProcessor.start();
    }

    @Override
    protected void onPause() {
        super.onPause();

        AudioProcessor.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        AudioProcessor.destroy();
    }


    @Override
    protected String getMainComponentName() {
        return "TarFX";
    }


}
