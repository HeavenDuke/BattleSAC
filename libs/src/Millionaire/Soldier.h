#pragma once
#include "rsa.h"
#include "osrng.h"
#include "nbtheory.h"

#include <iostream>
#include <sstream>

using namespace CryptoPP;

using namespace std;

inline CryptoPP::Integer Str2BigInt(std::string str) {
	return CryptoPP::Integer(str.c_str());
}
inline std::string BigInt2Str(CryptoPP::Integer bigint) {
	std::stringstream ss;
	std::string str;

	ss << bigint;
	ss >> str;
	return str;
}

class PublicKeyString {
public:
	std::string Modulus_n;
	std::string PublicExponent_e;
public:
	PublicKeyString() {}
	PublicKeyString(RSA::PublicKey key) {
		Modulus_n = BigInt2Str(key.GetModulus());
		PublicExponent_e = BigInt2Str(key.GetPublicExponent());
	}
	RSA::PublicKey toRSA_PublicKey() {
		RSA::PublicKey key;
		key.Initialize(
			Str2BigInt(Modulus_n),
			Str2BigInt(PublicExponent_e)
		);
		return key;
	}
	inline friend std::ostream& operator<<(std::ostream& out, PublicKeyString&	_this) {
		out << "PublicKeyString: " << std::endl;
		out << "\tn:" << _this.Modulus_n << std::endl;
		out << "\te:" << _this.PublicExponent_e << std::endl;
		return out;
	}
};

class PrivateKeyString {
public:
	std::string Modulus_n;
	std::string PublicExponent_e;
	std::string PrivateExponent_d;
public:
	PrivateKeyString() {}
	PrivateKeyString(RSA::PrivateKey key) {
		Modulus_n = BigInt2Str(key.GetModulus());
		PublicExponent_e = BigInt2Str(key.GetPublicExponent());
		PrivateExponent_d = BigInt2Str(key.GetPrivateExponent());
	}
	RSA::PrivateKey toRSA_PrivateKey() {
		RSA::PrivateKey key;
		key.Initialize(
			Str2BigInt(Modulus_n),
			Str2BigInt(PublicExponent_e),
			Str2BigInt(PrivateExponent_d)
		);
		return key;
	}
	inline friend std::ostream& operator<<(std::ostream& out, PrivateKeyString&	_this) {
		out << "PrivateKeyString: " << std::endl;
		out << "\tn:" << _this.Modulus_n << std::endl;
		out << "\te:" << _this.PublicExponent_e << std::endl;
		out << "\td:" << _this.PrivateExponent_d << std::endl;
		return out;
	}
};

class Soldier 
{
public:
	Soldier(int _title);
	Soldier(int _title, PublicKeyString publicKeyString, PrivateKeyString privateKeyString);
	RSA::PublicKey GetPublicKey();
	Integer EncryptTitle(RSA::PublicKey _publicKey);
	Integer* CalcArray(Integer c,Integer& p);
	int Decide(Integer p, Integer* cipher);
private:
	int title;
	int nbits;
	Integer random;
	RSA::PrivateKey privateKey;
	RSA::PublicKey publicKey;
	Integer PowBinMod(Integer x, Integer e, Integer mod) const;
};
