#pragma once
#include "Soldier.h"
#include <list>

using CryptoPP::Integer;
using namespace std;

class Millionaire {
private:
	int NaiveCmp(Soldier soldier1, Soldier soldier2);
public:
	Millionaire();
	int Cmp(Soldier soldier1, Soldier soldier2);
};

