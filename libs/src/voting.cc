#include <iostream>
#include <sstream>
#include <nan.h>

#pragma once
#include "rsa.h"
#include "osrng.h"
#include "nbtheory.h"

#include "encryption.h"

#include "Voter.h"
#include "Voter.cpp"

using namespace CryptoPP;

using namespace Nan;

using namespace std;

class Soldier
{
    public:
        Soldier(int _title);
        Soldier(int _title, PublicKeyString publicKeyString, PrivateKeyString privateKeyString);
        RSA::PublicKey GetPublicKey();
        Integer EncryptTitle(RSA::PublicKey _publicKey);
        Integer* CalcArray(Integer c,Integer& p);
        int Decide(Integer p, Integer* cipher);
        int getTitle() { return title; }
    private:
        int title;
        int nbits;
        Integer random;
        RSA::PrivateKey privateKey;
        RSA::PublicKey publicKey;
        Integer PowBinMod(Integer x, Integer e, Integer mod) const;
};

Soldier::Soldier(int _title)
{
	title = _title;

	nbits = 512;
	//generate key
	AutoSeededRandomPool rng;
	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 2 * nbits);

	RSA::PublicKey _publicKey(params);
	RSA::PrivateKey _privateKey(params);

	publicKey = _publicKey;
	privateKey = _privateKey;

	random = 0;
}

Soldier::Soldier(int _title, PublicKeyString publicKeyString, PrivateKeyString privateKeyString)
{
	title = _title;
	nbits = 512;
	publicKey = publicKeyString.toRSA_PublicKey();
	privateKey = privateKeyString.toRSA_PrivateKey();
}

RSA::PublicKey Soldier::GetPublicKey()
{
	return publicKey;
}

Integer Soldier::EncryptTitle(RSA::PublicKey _publicKey)
{
	//pick a random N - bit integer
	AutoSeededRandomPool rng;
	Integer _random(rng, nbits);
	random = _random;
	Integer c = PowBinMod(random, _publicKey.GetPublicExponent(), _publicKey.GetModulus()) - title;
	return c;
}

Integer* Soldier::CalcArray(Integer c, Integer& p)
{
	//generates a random prime p of N/2 bits
	AutoSeededRandomPool rng;
	PrimeAndGenerator pag(1, rng, nbits / 2);
	p = pag.Prime();
	Integer *cipher = new Integer[10];

	for (int i = 0; i < 10; i++) {
		cipher[i] = PowBinMod(c + i + 1, privateKey.GetPrivateExponent(), privateKey.GetModulus()) % p;
	}

	for (int i = title; i < 10; i++) {
		cipher[i] ++;
	}

	return cipher;
}

int Soldier::Decide(Integer p, Integer * cipher)
{
	if (cipher[title - 1] == random % p)
		return 1;//title1>=title2
	else
		return 0;//title1<title2
}

Integer Soldier::PowBinMod(Integer x, Integer e, Integer mod) const
{
	Integer res = 1;
	while (e > 0) {
		if (e.IsOdd()) res = res * x % mod;
		x = x * x % mod;
		e >>= 1;
	}
	return res % mod;
}

class Millionaire {
    private:
        int NaiveCmp(Soldier soldier1, Soldier soldier2);
    public:
        Millionaire();
        int Cmp(Soldier soldier1, Soldier soldier2);
};

Millionaire::Millionaire()
{
}

int Millionaire::NaiveCmp(Soldier soldier1, Soldier soldier2)
{
	Integer c = soldier2.EncryptTitle(soldier1.GetPublicKey());
	Integer* cipher;
	Integer p;
	cipher = soldier1.CalcArray(c, p);

	return soldier2.Decide(p, cipher);
}

int Millionaire::Cmp(Soldier soldier1, Soldier soldier2)
{
	int flag1 = NaiveCmp(soldier1, soldier2);

	if (!flag1)
		return -1;//title1<title2
	else
	{
		int flag2 = NaiveCmp(soldier2, soldier1);
		if (flag2)
			return 0;//title1=title2
		else
			return 1;//title1>title2
	}
}

