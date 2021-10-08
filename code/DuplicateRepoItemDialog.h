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

	~DuplicateRepoItemDialog() override;

	RepoTreeNode*	GetSrcNode();

protected:

	bool	OKToDeactivate() override;

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
