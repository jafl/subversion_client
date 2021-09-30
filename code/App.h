/******************************************************************************
 App.h

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#ifndef _H_App
#define _H_App

#include <jx-af/jx/JXApplication.h>

class App : public JXApplication
{
public:

	App(int* argc, char* argv[], bool* displayAbout, JString* prevVersStr);

	virtual	~App();

	void	DisplayAbout(const JString& prevVersStr = JString::empty);

	void	ReloadOpenFilesInIDE();

	static const JUtf8Byte*	GetAppSignature();
	static void				InitStrings();

protected:

	virtual bool	Close();
	virtual void		CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason);
};

#endif
