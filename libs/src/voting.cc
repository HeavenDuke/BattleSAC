#include <nan.h>

#include "Millionaire/Millionaire.h"

#include "Millionaire/Soldier.h"

using namespace Nan;

void CompareTitle(const FunctionCallbackInfo<v8::Value> &args) {

}

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("compareTitle").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(CompareTitle)->GetFunction());
}

NODE_MODULE(supplement, init)