#ifndef SERVERLIST
#define SERVERLIST

#include <vector.h>
#include <plush/KitObject.h>

#include "const.h"			// DEFAULT_PORT defined there

#define	SERVERVARNAME	"servers"

class	serverlist: public KitObject
{
	class	serverentry
	{
	public:
		serverentry()
		{
			host = "";
			port = DEFAULT_PORT;
			passwd = "";
			autoconnect = false;
		}

		cstring		host;
		int		port;
		cstring		passwd;
		bool		autoconnect;	// perhaps use 'flags'
	};

	typedef	vector<serverentry *>	sList;

public:
	serverlist()
	{
	}

	int	initialize(void)
	{
		char	*serverConfig = tk->getvar(SERVERVARNAME);

		int	code, argc;
		char	**argv;
		
		if(serverConfig == NULL)
			return 0;

		// do errorreporting
		
		code = tk->splitlist(serverConfig, &argc, &argv);

		for(int i = 0; i < argc; i++)
		{
			cout << "[" << i << " ] = " << argv[i] << endl;

			int	code, subArgc;
			char	**subArgv;

			code = tk->splitlist(argv[i], &subArgc, &subArgv);
	
			for(int j = 0; j < subArgc; j++)
				cout << "[" << i <<"][" << j << "] = " 
				     << subArgv[j] << ", ";

			cout << endl;
			// this is not entirely foolproof

			serverentry *e = new serverentry();

			if(subArgc > 0)
				e->host = subArgv[0];
			if(subArgc > 1)
				e->port = atoi(subArgv[1]);
			if(subArgc > 2)
				if(strcmp(subArgv[2], "startup") == 0)
					e->autoconnect = true;
				else
					e->passwd = subArgv[2];
			if(subArgc > 3)
				e->passwd = subArgv[3];
				
			servers.insert(servers.end(), e);
			
			tk->free((char *)subArgv);
		}
		tk->free((char *)argv);
		return argc;
	}
	

	int	size()
	{
		return servers.size();
	}

	serverentry 	*serverAt(int i)
	{
		return servers[i];
	}
private:
	sList	servers;
};

#endif // SERVERLIST
