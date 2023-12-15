/******************************************************************************
 TextBase.cpp

	BASE CLASS = JXStaticText, TabBase

	Copyright @ 2008 by John Lindal.

 ******************************************************************************/

#include "TextBase.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jcore/JProcess.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

	Derived class must call RefreshContent()

 ******************************************************************************/

TextBase::TextBase
	(
	MainDirector*	director,
	JXTextMenu*			editMenu,
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
	JXStaticText(JString::empty, false, true, true, scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	TabBase(director),
	itsProcess(nullptr),
	itsLink(nullptr)
{
	GetText()->SetDefaultFont(JFontManager::GetDefaultMonospaceFont());

	FitToEnclosure();

	ShareEditMenu(editMenu);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

TextBase::~TextBase()
{
	if (itsProcess != nullptr)
	{
		StopListening(itsProcess);
		itsProcess->Kill();
	}
	jdelete itsProcess;
	itsProcess = nullptr;	// avoid crash in ReceiveGoingAway()

	DeleteLink();
}

/******************************************************************************
 RefreshContent (virtual)

 ******************************************************************************/

void
TextBase::RefreshContent()
{
	if (itsProcess != nullptr)
	{
		JProcess* p = itsProcess;
		itsProcess  = nullptr;

		p->Kill();
		jdelete p;

		DeleteLink();
	}

	GetText()->SetText(JString::empty);

	JProcess* p;
	int outFD;
	const JError err = StartProcess(&p, &outFD);
	if (err.OK())
	{
		SetConnection(p, outFD);
	}
	else
	{
		err.ReportIfError();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
TextBase::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsLink && message.Is(JAsynchDataReceiverT::kDataReady))
	{
		ReceiveData(message);
	}
	else
	{
		JXStaticText::Receive(sender, message);
	}
}

/******************************************************************************
 ReceiveGoingAway (virtual protected)

 ******************************************************************************/

void
TextBase::ReceiveGoingAway
	(
	JBroadcaster* sender
	)
{
	if (sender == itsProcess)
	{
		itsProcess = nullptr;
		DeleteLink();
	}
	else
	{
		JXStaticText::ReceiveGoingAway(sender);
	}
}

/******************************************************************************
 ReceiveData (private)

 ******************************************************************************/

void
TextBase::ReceiveData
	(
	const Message& message
	)
{
	const auto* info =
		dynamic_cast<const JAsynchDataReceiverT::DataReady*>(&message);
	assert( info != nullptr );

	const DisplayState state = SaveDisplayState();

	SetCaretLocation(GetText()->GetText().GetCharacterCount()+1);
	Paste(info->GetData());
	GetText()->ClearUndo();

	RestoreDisplayState(state);
}

/******************************************************************************
 AdjustCursor (virtual protected)

 ******************************************************************************/

void
TextBase::AdjustCursor
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	if (itsProcess != nullptr)
	{
		DisplayCursor(kJXBusyCursor);
	}
	else
	{
		JXWidget::AdjustCursor(pt, modifiers);
	}
}

/******************************************************************************
 UpdateActionsMenu (virtual)

 ******************************************************************************/

#include "MainDirector-Actions.h"

void
TextBase::UpdateActionsMenu
	(
	JXTextMenu* menu
	)
{
	menu->EnableItem(kRefreshCmd);
}

/******************************************************************************
 UpdateInfoMenu (virtual)

 ******************************************************************************/

void
TextBase::UpdateInfoMenu
	(
	JXTextMenu* menu
	)
{
}

/******************************************************************************
 SetConnection (private)

 ******************************************************************************/

void
TextBase::SetConnection
	(
	JProcess*	p,
	const int	fd
	)
{
	itsProcess = p;
	ListenTo(itsProcess);
	p->ShouldDeleteWhenFinished();

	itsLink = new DataLink(fd);
	assert( itsLink != nullptr );
	ListenTo(itsLink);
}

/******************************************************************************
 DeleteLink (private)

 ******************************************************************************/

void
TextBase::DeleteLink()
{
	delete itsLink;
	itsLink = nullptr;
}
