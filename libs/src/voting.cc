#include <nan.h>

using namespace Nan;

void init(v8::Local<v8::Object> exports) {
//    exports->Set(Nan::New("generateCase").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(LagrangeJsonStrAPI::GenerateCase)->GetFunction());
//    exports->Set(Nan::New("openCase").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(LagrangeJsonStrAPI::OpenCase)->GetFunction());
}

NODE_MODULE(supplement, init)