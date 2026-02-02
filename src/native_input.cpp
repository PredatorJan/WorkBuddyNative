#include "native_input.h"
#include <windows.h>

JavaVM* g_vm = nullptr; // wird einmal bei JNI_OnLoad gesetzt

// --------------------- JVM speichern ---------------------
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    return JNI_VERSION_1_8;
}

// --------------------- Hilfsfunktion ---------------------
void callJavaKeyPressed(int keyCode) {
    if (!g_vm) return;

    JNIEnv* env = nullptr;
    bool attached = false;

    // Thread an JVM anhängen, falls nötig
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) != JNI_OK) {
        g_vm->AttachCurrentThread((void**)&env, nullptr);
        attached = true;
    }

    // Klasse finden
    jclass cls = env->FindClass("de/jan/nativebridge/NativeInputHandler");
    if (!cls) return;

    // Statische Methode finden
    jmethodID mid = env->GetStaticMethodID(cls, "onKeyPressed", "(I)V");
    if (!mid) return;

    // Methode aufrufen
    env->CallStaticVoidMethod(cls, mid, keyCode);

    // Thread ggf. wieder lösen
    if (attached) {
        g_vm->DetachCurrentThread();
    }
}

// --------------------- Windows Hook ---------------------
HHOOK g_keyboardHook = nullptr;
static int lastKey = -1;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        if (kbd->vkCode != lastKey) {
            callJavaKeyPressed((int)kbd->vkCode);
            lastKey = kbd->vkCode;
        }
    } else if(wParam == WM_KEYUP) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        if (kbd->vkCode == lastKey) {
            lastKey = -1;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// --------------------- JNI Start Hook ---------------------
JNIEXPORT void JNICALL
Java_de_jan_nativebridge_NativeInput_startKeyHook(JNIEnv*, jclass) {
    g_keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        KeyboardProc,
        GetModuleHandle(nullptr),
        0
    );
}

// --------------------- Hook stoppen ---------------------
JNIEXPORT void JNICALL
Java_de_jan_nativebridge_NativeInput_stopKeyHook(JNIEnv*, jclass) {
    if (g_keyboardHook) {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = nullptr;
    }
}