// This file was automatically generated by jx_menu_editor.
// Do not edit it directly!
// Any changes you make will be silently overwritten.

#ifndef _H_MainDirector_File
#define _H_MainDirector_File

static const JUtf8Byte* kFileMenuStr =
"* %i OpenDirectory::SVNMainDirector"
"|* %i BrowseRepo::SVNMainDirector %l"
"|* %i CheckOutRepo::SVNMainDirector"
"|* %i CheckOutCurrentRepo::SVNMainDirector"
"|* %i CheckOutSelection::SVNMainDirector %l"
"|* %i OpenFiles::SVNMainDirector"
"|* %i ShowFiles::SVNMainDirector %l"
"|* %i CloseWindow::JX %l"
"|* %i Quit::JX"
;

#include "MainDirector-File-enum.h"

#ifndef _H_jx_af_image_jx_jx_file_open
#define _H_jx_af_image_jx_jx_file_open
#include <jx-af/image/jx/jx_file_open.xpm>
#endif

static void ConfigureFileMenu(JXTextMenu* menu, const int offset = 0) {
	if (offset == 0 && JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle) {
		menu->SetShortcuts("#f");
	}
	menu->SetItemImage(kOpenDirectoryCmd + offset, jx_file_open);
};

#endif
