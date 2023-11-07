/******************************************************************************
 TabBase.cpp

	BASE CLASS = none

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "TabBase.h"
#include "MainDirector.h"
#include "globals.h"
#include <jx-af/jfs/JXFSBindingManager.h>
#include <jx-af/jx/JXWebBrowser.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/JSubstitute.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

TabBase::TabBase
	(
	MainDirector* director
	)
	:
	itsDirector(director)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

TabBase::~TabBase()
{
}

/******************************************************************************
 GetPath

 ******************************************************************************/

const JString&
TabBase::GetPath()
	const
{
	return itsDirector->GetPath();
}

/******************************************************************************
 GetSelectedFilesForDiff (virtual)

 ******************************************************************************/

void
TabBase::GetSelectedFilesForDiff
	(
	JPtrArray<JString>* fullNameList,
	JArray<JIndex>*		revList
	)
{
	revList->RemoveAll();
	return GetSelectedFiles(fullNameList);
}

/******************************************************************************
 OpenFiles (virtual)

 ******************************************************************************/

void
TabBase::OpenFiles()
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list);

	JPtrArray<JString> pathList(JPtrArrayT::kDeleteAll);

	const JSize count = list.GetItemCount();
	for (JIndex i=count; i>=1; i--)
	{
		JString* s = list.GetItem(i);
		if (JDirectoryExists(*s))
		{
			pathList.Append(s);
			list.RemoveItem(i);
		}
	}

	(JXGetWebBrowser())->ShowFileLocations(pathList);
	JXFSBindingManager::Exec(list);
}

/******************************************************************************
 ShowFiles (virtual)

 ******************************************************************************/

void
TabBase::ShowFiles()
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list);
	(JXGetWebBrowser())->ShowFileLocations(list);
}

/******************************************************************************
 GetBaseRevision (virtual)

 ******************************************************************************/

bool
TabBase::GetBaseRevision
	(
	JString* rev
	)
{
	rev->Clear();
	return false;
}

/******************************************************************************
 CompareEdited

	Most of the time, it doesn't make sense to compare with edited, since
	the edited version is being referenced.  We therefore require a base
	revision.

 ******************************************************************************/

void
TabBase::CompareEdited
	(
	const JString& rev
	)
{
	Compare(rev, false);
}

/******************************************************************************
 CompareCurrent

 ******************************************************************************/

void
TabBase::CompareCurrent
	(
	const JString& rev
	)
{
	JString r;
	if (!rev.IsEmpty())
	{
		r = rev;
		if (itsDirector->HasPath())
		{
			r += ":BASE";
		}
	}

	Compare(r, false);
}

/******************************************************************************
 ComparePrev

 ******************************************************************************/

void
TabBase::ComparePrev
	(
	const JString& revStr
	)
{
	JString r("PREV");
	if (!revStr.IsEmpty())
	{
		JUInt rev;
		if (revStr.ConvertToUInt(&rev) && rev > 0)
		{
			r  = JString((JUInt64) rev-1);
			r += ":";
			r += JString((JUInt64) rev);
		}
		else
		{
			r += ":";
			r += revStr;
		}
	}

	Compare(r, true);
}

/******************************************************************************
 Compare (private)

 ******************************************************************************/

void
TabBase::Compare
	(
	const JString&	rev,
	const bool	isPrev
	)
{
	PrefsManager::Integration type;
	JString cmd;
	const bool hasCmd =
		(GetPrefsManager())->GetCommand(PrefsManager::kDiffCmd, &type, &cmd);

	if (type == PrefsManager::kCodeCrusader)
	{
		ExecuteJCCDiff(rev, isPrev);
	}
	else if (hasCmd)
	{
		JString r = rev;
		if (!r.IsEmpty())
		{
			r.Prepend("-r ");
		}
		ExecuteDiff(cmd, r, isPrev);
	}
}

/******************************************************************************
 ExecuteDiff (private)

 ******************************************************************************/

bool
TabBase::ExecuteDiff
	(
	const JString&	origCmd,
	const JString&	rev,
	const bool	isPrev
	)
{
	JPtrArray<JString> fileList(JPtrArrayT::kDeleteAll);
	JArray<JIndex> revList;
	GetSelectedFilesForDiff(&fileList, &revList);
	if (fileList.IsEmpty())
	{
		return false;
	}

	JXGetApplication()->DisplayBusyCursor();

	JSubstitute subst;
	subst.DefineVariable("rev_option", rev);
	const bool customPrev = isPrev && !revList.IsEmpty();

	const JSize count = fileList.GetItemCount();
	JString cmd, fullName, r;
	for (JIndex i=1; i<=count; i++)
	{
		cmd      = origCmd;
		fullName = JPrepArgForExec(*(fileList.GetItem(i)));

		if (customPrev)
		{
			const JIndex j = revList.GetItem(i);
			r  = JString((JUInt64) j-1);
			r += ":";
			r += JString((JUInt64) j);
			subst.DefineVariable("rev_option", r);
		}

		subst.DefineVariable("file_name", fullName);
		subst.Substitute(&cmd);

		if (itsDirector->HasPath())
		{
			JSimpleProcess::Create(itsDirector->GetPath(), cmd, true);
		}
		else
		{
			JSimpleProcess::Create(cmd, true);
		}
	}

	return true;
}

/******************************************************************************
 ExecuteJCCDiff (private)

 ******************************************************************************/

