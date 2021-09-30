/******************************************************************************
 PropertiesList.cpp

	BASE CLASS = ListBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "PropertiesList.h"
#include "MainDirector.h"
#include "menus.h"
#include <jx-af/jx/JXGetStringDialog.h>
#include <jx-af/jx/JXTextSelection.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JTableSelectionIterator.h>
#include <jx-af/jcore/JSubstitute.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/jAssert.h>

static const JString kPropEditCmd("svn propedit $prop_name $file_name", JString::kNoCopy);
static const JString kPropRemoveCmd("svn propdel $prop_name $file_name", JString::kNoCopy);

static const JUtf8Byte* kSpecialPropertyList[] =
{
	"svn:ignore",
	"svn:keywords",
	"svn:executable",
	"svn:eol-style",
	"svn:mime-type",
	"svn:externals",
	"svn:needs-lock"
};

const JSize kSpecialPropertyCount = sizeof(kSpecialPropertyList) / sizeof(JUtf8Byte*);

/******************************************************************************
 Constructor

 ******************************************************************************/

PropertiesList::PropertiesList
	(
	MainDirector*	director,
	JXTextMenu*			editMenu,
	const JString&		fullName,
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
	ListBase(director, editMenu, GetCommand(fullName), false, false, false, false,
				scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	itsFullName(fullName),
	itsCreatePropertyDialog(nullptr)
{
	itsRemovePropertyCmdList = jnew JPtrArray<JString>(JPtrArrayT::kDeleteAll);
	assert( itsRemovePropertyCmdList != nullptr );

	itsProcessList = jnew JPtrArray<JProcess>(JPtrArrayT::kForgetAll);
	assert( itsProcessList != nullptr );
}

/******************************************************************************
 Destructor

 ******************************************************************************/

PropertiesList::~PropertiesList()
{
	jdelete itsRemovePropertyCmdList;
	jdelete itsProcessList;
}

/******************************************************************************
 GetCommand (static private)

 ******************************************************************************/

JString
PropertiesList::GetCommand
	(
	const JString& fullName
	)
{
	JString cmd("svn --non-interactive proplist ");
	cmd += JPrepArgForExec(fullName);
	return cmd;
}

/******************************************************************************
 ShouldDisplayLine (virtual protected)

	Return false if the line should not be displayed.

 ******************************************************************************/

bool
PropertiesList::ShouldDisplayLine
	(
	JString* line
	)
	const
{
	if (line->BeginsWith(" "))
	{
		line->TrimWhitespace();
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 StyleLine (virtual protected)

 ******************************************************************************/

static const JFontStyle theBoldStyle(true, false, 0, false);

void
PropertiesList::StyleLine
	(
	const JIndex		index,
	const JString&		line,
	const JFontStyle&	errorStyle,
	const JFontStyle&	addStyle,
	const JFontStyle&	removeStyle
	)
{
	for (JUnsignedOffset i=0; i<kSpecialPropertyCount; i++)
	{
		if (line == kSpecialPropertyList[i])
		{
			SetStyle(index, theBoldStyle);
		}
	}
}

/******************************************************************************
 OpenSelectedItems (virtual)

 ******************************************************************************/

void
PropertiesList::OpenSelectedItems()
{
	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	JPoint cell;
	JString cmd, prop, file = JPrepArgForExec(itsFullName);
	JSubstitute subst;
	while (iter.Next(&cell))
	{
		const JString* line = (GetStringList()).GetElement(cell.y);

		prop = JPrepArgForExec(*line);

		cmd = kPropEditCmd;
		subst.DefineVariable("prop_name", prop);
		subst.DefineVariable("file_name", file);
		subst.Substitute(&cmd);

		JSimpleProcess::Create(cmd, true);
	}
}

/******************************************************************************
 CopySelectedItems (virtual protected)

 ******************************************************************************/

void
PropertiesList::CopySelectedItems
	(
	const bool fullPath
	)
{
	JTableSelection& s = GetTableSelection();
	if (!s.HasSelection())
	{
		return;
	}

	JPtrArray<JString> list(JPtrArrayT::kDeleteAll);
	JTableSelectionIterator iter(&s);
	JPoint cell;
	while (iter.Next(&cell))
	{
		list.Append(*((GetStringList()).GetElement(cell.y)));
	}

	auto* data = jnew JXTextSelection(GetDisplay(), list);
	assert( data != nullptr );

	GetSelectionManager()->SetData(kJXClipboardName, data);
}

/******************************************************************************
 GetSelectedFiles (virtual)

 ******************************************************************************/

void
PropertiesList::GetSelectedFiles
	(
	JPtrArray<JString>*	fullNameList,
	const bool		includeDeleted
	)
{
	fullNameList->CleanOut();
}

/******************************************************************************
 ExtractRelativePath (virtual protected)

 ******************************************************************************/

JString
PropertiesList::ExtractRelativePath
	(
	const JString& line
	)
	const
{
	return line;
}

/******************************************************************************
 UpdateActionsMenu (virtual)

 ******************************************************************************/

void
PropertiesList::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
	menu->EnableItem(kRefreshCmd);
	menu->EnableItem(kCreatePropertyCmd);

	JTableSelection& s = GetTableSelection();
	if (s.HasSelection())
	{
		menu->EnableItem(kRemoveSelectedPropertiesCmd);
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
PropertiesList::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsCreatePropertyDialog && message.Is(JXDialogDirector::kDeactivated))
	{
		const auto* info =
			dynamic_cast<const JXDialogDirector::Deactivated*>(&message);
		assert( info != nullptr );

		if (info->Successful())
		{
			CreateProperty1();
		}

		itsCreatePropertyDialog = nullptr;
	}

	else if (message.Is(JProcess::kFinished))
	{
		const JSize count = itsProcessList->GetElementCount();
		for (JIndex i=1; i<=count; i++)
		{
			if (sender == itsProcessList->GetElement(i))
			{
				itsProcessList->RemoveElement(i);
				RemoveNextProperty();
				break;
			}
		}
	}

	else
	{
		ListBase::Receive(sender, message);
	}
}

/******************************************************************************
 CreateProperty (virtual)

 ******************************************************************************/

bool
PropertiesList::CreateProperty()
{
	assert( itsCreatePropertyDialog == nullptr );

	itsCreatePropertyDialog =
		jnew JXGetStringDialog(
			GetDirector(), JGetString("CreatePropertyWindowTitle::PropertiesList"),
			JGetString("CreatePropertyPrompt::PropertiesList"), JString::empty);
	assert( itsCreatePropertyDialog != nullptr );
	ListenTo(itsCreatePropertyDialog);
	itsCreatePropertyDialog->BeginDialog();
	return true;
}

/******************************************************************************
 CreateProperty1 (private)

 ******************************************************************************/

bool
PropertiesList::CreateProperty1()
{
	assert( itsCreatePropertyDialog != nullptr );

	const JString prop = JPrepArgForExec(itsCreatePropertyDialog->GetString());
	const JString file = JPrepArgForExec(itsFullName);

	JSubstitute subst;
	subst.DefineVariable("prop_name", prop);
	subst.DefineVariable("file_name", file);

	JString cmd = kPropEditCmd;
	subst.Substitute(&cmd);

	JSimpleProcess* p;
	const JError err = JSimpleProcess::Create(&p, cmd, true);
	if (err.OK())
	{
		itsProcessList->Append(p);
		ListenTo(p);
		return true;
	}
	else
	{
		err.ReportIfError();
		return false;
	}
}

/******************************************************************************
 SchedulePropertiesForRemove (virtual)

 ******************************************************************************/

bool
PropertiesList::SchedulePropertiesForRemove()
{
	JTableSelection& s = GetTableSelection();
	JTableSelectionIterator iter(&s);
	JPoint cell;
	JString cmd, prop, file = JPrepArgForExec(itsFullName);
	JSubstitute subst;
	while (iter.Next(&cell))
	{
		const JString* line = (GetStringList()).GetElement(cell.y);

		prop = JPrepArgForExec(*line);

		cmd = kPropRemoveCmd;
		subst.DefineVariable("prop_name", prop);
		subst.DefineVariable("file_name", file);
		subst.Substitute(&cmd);

		itsRemovePropertyCmdList->Append(cmd);
	}

	RemoveNextProperty();
	return true;
}

/******************************************************************************
 RemoveNextProperty (private)

 ******************************************************************************/

bool
PropertiesList::RemoveNextProperty()
{
	if (!itsRemovePropertyCmdList->IsEmpty())
	{
		const JString cmd = *itsRemovePropertyCmdList->GetFirstElement();
		itsRemovePropertyCmdList->DeleteElement(1);

		JSimpleProcess* p;
		const JError err = JSimpleProcess::Create(&p, cmd, true);
		if (err.OK())
		{
			itsProcessList->Append(p);
			ListenTo(p);
			return true;
		}
		else
		{
			err.ReportIfError();
			return false;
		}
	}
	else
	{
		if (GetDirector()->OKToStartActionProcess())
		{
			RefreshContent();
		}

		GetDirector()->ScheduleStatusRefresh();
		return true;
	}
}
