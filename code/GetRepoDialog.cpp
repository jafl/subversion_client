/******************************************************************************
 GetRepoDialog.cpp

	BASE CLASS = JXDialogDirector

	Copyright (C) 2011 by John Lindal.

 ******************************************************************************/

#include "GetRepoDialog.h"
#include "globals.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXStringHistoryMenu.h>
#include <jx-af/jcore/jAssert.h>

const JSize kHistoryLength = 20;

const JFileVersion kCurrentPrefsVersion = 1;

	// version  1 adds window geometry

/******************************************************************************
 Constructor

 ******************************************************************************/

GetRepoDialog::GetRepoDialog
	(
	JXDirector*		supervisor,
	const JString&	windowTitle
	)
	:
	JXDialogDirector(supervisor, true),
	JPrefObject(GetPrefsManager(), kGetRepoDialogID)
{
	UseModalPlacement(false);
	BuildWindow(windowTitle);

	JPrefObject::ReadPrefs();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

GetRepoDialog::~GetRepoDialog()
{
	JPrefObject::WritePrefs();
}

/******************************************************************************
 GetRepo

 ******************************************************************************/

const JString&
GetRepoDialog::GetRepo()
	const
{
	const JString& s = itsRepoInput->GetText()->GetText();
	itsRepoHistoryMenu->AddString(s);
	return s;
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
GetRepoDialog::BuildWindow
	(
	const JString& windowTitle
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 310,110, JString::empty);
	assert( window != nullptr );

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::GetRepoDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 190,80, 60,20);
	assert( okButton != nullptr );
	okButton->SetShortcuts(JGetString("okButton::GetRepoDialog::shortcuts::JXLayout"));

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::GetRepoDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,80, 60,20);
	assert( cancelButton != nullptr );
	cancelButton->SetShortcuts(JGetString("cancelButton::GetRepoDialog::shortcuts::JXLayout"));

	itsRepoInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,40, 240,20);
	assert( itsRepoInput != nullptr );

	itsRepoHistoryMenu =
		jnew JXStringHistoryMenu(kHistoryLength, JString::empty, window,
					JXWidget::kFixedRight, JXWidget::kFixedTop, 260,40, 30,20);
	assert( itsRepoHistoryMenu != nullptr );

	auto* repoUrlLabel =
		jnew JXStaticText(JGetString("repoUrlLabel::GetRepoDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,20, 270,20);
	assert( repoUrlLabel != nullptr );
	repoUrlLabel->SetToLabel();

// end JXLayout

	window->SetTitle(windowTitle);
	window->LockCurrentMinSize();
	SetButtons(okButton, cancelButton);

	itsRepoInput->SetIsRequired();
	ListenTo(itsRepoHistoryMenu);
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
GetRepoDialog::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsRepoHistoryMenu && message.Is(JXMenu::kItemSelected))
	{
		itsRepoInput->GetText()->SetText(itsRepoHistoryMenu->GetItemText(message));
	}
	else
	{
		JXDialogDirector::Receive(sender, message);
	}
}

/******************************************************************************
 ReadPrefs (virtual protected)

 ******************************************************************************/

void
GetRepoDialog::ReadPrefs
	(
	std::istream& input
	)
{
	JFileVersion vers;
	input >> vers;
	if (vers > kCurrentPrefsVersion)
	{
		return;
	}

	if (vers >= 1)
	{
		GetWindow()->ReadGeometry(input);
	}
	else
	{
		GetWindow()->PlaceAsDialogWindow();
	}

	itsRepoHistoryMenu->ReadSetup(input);
}

/******************************************************************************
 WritePrefs (virtual protected)

 ******************************************************************************/

void
GetRepoDialog::WritePrefs
	(
	std::ostream& output
	)
	const
{
	output << kCurrentPrefsVersion;
	output << ' ';
	GetWindow()->WriteGeometry(output);
	output << ' ';
	itsRepoHistoryMenu->WriteSetup(output);
}
