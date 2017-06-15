#include "Voter.h"

Voter::Voter(RSA::PublicKey pubKey, RSA::PrivateKey privKey):
	m_pubKey(pubKey), m_privKey(privKey), m_nBlinder(0L)
{
}

Voter::Voter(){}

void Voter::Initialize(RSA::PublicKey pubKey, RSA::PrivateKey privKey) {
	m_pubKey = pubKey;
	m_privKey = privKey;
	m_nBlinder = 0;
}

void Voter::SetVote(int vid, int vote) {
	if (vid >= (1 << 24) || vid < 0) {
		throw std::runtime_error("Vid out of bound!");
	}
	this->m_unVote = MAKE_VOTE(vid, vote);
}

Integer Voter::SendForSign(const Voter& signer) {
	AutoSeededRandomPool rng;
	Integer s = rng.GenerateWord32();
	m_nSignerModulos = signer.GetPubKey().GetModulus();
	m_nSignerExponent = signer.GetPubKey().GetPublicExponent();
	while (Integer::Gcd(m_nSignerModulos,s) != 1) {
		s++;
	}
	m_nBlinder = s;
	Integer blindVote = PowBinMod(m_nBlinder, m_nSignerExponent,m_nSignerModulos);
	Integer signedVote = signer.SignVote(blindVote*m_unVote);
	return signedVote;
}

Integer Voter::SignVote(const Integer& unVote) const{
	Integer p = m_privKey.GetPrivateExponent();
	Integer n = m_privKey.GetModulus();
	Integer vote(unVote);
	Integer signedVote = PowBinMod(vote, p, n);
	return signedVote;
}

Integer Voter::RemoveBlind(const Integer& blindVote) {
	Integer kprime = m_nBlinder.InverseMod(m_nSignerModulos);
	//std::cout << "****" << kprime*m_nBlinder%m_nSignerModulos << std::endl;
	Integer res = blindVote * kprime % m_nSignerModulos;
	return res;
}

Integer Voter::PowBinMod(Integer x, Integer e, Integer mod) const{
	Integer res = 1;
	while (e > 0) {
		if (e.IsOdd()) res = res * x % mod;
		x = x * x % mod;
		e >>= 1;
	}
	return res % mod;
}

RSA::PublicKey Voter::GetPubKey() const {
	return m_pubKey;
}

unsigned Voter::GetVote() const {
	return m_unVote;
}

void Voter::AddVote(const std::string& vote) {
	m_votes.push_back(vote);
}

void Voter::Shuffle() {
	int n = m_votes.size();
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		int r = rand() % n;
		std::string t = m_votes[i];
		m_votes[i] = m_votes[r];
		m_votes[r] = t;
	}
}

void Voter::SendTo(Voter& voter, std::string s) {
	voter.m_sMsg = s;
}

std::string Voter::Decrypt(const std::string& s) {
	std::string recovered = "";
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(m_privKey);

	int nCiphertextLength = d.FixedCiphertextLength() * 2;

	for (int i = s.size(), j = 0; i > 0; i -= nCiphertextLength, j += nCiphertextLength) {
		std::string partCipher = s.substr(j, nCiphertextLength);
		std::string partPlain;
		CryptoPP::StringSource(partCipher, true, new CryptoPP::HexDecoder(
			new CryptoPP::PK_DecryptorFilter(m_rng, d, new CryptoPP::StringSink(partPlain))));
		recovered += partPlain;
	}

	//CryptoPP::StringSource ss2(s, true,
	//	new CryptoPP::PK_DecryptorFilter(m_rng, d,
	//		new CryptoPP::StringSink(recovered)
	//		) // PK_DecryptorFilter
	//	); // StringSource
	return recovered;
}

