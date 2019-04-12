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

    /**
     * 解码
     * @param inStr 输入视频文件的位置
     * @param outStr 输出视频文件的位置
     */
    public static native int decoder(String inStr,String outStr);
}
