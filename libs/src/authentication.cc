#include <string>
#include <iostream>
#include <sstream>
#include <nan.h>

#include "rsa.h"
using CryptoPP::RSA;
using CryptoPP::RSASS;
using CryptoPP::InvertibleRSAFunction;

#include "pssr.h"
using CryptoPP::PSS;

#include "sha.h"
using CryptoPP::SHA1;

#include "files.h"
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include "filters.h"
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;
using CryptoPP::StringSink;
using CryptoPP::StringSource;

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "secblock.h"
using CryptoPP::SecByteBlock;

using namespace Nan;

namespace JsonAPI{

	inline CryptoPP::Integer Str2BigInt(std::string str){
		return CryptoPP::Integer(str.c_str());
	}
	inline std::string BigInt2Str(CryptoPP::Integer bigint){
		std::stringstream ss;
		std::string str;

		ss << bigint;
		ss >> str;
		return str;
	}

	class PublicKeyString{
	private:
		std::string Modulus_n;
		std::string PublicExponent_e;
	public:
		PublicKeyString(RSA::PublicKey key){
			Modulus_n = BigInt2Str(key.GetModulus());
			PublicExponent_e = BigInt2Str(key.GetPublicExponent());
		}
		RSA::PublicKey toRSA_PublicKey(){
			RSA::PublicKey key;
			key.Initialize(
				Str2BigInt(Modulus_n),
				Str2BigInt(PublicExponent_e)
				);
			return key;
		}
		std::string getModulus() {
		    return Modulus_n;
		}
		std::string getPublicExponent() {
            return  PublicExponent_e;
        }
		inline friend std::ostream& operator<<(std::ostream& out, PublicKeyString&	_this){
			out << "PublicKeyString: " << std::endl;
			out << "\tn:" << _this.Modulus_n << std::endl;
			out << "\te:" << _this.PublicExponent_e << std::endl;
			return out;
		}
	};

	class PrivateKeyString{
	private:

		std::string Modulus_n;
		std::string PublicExponent_e;
		std::string PrivateExponent_d;
	public:
		PrivateKeyString(RSA::PrivateKey key){
			Modulus_n = BigInt2Str(key.GetModulus());
			PublicExponent_e = BigInt2Str(key.GetPublicExponent());
			PrivateExponent_d = BigInt2Str(key.GetPrivateExponent());
		}
		RSA::PrivateKey toRSA_PrivateKey(){
			RSA::PrivateKey key;
			key.Initialize(
				Str2BigInt(Modulus_n),
				Str2BigInt(PublicExponent_e),
				Str2BigInt(PrivateExponent_d)
				);
			return key;
		}
		std::string getModulus() {
            return Modulus_n;
        }
        std::string getPublicExponent() {
            return  PublicExponent_e;
        }
        std::string getPrivateExponent() {
            return  PrivateExponent_d;
        }
		inline friend std::ostream& operator<<(std::ostream& out, PrivateKeyString&	_this){
			out << "PrivateKeyString: " << std::endl;
			out << "\tn:" << _this.Modulus_n << std::endl;
			out << "\te:" << _this.PublicExponent_e << std::endl;
			out << "\td:" << _this.PrivateExponent_d << std::endl;
			return out;
		}
	};

	typedef std::pair<PublicKeyString, PrivateKeyString> KeyPairString;

	inline KeyPairString RandomlyGenerateKey(){
		AutoSeededRandomPool rng;
		rng.Reseed();
		InvertibleRSAFunction parameters;
		parameters.GenerateRandomWithKeySize(rng, 1024);
		RSA::PrivateKey privateKey(parameters);
		RSA::PublicKey publicKey(parameters);
		return std::make_pair(
			PublicKeyString(publicKey),
			PrivateKeyString(privateKey)
			);
	}

	void Generate(const FunctionCallbackInfo<v8::Value> &args) {
	    KeyPairString pairStr = RandomlyGenerateKey();
	    PublicKeyString pubStr = pairStr.first;
	    PrivateKeyString priStr = pairStr.second;
        v8::Local<v8::Object> obj = Nan::New<v8::Object>();
        v8::Local<v8::Object> pubObj = Nan::New<v8::Object>();
        pubObj->Set(Nan::New("n").ToLocalChecked(), Nan::New(pubStr.getModulus()).ToLocalChecked());
        pubObj->Set(Nan::New("e").ToLocalChecked(), Nan::New(pubStr.getPublicExponent()).ToLocalChecked());
        v8::Local<v8::Object> priObj = Nan::New<v8::Object>();
        priObj->Set(Nan::New("n").ToLocalChecked(), Nan::New(priStr.getModulus()).ToLocalChecked());
        priObj->Set(Nan::New("e").ToLocalChecked(), Nan::New(priStr.getPublicExponent()).ToLocalChecked());
        priObj->Set(Nan::New("d").ToLocalChecked(), Nan::New(priStr.getPrivateExponent()).ToLocalChecked());
        obj->Set(Nan::New("public").ToLocalChecked(), pubObj);
        obj->Set(Nan::New("private").ToLocalChecked(), priObj);
        args.GetReturnValue().Set(obj);
	}
}

// 封装了一些签名验证的底层细节
namespace Signature{

	// 由明文消息，根据私钥生成签名
	inline std::string FromMessage(std::string message, RSA::PrivateKey privateKey){

		RSASS<PSS, SHA1>::Signer signer(privateKey);
		std::string signature;
		AutoSeededRandomPool rng;//????不确定这样是否可以,为什么前面需要随机种子？
		rng.Reseed();
		StringSource(message, true,
			new SignerFilter(rng, signer,
			new StringSink(signature)
			) // SignerFilter
			); // StringSource
		return signature;
	}
	inline std::string FromMessage(std::string message, JsonAPI::PrivateKeyString privateKey){
		return FromMessage(message, privateKey.toRSA_PrivateKey());
	}
	// 由消息、前面，验证是否为该公钥人所持有
	inline bool Verify(std::string message, std::string signature, RSA::PublicKey publicKey){
		try{
			// Verify and Recover
			RSASS<PSS, SHA1>::Verifier verifier(publicKey);

			StringSource(message + signature, true,
				new SignatureVerificationFilter(
				verifier, NULL,
				SignatureVerificationFilter::THROW_EXCEPTION
				) // SignatureVerificationFilter
				); // StringSource

			std::cout << "身份验证通过" << std::endl;

		} // try

		catch (CryptoPP::Exception& e) {
			std::cerr << "身份验证失败" << std::endl;
			return false;
		}
		return true;
	}

	inline bool Verify(std::string message, std::string signature, JsonAPI::PublicKeyString publicKey){
		return Verify(message, signature, publicKey.toRSA_PublicKey());
	}

	void SignMessage(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();

	}

	void VerifyMessage(const FunctionCallbackInfo<v8::Value> &args) {

	}
};

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("generateKey").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(JsonAPI::Generate)->GetFunction());
    exports->Set(Nan::New("signMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Signature::SignMessage)->GetFunction());
    exports->Set(Nan::New("verifyMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Signature::VerifyMessage)->GetFunction());
}

NODE_MODULE(authentication, init)