Soldier constructSoldier(v8::Local<v8::Object> soldierObj) {
    PublicKeyString pubStr = PublicKeyString();
    PrivateKeyString priStr = PrivateKeyString();

    v8::Local<v8::Object> pubObj = soldierObj->Get(Nan::New("public").ToLocalChecked())->ToObject();
    v8::Local<v8::String> text = pubObj->Get(Nan::New("n").ToLocalChecked())->ToString();
    std::string t = std::string(*(v8::String::Utf8Value(text)));
    pubStr.Modulus_n = t;
    text = pubObj->Get(Nan::New("e").ToLocalChecked())->ToString();
    t = std::string(*(v8::String::Utf8Value(text)));
    pubStr.PublicExponent_e = t;

    v8::Local<v8::Object> priObj = soldierObj->Get(Nan::New("private").ToLocalChecked())->ToObject();
    text = priObj->Get(Nan::New("n").ToLocalChecked())->ToString();
    t = std::string(*(v8::String::Utf8Value(text)));
    priStr.Modulus_n = t;
    text = priObj->Get(Nan::New("e").ToLocalChecked())->ToString();
    t = std::string(*(v8::String::Utf8Value(text)));
    priStr.PublicExponent_e = t;
    text = priObj->Get(Nan::New("d").ToLocalChecked())->ToString();
    t = std::string(*(v8::String::Utf8Value(text)));
    priStr.PrivateExponent_d = t;

    int _title = soldierObj->Get(Nan::New("title").ToLocalChecked())->NumberValue();

    return Soldier(_title, pubStr, priStr);
}

void CompareTitle(const FunctionCallbackInfo<v8::Value> &args) {
    v8::Local<v8::Object> params = args[0]->ToObject();
    Soldier launcher = constructSoldier(params->Get(Nan::New("launcher").ToLocalChecked())->ToObject());
    Soldier receiver = constructSoldier(params->Get(Nan::New("receiver").ToLocalChecked())->ToObject());
    Millionaire compareOperator;
    v8::Local<v8::Int32> valid = Nan::New(compareOperator.Cmp(launcher, receiver));
    args.GetReturnValue().Set(valid);
}

void AnonymousVoting(const FunctionCallbackInfo<v8::Value> &args) {
    std::map<int, std::pair<int, int>> votes;
    std::map<int, std::pair<PublicKeyString, PrivateKeyString>> keyPairs;
    v8::Local<v8::Array> election = v8::Local<v8::Array>::Cast(args[0]);

    for(int i = 0; i < election->Length(); i++) {
        v8::Local<v8::Object> vote = election->Get(i)->ToObject();
        int voterId = vote->Get(Nan::New("voterId").ToLocalChecked())->NumberValue();
        int code = vote->Get(Nan::New("code").ToLocalChecked())->NumberValue();
        int votedId = vote->Get(Nan::New("votedId").ToLocalChecked())->NumberValue();
        votes[voterId] = std::pair<int, int>(code, votedId);

        PublicKeyString pubStr = PublicKeyString();
        PrivateKeyString priStr = PrivateKeyString();

        v8::Local<v8::Object> pubObj = vote->Get(Nan::New("public").ToLocalChecked())->ToObject();
        v8::Local<v8::String> text = pubObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        std::string t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.Modulus_n = t;
        text = pubObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.PublicExponent_e = t;

        v8::Local<v8::Object> priObj = vote->Get(Nan::New("private").ToLocalChecked())->ToObject();
        text = priObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.Modulus_n = t;
        text = priObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PublicExponent_e = t;
        text = priObj->Get(Nan::New("d").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PrivateExponent_d = t;

        keyPairs[voterId] = std::pair<PublicKeyString, PrivateKeyString>(pubStr, priStr);
    }

    std::map<int, std::vector<std::pair<int,int>> > _notification = GetVote(votes, keyPairs);
    v8::Local<v8::Object> notification = Nan::New<v8::Object>();
    std::map<int, std::vector<std::pair<int,int>> >::iterator it;

    it = _notification.begin();

    while(it != _notification.end()) {
        v8::Local<v8::Array> result = Nan::New<v8::Array>();
        std::vector<std::pair<int,int>> _result = it->second;
        for(int i = 0; i < _result.size(); i++) {
            v8::Local<v8::Object> _vote = Nan::New<v8::Object>();
            _vote->Set(Nan::New("code").ToLocalChecked(), Nan::New(_result[i].first));
            _vote->Set(Nan::New("votedId").ToLocalChecked(), Nan::New(_result[i].second));
            result->Set(i, _vote);
        }
        notification->Set(Nan::New(it->first)->ToString(), result);
        it++;
    }

    args.GetReturnValue().Set(notification);
}

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("compareTitle").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(CompareTitle)->GetFunction());
    exports->Set(Nan::New("anonymousVoting").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(AnonymousVoting)->GetFunction());
}

NODE_MODULE(supplement, init)