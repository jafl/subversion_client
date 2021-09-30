/******************************************************************************
 CreateRepoDirectoryDialog.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_CreateRepoDirectoryDialog
#define _H_CreateRepoDirectoryDialog

#include <jx-af/jx/JXGetStringDialog.h>

class RepoTreeNode;

class CreateRepoDirectoryDialog : public JXGetStringDialog
{
public:

	CreateRepoDirectoryDialog(JXDirector* supervisor, const JString& windowTitle,
							const JString& prompt, const JString& initialName,
							RepoTreeNode* parentNode);

	virtual ~CreateRepoDirectoryDialog();

	RepoTreeNode*	GetParentNode();

protected:

	virtual bool	OKToDeactivate();

private:

	RepoTreeNode*	itsParentNode;
};


/******************************************************************************
 GetParentNode

 ******************************************************************************/

inline RepoTreeNode*
CreateRepoDirectoryDialog::GetParentNode()
{
	return itsParentNode;
}

#endif
