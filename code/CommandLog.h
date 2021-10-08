/******************************************************************************
 CommandLog.h

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_CommandLog
#define _H_CommandLog

#include "TextBase.h"

class CommandLog : public TextBase
{
public:

	CommandLog(MainDirector* director,  JXTextMenu* editMenu,
				  const JString& cmd, const bool refreshRepo,
				  const bool refreshStatus, const bool reloadOpenFiles,
				  JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				  const HSizingOption hSizing, const VSizingOption vSizing,
				  const JCoordinate x, const JCoordinate y,
				  const JCoordinate w, const JCoordinate h);

	~CommandLog() override;

	void	UpdateActionsMenu(JXTextMenu* menu) override;
	void	RefreshContent() override;
	void	GetSelectedFiles(JPtrArray<JString>* fullNameList,
							 const bool includeDeleted = false) override;

protected:

	JError	StartProcess(JProcess** p, int* outFD) override;

private:

	const JString	itsCmd;
	bool			itsHasRefreshedFlag;
	bool			itsRefreshRepoFlag;
	bool			itsRefreshStatusFlag;
	bool			itsReloadOpenFilesFlag;
};

#endif