bool
TabBase::ExecuteJCCDiff
	(
	const JString&	rev,
	const bool	isPrev
	)
{
	JPtrArray<JString> fileList(JPtrArrayT::kDeleteAll);
	JArray<JIndex> revList;
	GetSelectedFilesForDiff(&fileList, &revList);
	if (fileList.IsEmpty())
	{
		return false;
	}

	const bool customPrev = isPrev && !revList.IsEmpty();

	const JSize count = fileList.GetItemCount();
	JString cmd, s, fullName;
	JSubstitute subst;
	for (JIndex i=1; i<=count; i++)
	{
		if (customPrev)
		{
			const JIndex j = revList.GetItem(i);
			s  = JString((JUInt64) j-1);
			s += ":";
			s += JString((JUInt64) j);
		}
		else
		{
			s = rev;
		}
		s += " $file_name";

		fullName = JPrepArgForExec(*(fileList.GetItem(i)));
		subst.DefineVariable("file_name", fullName);
		subst.Substitute(&s);

		if (count == 1)
		{
			s.Prepend(" --svn-diff ");
		}
		else
		{
			s.Prepend(" --svn-diff-silent ");
		}

		cmd += s;
	}

	cmd.Prepend("jcc");

	if (itsDirector->HasPath())
	{
		JSimpleProcess::Create(itsDirector->GetPath(), cmd, true);
	}
	else
	{
		JSimpleProcess::Create(cmd, true);
	}

	return true;
}

/******************************************************************************
 ScheduleForAdd (virtual)

 ******************************************************************************/

bool
TabBase::ScheduleForAdd()
{
	if (Execute(JString("svn add $file_name", JString::kNoCopy)))
	{
		itsDirector->RefreshStatus();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 ScheduleForRemove (virtual)

 ******************************************************************************/

bool
TabBase::ScheduleForRemove()
{
	if (Execute(JString("svn remove $file_name", JString::kNoCopy), "WarnRemove::TabBase", true))
	{
		itsDirector->RefreshStatus();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 ForceScheduleForRemove (virtual)

 ******************************************************************************/

bool
TabBase::ForceScheduleForRemove()
{
	if (Execute(JString("svn remove --force $file_name", JString::kNoCopy), "WarnRemove::TabBase", true))
	{
		itsDirector->RefreshStatus();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 Resolved (virtual)

 ******************************************************************************/

bool
TabBase::Resolved()
{
	if (Execute(JString("svn resolved $file_name", JString::kNoCopy)))
	{
		itsDirector->RefreshStatus();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 Commit (virtual)

 ******************************************************************************/

void
TabBase::Commit()
{
	JString cmd("svn commit $file_name");
	if (Prepare(&cmd, nullptr, true))
	{
		itsDirector->Commit(cmd);
	}
}

/******************************************************************************
 Revert (virtual)

 ******************************************************************************/

bool
TabBase::Revert()
{
	if (Execute(JString("svn revert $file_name", JString::kNoCopy), "WarnRevert::TabBase", true))
	{
		itsDirector->RefreshStatus();
		(GetApplication())->ReloadOpenFilesInIDE();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 CreateDirectory (virtual)

 ******************************************************************************/

void
TabBase::CreateDirectory()
{
}

/******************************************************************************
 DuplicateItem (virtual)

 ******************************************************************************/

void
TabBase::DuplicateItem()
{
}

/******************************************************************************
 CreateProperty (virtual)

 ******************************************************************************/

void
TabBase::CreateProperty()
{
}

/******************************************************************************
 SchedulePropertiesForRemove (virtual)

 ******************************************************************************/

bool
TabBase::SchedulePropertiesForRemove()
{
	return false;
}

/******************************************************************************
 Ignore (virtual)

 ******************************************************************************/

bool
TabBase::Ignore()
{
	return false;
}

/******************************************************************************
 CanCheckOutSelection (virtual)

 ******************************************************************************/

bool
TabBase::CanCheckOutSelection()
	const
{
	return false;
}

/******************************************************************************
 CheckOutSelection (virtual)

 ******************************************************************************/

void
TabBase::CheckOutSelection()
{
}

/******************************************************************************
 Prepare (private)

 ******************************************************************************/

bool
TabBase::Prepare
	(
	JString*			cmd,
	const JUtf8Byte*	warnMsgID,
	const bool		includeDeleted
	)
{
	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	GetSelectedFiles(&list, includeDeleted);
	if (list.IsEmpty())
	{
		return false;
	}

	if (warnMsgID != nullptr && !JGetUserNotification()->AskUserNo(JGetString(warnMsgID)))
	{
		return false;
	}

	const JSize count = list.GetItemCount();
	JString fileList;
	for (JIndex i=1; i<=count; i++)
	{
		fileList += JPrepArgForExec(*(list.GetItem(i)));
		fileList += " ";
	}

	JSubstitute subst;
	subst.DefineVariable("file_name", fileList);
	subst.Substitute(cmd);

	return true;
}

/******************************************************************************
 Execute (private)

 ******************************************************************************/

bool
TabBase::Execute
	(
	const JString&		origCmd,
	const JUtf8Byte*	warnMsgID,
	const bool		includeDeleted,
	const bool		blocking
	)
{
	JString cmd = origCmd;
	if (!Prepare(&cmd, warnMsgID, includeDeleted))
	{
		return false;
	}

	if (blocking)
	{
		JXGetApplication()->DisplayBusyCursor();

		JSimpleProcess* p;
		if (itsDirector->HasPath())
		{
			JSimpleProcess::Create(&p, itsDirector->GetPath(), cmd, true);
		}
		else
		{
			JSimpleProcess::Create(&p, cmd, true);
		}
		p->WaitUntilFinished();
	}
	else if (itsDirector->HasPath())
	{
		JSimpleProcess::Create(itsDirector->GetPath(), cmd, true);
	}
	else
	{
		JSimpleProcess::Create(cmd, true);
	}

	return true;
}
