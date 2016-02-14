class signal
{

}

#define	SIGNAL_DISPATCHER

	template< >
	static	void	signal_dispatcher(A obj, A::(*foo)(PARAMS))
	{
		obj->foo(


class Iets {

SIGNAL	ValueChanged(int)			
**	-> signal<int>	ValueChanged;

	int	doit(int x)
	{
		if(x > 10)
			EMIT ValueChanged(x);	
**	-> ValueChanged(x);
	}
}

class Fred {

SLOT	SetValue(int a)
-> slot<int>	Fred::Fred_SetValue;

	{
		...
	}
}

En dan:

	Iets a, Fred b;
	
	CONNECT(a, ValueChanged, b, SetValue);
	
**	-> a->ValueChanged.connect(b, SetValue); - gaat niet werken

	
class	signallist
{
	Iets	