std::string Voter::Encrypt(const std::string& s) {
	std::string cipher = "";

	CryptoPP::RSAES_OAEP_SHA_Encryptor e(m_pubKey);

	int nMaxMsgLength = e.FixedMaxPlaintextLength();

	for (int i = s.size(), j = 0; i > 0; i -= nMaxMsgLength, j += nMaxMsgLength) {
		std::string partPlain = s.substr(j, nMaxMsgLength);
		std::string partCipher;
		CryptoPP::StringSource(partPlain, true,
			new CryptoPP::PK_EncryptorFilter(m_rng, e,
				new CryptoPP::HexEncoder(new CryptoPP::StringSink(partCipher))));
		cipher += partCipher;
	}

	//CryptoPP::StringSource ss1(s, true,
	//	new CryptoPP::PK_EncryptorFilter(m_rng, e,
	//		new CryptoPP::StringSink(cipher)
	//		) // PK_EncryptorFilter
	//	); // StringSource
	return cipher;
}

std::string Voter::ParseVotesToString() {
	std::ostringstream os;
	for (int i = 0; i < m_votes.size(); i++) {
		os << m_votes[i];
		if (i != m_votes.size() - 1) os << "|";
	}
	return os.str();
}

void split(const std::string& s, const std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}

std::string Voter::DecryptVote() {
	m_sMsg = this->Decrypt(m_sMsg);
	return m_sMsg;
}

void Voter::ParseStringToVotes(const std::string& sVotes) {
	m_votes.clear();
	split(sVotes, "|", &m_votes);
}

std::map<int, std::vector<std::pair<int,int>> > GetVote(const std::map< int, std::pair<int,int> > &votes, 
						const std::map<int, std::pair<PublicKeyString, PrivateKeyString> > &voterskey) {
	std::map<int, std::vector<std::pair<int,int>> > notification;
	std::vector<std::pair<int,int>> result;
	std::map< int, std::pair<int,int> >::const_iterator it;

    /*
    it = votes.begin();

    while(it != votes.end())
    {
        result.push_back(it->second);
        it++;
    }
    */
    int nVoter = votes.size();

    it = votes.begin();
    std::map< int, std::pair<PublicKeyString, PrivateKeyString> >::const_iterator it_key = voterskey.begin();
    Voter *voters = new Voter[nVoter];
    for(int i = 0 ; i < nVoter ; i++,it++,it_key++ ) {
    	voters[i].Initialize(it_key->second.first.toRSA_PublicKey(),it_key->second.second.toRSA_PrivateKey());
    	voters[i].SetVote(it->second.first, it->second.second);
    }
    
    for(int i = 0 ; i < nVoter ; i++ ) {
    	Integer vote = voters[i].SendForSign(voters[0]);
    	vote = voters[i].RemoveBlind(vote);
    	ostringstream os;
		os << vote;
		string s = os.str();
		for (int j = nVoter-1; j >= 1; j--) {
			std::string cipher = voters[j].Encrypt(s);
			s = cipher;
		}
		voters[0].AddVote(s);
    }

    for (int i = 0; i < nVoter-1; i++) {
		voters[i].Shuffle();
		string voteSending = voters[i+1].Encrypt(voters[i].ParseVotesToString());
		//cout << "voter "<< i << " send to voter "<< i + 1 << ": " << voteSending << endl;
		voters[i].SendTo(voters[i+1], voteSending);
		string voteString = voters[i+1].DecryptVote();
		//cout << "voter " << i + 1 << " decrypt vote and get: " << voteString << endl;
		voters[i+1].ParseStringToVotes(voteString);
		for (int j = 0; j < voters[i+1].m_votes.size(); j++) {
			voters[i + 1].m_votes[j] = voters[i+1].Decrypt(voters[i+1].m_votes[j]);
		}
	}

	it = votes.begin();
	for (int i = 0; i < voters[nVoter-1].m_votes.size(); i++) {
		Integer nVote(voters[nVoter-1].m_votes[i].c_str());
		Integer nowVote = pow_bin(nVote, voters[0].GetPubKey().GetPublicExponent(), voters[0].GetPubKey().GetModulus());
		unsigned unVote = nowVote.ConvertToLong();
		result.push_back(std::make_pair(GET_VID(unVote),GET_VOTE(unVote)));
	}

    it = votes.begin();

    while(it != votes.end())
    {
        notification[it->first] = result;
        it++;
    }

    delete[] voters;
	return notification;
}