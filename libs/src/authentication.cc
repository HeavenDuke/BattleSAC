#include <string>
#include <iostream>
#include <vector>
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

#include "hex.h"

using namespace Nan;

#pragma comment(lib,"cryptlib.lib")

// 签名验证的底层细节
namespace Signature{

	// 由明文消息，根据私钥生成签名
	inline std::string SignMessage(std::string message, RSA::PrivateKey privateKey){

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

//			std::cout << "身份验证通过" << std::endl;

		} // try

		catch (CryptoPP::Exception& e) {
//			std::cerr << "身份验证失败" << std::endl;
			return false;
		}
		return true;
	}
};


// 对Signature的进一步封装
// 一切交互均已字符串为接口
namespace SignatureJsonStrAPI{

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
    public:
        std::string Modulus_n;
        std::string PublicExponent_e;
    public:
        PublicKeyString() {}
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
        inline friend std::ostream& operator<<(std::ostream& out, PublicKeyString&	_this){
            out << "PublicKeyString: " << std::endl;
            out << "\tn:" << _this.Modulus_n << std::endl;
            out << "\te:" << _this.PublicExponent_e << std::endl;
            return out;
        }
    };

    class PrivateKeyString{
    public:
        std::string Modulus_n;
        std::string PublicExponent_e;
        std::string PrivateExponent_d;
    public:
        PrivateKeyString() {}
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

    inline std::vector<byte> SignMessage(std::string message, PrivateKeyString privateKey){
        std::string s = Signature::SignMessage(message, privateKey.toRSA_PrivateKey());

        std::vector<byte> _signed_message;
        for (int i = 0; i < s.length(); ++i)
            _signed_message.push_back((byte)(s[i]));
        return _signed_message;
    }
    inline bool Verify(std::string message, std::vector<byte> signature, PublicKeyString publicKey){
        std::string _signed_message;
        //_signed_message.append()
        for (int i = 0; i < signature.size(); ++i)
            _signed_message.push_back((char)(signature[i]));
            // 不太确定 string::push_back 如何处理结尾的\0 但是测试没问题

        return Signature::Verify(message, _signed_message, publicKey.toRSA_PublicKey());
    }

    std::string Decrypt(PrivateKeyString privKey,const std::vector<byte>& ss) {
        AutoSeededRandomPool m_rng;
        m_rng.Reseed();
        std::string recovered = "";
        CryptoPP::RSAES_OAEP_SHA_Decryptor d(privKey.toRSA_PrivateKey());
        std::string s = "";
        for(int i = 0 ; i < ss.size() ; i++ ) {
            s = s+char(ss[i]);
        }

        int nCiphertextLength = d.FixedCiphertextLength() * 2;
        for (int i = s.size(), j = 0; i > 0; i -= nCiphertextLength, j += nCiphertextLength) {
            std::string partCipher = s.substr(j, nCiphertextLength);
            std::string partPlain;
            CryptoPP::StringSource(partCipher, true, new CryptoPP::HexDecoder(
                new CryptoPP::PK_DecryptorFilter(m_rng, d, new CryptoPP::StringSink(partPlain))));
            recovered += partPlain;
        }
        return recovered;
    }

    std::vector<byte> Encrypt(PublicKeyString pubKey ,const std::string& s) {
        AutoSeededRandomPool m_rng;
        m_rng.Reseed();
        std::string cipher = "";
        CryptoPP::RSAES_OAEP_SHA_Encryptor e(pubKey.toRSA_PublicKey());

        int nMaxMsgLength = e.FixedMaxPlaintextLength();

        for (int i = s.size(), j = 0; i > 0; i -= nMaxMsgLength, j += nMaxMsgLength) {
            std::string partPlain = s.substr(j, nMaxMsgLength);
            std::string partCipher;
            CryptoPP::StringSource(partPlain, true, new CryptoPP::PK_EncryptorFilter(m_rng, e,new CryptoPP::HexEncoder(new CryptoPP::StringSink(partCipher))));
            cipher += partCipher;
        }

        std::vector<byte> res;
        for(int i = 0 ; i < cipher.size() ; i++ ) {
            res.push_back(byte(cipher[i]));
        }

        return res;
    }


	void Generate(const FunctionCallbackInfo<v8::Value> &args) {
        KeyPairString pairStr = RandomlyGenerateKey();
        PublicKeyString pubStr = pairStr.first;
        PrivateKeyString priStr = pairStr.second;
        v8::Local<v8::Object> obj = Nan::New<v8::Object>();
        v8::Local<v8::Object> pubObj = Nan::New<v8::Object>();
        pubObj->Set(Nan::New("n").ToLocalChecked(), Nan::New(pubStr.Modulus_n).ToLocalChecked());
        pubObj->Set(Nan::New("e").ToLocalChecked(), Nan::New(pubStr.PublicExponent_e).ToLocalChecked());
        v8::Local<v8::Object> priObj = Nan::New<v8::Object>();
        priObj->Set(Nan::New("n").ToLocalChecked(), Nan::New(priStr.Modulus_n).ToLocalChecked());
        priObj->Set(Nan::New("e").ToLocalChecked(), Nan::New(priStr.PublicExponent_e).ToLocalChecked());
        priObj->Set(Nan::New("d").ToLocalChecked(), Nan::New(priStr.PrivateExponent_d).ToLocalChecked());
        obj->Set(Nan::New("public").ToLocalChecked(), pubObj);
        obj->Set(Nan::New("private").ToLocalChecked(), priObj);
        args.GetReturnValue().Set(obj);
    }

	void _SignMessage(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();
        v8::Local<v8::Object> priObj = params->Get(Nan::New("key").ToLocalChecked())->ToObject();
        PrivateKeyString priStr = PrivateKeyString();
        v8::Local<v8::String> text = priObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        std::string t = std::string(*(v8::String::Utf8Value(text)));
        priStr.Modulus_n = t;
        text = priObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PublicExponent_e = t;
        text = priObj->Get(Nan::New("d").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PrivateExponent_d = t;
        text = params->Get(Nan::New("message").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));

        std::vector<byte> secret = SignMessage(t, priStr);
        v8::Local<v8::Array> _secret = Nan::New<v8::Array>();
        for(int i = 0; i < secret.size(); i++) {
            _secret->Set(i, Nan::New(secret[i]));
        }
        args.GetReturnValue().Set(_secret);
    }

    void VerifyMessage(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();
        v8::Local<v8::Object> pubObj = params->Get(Nan::New("key").ToLocalChecked())->ToObject();
        PublicKeyString pubStr = PublicKeyString();
        v8::Local<v8::String> text = pubObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        std::string t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.Modulus_n = t;
        text = pubObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.PublicExponent_e = t;
        text = params->Get(Nan::New("message").ToLocalChecked())->ToString();
        std::string message = std::string(*(v8::String::Utf8Value(text)));
        v8::Local<v8::Array> signature = v8::Local<v8::Array>::Cast(params->Get(Nan::New("signature").ToLocalChecked()));
        std::vector<byte> _signature;
        for(int i = 0; i < signature->Length(); i++) {
            _signature.push_back(signature->Get(i)->NumberValue());
        }
        v8::Local<v8::Boolean> valid = Nan::New(Verify(message, _signature, pubStr));
        args.GetReturnValue().Set(valid);
    }

    void DecryptMessage(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();
        v8::Local<v8::Object> priObj = params->Get(Nan::New("key").ToLocalChecked())->ToObject();
        PrivateKeyString priStr = PrivateKeyString();
        v8::Local<v8::String> text = priObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        std::string t = std::string(*(v8::String::Utf8Value(text)));
        priStr.Modulus_n = t;
        text = priObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PublicExponent_e = t;
        text = priObj->Get(Nan::New("d").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        priStr.PrivateExponent_d = t;
        text = params->Get(Nan::New("message").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));

        v8::Local<v8::Array> message = v8::Local<v8::Array>::Cast(params->Get(Nan::New("message").ToLocalChecked()));
        std::vector<byte> _message;
        for(int i = 0; i < message->Length(); i++) {
            _message.push_back(message->Get(i)->NumberValue());
        }
        v8::Local<v8::String> result = Nan::New(Decrypt(priStr, _message)).ToLocalChecked();
        args.GetReturnValue().Set(result);
    }

    void EncryptMessage(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();
        v8::Local<v8::Object> pubObj = params->Get(Nan::New("key").ToLocalChecked())->ToObject();
        PublicKeyString pubStr = PublicKeyString();
        v8::Local<v8::String> text = pubObj->Get(Nan::New("n").ToLocalChecked())->ToString();
        std::string t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.Modulus_n = t;
        text = pubObj->Get(Nan::New("e").ToLocalChecked())->ToString();
        t = std::string(*(v8::String::Utf8Value(text)));
        pubStr.PublicExponent_e = t;
        text = params->Get(Nan::New("message").ToLocalChecked())->ToString();
        std::string message = std::string(*(v8::String::Utf8Value(text)));

        std::vector<byte> secret = Encrypt(pubStr, message);
        v8::Local<v8::Array> _secret = Nan::New<v8::Array>();
        for(int i = 0; i < secret.size(); i++) {
            _secret->Set(i, Nan::New(secret[i]));
        }
        args.GetReturnValue().Set(_secret);
    }
}

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("generateKey").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(SignatureJsonStrAPI::Generate)->GetFunction());
    exports->Set(Nan::New("signMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(SignatureJsonStrAPI::_SignMessage)->GetFunction());
    exports->Set(Nan::New("verifyMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(SignatureJsonStrAPI::VerifyMessage)->GetFunction());
    exports->Set(Nan::New("encryptMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(SignatureJsonStrAPI::EncryptMessage)->GetFunction());
    exports->Set(Nan::New("decryptMessage").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(SignatureJsonStrAPI::DecryptMessage)->GetFunction());
}

NODE_MODULE(authentication, init)