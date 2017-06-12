#pragma once
#include "rsa.h"
#include "osrng.h"
#include "nbtheory.h"
using namespace CryptoPP;

class Soldier 
{
public:
	Soldier();
	Soldier(int _title);
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
