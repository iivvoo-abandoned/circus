#ifndef GUIHANDLER_H
#define GUIHANDLER_H

#include "cmdhandler.h" 

class	guihandler: public cmdhandler
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(guihandler *h, 
	                    int (guihandler:: *f)(int, char **)): KitObject()
		{
			_c = h;		// class
			_m = f;		// method
			_c->_register(this);
		};
		~func_packer()
		{};
		int	execute( int argc, char **argv)
		{
			return (_c->*_m)(argc, argv);
		};
	private:
		guihandler	*_c;
		int		(guihandler::* _m)(int, char **);
	};
public:
	guihandler();
	virtual ~guihandler();

	virtual int operator()();
private:
	int	handleinput(int, char **);
	int	nickchange(int, char **);
	int	targetchange(int, char **);
	int	targetlistchange(int, char **);
	int	open_target_menu(int, char **);
	int	wmodechange(int, char **);
	int	imodechange(int, char **);
	int	rmodechange(int, char **);

	int 	handlecommand(cstring);
	int	handle_modech(int, char **, int);
	int	handle_menu_gen_op(int, char **);
	int	handle_menu_ch_op(int, char **);
	int	handle_menu_ctcp_op(int, char **);
	
	void	handle_invite(const cstring &channel);
	map<cstring, KitObject *, mapcmp>	dispatch_table;
};

#endif // GUIHANDLER_H
