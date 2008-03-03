////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2004
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

#include <server/ServerAuthHandlerMinKills.h>
#include <server/ScorchedServer.h>
#include <common/StatsLogger.h>
#include <common/Logger.h>
#include <common/OptionsScorched.h>
#include <common/Defines.h>

ServerAuthHandlerMinKills::ServerAuthHandlerMinKills() : 
	minKills_(-1), maxKills_(-1)
{
}

ServerAuthHandlerMinKills::~ServerAuthHandlerMinKills()
{
}

void createAuthentication(ComsConnectAuthMessage &authMessage)
{
}

void ServerAuthHandlerMinKills::createAuthentication(ComsConnectAuthMessage &authMessage)
{
}

bool ServerAuthHandlerMinKills::authenticateUser(ComsConnectAuthMessage &authMessage, 
	std::string &message)
{
	setup();

	int killCount = StatsLogger::instance()->getKillCount(authMessage.getUniqueId());
	if (minKills_ > 0 && killCount < minKills_)
	{
		message = S3D::formatStringBuffer(
			"You need to have at least %i kills to play on this server.\n"
			"You only have %i kills.\n",
			minKills_, killCount);
		return false;
	}
	if (maxKills_ > 0 && killCount > maxKills_)
	{
		message = S3D::formatStringBuffer(
			"You have too many kills to play on this server.\n"
			"You have %i kills and the server allows %i kills.\n",
			killCount, maxKills_);
		return false;
	}	
	
	return true;
}

bool ServerAuthHandlerMinKills::authenticateUserName(const char *uniqueId, 
	const char *playername)
{
	return true;
}

void ServerAuthHandlerMinKills::banUser(const char *uniqueId)
{
}

void ServerAuthHandlerMinKills::setup()
{
	if (minKills_ != -1) return;

	XMLFile file;
	std::string fileName = S3D::getSettingsFile(S3D::formatStringBuffer("minkills-%i.xml",
		ScorchedServer::instance()->getOptionsGame().getPortNo()));

	if (!file.readFile(fileName) ||
		!file.getRootNode())
	{
		Logger::log(S3D::formatStringBuffer("Failed to parse %s settings file. Error: %s", 
			fileName.c_str(),
			file.getParserError()));
		return;
	}

	if (!file.getRootNode()->getNamedChild("minkills", minKills_) ||
		!file.getRootNode()->getNamedChild("maxkills", maxKills_)) 
	{
		Logger::log(S3D::formatStringBuffer("Failed to parse %s settings file.", fileName.c_str()));
		return;
	}
	Logger::log(S3D::formatStringBuffer("ServerAuthHandlerMinKills : minkills=%i, maxkills=%i",
		minKills_, maxKills_));
}
