////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2003
//
//    This file is part of Scorched3D.
//
//    Scorched3D is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Scorched3D is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Scorched3D; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

#include <tank/TankModelStore.h>
#include <dialogs/PlayerDialog.h>
#include <client/ClientState.h>
#include <client/ScorchedClient.h>
#include <tankai/TankAIStore.h>
#include <tankai/TankAIStrings.h>
#include <tank/TankContainer.h>
#include <tank/TankColorGenerator.h>
#include <tank/TankState.h>
#include <tank/TankAvatar.h>
#include <client/ClientParams.h>
#include <graph/OptionsDisplay.h>
#include <common/OptionsTransient.h>
#include <common/OptionsScorched.h>
#include <common/Logger.h>
#include <common/Defines.h>
#include <GLW/GLWWindowManager.h>
#include <GLW/GLWTextButton.h>
#include <image/ImageFactory.h>
#include <coms/ComsAddPlayerMessage.h>
#include <coms/ComsMessageSender.h>
#include <stdio.h>

PlayerDialog *PlayerDialog::instance_ = 0;

PlayerDialog *PlayerDialog::instance()
{
	if (!instance_)
	{
		instance_ = new PlayerDialog;
	}
	return instance_;
}

PlayerDialog::PlayerDialog() : 
	GLWWindow("Team", 10.0f, 10.0f, 465.0f, 340.0f, eSmallTitle,
		"Allows the player to make changes to their\n"
		"name, their tank and to change teams."),
	allocatedTeam_(0), cancelId_(0), viewer_(0)
{
	needCentered_ = true;

	// Add buttons
	okId_ = addWidget(new GLWTextButton("Ok", 395, 10, 55, this, 
		GLWButton::ButtonFlagOk | GLWButton::ButtonFlagCenterX))->getId();
	if (ClientParams::instance()->getConnectedToServer())
	{
		cancelId_ = addWidget(new GLWTextButton("Cancel", 300, 10, 85, this, 
			GLWButton::ButtonFlagCancel | GLWButton::ButtonFlagCenterX))->getId();
	}

	GLWPanel *infoPanel = new GLWPanel(10.0f, 260.0f, 445.0f, 70.0f,
		false, true, true);
	addWidget(infoPanel);

	// Create a texture to display the player color selector
	ImageHandle map = ImageFactory::loadImageHandle(S3D::getDataFile("data/windows/white.bmp"));
	colorTexture_.create(map);

	// Create players avatar choice
	avatarTip1_.setText(ToolTip::ToolTipHelp, "Avatar", 
		"The current player's avatar.\n"
		"Click to change.\n");
	avatarTip2_.setText(ToolTip::ToolTipHelp, "Avatar", 
		"The current player's avatar.\n"
		"CANNOT be changed while playing,\n"
		"you must quit to change.");
	imageList_ = new GLWImageList(10.0f, 20.0f);
	imageList_->addDirectory(S3D::getSettingsFile("avatars"));
	imageList_->addDirectory(S3D::getDataFile("data/avatars"));
	imageList_->setCurrentShortPath("player.png");
	imageList_->setToolTip(&avatarTip1_);
	imageList_->setName("Avatar");
	infoPanel->addWidget(imageList_);

	// Create player name choice
	ToolTip *nameTip = new ToolTip(ToolTip::ToolTipHelp, "Player Name",
		"The name of this player.\n"
		"Use the backspace or delete key to remove this name.\n"
		"Type in a new player name via the keyboad to change.");
	GLWLabel *nameLabel = (GLWLabel *) 
		infoPanel->addWidget(new GLWLabel(50, 40, "Name:"));
	nameLabel->setToolTip(nameTip);
	playerName_ = (GLWTextBox *) 
		infoPanel->addWidget(new GLWTextBox(120, 40, 320, "Player"));
	playerName_->setMaxTextLen(22);
	playerName_->setToolTip(nameTip);
	playerName_->setName("Name");
	
	// Create team choice
	ToolTip *teamTip = new ToolTip(ToolTip::ToolTipHelp, "Team Selection",
		"Change the team this player will join.\n"
		"This is only available when playing team games.");
	teamLabel_ = (GLWLabel *) 
		infoPanel->addWidget(new GLWLabel(250, 5, "Team:"));
	teamLabel_->setToolTip(teamTip);
	teamDropDown_ = (GLWDropDownText *) 
		infoPanel->addWidget(new GLWDropDownText(320, 5, 120));
	teamDropDown_->setHandler(this);
	teamDropDown_->setToolTip(teamTip);
	teamDropDown_->setName("Team");

	// Create color choice
	ToolTip *colorTip = new ToolTip(ToolTip::ToolTipHelp, "Color Selection",
		"Change the color this player displayed as.\n"
		"This is only available when playing non-team games.");
	colorLabel_ = (GLWLabel *) 
		infoPanel->addWidget(new GLWLabel(250, 5, "Color:"));
	colorLabel_->setToolTip(colorTip);
	colorDropDown_ = (GLWDropDownColor *) 
		infoPanel->addWidget(new GLWDropDownColor(320, 5, 120));
	colorDropDown_->setHandler(this);
	colorDropDown_->setToolTip(colorTip);
	colorDropDown_->setName("Color");

	// Create computer type choice
	ToolTip *typeTip = new ToolTip(ToolTip::ToolTipHelp, "Player Type",
		"Change between human and computer controlled\n"
		"players.  This is only available when playing\n"
		"single player games.");
	GLWLabel *typeLabel = (GLWLabel *) 
		infoPanel->addWidget(new GLWLabel(50, 5, "Type:"));
	typeLabel->setToolTip(typeTip);
	typeDropDown_ = (GLWDropDownText *) 
		infoPanel->addWidget(new GLWDropDownText(120, 5, 120));
	typeDropDown_->setHandler(this);
	typeDropDown_->setToolTip(typeTip);
	typeDropDown_->setName("Type");

	humanToolTip_.setText(ToolTip::ToolTipHelp, "Human", "A human controlled player.");
}

