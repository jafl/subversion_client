/******************************************************************************
 GetRepoDialog.cpp

	BASE CLASS = JXModalDialogDirector

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
	const JString& windowTitle
	)
	:
	JXModalDialogDirector(),
	JPrefObject(GetPrefsManager(), kGetRepoDialogID)
{
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

	auto* repoUrlLabel =
		jnew JXStaticText(JGetString("repoUrlLabel::GetRepoDialog::JXLayout"), window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,20, 270,20);
	repoUrlLabel->SetToLabel(false);

	itsRepoHistoryMenu =
		jnew JXStringHistoryMenu(kHistoryLength, window,
					JXWidget::kFixedRight, JXWidget::kFixedTop, 260,40, 30,20);

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::GetRepoDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,80, 60,20);
	cancelButton->SetShortcuts(JGetString("cancelButton::shortcuts::GetRepoDialog::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::GetRepoDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 190,80, 60,20);
	okButton->SetShortcuts(JGetString("okButton::shortcuts::GetRepoDialog::JXLayout"));

	itsRepoInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 20,40, 240,20);
	itsRepoInput->SetIsRequired();

// end JXLayout

	window->SetTitle(windowTitle);
	window->LockCurrentMinSize();
	SetButtons(okButton, cancelButton);

	itsRepoInput->SetIsRequired();
	ListenTo(itsRepoHistoryMenu, std::function([this](const JXMenu::ItemSelected& msg)
	{
		itsRepoInput->GetText()->SetText(itsRepoHistoryMenu->GetItemText(msg));
	}));
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
