#ifndef GARBAGE_H
#define GARBAGE_H

#include <map.h>	// use STL map to store objects

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

		map<T, T, less<T> >::iterator i;

		for(i = bin.begin(); i != bin.end(); i++) 
			delete (*i).second;
	}

	void	_register(T rubbish) 
	{
		bin[rubbish] = rubbish;
	}

	void	unregister(T rubbish) 
	{
	// This is not very efficient. We might consider a 
	// different container class

		if(allow_unreg)
		{
			map<T, T, less<T> >::iterator i;

			i = bin.find(rubbish);

			if(i != bin.end())
				bin.erase(i);
		}	
	}
private:
	int		allow_unreg;
	map<T, T, less<T> >	bin;
};

#endif // GARBAGE_H
