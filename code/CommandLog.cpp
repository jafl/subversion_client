/******************************************************************************
 CommandLog.cpp

	BASE CLASS = TextBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "CommandLog.h"
#include "MainDirector.h"
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/JStdError.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

CommandLog::CommandLog
	(
	MainDirector*	director,
	JXTextMenu*			editMenu,
	const JString&		cmd,
	const bool		refreshRepo,
	const bool		refreshStatus,
	const bool		reloadOpenFiles,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption hSizing,
	const VSizingOption vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	TextBase(director, editMenu, scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	itsCmd(cmd),
	itsHasRefreshedFlag(false),
	itsRefreshRepoFlag(refreshRepo),
	itsRefreshStatusFlag(refreshStatus),
	itsReloadOpenFilesFlag(reloadOpenFiles)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CommandLog::~CommandLog()
{
}

/******************************************************************************
 RefreshContent (virtual)

 ******************************************************************************/

void
CommandLog::RefreshContent()
{
	if (!itsHasRefreshedFlag)
{
		TextBase::RefreshContent();
		itsHasRefreshedFlag = true;
}
}

/******************************************************************************
 StartProcess (virtual protected)

 ******************************************************************************/

JError
CommandLog::StartProcess
	(
	JProcess**	p,
	int*		outFD
	)
{
	if (!GetDirector()->OKToStartActionProcess())
	{
		return JWouldHaveBlocked();
	}

	JError err = JNoError();
	if (GetDirector()->HasPath())
	{
		err = JProcess::Create(p, GetPath(), itsCmd,
							   kJIgnoreConnection, nullptr,
							   kJCreatePipe, outFD,
							   kJAttachToFromFD);
	}
	else
	{
		err = JProcess::Create(p, itsCmd,
							   kJIgnoreConnection, nullptr,
							   kJCreatePipe, outFD,
							   kJAttachToFromFD);
	}

	if (err.OK())
	{
		GetDirector()->RegisterActionProcess(this, *p, itsRefreshRepoFlag,
											   itsRefreshStatusFlag, itsReloadOpenFilesFlag);
	}

	return err;
}

/******************************************************************************
 UpdateActionsMenu (virtual)

 ******************************************************************************/

void
CommandLog::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
}

/******************************************************************************
 GetSelectedFiles (virtual)

 ******************************************************************************/

void
CommandLog::GetSelectedFiles
	(
	JPtrArray<JString>*	fullNameList,
	const bool		includeDeleted
	)
{
	fullNameList->CleanOut();
}
