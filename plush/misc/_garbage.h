#ifndef GARBAGE_H
#define GARBAGE_H

#include <vector.h>	// use STL vector to store objects

/*
 * Damn I hate to use templates...
 */

template<class T>
class	garbage
{
public:
	garbage()
	{
		allow_unreg = true;
	}

	~garbage() 
	{ 
	// this is to stop the destructors from calling unregister
		allow_unreg = false;

		for(unsigned int i = 0; i < bin.size(); i++) 
			delete bin[i];
	}

	void	_register(T rubbish) 
	{
		bin[rubbish] = rubbish;
//		bin.push_back(rubbish);
	}

	void	unregister(T rubbish) 
	{
	// This is not very efficient. We might consider a 
	// different container class

		if(allow_unreg)
		{
			vector<T>::iterator i;

			i = bin.find(rubbish);

			if(i != bin.end())
				bin.erase(i);
		}	
	}
private:
	int		allow_unreg;
	map<T, T, less<T>>	bin;
};

#endif // GARBAGE_H
