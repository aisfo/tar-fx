package com.amirgu.tarfx;


public class AudioProcessor {

    private AudioProcessor() {}

    static {
        System.loadLibrary("AudioProcessor");
    }

    public static native void initialize();
    public static native void start();
    public static native void stop();
    public static native void destroy();

}
