/******************************************************************************
 AboutDialog.cpp

	BASE CLASS = JXModalDialogDirector

	Copyright (C) 2008 by John Lindal.

 ******************************************************************************/

#include "AboutDialog.h"
#include "globals.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXImageWidget.h>
#include <jx-af/jx/JXImageCache.h>
#include <jx-af/jx/JXHelpManager.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

	If prevVersStr is not empty, we change the Help button to "Changes".

 ******************************************************************************/

AboutDialog::AboutDialog
	(
	const JString& prevVersStr
	)
	:
	JXModalDialogDirector()
{
	itsIsUpgradeFlag = false;

	BuildWindow(prevVersStr);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

AboutDialog::~AboutDialog()
{
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
AboutDialog::BuildWindow
	(
	const JString& prevVersStr
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 430,200, JGetString("WindowTitle::AboutDialog::JXLayout"));

	auto* imageWidget =
		jnew JXImageWidget(window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,20, 40,40);
#ifndef _H_svn_about_icon
#define _H_svn_about_icon
#include "svn_about_icon.xpm"
#endif
	imageWidget->SetImage(GetDisplay()->GetImageCache()->GetImage(svn_about_icon), false);

	auto* textWidget =
		jnew JXStaticText(JString::empty, true, false, false, nullptr, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 90,20, 330,130);
	textWidget->SetBorderWidth(0);

	auto* npsIcon =
		jnew JXImageWidget(window,
					JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,75, 65,65);
#ifndef _H_jx_af_image_jx_new_planet_software
#define _H_jx_af_image_jx_new_planet_software
#include <jx-af/image/jx/new_planet_software.xpm>
#endif
	npsIcon->SetImage(GetDisplay()->GetImageCache()->GetImage(new_planet_software), false);

	itsCreditsButton =
		jnew JXTextButton(JGetString("itsCreditsButton::AboutDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 50,170, 60,20);

	itsHelpButton =
		jnew JXTextButton(JGetString("itsHelpButton::AboutDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 185,170, 60,20);
	itsHelpButton->SetShortcuts(JGetString("itsHelpButton::shortcuts::AboutDialog::JXLayout"));

	auto* okButton =
		jnew JXTextButton(JGetString("okButton::AboutDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 320,170, 60,20);
	okButton->SetShortcuts(JGetString("okButton::shortcuts::AboutDialog::JXLayout"));

// end JXLayout

	SetButtons(okButton, nullptr);

	ListenTo(itsHelpButton);
	ListenTo(itsCreditsButton);

	JString text = GetVersionStr();
	if (!prevVersStr.IsEmpty())
	{
		const JUtf8Byte* map[] =
		{
			"vers", prevVersStr.GetBytes()
		};
		text += JGetString("UpgradeNotice::AboutDialog");
		JGetStringManager()->Replace(&text, map, sizeof(map));
		itsHelpButton->SetLabel(JGetString("ChangeButtonLabel::AboutDialog"));
		itsIsUpgradeFlag = true;
	}
	textWidget->GetText()->SetText(text);

	// adjust window to fit text

	const JSize bdh = textWidget->GetBoundsHeight();
	const JSize aph = textWidget->GetApertureHeight();
	if (bdh > aph)
	{
		window->AdjustSize(0, bdh - aph);	// safe to calculate once bdh > aph
	}
}

/******************************************************************************
 Receive (virtual protected)

	Handle itsHelpButton.

 ******************************************************************************/

void
AboutDialog::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsHelpButton && message.Is(JXButton::kPushed))
	{
		if (itsIsUpgradeFlag)
		{
			(JXGetHelpManager())->ShowChangeLog();
		}
		else
		{
			(JXGetHelpManager())->ShowTOC();
		}
		EndDialog(true);
	}

	else if (sender == itsCreditsButton && message.Is(JXButton::kPushed))
	{
		(JXGetHelpManager())->ShowCredits();
		EndDialog(true);
	}

	else
	{
		JXModalDialogDirector::Receive(sender, message);
	}
}
