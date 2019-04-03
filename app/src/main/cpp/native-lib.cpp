#include <string>
#include <jni.h>
#include <android/log.h>



extern "C" JNIEXPORT jstring JNICALL
Java_com_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
