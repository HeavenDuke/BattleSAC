#include "Millionaire.h"

Millionaire::Millionaire()
{
}

int Millionaire::NaiveCmp(Soldier soldier1, Soldier soldier2)
{
	Integer c = soldier2.EncryptTitle(soldier1.GetPublicKey());
	Integer* cipher;
	Integer p;
	cipher = soldier1.CalcArray(c, p);

	return soldier2.Decide(p, cipher);
}

int Millionaire::Cmp(Soldier soldier1, Soldier soldier2)
{
	int flag1 = NaiveCmp(soldier1, soldier2);
	if (!flag1)
		return -1;//title1<title2
	else
	{
		int flag2 = NaiveCmp(soldier2, soldier1);
		if (flag2)
			return 0;//title1=title2
		else
			return 1;//title1>title2
	}
}