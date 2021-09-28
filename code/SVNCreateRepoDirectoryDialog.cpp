/******************************************************************************
 SVNCreateRepoDirectoryDialog.cpp

	BASE CLASS = JXGetStringDialog

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "SVNCreateRepoDirectoryDialog.h"
#include "SVNRepoTreeNode.h"
#include <jx-af/jx/JXInputField.h>
#include <jx-af/jx/JXChooseSaveFile.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

SVNCreateRepoDirectoryDialog::SVNCreateRepoDirectoryDialog
	(
	JXDirector*			supervisor,
	const JString&		windowTitle,
	const JString&		prompt,
	const JString&		initialName,
	SVNRepoTreeNode*	parentNode
	)
	:
	JXGetStringDialog(supervisor, windowTitle, prompt, initialName),
	itsParentNode(parentNode)
{
	assert( itsParentNode != nullptr );

	GetInputField()->GetText()->SetCharacterInWordFunction(JXChooseSaveFile::IsCharacterInWord);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

SVNCreateRepoDirectoryDialog::~SVNCreateRepoDirectoryDialog()
{
}

/******************************************************************************
 OKToDeactivate (virtual protected)

 ******************************************************************************/

bool
SVNCreateRepoDirectoryDialog::OKToDeactivate()
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
		JGetUserNotification()->ReportError(JGetString("NameUsed::SVNCreateRepoDirectoryDialog"));
		return false;
	}
	else
	{
		return true;
	}
}
