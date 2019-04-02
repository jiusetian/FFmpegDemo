package com.ffmpegdemo;

/**
 * Author：Alex
 * Date：2019/4/2
 * Note：
 */
public class AvUtil {

    static {
        System.loadLibrary("av-lib");
    }

    public static native String avcodeInfo();
}