PlayerDialog::~PlayerDialog()
{
}

void PlayerDialog::draw()
{
	if (ScorchedClient::instance()->getOptionsGame().getTeams() != 1)
	{
		// Auto select the team with the least players
		unsigned int newTeam = 
			ScorchedClient::instance()->getOptionsTransient().getLeastUsedTeam(
			ScorchedClient::instance()->getTankContainer());
		if (newTeam != allocatedTeam_)
		{
			teamDropDown_->setCurrentPosition(newTeam - 1);
			allocatedTeam_ = newTeam;
			viewer_->setTeam(newTeam);
		}
	}
	GLWWindow::draw();
}

void PlayerDialog::select(unsigned int id, const int pos, 
	GLWSelectorEntry value)
{
	if (id == typeDropDown_->getId())
	{
		if (0 == strcmp("Human", value.getText()))
		{
			imageList_->setCurrentShortPath("player.png");
		}
		else
		{
			imageList_->setCurrentShortPath("computer.png");
		}
	}
	else if (id == teamDropDown_->getId())
	{
		viewer_->setTeam(getCurrentTeam());
	}
}

void PlayerDialog::keyDown(char *buffer, unsigned int keyState, 
		KeyboardHistory::HistoryElement *history, int hisCount, 
		bool &skipRest)
{
	GLWWindow::keyDown(buffer, keyState, history, hisCount, skipRest);
	if (ClientParams::instance()->getConnectedToServer()) skipRest = true;
}

