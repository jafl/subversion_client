/******************************************************************************
 CreateRepoDirectoryDialog.cpp

	BASE CLASS = JXGetStringDialog

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "CreateRepoDirectoryDialog.h"
#include "RepoTreeNode.h"
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXCSFDialogBase.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

CreateRepoDirectoryDialog::CreateRepoDirectoryDialog
	(
	const JString&	windowTitle,
	const JString&	prompt,
	const JString&	initialName,
	RepoTreeNode*	parentNode
	)
	:
	JXGetStringDialog(windowTitle, prompt, initialName),
	itsParentNode(parentNode)
{
	assert( itsParentNode != nullptr );

	GetInputField()->GetText()->SetCharacterInWordFunction(JXCSFDialogBase::IsCharacterInWord);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CreateRepoDirectoryDialog::~CreateRepoDirectoryDialog()
{
}

/******************************************************************************
 OKToDeactivate (virtual protected)

 ******************************************************************************/

bool
CreateRepoDirectoryDialog::OKToDeactivate()
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
	if (itsParentNode->FindNamedChild(name, &node))
	{
		JGetUserNotification()->ReportError(JGetString("NameUsed::CreateRepoDirectoryDialog"));
		return false;
	}
	else
	{
		return true;
	}
}
