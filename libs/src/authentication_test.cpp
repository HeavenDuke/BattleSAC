#include "authentication.hpp"

using namespace std;


void test_lagrange(){

	//int nAll = 5;	// 总人数
	//int nEnough = 3; // 解密所需的最小人数
	int nAll = 10;	// 总人数
	int nEnough = nAll/2; // 解密所需的最小人数
	const char* secret = "31241932784987402136721342314";

	std::vector<Lagrange::SecretPart> keyArray = 
		Lagrange::SecretDivide(Lagrange::Math::BigInt(secret), nAll, nEnough);

	std::cout << "秘密分解的结果为：" << std::endl;
	for (int i = 0; i < nAll; ++i){
		std::cout << keyArray[i] << std::endl;
	}
	std::cout << std::endl;


	Lagrange::Math::BigInt reCons = Lagrange::SecretReconstruct(keyArray, nEnough);
	std::cout << "分解前：" << secret << std::endl;
	std::cout << "重构后：" << reCons << std::endl;
	bool equal = (Lagrange::Math::BigInt(secret) == reCons);
	std::cout << "是否相同：" << (equal?"true":"false") << std::endl;
}

void test_signature(){

		JsonAPI::KeyPairString key_tupe = JsonAPI::RandomlyGenerateKey();
		JsonAPI::PublicKeyString publicKey1 = key_tupe.first;
		JsonAPI::PrivateKeyString privateKey1 = key_tupe.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;

		JsonAPI::KeyPairString key_tupe2 = JsonAPI::RandomlyGenerateKey();
		JsonAPI::PublicKeyString publicKey2 = key_tupe2.first;
		JsonAPI::PrivateKeyString privateKey2 = key_tupe2.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;
	
		std::string message_user1 = "xufeifan test message for user1";
		std::string signature = Signature::FromMessage(message_user1, privateKey1);

		bool is_user1 = Signature::Verify(message_user1, signature, publicKey1);
		std::cout << is_user1 << std::endl;// true
		is_user1 = Signature::Verify(message_user1, signature, publicKey2);
		std::cout << is_user1 << std::endl;// false
}

int main(int argc, char* argv[]){

	test_lagrange();
	test_signature();

	return 0;
}