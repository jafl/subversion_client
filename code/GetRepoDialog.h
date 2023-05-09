/******************************************************************************
 GetRepoDialog.h

	Copyright (C) 2011 by John Lindal.

 ******************************************************************************/

#ifndef _H_GetRepoDialog
#define _H_GetRepoDialog

#include <jx-af/jx/JXModalDialogDirector.h>
#include <jx-af/jcore/JPrefObject.h>

class JXInputField;
class JXStringHistoryMenu;

class GetRepoDialog : public JXModalDialogDirector, public JPrefObject
{
public:

	GetRepoDialog(const JString& windowTitle);

	~GetRepoDialog() override;

	const JString&	GetRepo() const;

protected:

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

private:

// begin JXLayout

	JXInputField*        itsRepoInput;
	JXStringHistoryMenu* itsRepoHistoryMenu;

// end JXLayout

private:

	void	BuildWindow(const JString& windowTitle);
};

#endif
