/******************************************************************************
 SVNMDIServer.h

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#ifndef _H_SVNMDIServer
#define _H_SVNMDIServer

#include <jx-af/jx/JXMDIServer.h>

class SVNMDIServer : public JXMDIServer
{
public:

	SVNMDIServer();

	virtual	~SVNMDIServer();

	static void		PrintCommandLineHelp();

protected:

	virtual void	HandleMDIRequest(const JString& dir,
									 const JPtrArray<JString>& argList) override;
};

#endif
