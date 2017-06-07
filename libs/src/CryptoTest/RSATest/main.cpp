#include <rsa.h>
#include <osrng.h>
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSA;
#include <iostream>
#include <string>
#include <integer.h>
using CryptoPP::Integer;

#include "Voter.h"

using namespace std;

Integer pow_bin(Integer x, Integer e, Integer mod) {
	Integer res = 1;
	while (e>0) {
		if (e.IsOdd()) res = res * x % mod;
		x = x * x % mod;
		e >>= 1;
	}
	return res % mod;
}

int main() {
	AutoSeededRandomPool rng;
	InvertibleRSAFunction params;
	
	Voter voters[5];
	int vid, vote;
	for (int i = 0; i < 5; i++) {
		params.GenerateRandomWithKeySize(rng, 2048);
		voters[i].Initialize(RSA::PublicKey(params), RSA::PrivateKey(params));
		cout << "Set vid for voter " << i << ": " << flush;
		cin >> vid;
		cout << "Set vote for voter " << i << ": " << flush;
		cin >> vote;
		voters[i].SetVote(vid, vote);
	}

	cout << "Set Vote Done!" << endl;
	for (int i = 0; i < 5; i++) {
		Integer vote = voters[i].SendForSign(voters[0]);
		cout << "Voter " << i << " send vote to voter 0 to sign a vote and gets: " << vote << endl;
		vote = voters[i].RemoveBlind(vote);
		cout << "Voter get vote without blind: " << vote << endl;
		ostringstream os;
		os << vote;
		string s = os.str();
		for (int j = 4; j >= 1; j--) {
			std::string cipher = voters[j].Encrypt(s);
			s = cipher;
		}
		cout << "Voter " << i << " send cipher vote to voter 0 to shuffle: " << s << endl;
		voters[0].AddVote(s);
	}

	for (int i = 0; i < 4; i++) {
		voters[i].Shuffle();
		string voteSending = voters[i+1].Encrypt(voters[i].ParseVotesToString());
		cout << "voter "<< i << " send to voter "<< i + 1 << ": " << voteSending << endl;
		voters[i].SendTo(voters[i+1], voteSending);
		string voteString = voters[i+1].DecryptVote();
		cout << "voter " << i + 1 << " decrypt vote and get: " << voteString << endl;
		voters[i+1].ParseStringToVotes(voteString);
		for (int j = 0; j < voters[i+1].m_votes.size(); j++) {
			voters[i + 1].m_votes[j] = voters[i+1].Decrypt(voters[i+1].m_votes[j]);
		}
	}
	
	

	for (int i = 0; i < voters[4].m_votes.size(); i++) {
		//cout << voters[4].m_votes[i] << endl;
		Integer nVote(voters[4].m_votes[i].c_str());
		Integer nowVote = pow_bin(nVote, voters[0].GetPubKey().GetPublicExponent(), voters[0].GetPubKey().GetModulus());
		unsigned unVote = nowVote.ConvertToLong();
		cout << GET_VID(unVote) << ": " << GET_VOTE(unVote) << endl;
	}

	return 0;
}