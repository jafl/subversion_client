/******************************************************************************
 PrefsManager.h

	Copyright (C) 2008 by John Lindal.

 *****************************************************************************/

#ifndef _H_PrefsManager
#define _H_PrefsManager

#include <jx-af/jx/JXPrefsManager.h>

class JPoint;
class JString;
class JXWindow;

// Preferences -- do not change ID's once they are assigned

enum
{
	kProgramVersionID = 1,
	kgCSFSetupID,
	kMainToolBarID,
	kDocMgrStateID,
	kMainDirectorWindSizeID,
	kIntegrationID,
	kVersionCheckID,
	kExpireTimeStampID,
	kGetRepoDialogID
};

class PrefsManager : public JXPrefsManager
{
public:

	// do not change ID's once they are assigned, because they are stored in files

	enum Integration
	{
		kCodeCrusader,
		kCmdLine,
		kCustom
	};

	enum Command
	{
		kCommitEditor,
		kDiffCmd,
		kReloadChangedCmd
	};

public:

	PrefsManager(bool* isNew);

	~PrefsManager() override;

	bool	GetExpirationTimeStamp(time_t* t) const;
	void	SetExpirationTimeStamp(const time_t t);

	JString GetPrevVersionStr() const;
	void	EditPrefs();

	bool	RestoreProgramState();
	void	SaveProgramState();
	void	ForgetProgramState();

	bool	GetWindowSize(const JPrefID& id, JPoint* desktopLoc,
						  JCoordinate* width, JCoordinate* height) const;
	void	SaveWindowSize(const JPrefID& id, JXWindow* window);

	bool	GetCommand(const Command cmdType, Integration* type, JString* cmd) const;

protected:

	void	UpgradeData(const bool isNew, const JFileVersion currentVersion) override;
	void	SaveAllBeforeDestruct() override;

private:

	bool	GetIntegration(Integration* type, JString* commitEditor,
						   JString* diffCmd, JString* reloadChangedCmd) const;
	void	SetIntegration(const Integration type, const JString& commitEditor,
						   const JString& diffCmd, const JString& reloadChangedCmd);
};

std::istream&	operator>>(std::istream& input, PrefsManager::Integration& type);
std::ostream&	operator<<(std::ostream& output, const PrefsManager::Integration type);

#endif
