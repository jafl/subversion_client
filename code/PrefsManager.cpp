/******************************************************************************
 PrefsManager.cpp

	BASE CLASS = public JXPrefsManager

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#include "PrefsManager.h"
#include "PrefsDialog.h"
#include "globals.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jcore/jProcessUtil.h>
#include <jx-af/jcore/jAssert.h>

const JFileVersion kCurrentPrefsFileVersion = 1;

	// version  1 adds commit editor to kIntegrationID.

// integration

static const JUtf8Byte* kCodeCrusaderCmd[] =
{
	"jcc --vcs-commit",
	"",		// not used
	"jcc --reload-open"
};

static const JUtf8Byte* kCmdLineCmd[] =
{
	"gnome-terminal -t \"Commit\" -x vi",
	"gnome-terminal -t \"svn diff $rev_option $file_name\" -x /bin/sh -c \"svn diff $rev_option $file_name | less\"",
	""		// not possible
};

static const JUtf8Byte** kIntegrationCmd[2] =
{
	kCodeCrusaderCmd,
	kCmdLineCmd
};

/******************************************************************************
 Constructor

 *****************************************************************************/

PrefsManager::PrefsManager
	(
	bool* isNew
	)
	:
	JXPrefsManager(kCurrentPrefsFileVersion, true, kgCSFSetupID)
{
	*isNew = JPrefsManager::UpgradeData();
}

/******************************************************************************
 Destructor

 *****************************************************************************/

PrefsManager::~PrefsManager()
{
	SaveAllBeforeDestruct();
}

/******************************************************************************
 SaveAllBeforeDestruct (virtual protected)

 ******************************************************************************/

void
PrefsManager::SaveAllBeforeDestruct()
{
	SetData(kProgramVersionID, GetVersionNumberStr());

	JXPrefsManager::SaveAllBeforeDestruct();
}

/******************************************************************************
 GetExpirationTimeStamp

 ******************************************************************************/

