#pragma once
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_de_jan_nativebridge_NativeInput_startKeyHook(JNIEnv*, jclass);

JNIEXPORT void JNICALL
Java_de_jan_nativebridge_NativeInput_stopKeyHook(JNIEnv*, jclass);

#ifdef __cplusplus
}
#endif