#include "authentication.hpp"

using namespace std;


void test_lagrange(){

	//int nAll = 5;	// ������
	//int nEnough = 3; // �����������С����
	int nAll = 10;	// ������
	int nEnough = nAll/2; // �����������С����
	const char* secret = "31241932784987402136721342314";

	auto keyArray = Lagrange::SecretDivide(Lagrange::Math::BigInt(secret), nAll, nEnough);

	std::cout << "���ֽܷ�Ľ��Ϊ��" << std::endl;
	for (int i = 0; i < nAll; ++i){
		std::cout << keyArray[i] << std::endl;
	}
	std::cout << std::endl;


	Lagrange::Math::BigInt reCons = Lagrange::SecretReconstruct(keyArray, nEnough);
	std::cout << "�ֽ�ǰ��" << secret << std::endl;
	std::cout << "�ع���" << reCons << std::endl;
	bool equal = (Lagrange::Math::BigInt(secret) == reCons);
	std::cout << "�Ƿ���ͬ��" << (equal?"true":"false") << std::endl;
}

void test_signature(){

		auto key_tupe = JsonAPI::RandomlyGenerateKey();
		auto publicKey1 = key_tupe.first;
		auto privateKey1 = key_tupe.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;

		auto key_tupe2 = JsonAPI::RandomlyGenerateKey();
		auto publicKey2 = key_tupe2.first;
		auto privateKey2 = key_tupe2.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;
	
		// Message
		std::string message_user1 = "xufeifan test";
		std::string signature = Signature::FromMessage(message_user1, privateKey1);

		bool is_user1 = Signature::Verify(message_user1, signature, publicKey1);
		std::cout << is_user1 << std::endl;// true
		is_user1 = Signature::Verify(message_user1, signature, publicKey2);
		std::cout << is_user1 << std::endl;// false
}

int main(int argc, char* argv[]){

	test_signature();

	return 0;
}