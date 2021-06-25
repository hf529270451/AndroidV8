//
// Created by hfyd on 6/24/21.
//

#include <jni.h>
#include <android/log.h>
#include "v8.h"
#include "libplatform/libplatform.h"

class V8Context {
public:
    v8::Isolate *isolate_;

    explicit V8Context(v8::Isolate *isolate) {
        isolate_ = isolate;
    }
};

static const std::string className = "com/example/myapplication/MainActivity";


jstring char2Jstring(JNIEnv *env, const char *pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取java String类方法String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray((jsize) strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, (jsize) strlen(pat), (jbyte *) pat);
    //设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);

}

v8::Global<v8::Context> context_;
JNIEnv *env_;
jobject *instance_;

std::string ObjectToString(v8::Isolate *isolate, v8::Local<v8::Value> value) {
    v8::String::Utf8Value utf8_value(isolate, value);
    return std::string(*utf8_value);
}

static void callTestFunction(
        const v8::FunctionCallbackInfo<v8::Value> &args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Value> arg = args[0];
    std::string params = ObjectToString(isolate, arg);

    jclass jclz = env_->FindClass("com/example/myapplication/MainActivity");
    jmethodID mid = env_->GetMethodID(jclz, "drawNode", "(Ljava/lang/String;)V");

    __android_log_print(ANDROID_LOG_ERROR, "V8Core", "callTestFunction:%s", params.data());
    env_->CallVoidMethod(*instance_, mid, char2Jstring(env_, params.data()));
}

void evaluateJavaScript2(v8::Isolate *isolate, v8::Local<v8::String> string) {
    v8::HandleScope scopedIsolate(isolate);
    v8::TryCatch tryCatch(isolate);

    v8::Local<v8::Script> compiledScript;
    v8::Local<v8::Context> context(isolate->GetCurrentContext());

    if (!v8::Script::Compile(context, string).ToLocal(&compiledScript)) {
        __android_log_print(ANDROID_LOG_ERROR, "V8Core", "CompileError");
        return;
    }

    v8::Local<v8::Value> result;
    bool a = compiledScript->Run(context).ToLocal(&result);
    __android_log_print(ANDROID_LOG_ERROR, "V8Core", "result:%d", a);
}

/*void evaluateJavaScript(v8::Isolate *isolate, v8::Local<v8::String> string) {
    v8::HandleScope scopedIsolate(isolate);
    v8::TryCatch tryCatch(isolate);

    v8::Local<v8::Script> compiledScript;
    v8::Local<v8::Context> context(isolate->GetCurrentContext());

    if (!v8::Script::Compile(context, string).ToLocal(&compiledScript)) {
        __android_log_print(ANDROID_LOG_ERROR, "V8Core", "CompileError");
        return;
    }

    v8::Local<v8::Value> result;
    bool a = compiledScript->Run(context).ToLocal(&result);
    __android_log_print(ANDROID_LOG_ERROR, "V8Core", "result:%d", a);
}*/

/*
v8::MaybeLocal<v8::String> toV8String(std::string script, v8::Isolate *isolate) {
    v8::EscapableHandleScope scopedIsolate(isolate);
    v8::MaybeLocal<v8::String> ret = v8::String::NewFromUtf8(
            isolate,
            script.data(),
            v8::NewStringType::kNormal,
            script.length());
    return scopedIsolate.EscapeMaybe(ret);
}
*/

std::string jstring2str(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    std::string stemp(rtn);
    free(rtn);
    return stemp;
}

v8::Local<v8::Context> CreateGlobalContext(v8::Isolate *isolate) {
    v8::HandleScope scopedIsolate(isolate);
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    global->Set(
            v8::String::NewFromUtf8(isolate, "callTestFunction", v8::NewStringType::kNormal)
                    .ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, callTestFunction));
    return v8::Context::New(isolate, nullptr, global);
}


struct fields_t {
    jfieldID v8Context;
};
static struct fields_t gFields;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_evaluateJavaScript(JNIEnv *env, jobject instance,
                                                               jstring jscript) {
    env_ = env;
    instance_ = &instance;

    std::string script = jstring2str(env, jscript);

    std::unique_ptr<v8::Platform> s_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializeICU();
    v8::V8::InitializePlatform(s_platform.get());
    v8::V8::Initialize();

    std::unique_ptr<v8::ArrayBuffer::Allocator> arrayBufferAllocator_;
    arrayBufferAllocator_.reset(
            v8::ArrayBuffer::Allocator::NewDefaultAllocator());

    v8::Isolate::CreateParams createParams;
    // array_buffer_allocator 该ArrayBuffer
    // ::分配器用于分配和释放ArrayBuffers的后备存储。
    createParams.array_buffer_allocator = arrayBufferAllocator_.get();
    // Isolate 提供了一个v8虚拟机的实例
    v8::Isolate *isolate = v8::Isolate::New(createParams);

    isolate->Enter();
    v8::HandleScope scopedIsolate(isolate);
    context_.Reset(isolate, CreateGlobalContext(isolate));
    context_.Get(isolate)->Enter();

    /*auto *v8Context = new V8Context(isolate);

    jclass clazz = env->FindClass(className.c_str());
    gFields.v8Context = env->GetFieldID(clazz, "mV8ContextPtr", "J");
    env->SetLongField(instance, gFields.v8Context, (jlong) v8Context);
*/
    evaluateJavaScript2(isolate, v8::String::NewFromUtf8(isolate, script.data(),
                                                         v8::NewStringType::kNormal).ToLocalChecked());
}



/*extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_evaluateJavaScript(JNIEnv *env, jobject instance,
                                                               jstring jscript) {
    auto *v8Context = (V8Context *) env->GetLongField(instance, gFields.v8Context);
    std::string script = jstring2str(env, jscript);
    v8::HandleScope scopedIsolate(v8Context->isolate_);
    v8::Local<v8::String> string;
    if (toV8String(script, v8Context->isolate_).ToLocal(&string)) {
        evaluateJavaScript(v8Context->isolate_, string);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "V8Core", "NewFromUtf8 error");
    }
}*/
