#include <iostream>
#include <sstream>

#pragma once
#include "rsa.h"
#include "osrng.h"
#include "nbtheory.h"

using namespace CryptoPP;

CryptoPP::Integer pow_bin(CryptoPP::Integer x, CryptoPP::Integer e, CryptoPP::Integer mod) {
	Integer res = 1;
	while (e>0) {
		if (e.IsOdd()) res = res * x % mod;
		x = x * x % mod;
		e >>= 1;
	}
	return res % mod;
}

CryptoPP::Integer Str2BigInt(std::string str) {
	return CryptoPP::Integer(str.c_str());
}

std::string BigInt2Str(CryptoPP::Integer bigint) {
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
        RSA::PublicKey toRSA_PublicKey() const {
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
        RSA::PrivateKey toRSA_PrivateKey() const {
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