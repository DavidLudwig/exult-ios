

#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "Configuration.h"

#include <cstdio>



Configuration::Configuration() :filename("")
{}

Configuration::Configuration(const char *s) : filename("")
{
	filename=s;
	read_config_file(s);
}

Configuration::~Configuration()
{}


string	&Configuration::value(const char *key,bool &exists)
{
	string	s=key;
	exists=false;
	return xmltree.reference(s,exists);
}

void	Configuration::value(const char *key,string &s,const char *defaultvalue)
{
	bool	exists;
	s=value(key,exists);
	if(!exists)
		s=defaultvalue;
}

void	Configuration::value(const char *key,int &n,int defaultvalue)
{
	n=0;
	string	s;
	bool	exists;

	s=value(key,exists);
	if(!exists)
		n=defaultvalue;
	else
		n=atoi(s.c_str());
}

void	Configuration::set(string &key,string &value)
{
	string	k=key;
	XMLnode *walk;

	walk=&xmltree;

	// Incomplete

	// Break k up into '/' separated elements.
	// start advancing walk, one element at a time, creating nodes
	// as needed.
	// At the end of that, walk is the target node, and we
	// can set the value.

	// We must also properly encode the value before writing it out.
	// Must remember that.

}


extern	void    xmlparse(string &s,size_t &pos,XMLnode *x);

bool	Configuration::read_config_file(const char *n)
{
        char    buf[4096];
        string  sbuf;

	FILE	*fp=fopen(n,"r");
	if(!fp)
		return false;

        while(fgets(buf,sizeof(buf),fp))
                {
                sbuf+=buf;
                }

        size_t  nn=1;
        xmlparse(sbuf,nn,&xmltree);

	fclose(fp);
	return true;
}


