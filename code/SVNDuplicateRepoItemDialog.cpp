/******************************************************************************
 SVNDuplicateRepoItemDialog.cpp

	BASE CLASS = JXGetStringDialog

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "SVNDuplicateRepoItemDialog.h"
#include "SVNRepoTreeNode.h"
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXChooseSaveFile.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

SVNDuplicateRepoItemDialog::SVNDuplicateRepoItemDialog
	(
	JXDirector*			supervisor,
	const JString&		windowTitle,
	const JString&		prompt,
	const JString&		initialName,
	SVNRepoTreeNode*	srcNode
	)
	:
	JXGetStringDialog(supervisor, windowTitle, prompt, initialName),
	itsSrcNode(srcNode)
{
	assert( itsSrcNode != nullptr );

	GetInputField()->GetText()->SetCharacterInWordFunction(JXChooseSaveFile::IsCharacterInWord);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

SVNDuplicateRepoItemDialog::~SVNDuplicateRepoItemDialog()
{
}

/******************************************************************************
 OKToDeactivate (virtual protected)

 ******************************************************************************/

bool
SVNDuplicateRepoItemDialog::OKToDeactivate()
{
	if (!JXGetStringDialog::OKToDeactivate())
	{
		return false;
	}
	else if (Cancelled())
	{
		return true;
	}

	const JString& name = GetString();
	JNamedTreeNode* node;
	if ((itsSrcNode->GetNamedParent())->FindNamedChild(name, &node))
	{
		JGetUserNotification()->ReportError(JGetString("NameUsed::SVNDuplicateRepoItemDialog"));
		return false;
	}
	else
	{
		return true;
	}
}
