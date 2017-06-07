#include "Solider.h"

Solider::Solider()
{
}

Solider::Solider(int _title)
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

RSA::PublicKey Solider::GetPublicKey()
{
	return publicKey;
}

Integer Solider::EncryptTitle(RSA::PublicKey _publicKey)
{
	//pick a random N - bit integer
	AutoSeededRandomPool rng;
	Integer _random(rng, nbits);
	random = _random;
	Integer c = PowBinMod(random, _publicKey.GetPublicExponent(), _publicKey.GetModulus()) - title;
	return c;
}

Integer* Solider::CalcArray(Integer c, Integer& p)
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

int Solider::Decide(Integer p, Integer * cipher)
{
	if (cipher[title - 1] == random % p)
		return 1;//title1>=title2
	else
		return 0;//title1<title2
}

Integer Solider::PowBinMod(Integer x, Integer e, Integer mod) const
{
	Integer res = 1;
	while (e > 0) {
		if (e.IsOdd()) res = res * x % mod;
		x = x * x % mod;
		e >>= 1;
	}
	return res % mod;
}
