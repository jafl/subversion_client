/******************************************************************************
 PrefsDialog.cpp

	BASE CLASS = JXModalDialogDirector

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "PrefsDialog.h"
#include "globals.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXRadioGroup.h>
#include <jx-af/jx/JXTextRadioButton.h>
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jcore/jProcessUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

PrefsDialog::PrefsDialog
	(
	const PrefsManager::Integration	type,

	const JString&	commitEditor,
	const JString&	diffCmd,
	const JString&	reloadChangedCmd
	)
	:
	JXModalDialogDirector()
{
	BuildWindow(type, commitEditor,diffCmd, reloadChangedCmd);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

PrefsDialog::~PrefsDialog()
{
}

/******************************************************************************
 GetData

 ******************************************************************************/

void
PrefsDialog::GetData
	(
	PrefsManager::Integration*	type,

	JString*	commitEditor,
	JString*	diffCmd,
	JString*	reloadChangedCmd
	)
	const
{
	*type = (PrefsManager::Integration) itsIntegrationRG->GetSelectedItem();

	*commitEditor     = itsCommitEditor->GetText()->GetText();
	*diffCmd          = itsDiffCmd->GetText()->GetText();
	*reloadChangedCmd = itsReloadChangedCmd->GetText()->GetText();
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
PrefsDialog::BuildWindow
	(
	const PrefsManager::Integration	type,

	const JString&	commitEditor,
	const JString&	diffCmd,
	const JString&	reloadChangedCmd
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 400,250, JGetString("WindowTitle::PrefsDialog::JXLayout"));

	itsIntegrationRG =
		jnew JXRadioGroup(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,10, 140,90);

	itsJCCIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCodeCrusader, JGetString("itsJCCIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,10, 120,20);

	auto* integrateWithLabel =
		jnew JXStaticText(JGetString("integrateWithLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 60,45, 95,20);
	integrateWithLabel->SetToLabel(false);

	itsCmdLineIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCmdLine, JGetString("itsCmdLineIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,35, 120,20);

	itsCustomIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCustom, JGetString("itsCustomIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,60, 120,20);

	auto* customIntegrationLabel =
		jnew JXStaticText(JGetString("customIntegrationLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,120, 120,20);
	customIntegrationLabel->SetToLabel(false);

	auto* commitEditorLabel =
		jnew JXStaticText(JGetString("commitEditorLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,140, 140,20);
	commitEditorLabel->SetToLabel(false);

	auto* compareRevisionsLabel =
		jnew JXStaticText(JGetString("compareRevisionsLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,160, 140,20);
	compareRevisionsLabel->SetToLabel(false);

	auto* reloadChangedFilesLabel =
		jnew JXStaticText(JGetString("reloadChangedFilesLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,180, 140,20);
	reloadChangedFilesLabel->SetToLabel(false);

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,220, 70,20);
	cancelButton->SetShortcuts(JGetString("cancelButton::shortcuts::PrefsDialog::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 239,219, 72,22);
	okButton->SetShortcuts(JGetString("okButton::shortcuts::PrefsDialog::JXLayout"));

	itsCommitEditor =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,140, 220,20);
	itsCommitEditor->SetIsRequired();

	itsDiffCmd =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,160, 220,20);
	itsDiffCmd->SetIsRequired();

	itsReloadChangedCmd =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,180, 220,20);

// end JXLayout

	SetButtons(okButton, cancelButton);

	itsIntegrationRG->SelectItem(type);

	itsCommitEditor->GetText()->SetText(commitEditor);
	itsCommitEditor->SetFont(JFontManager::GetDefaultMonospaceFont());

	itsDiffCmd->GetText()->SetText(diffCmd);
	itsDiffCmd->SetFont(JFontManager::GetDefaultMonospaceFont());

	itsReloadChangedCmd->GetText()->SetText(reloadChangedCmd);
	itsReloadChangedCmd->SetFont(JFontManager::GetDefaultMonospaceFont());

	UpdateDisplay();

	ListenTo(itsIntegrationRG);
}

/******************************************************************************
 UpdateDisplay (private)

 ******************************************************************************/

void
PrefsDialog::UpdateDisplay()
{
	if (JProgramAvailable(JGetString("CodeCrusaderBinary::global")))
	{
		itsJCCIntegrationRB->Activate();
	}
	else
	{
		itsJCCIntegrationRB->Deactivate();
		if (itsJCCIntegrationRB->IsChecked())
		{
			itsCmdLineIntegrationRB->Select();
		}
	}

	const bool enableCmds = itsIntegrationRG->GetSelectedItem() == PrefsManager::kCustom;

	itsCommitEditor->SetActive(enableCmds);
	itsDiffCmd->SetActive(enableCmds);
	itsReloadChangedCmd->SetActive(enableCmds);

	if (enableCmds)
	{
		itsCommitEditor->Focus();
	}
	else
	{
		GetWindow()->KillFocus();
	}
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
PrefsDialog::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsIntegrationRG && message.Is(JXRadioGroup::kSelectionChanged))
	{
		UpdateDisplay();
	}
	else
	{
		JXModalDialogDirector::Receive(sender, message);
	}
}
