#pragma once
#include "Solider.h"
#include <list>

using CryptoPP::Integer;
using namespace std;

class Millionaire {
private:
	int NaiveCmp(Solider solider1, Solider solider2);
	int Cmp(Solider solider1, Solider solider2);
public:
	Millionaire();
	list<Solider> SelectHighestTitle(Solider soliders[5]);
};

