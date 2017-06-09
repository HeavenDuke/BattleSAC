#include "Lagrange.hpp"
#include "Signature.hpp"

using namespace std;


void test_lagrange(){

	//int nAll = 5;	// ������
	//int nEnough = 3; // �����������С����
	int nAll = 10;	// ������
	int nEnough = nAll/2; // �����������С����

	/*��ע�⡿����������Ҫ�ԡ����.��Ϊ��β
		��ΪCrypto�Ĵ�����ת�ַ���ʱ��ӵ�
		������ӵ㣬�ع�������ܿ��ܺ�ԭ�Ĳ�һ��
		����β��һ����ţ�*/

	std::string secret = "31241932784987402136721342314.";

	std::vector<LagrangeJsonStrAPI::SecretPartString> keyArray =
		LagrangeJsonStrAPI::SecretDivide(secret, nAll, nEnough);

	std::cout << "���ֽܷ�Ľ��Ϊ��" << std::endl;
	for (int i = 0; i < nAll; ++i){
		std::cout << keyArray[i] << std::endl;
	}
	std::cout << std::endl;


	std::string reCons = LagrangeJsonStrAPI::SecretReconstruct(keyArray, nEnough);
	std::cout << "�ֽ�ǰ��" << secret << std::endl;
	std::cout << "�ع���" << reCons << std::endl;
	bool equal = (secret == reCons);
	std::cout << "�Ƿ���ͬ��" << (equal?"true":"false") << std::endl;
}

void test_signature(){

		SignatureJsonStrAPI::KeyPairString key_tupe = SignatureJsonStrAPI::RandomlyGenerateKey();
		SignatureJsonStrAPI::PublicKeyString publicKey1 = key_tupe.first;
		SignatureJsonStrAPI::PrivateKeyString privateKey1 = key_tupe.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;

		SignatureJsonStrAPI::KeyPairString key_tupe2 = SignatureJsonStrAPI::RandomlyGenerateKey();
		SignatureJsonStrAPI::PublicKeyString publicKey2 = key_tupe2.first;
		SignatureJsonStrAPI::PrivateKeyString privateKey2 = key_tupe2.second;
		//std::cout << publicKey << std::endl;
		//std::cout << privateKey << std::endl;
	
		std::string message_user1 = "xufeifan test message for user1";
		std::string signature = SignatureJsonStrAPI::SignMessage(message_user1, privateKey1);

		bool is_user1 = SignatureJsonStrAPI::Verify(message_user1, signature, publicKey1);
		std::cout << is_user1 << std::endl;// true
		is_user1 = SignatureJsonStrAPI::Verify(message_user1, signature, publicKey2);
		std::cout << is_user1 << std::endl;// false
}

int main(int argc, char* argv[]){

	test_lagrange();
	test_signature();

	return 0;
}