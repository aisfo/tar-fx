package com.amirgu.tarfx;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;


public class ReactModule extends ReactContextBaseJavaModule {

    @Override
    public String getName() {
        return "ReactModule";
    }

    public ReactModule(ReactApplicationContext reactContext) {
        super(reactContext);
    }

    @ReactMethod
    public void test(Promise promise) {
        promise.resolve("test");
    }

}

