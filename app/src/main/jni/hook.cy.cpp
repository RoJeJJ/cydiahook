//
// Created by pc on 2018/4/4.
//
#include <jni.h>
#include "substrate.h"
#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>

#define TAG "HOOKTEST"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)


MSConfig(MSFilterLibrary, "libdvm.so");

bool (*_dvmLoadNativeCode)(char* pathName, void* classLoader, char** detail);

bool fake_dvmLoadNativeCode(char* soPath, void* classLoader, char** detail)
{
    LOGD("fake_dvmLoadNativeCode soPath:%s", soPath);
    return _dvmLoadNativeCode(soPath,classLoader,detail);
}
void (*_dvmCallJNIMethod)(void* args,void* pResult,Method* method,void* self);
void fake_dvmCallJNIMethod(void* args,void* pResult,Method* method,void* self)
{
    //LOGD("my_dvmUseJNIBridge method.name: %s\tfunc:%.8X",*(char**)(method+16), func);
    LOGD("fake_dvmCallJNIMethod method %s, addr:%p", method->name, method->insns);
    return _dvmCallJNIMethod(args,pResult,method,self);
}

//Substrate entry point
MSInitialize{
    //LOGD("Substrate initialized.");
    MSImageRef image;
    image = MSGetImageByName("/system/lib/libdvm.so"); // 绝对路径
    if (image != NULL)
    {
        //LOGD("dvm image: 0x%08X", (void*)image);

        void * dvmload = MSFindSymbol(image, "_Z17dvmLoadNativeCodePKcP6ObjectPPc");
        if(dvmload == NULL)
        {
            //LOGD("error find dvmLoadNativeCode.");
        }
        else
        {
            MSHookFunction(dvmload,(void*)&fake_dvmLoadNativeCode,(void **)&_dvmLoadNativeCode);
            //LOGD("hook dvmLoadNativeCode sucess.");
        }
       void * dvmCallJni = MSFindSymbol(image, "_Z16dvmCallJNIMethodPKjP6JValuePK6MethodP6Thread");
       if(dvmCallJni == NULL)
       {
            //LOGD("error find dvmCallJNIMethod.");
        }
        else
       {
            MSHookFunction(dvmCallJni,(void*)&fake_dvmCallJNIMethod,(void **)&_dvmCallJNIMethod);
            //LOGD("hook dvmCallJNIMethod sucess.");
        }
    }
    else{
       // LOGD("can not find libdvm.");
    }
}

