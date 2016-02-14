/*
 * main.c - main circus routine
 * Copyright (C) 1998 Ivo van der Wijk (ivo@cs.vu.nl) and others
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <plush.h>
#include <plush/session.h>

#include <iostream.h>
#include <stdlib.h>		

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>

#include <sys/stat.h>

#include "ircclient.h"

#include "images.h"
#include "cstring.h"
#include "const.h"

images	*i = new images();			// global var
images	&img = *i;

int	dbg = 0;				// global var

class   application: public session
{
public:
        application(int argc, char* argv[]): session() // argc, argv, ".cIRCus")
        {
	    int	i = 1;
	    
	    circus_root = NULL;

	    while(i < argc)
	    {
		if(!strcasecmp(argv[i], "-debug"))
		{
		    argv[i] = "-x";
		    if(i < argc - 1)
		    {
		        dbg = atoi(argv[++i]);
		        argv[i] = "-x";
		        cout << "Debuglevel set to " << dbg << endl;
		    }
		    else
		    {
		         cout << "Argument expected after -debug" << endl;
		         giveup();
		    }
		} 
		else if(!strcasecmp(argv[i], "-root"))
		{
		    argv[i] = "-x";
		    if(i < argc - 1)
		    {
		    	// check if it is a directory, etc!
		        circus_root = argv[++i];
		        argv[i] = "-x";
		        cout << "Using cIRCus resource directory: " 
			     << circus_root << endl;
		    }
		    else
		    {
		        cout << "Directory expected after -root" << endl;
		        giveup();
		    }
		}
	    	i++;
	    }
	    pw = getpwuid(getuid());
            tk->trace(KIT_TRACE);
        }
	~application()
	{}
	void	prelude()
	{
	}
        int     main()
        {
		cout << CIRCUS_VERSION 
		     << " - (c) 1996-97 Ivo van der Wijk and Mark de Boer" 
		     << endl;
		cout << "For more information, check "
		     << "http://www.nijenrode.nl/~ivo/circus/"
		     << endl;

		resource();

		root->title(CIRCUS_VERSION);
		intro = new label(root, ".l", 
//		       "-font -itc-zapfchancery-*-*-normal-*-30-*-*-*-*-*-*-* "
		       "-font -*-courier-*-*-*-*-30-*-*-*-*-*-*-* "
		       "-bg blue -fg yellow");
		intro2 = new label(root, ".l2");
		intro->text("Loading...");
		intro2->image(img["cIRCus2x.gif"]);
		intro2->pack("-side top -expand true -fill both");
		intro->pack("-side top -expand true -fill both");
		tk->update();

		cout << "VAR: " << tk->getvar("servers") << endl;

		char	**argv;
		int	argc;

		tk->splitlist(tk->getvar("servers"), &argc, &argv);
		for(int j = 0; j < argc; j++)
			cout << "arg[" << j << "] = " << argv[j] << endl;
		tk->free((char *)argv);
		irc = new ircclient(root, ".cIRCus-main"); 
		irc->pack("-fill both -expand true");

		intro->destroy();
		intro2->destroy();
		tk->update();

		irc->start();
                return 0;
        }
	int	load(char *file)
	{
		struct	stat	f;

		if(stat(file, &f) == -1)
			return false;
		if(!S_ISREG(f.st_mode))
			return false;
		tk->source(file);
		return true;
	}
	void	resource(void)
	// load a resourcefile. First find a global one, then load the
	// one in the user directory
	{
		struct	stat	f;
		int	system_loaded =0;
		int	i;
		char	*rootdir = NULL;
		
		char	*dirs[] = {
			"/usr/local/circus/", 
			"/usr/local/lib/circus/", 
			"/usr/lib/circus/", 
			"/usr/circus/", 
			"./",
			NULL
			};

		if(circus_root)
		{
			if((stat(circus_root, &f) != -1) && S_ISDIR(f.st_mode))
			{
				cout << "Using " << circus_root 
				     << " as cIRCus resource directory" << endl;
				cstring resourcefile;

				resourcefile << circus_root << "/system.circusrc";

				rootdir = circus_root;
				if(load(resourcefile))
				{
					cout << "Loading " << resourcefile
					     << endl;
					system_loaded = true;
				}
				else
					cout << "Could not load "
					     << resourcefile << "!" << endl;
			}
		}
		else for(i = 0; dirs[i] && !rootdir; i++)
			if((stat(dirs[i], &f) != -1) && S_ISDIR(f.st_mode))
			{
				cout << "Using " << dirs[i] 
				     << " as cIRCus resource directory" << endl;
				cstring resourcefile;

				resourcefile << dirs[i] << "system.circusrc";

				rootdir = dirs[i];
				if(load(resourcefile))
					system_loaded = true;
				else
					cout << "Could not load "
					     << resourcefile << "!" << endl;
			}

//		if(!system_loaded && circus_root)
//		{
//			cout << "Loading " << file << endl;
//			tk->source(file);
//			system_loaded = true;
//		}


		// 'user' resourcefile
		// be more verbose
		cstring	p = pw->pw_dir;
		if((stat(p + "/.circusrc", &f) != -1) && S_ISREG(f.st_mode))
		{
			cout << "Loading " << p << "/.circusrc" << endl;
			tk->source(p + "/.circusrc");
			system_loaded = true;
		}

		if(!system_loaded)
		{
			cout << "No cIRCus resource file found!" << endl;
			cout << "Not in " << rootdir << "/system.circusrc" 
			     << endl;
//			cout << "Not in: " << endl;
//			for(i = 0; dirs[i]; i++)
//				cout << "        " << dirs[i] << "," << endl; 
			cout << "And not in " << p << "/.circusrc" << endl;
			cout << "cIRCus might not work correctly!" << endl;
		}
					
		if(rootdir)
		{
			cstring picdir;
			picdir << rootdir << "/pics";
			img.dir(picdir);
		}
		else
		{
			cout << "cIRCus could not locate it's resource "
			     << "directory. Please specify one using the " 
			     << endl;
			cout << "-root option, else cIRCus will not work!"
			     << endl;
			giveup();
		}
		
	}
	void	giveup(void)
	{
		exit(-1);
	}
private:
	ircclient	*irc;
	label		*intro, *intro2;
	struct	passwd 	*pw;
	char		*circus_root;
};

int	main(int argc, char **argv)
{
        session *client = new application(argc, argv);
        client->run();
	delete client;
	delete i;
	session::statistics();
	cstring::stats();
	cstring_iter::stats();
        return 0;
}
