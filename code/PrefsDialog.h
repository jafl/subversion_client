/******************************************************************************
 PrefsDialog.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_PrefsDialog
#define _H_PrefsDialog

#include <jx-af/jx/JXModalDialogDirector.h>
#include "PrefsManager.h"	// for Integration

class JXRadioGroup;
class JXTextRadioButton;
class JXInputField;

class PrefsDialog : public JXModalDialogDirector
{
public:

	PrefsDialog(const PrefsManager::Integration type,
				const JString& commitEditor,
				const JString& diffCmd,
				const JString& reloadChangedCmd);

	~PrefsDialog() override;

	void	GetData(PrefsManager::Integration* type, JString* commitEditor,
					JString* diffCmd, JString* reloadChangedCmd) const;

protected:

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

// begin JXLayout

	JXRadioGroup*      itsIntegrationRG;
	JXTextRadioButton* itsJCCIntegrationRB;
	JXTextRadioButton* itsCmdLineIntegrationRB;
	JXTextRadioButton* itsCustomIntegrationRB;
	JXInputField*      itsCommitEditor;
	JXInputField*      itsDiffCmd;
	JXInputField*      itsReloadChangedCmd;

// end JXLayout

private:

	void	BuildWindow(const PrefsManager::Integration type,
						const JString& commitEditor, const JString& diffCmd,
						const JString& reloadChangedCmd);
	void	UpdateDisplay();
};

#endif