void PlayerDialog::display()
{	
	GLWWindow::display();

	if (!viewer_)
	{
		GLWPanel *infoPanel = new GLWPanel(10.0f, 40.0f, 445.0f, 210.0f,
			false, true, true);
		viewer_ = new GLWTankViewer(5.0f, 5.0f, 4, 3);
		viewer_->setName("Tank");
		infoPanel->addWidget(viewer_);
		addWidget(infoPanel);
	}

	static TankAIStore tankAIStore;
	static bool init = false;
	if (!init)
	{
		init = true;
		tankAIStore.loadAIs(true);
	}

	// Add teams
	teamDropDown_->clear();
	if (ScorchedClient::instance()->getOptionsGame().getTeams() == 1)
	{
		teamDropDown_->addText("None");
		teamDropDown_->setVisible(false);
		teamLabel_->setVisible(false);
	}
	else
	{
		for (int i=1; i<=ScorchedClient::instance()->getOptionsGame().getTeams(); i++)
		{
			const char *name = TankColorGenerator::getTeamName(i);
			GLWSelectorEntry entry(name, 0, false, &colorTexture_, 0);
			entry.getColor() = TankColorGenerator::getTeamColor(i);
			teamDropDown_->addEntry(entry);
		}	
		colorDropDown_->setVisible(false);
		colorLabel_->setVisible(false);
	}

	// Add player types
	typeDropDown_->clear();
	typeDropDown_->addEntry(GLWSelectorEntry("Human", &humanToolTip_));
	if (!ClientParams::instance()->getConnectedToServer() &&
		!ScorchedClient::instance()->getOptionsGame().getTutorial()[0])
	{
		std::list<TankAI *>::iterator aiitor;
		for (aiitor = tankAIStore.getAis().begin();
			aiitor != tankAIStore.getAis().end();
			aiitor++)
		{
			TankAI *ai = (*aiitor);
			if (ai->availableForPlayers())
			{
				typeDropDown_->addEntry(
					GLWSelectorEntry(ai->getName(),
						(*aiitor)->getToolTip()));
			}
		}
	}
	currentPlayerId_ = 0;
	nextPlayer();
}

void PlayerDialog::nextPlayer()
{
	allocatedTeam_ = 0;
	currentPlayerId_ = getNextPlayer(currentPlayerId_);
	if (currentPlayerId_ == 0)
	{
		GLWWindowManager::instance()->hideWindow(getId());
		return;
	}

	Tank *tank = 
		ScorchedClient::instance()->getTankContainer().getTankById(currentPlayerId_);
	if (ClientParams::instance()->getConnectedToServer())
	{
		// If we are connected online then use the online name
		playerName_->setText(
			OptionsDisplay::instance()->getOnlineUserName());
		viewer_->selectModelByName(
			OptionsDisplay::instance()->getOnlineTankModel());
		if (!imageList_->setCurrentShortPath(
			OptionsDisplay::instance()->getOnlineUserIcon()))
		{
			imageList_->setCurrentShortPath("player.png");
		}
	}
	else
	{
		// Else use the default names
		if (tank) playerName_->setText(tank->getName());
	}
		
	if (tank->getState().getSpectator())
	{
		imageList_->setEnabled(true);
		imageList_->setToolTip(&avatarTip1_);
	}
	else
	{
		imageList_->setEnabled(false);
		imageList_->setToolTip(&avatarTip2_);
	}

	// Add colors
	colorDropDown_->clear();
	if (ScorchedClient::instance()->getOptionsGame().getTeams() == 1)
	{
		std::map<unsigned int, Tank *> tanks =
			ScorchedClient::instance()->getTankContainer().getPlayingTanks();
		std::vector<Vector *> availableColors =
			TankColorGenerator::instance()->getAvailableColors(tanks, tank);
		std::vector<Vector *>::iterator itor;
		for (itor = availableColors.begin();
			itor != availableColors.end();
			itor++)
		{
			Vector &color = *(*itor);
			colorDropDown_->addColor(color);
		}

		if (ClientParams::instance()->getConnectedToServer())
		{
			Vector onlineColor = 
				OptionsDisplay::instance()->getOnlineColor();
			colorDropDown_->setCurrentColor(onlineColor);
			if (colorDropDown_->getCurrentColor() != onlineColor)
			{
				colorDropDown_->setCurrentColor(tank->getColor());	
			}
		}
		else
		{
			colorDropDown_->setCurrentColor(tank->getColor());			
		}
	}
}

