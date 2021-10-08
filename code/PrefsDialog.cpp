/******************************************************************************
 PrefsDialog.cpp

	BASE CLASS = JXDialogDirector

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
	JXDirector*							supervisor,
	const PrefsManager::Integration	type,
	const JString&						commitEditor,
	const JString&						diffCmd,
	const JString&						reloadChangedCmd
	)
	:
	JXDialogDirector(supervisor, true)
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
	JString*						commitEditor,
	JString*						diffCmd,
	JString*						reloadChangedCmd
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
	const JString&						commitEditor,
	const JString&						diffCmd,
	const JString&						reloadChangedCmd
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 400,250, JString::empty);
	assert( window != nullptr );

	itsCommitEditor =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,140, 220,20);
	assert( itsCommitEditor != nullptr );

	itsDiffCmd =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,160, 220,20);
	assert( itsDiffCmd != nullptr );

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,220, 70,20);
	assert( cancelButton != nullptr );
	cancelButton->SetShortcuts(JGetString("cancelButton::PrefsDialog::shortcuts::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 240,220, 70,20);
	assert( okButton != nullptr );
	okButton->SetShortcuts(JGetString("okButton::PrefsDialog::shortcuts::JXLayout"));

	itsIntegrationRG =
		jnew JXRadioGroup(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,10, 140,90);
	assert( itsIntegrationRG != nullptr );

	itsJCCIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCodeCrusader, JGetString("itsJCCIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,10, 120,20);
	assert( itsJCCIntegrationRB != nullptr );

	itsCustomIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCustom, JGetString("itsCustomIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,60, 120,20);
	assert( itsCustomIntegrationRB != nullptr );

	auto* integrateWithLabel =
		jnew JXStaticText(JGetString("integrateWithLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 60,45, 95,20);
	assert( integrateWithLabel != nullptr );
	integrateWithLabel->SetToLabel();

	auto* customIntegrationLabel =
		jnew JXStaticText(JGetString("customIntegrationLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,120, 120,20);
	assert( customIntegrationLabel != nullptr );
	customIntegrationLabel->SetToLabel();

	auto* reloadChangedFilesLabel =
		jnew JXStaticText(JGetString("reloadChangedFilesLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,180, 140,20);
	assert( reloadChangedFilesLabel != nullptr );
	reloadChangedFilesLabel->SetToLabel();

	auto* compareRevisionsLabel =
		jnew JXStaticText(JGetString("compareRevisionsLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,160, 140,20);
	assert( compareRevisionsLabel != nullptr );
	compareRevisionsLabel->SetToLabel();

	itsCmdLineIntegrationRB =
		jnew JXTextRadioButton(PrefsManager::kCmdLine, JGetString("itsCmdLineIntegrationRB::PrefsDialog::JXLayout"), itsIntegrationRG,
					JXWidget::kHElastic, JXWidget::kFixedTop, 10,35, 120,20);
	assert( itsCmdLineIntegrationRB != nullptr );

	itsReloadChangedCmd =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 160,180, 220,20);
	assert( itsReloadChangedCmd != nullptr );

	auto* commitEditorLabel =
		jnew JXStaticText(JGetString("commitEditorLabel::PrefsDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,140, 140,20);
	assert( commitEditorLabel != nullptr );
	commitEditorLabel->SetToLabel();

// end JXLayout

	window->SetTitle(JGetString("WindowTitle::PrefsDialog"));
	SetButtons(okButton, cancelButton);

	itsIntegrationRG->SelectItem(type);

	itsCommitEditor->GetText()->SetText(commitEditor);
	itsCommitEditor->SetIsRequired();
	itsCommitEditor->SetFont(JFontManager::GetDefaultMonospaceFont());

	itsDiffCmd->GetText()->SetText(diffCmd);
	itsDiffCmd->SetIsRequired();
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
		JXDialogDirector::Receive(sender, message);
	}
}
