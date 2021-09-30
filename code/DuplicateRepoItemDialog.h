/******************************************************************************
 DuplicateRepoItemDialog.h

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#ifndef _H_DuplicateRepoItemDialog
#define _H_DuplicateRepoItemDialog

#include <jx-af/jx/JXGetStringDialog.h>

class RepoTreeNode;

class DuplicateRepoItemDialog : public JXGetStringDialog
{
public:

	DuplicateRepoItemDialog(JXDirector* supervisor, const JString& windowTitle,
							   const JString& prompt, const JString& initialName,
							   RepoTreeNode* srcNode);

	virtual ~DuplicateRepoItemDialog();

	RepoTreeNode*	GetSrcNode();

protected:

	virtual bool	OKToDeactivate();

private:

	RepoTreeNode*	itsSrcNode;
};


/******************************************************************************
 GetSrcNode

 ******************************************************************************/

inline RepoTreeNode*
DuplicateRepoItemDialog::GetSrcNode()
{
	return itsSrcNode;
}

#endif
