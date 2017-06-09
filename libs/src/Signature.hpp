#include <string>
#include <iostream>
#include <sstream>

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

#include "SecBlock.h"
using CryptoPP::SecByteBlock;


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

			std::cout << "身份验证通过" << std::endl;

		} // try

		catch (CryptoPP::Exception& e) {
			std::cerr << "身份验证失败" << std::endl;
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
}