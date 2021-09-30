/******************************************************************************
 GetRepoDialog.h

	Copyright (C) 2011 by John Lindal.

 ******************************************************************************/

#ifndef _H_GetRepoDialog
#define _H_GetRepoDialog

#include <jx-af/jx/JXDialogDirector.h>
#include <jx-af/jcore/JPrefObject.h>

class JXInputField;
class JXStringHistoryMenu;

class GetRepoDialog : public JXDialogDirector, public JPrefObject
{
public:

	GetRepoDialog(JXDirector* supervisor, const JString& windowTitle);

	virtual ~GetRepoDialog();

	const JString&	GetRepo() const;

protected:

	virtual void	ReadPrefs(std::istream& input) override;
	virtual void	WritePrefs(std::ostream& output) const override;

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

// begin JXLayout

	JXInputField*        itsRepoInput;
	JXStringHistoryMenu* itsRepoHistoryMenu;

// end JXLayout

private:

	void	BuildWindow(const JString& windowTitle);
};

#endif
