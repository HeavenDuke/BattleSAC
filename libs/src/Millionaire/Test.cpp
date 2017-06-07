#include "Millionaire.h"
#include "Solider.h"
#pragma comment(lib, "cryptopp\\lib\\cryptlib.lib")
using namespace std;

void main() {

	Solider soliders[5];
	soliders[0] = Solider(8);
	soliders[1] = Solider(9);
	soliders[2] = Solider(7);
	soliders[3] = Solider(5);
	soliders[4] = Solider(9);

	Millionaire m;
	cout << m.SelectHighestTitle(soliders).size() << endl;
}