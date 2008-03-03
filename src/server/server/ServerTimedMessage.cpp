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

#include <common/OptionsScorched.h>
#include <common/Logger.h>
#include <common/Defines.h>
#include <server/ServerTimedMessage.h>
#include <server/ServerChannelManager.h>
#include <server/ServerCommon.h>
#include <server/ScorchedServer.h>
#include <server/ServerState.h>
#include <engine/GameState.h>
#include <XML/XMLFile.h>
#include <time.h>

ServerTimedMessage::ServerTimedMessage() : 
	lastReadTime_(0), lastCheckTime_(0)
{
}

ServerTimedMessage::~ServerTimedMessage()
{
}

void ServerTimedMessage::simulate()
{
#ifndef S3D_SERVER
	return;
#endif

	if (ScorchedServer::instance()->getGameState().getState() ==
		ServerState::ServerStateTooFewPlayers) return;

	time_t currentTime = time(0);
	if (currentTime > lastCheckTime_ + 5)
	{
		lastCheckTime_ = currentTime;

		load();
		checkEntries(currentTime);
	}
}

void ServerTimedMessage::checkEntries(time_t currentTime)
{
	std::list<TimedMessageEntry>::iterator itor;
	for (itor = entries_.begin();
		itor != entries_.end();
		itor++)
	{
		TimedMessageEntry &entry = (*itor);
		if (entry.lastTime + (time_t) entry.timeInterval < currentTime)
		{
			entry.lastTime = currentTime;
			
			std::string message = entry.messages.front();
 			/* Note message.c_str() is a user supplied printf format string
			   with possible a conversion specifier to display the ctime.
			   Since these messages are loaded from a local file this is
			   reasonable safe, although it would be better to just
			   always prefix times-messages with the ctime and to not
			   interpreted user supplied data this way. */

			ChannelText textMessage("announce", 
				S3D::formatStringBuffer(message.c_str(), ctime(&currentTime)));
			ServerChannelManager::instance()->sendText(textMessage, false);
			entry.messages.pop_front();
			entry.messages.push_back(message);
		}
	}


}

bool ServerTimedMessage::load()
{
	std::string filename = 
		S3D::getSettingsFile(S3D::formatStringBuffer("messages-%i.xml", 
			ScorchedServer::instance()->getOptionsGame().getPortNo()));
	if (!S3D::fileExists(filename)) return true;

	time_t fileTime = S3D::fileModTime(filename);
	if (fileTime == lastReadTime_) return true;

	XMLFile file;
	if (!file.readFile(filename))
	{
		Logger::log(S3D::formatStringBuffer("Failed to parse user file \"%s\"\n%s", 
			filename.c_str(), file.getParserError()));
		return false;
	}

	Logger::log(S3D::formatStringBuffer("Refreshing message list %s", filename.c_str()));
	lastReadTime_ = fileTime;
	entries_.clear();
	if (!file.getRootNode()) return true; // Empty File

	std::list<XMLNode *>::iterator childrenItor;
	std::list<XMLNode *> &children = file.getRootNode()->getChildren();
	for (childrenItor = children.begin();
		 childrenItor != children.end();
		childrenItor++)
	{
		XMLNode *currentNode = (*childrenItor);

		std::string text;
		TimedMessageEntry entry;
		if (!currentNode->getNamedChild("repeattime", entry.timeInterval)) return false;
		while (currentNode->getNamedChild("text", text, false))
		{
			entry.messages.push_back(text);
		}
		entries_.push_back(entry);
	}
	return true;
}