unsigned int PlayerDialog::getNextPlayer(unsigned int current)
{
	std::map<unsigned int, Tank *> &tanks = 
		ScorchedClient::instance()->getTankContainer().getPlayingTanks();
	std::map<unsigned int, Tank *>::iterator itor;
	for (itor = tanks.begin();
		itor != tanks.end();
		itor++)
	{
		Tank *tank = (*itor).second;
		if ((tank->getDestinationId() == 
			ScorchedClient::instance()->getTankContainer().getCurrentDestinationId()) &&
			(tank->getPlayerId() != TargetID::SPEC_TANK_ID) &&
			(tank->getState().getState() != TankState::sNormal))
		{
			if (current == 0)
			{
				return tank->getPlayerId();
			}
			else if (tank->getPlayerId() == current) current = 0;
		}
	}
	return 0;
}

int PlayerDialog::getCurrentTeam()
{
	return ((ScorchedClient::instance()->getOptionsGame().getTeams() > 1)?
		teamDropDown_->getCurrentPosition() + 1:0);
}

void PlayerDialog::buttonDown(unsigned int id)
{
	if (id == okId_)
	{
		if (!playerName_->getText().empty())
		{
			// If we are connected online save this players name
			if (ClientParams::instance()->getConnectedToServer())
			{
				OptionsDisplay::instance()->getOnlineUserNameEntry().setValue(
					playerName_->getText().c_str());
				OptionsDisplay::instance()->getOnlineTankModelEntry().setValue(
					viewer_->getModelName());
				OptionsDisplay::instance()->getOnlineUserIconEntry().setValue(
					imageList_->getCurrentShortPath());
				if (ScorchedClient::instance()->getOptionsGame().getTeams() == 1)
				{
					OptionsDisplay::instance()->getOnlineColorEntry().setValue(
						colorDropDown_->getCurrentColor());
				}
			}

			// Get the model type (turns a "Random" choice into a proper name)
			TankModel *model = 
				ScorchedClient::instance()->getTankModels().
					getModelByName(viewer_->getModelName(), 
					getCurrentTeam(), false);

			// Get the player type
			const char *playerType = typeDropDown_->getCurrentText();

			// Add this player
			ComsAddPlayerMessage message(currentPlayerId_,
				playerName_->getText().c_str(),
				colorDropDown_->getCurrentColor(),
				model->getName(),
				model->getTypeName(),
				ScorchedClient::instance()->getTankContainer().getCurrentDestinationId(),
				getCurrentTeam(),
				playerType);
			// Add avatar (if not one)
			Tank *tank = ScorchedClient::instance()->getTankContainer().
				getTankById(currentPlayerId_);
			if (tank && !tank->getAvatar().getName()[0])
			{
				if (tank->getAvatar().loadFromFile(imageList_->getCurrentLongPath()))
				{
					if (tank->getAvatar().getFile().getBufferUsed() <=
						(unsigned) ScorchedClient::instance()->getOptionsGame().getMaxAvatarSize())
					{
						message.setPlayerIconName(imageList_->getCurrentShortPath());
						message.getPlayerIcon().addDataToBuffer(
							tank->getAvatar().getFile().getBuffer(),
							tank->getAvatar().getFile().getBufferUsed());
					}
					else
					{
						Logger::log( "Warning: Avatar too large to send to server");
					}
				}
			}
			ComsMessageSender::sendToServer(message);

			nextPlayer();
		}
	}
	else if (id == cancelId_)
	{
		GLWWindowManager::instance()->hideWindow(getId());
	}
}
