//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_SERVER_H_
#define _RME_LIVE_SERVER_H_

#include "live_socket.h"
#include "net_connection.h"
#include "action.h"

class LivePeer;
class LiveLogTab;
class QTreeNode;

class LiveServer : public LiveSocket {
public:
	LiveServer(Editor& editor);
	~LiveServer();

	//
	bool bind();
	void close();

	void acceptClient();
	void removeClient(uint32_t id);

	//
	void receiveHeader() override { }
	void receive(uint32_t packetSize) override { }
	void send(NetworkMessage& message) override { }
	void sendChat(const wxString& chatMessage) override;

	//
	void updateCursor(const Position& position) override;
	void updateClientList() const;

	//
	LiveLogTab* createLogWindow(wxWindow* parent);

	//
	uint16_t getPort() const;
	bool setPort(int32_t newPort);

	Editor* getEditor() const {
		return editor;
	}

	uint32_t getFreeClientId();
	std::string getHostName() const;

	//
	void broadcastNodes(DirtyList& dirtyList);
	void broadcastChat(const wxString& speaker, const wxString& chatMessage);
	void broadcastCursor(const LiveCursor& cursor);
	void broadcastColorChange(uint32_t clientId, const wxColor& color);

	void startOperation(const wxString& operationMessage);
	void updateOperation(int32_t percent);

	// Override socket type check
	bool IsServer() const override { return true; }

	void setUsedColor(const wxColor& color);
	wxColor getUsedColor() const { return usedColor; }
	
	const std::unordered_map<uint32_t, LivePeer*>& getClients() const { return clients; }

	// Helper method for writing cursor data to messages
	void writeCursorToMessage(NetworkMessage& message, const LiveCursor& cursor) {
		writeCursor(message, cursor);
	}

protected:
	std::unordered_map<uint32_t, LivePeer*> clients;

	std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
	std::shared_ptr<boost::asio::ip::tcp::socket> socket;

	Editor* editor;

	uint32_t clientIds;
	uint16_t port;

	bool stopped;
	bool drawingReady;  // Flag indicating server is ready for drawing operations

	wxColor usedColor;
};

#endif
