////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2009
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

#if !defined(__INCLUDE_ServerSyncCheckh_INCLUDE__)
#define __INCLUDE_ServerSyncCheckh_INCLUDE__

#include <time.h>
#include <coms/ComsSyncCheckMessage.h>
#include <coms/ComsMessageHandler.h>
#include <map>
#include <set>

class ServerSyncCheck : public ComsMessageHandlerI 
{
public:
	static ServerSyncCheck *instance();

	void enterState();
	void simulate();

	void addServerSyncCheck(ComsSyncCheckMessage *message);
	void sendSyncCheck();
	void sentSyncCheck(unsigned int syncId);

	// Inherited from ComsMessageHandlerI
	virtual bool processMessage(
		NetMessage &message,
		const char *messageType,
		NetBufferReader &reader);

protected:
	static ServerSyncCheck *instance_;
	time_t lastTime_;

	struct SyncContext
	{
		SyncContext();
		~SyncContext();

		ComsSyncCheckMessage *serverMessage;
		std::map<unsigned int, ComsSyncCheckMessage*> clientMessages;
		std::set<unsigned int> clientDestinations;
	};
	std::map<unsigned int, SyncContext*> contexts_;

	bool checkContext(SyncContext *context);
	bool compareSyncChecks(ComsSyncCheckMessage *server, 
		unsigned int destinationId, ComsSyncCheckMessage *client);

private:
	ServerSyncCheck();
	virtual ~ServerSyncCheck();
};

#endif
