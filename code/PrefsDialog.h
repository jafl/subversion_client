/******************************************************************************
 PrefsDialog.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_PrefsDialog
#define _H_PrefsDialog

#include <jx-af/jx/JXDialogDirector.h>
#include "PrefsManager.h"	// need defn of Integration

class JXRadioGroup;
class JXTextRadioButton;
class JXInputField;

class PrefsDialog : public JXDialogDirector
{
public:

	PrefsDialog(JXDirector* supervisor,
				   const PrefsManager::Integration type,
				   const JString& commitEditor,
				   const JString& diffCmd,
				   const JString& reloadChangedCmd);

	virtual	~PrefsDialog();

	void	GetData(PrefsManager::Integration* type, JString* commitEditor,
					JString* diffCmd, JString* reloadChangedCmd) const;

protected:

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

// begin JXLayout

	JXInputField*      itsCommitEditor;
	JXInputField*      itsDiffCmd;
	JXRadioGroup*      itsIntegrationRG;
	JXTextRadioButton* itsJCCIntegrationRB;
	JXTextRadioButton* itsCustomIntegrationRB;
	JXTextRadioButton* itsCmdLineIntegrationRB;
	JXInputField*      itsReloadChangedCmd;

// end JXLayout

private:

	void	BuildWindow(const PrefsManager::Integration type,
						const JString& commitEditor, const JString& diffCmd,
						const JString& reloadChangedCmd);
	void	UpdateDisplay();
};

#endif
