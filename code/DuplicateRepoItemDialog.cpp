/******************************************************************************
 DuplicateRepoItemDialog.cpp

	BASE CLASS = JXGetStringDialog

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "DuplicateRepoItemDialog.h"
#include "RepoTreeNode.h"
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXCSFDialogBase.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

DuplicateRepoItemDialog::DuplicateRepoItemDialog
	(
	const JString&	windowTitle,
	const JString&	prompt,
	const JString&	initialName,
	RepoTreeNode*	srcNode
	)
	:
	JXGetStringDialog(windowTitle, prompt, initialName),
	itsSrcNode(srcNode)
{
	assert( itsSrcNode != nullptr );

	GetInputField()->GetText()->SetCharacterInWordFunction(JXCSFDialogBase::IsCharacterInWord);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

DuplicateRepoItemDialog::~DuplicateRepoItemDialog()
{
}

/******************************************************************************
 OKToDeactivate (virtual protected)

 ******************************************************************************/

bool
DuplicateRepoItemDialog::OKToDeactivate()
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
		JGetUserNotification()->ReportError(JGetString("NameUsed::DuplicateRepoItemDialog"));
		return false;
	}
	else
	{
		return true;
	}
}
