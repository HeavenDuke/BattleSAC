#include "Millionaire.h"

Millionaire::Millionaire()
{
}

int Millionaire::NaiveCmp(Solider solider1, Solider solider2)
{
	Integer c = solider2.EncryptTitle(solider1.GetPublicKey());
	Integer* cipher;
	Integer p;
	cipher = solider1.CalcArray(c, p);

	return solider2.Decide(p, cipher);
}

int Millionaire::Cmp(Solider solider1, Solider solider2)
{
	int flag1 = NaiveCmp(solider1, solider2);
	if (!flag1)
		return -1;//title1<title2
	else
	{
		int flag2 = NaiveCmp(solider2, solider1);
		if (flag2)
			return 0;//title1=title2
		else
			return 1;//title1>title2
	}
}

list<Solider> Millionaire::SelectHighestTitle(Solider soliders[5])
{
	list<Solider> son[5];
	for (int i = 0; i < 5; i++)
		son[i].push_back(soliders[i]);

	list< list<Solider> > father;
	for (int i = 0; i < 5; i++)
		father.push_back(son[i]);

	while (father.size()>1)
	{
		
		int result = Cmp(father.front().front(), father.back().front());
		if (result == -1)
			father.pop_front();
		else if(result == 1)
		{
			father.pop_back();
		}
		else
		{
			father.front().push_back(father.back().front());
			father.pop_back();
		}

	}
	
	return father.front();
}