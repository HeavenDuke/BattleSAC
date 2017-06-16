#ifndef RSA_VOTER_H_
#define RSA_VOTER_H_

#include <string>
#include <rsa.h>
#include <integer.h>
#include <osrng.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <hex.h>
#include <vector>
#include <map>
#include "encryption.h"

using CryptoPP::AutoSeededRandomPool;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSA;
using CryptoPP::Integer;

#define MAKE_VOTE(x,y) ((x<<8)|y)
#define GET_VID(x) ((x>>8)&((1<<16)-1))
#define GET_VOTE(x) (x&((1<<8)-1))

class Voter {
public:
	Voter(RSA::PublicKey pubKey, RSA::PrivateKey privKey);
	Voter();
	Voter(const Voter& voter);
	Voter& operator=(const Voter &);
	void Initialize(RSA::PublicKey pubKey, RSA::PrivateKey privKey);
	void SetVote(int vid, int vote);
	Integer SendForSign(const Voter& signer,Integer nVote);
	Integer SignVote(const Integer& unVote) const;
	RSA::PublicKey GetPubKey() const;
	unsigned GetVote() const;
	Integer RemoveBlind(const Integer& blindVote);
	void AddVote(const std::string& vote);
	void Shuffle();
	std::string Decrypt(const std::string& s);
	std::string Encrypt(const std::string& s);
	void SendTo(Voter& voter,std::string s);
	std::string ParseVotesToString();
	void ParseStringToVotes(const std::string& sVotes);
	std::string DecryptVote();
	bool operator<(const Voter& b) const;
private:
	Integer PowBinMod(Integer x, Integer e, Integer mod) const;
private:
	RSA::PublicKey m_pubKey;
	RSA::PrivateKey m_privKey;
	Integer m_nBlinder;
	unsigned m_unVote;
	Integer m_nSignerModulos;
	Integer m_nSignerExponent;
	std::string m_sMsg;
	AutoSeededRandomPool m_rng;
	Integer m_nModulos;
public:
	std::vector<std::string> m_votes;
};

#endif