bool
PrefsManager::GetExpirationTimeStamp
	(
	time_t* t
	)
	const
{
	std::string data;
	if (GetData(kExpireTimeStampID, &data))
	{
		std::istringstream input(data);
		input >> *t;
		return !input.fail();
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 SetExpirationTimeStamp

 ******************************************************************************/

void
PrefsManager::SetExpirationTimeStamp
	(
	const time_t t
	)
{
	std::ostringstream data;
	data << t;

	SetData(kExpireTimeStampID, data);
}

/******************************************************************************
 UpgradeData (virtual protected)

 ******************************************************************************/

void
PrefsManager::UpgradeData
	(
	const bool		isNew,
	const JFileVersion	currentVersion
	)
{
	if (isNew)
	{
		SetData(kProgramVersionID, GetVersionNumberStr());
	}

	if (!isNew && currentVersion < 1)
	{
		Integration type;
		JString commitEditor, diffCmd, reloadChangedCmd;

		std::string data;
		if (GetData(kIntegrationID, &data))
		{
			std::istringstream dataStream(data);
			dataStream >> type;
			dataStream >> diffCmd;
			dataStream >> reloadChangedCmd;

			const JUtf8Byte* editor = getenv("_EDITOR");
			if (type == kCodeCrusader)
			{
				commitEditor = kCodeCrusaderCmd[ kCommitEditor ];
			}
			else if (type == kCustom && !JString::IsEmpty(editor))
			{
				commitEditor = editor;
			}
			else	// type == kCmdLine
			{
				commitEditor = kCmdLineCmd[ kCommitEditor ];
			}

			SetIntegration(type, commitEditor, diffCmd, reloadChangedCmd);
		}
	}

	// check if Code Crusader is available

	const bool hasJCC = JProgramAvailable(JGetString("CodeCrusaderBinary::global"));

	Integration type;
	JString commitEditor, diffCmd, reloadChangedCmd;
	if (GetIntegration(&type, &commitEditor, &diffCmd, &reloadChangedCmd))
	{
		bool changed = false;
		if (hasJCC && type != kCodeCrusader)
		{
			type    = kCodeCrusader;
			changed = true;
		}
		else if (!hasJCC && type == kCodeCrusader)
		{
			type    = kCmdLine;
			changed = true;
		}

		if (changed)
		{
			SetIntegration(type, commitEditor, diffCmd, reloadChangedCmd);
		}
		else
		{
			GetCommand(kCommitEditor, &type, &commitEditor);
			setenv("_EDITOR", commitEditor.GetBytes(), 1);
		}
	}
	else
	{
		SetIntegration(hasJCC ? kCodeCrusader : kCmdLine,
					   JString(kCmdLineCmd[ kCommitEditor ], JString::kNoCopy),
					   JString(kCmdLineCmd[ kDiffCmd ], JString::kNoCopy),
					   JString(kCmdLineCmd[ kReloadChangedCmd ], JString::kNoCopy));
	}
}

/******************************************************************************
 GetPrevVersionStr

 ******************************************************************************/

JString
PrefsManager::GetPrevVersionStr()
	const
{
	std::string data;
	const bool ok = GetData(kProgramVersionID, &data);
	assert( ok );
	return JString(data);
}

/******************************************************************************
 Program state

	RestoreProgramState() returns true if there was any state to restore.

	ForgetProgramState() is required because we have to save state before
	anything is closed.  If the close fails, we shouldn't save the state.

 ******************************************************************************/

bool
PrefsManager::RestoreProgramState()
{
	std::string data;
	if (GetData(kDocMgrStateID, &data))
	{
		std::istringstream dataStream(data);
		const bool restored =
			(GetWDManager())->RestoreState(dataStream);
		RemoveData(kDocMgrStateID);

		return restored;
	}
	else
	{
		return false;
	}
}

void
PrefsManager::SaveProgramState()
{
	std::ostringstream data;
	if ((GetWDManager())->SaveState(data))
	{
		SetData(kDocMgrStateID, data);
	}
	else
	{
		RemoveData(kDocMgrStateID);
	}
}

void
PrefsManager::ForgetProgramState()
{
	RemoveData(kDocMgrStateID);
}

/******************************************************************************
 EditPrefs

 ******************************************************************************/

void
PrefsManager::EditPrefs()
{
	Integration type;
	JString commitEditor, diffCmd, reloadChangedCmd;
	const bool exists = GetIntegration(&type, &commitEditor, &diffCmd, &reloadChangedCmd);
	assert( exists );

	auto* dlog = jnew PrefsDialog(type, commitEditor, diffCmd, reloadChangedCmd);
	assert( dlog != nullptr );

	if (dlog->DoDialog())
	{
		Integration type;
		JString commitEditor, diffCmd, reloadChangedCmd;
		dlog->GetData(&type, &commitEditor, &diffCmd, &reloadChangedCmd);

		SetIntegration(type, commitEditor, diffCmd, reloadChangedCmd);
	}
}

/******************************************************************************
 GetWindowSize

 ******************************************************************************/

bool
PrefsManager::GetWindowSize
	(
	const JPrefID&	id,
	JPoint*			desktopLoc,
	JCoordinate*	width,
	JCoordinate*	height
	)
	const
{
	std::string data;
	if (GetData(id, &data))
	{
		std::istringstream dataStream(data);
		dataStream >> *desktopLoc >> *width >> *height;
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 SaveWindowSize

 ******************************************************************************/

void
PrefsManager::SaveWindowSize
	(
	const JPrefID&	id,
	JXWindow*		window
	)
{
	std::ostringstream data;
	data << window->GetDesktopLocation();
	data << ' ' << window->GetFrameWidth();
	data << ' ' << window->GetFrameHeight();

	SetData(id, data);
}

/******************************************************************************
 GetCommand (private)

	Returns false if no command is available.

 ******************************************************************************/

bool
PrefsManager::GetCommand
	(
	const Command	cmdType,
	Integration*	type,
	JString*		cmd
	)
	const
{
	JString commitEditor, diffCmd, reloadChangedCmd;
	const bool exists = GetIntegration(type, &commitEditor, &diffCmd, &reloadChangedCmd);
	assert( exists );

	if (*type == kCustom && cmdType == kCommitEditor)
	{
		*cmd = commitEditor;
	}
	else if (*type == kCustom && cmdType == kDiffCmd)
	{
		*cmd = diffCmd;
	}
	else if (*type == kCustom && cmdType == kReloadChangedCmd)
	{
		*cmd = reloadChangedCmd;
	}
	else if (*type != kCustom)
	{
		*cmd = kIntegrationCmd[ *type ][ cmdType ];
	}

	return !cmd->IsEmpty();
}

/******************************************************************************
 GetIntegration (private)

 ******************************************************************************/

bool
PrefsManager::GetIntegration
	(
	Integration*	type,
	JString*		commitEditor,
	JString*		diffCmd,
	JString*		reloadChangedCmd
	)
	const
{
	std::string data;
	if (GetData(kIntegrationID, &data))
	{
		std::istringstream dataStream(data);
		dataStream >> *type;
		dataStream >> *commitEditor;
		dataStream >> *diffCmd;
		dataStream >> *reloadChangedCmd;
		return true;
	}
	else
	{
		return false;
	}
}

/******************************************************************************
 SetIntegration (private)

 ******************************************************************************/

void
PrefsManager::SetIntegration
	(
	const Integration	type,
	const JString&		commitEditor,
	const JString&		diffCmd,
	const JString&		reloadChangedCmd
	)
{
	std::ostringstream data;
	data << type;
	data << ' ' << commitEditor;
	data << ' ' << diffCmd;
	data << ' ' << reloadChangedCmd;

	SetData(kIntegrationID, data);

	Integration t;
	JString e;
	GetCommand(kCommitEditor, &t, &e);
	setenv("_EDITOR", e.GetBytes(), 1);
}

/******************************************************************************
 Stream operators

 ******************************************************************************/

std::istream&
operator>>
	(
	std::istream&						input,
	PrefsManager::Integration&	type
	)
{
	long temp;
	input >> temp;
	type = (PrefsManager::Integration) temp;
	assert( PrefsManager::kCodeCrusader <= type && type <= PrefsManager::kCustom );
	return input;
}

std::ostream&
operator<<
	(
	std::ostream&							output,
	const PrefsManager::Integration	type
	)
{
	output << (long) type;
	return output;
}
