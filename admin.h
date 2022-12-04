//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifdef __REMOTE_CONTROL__
#ifndef __OTSERV_ADMIN_H__
#define __OTSERV_ADMIN_H__

// -> server
// command(1 byte) | size(2 bytes) | parameters(size bytes)
// commands:
//	login
//		password(string)
//  encryption
//		encryption type(1 byte)
//			RSA1024+XTEA
//				:128 bytes encrypted using 1024 bytes public key
//				16 bytes XTEA key
//  key-exchange
//		public_key_type(1 byte)
//			RSA1024+XTEA
//  command
//		command + paramters(string)
//	no_operation/ping
//		nothing
//
// <- server
// ret-code(1 byte)| size(2 bytes) | parameters(size bytes)
// ret-codes:
//	hello
//		server_version(4 bytes)
//		server_string(string)
//		security_policy(2 bytes flags)
//			required_login
//			required_encryption
//		accepted_encryptions(4 bytes flags)
//			RSA1024+XTEA
//	key-exchange-ok
//		public_key_type(1 byte)
//			RSA1024+XTEA
//				:128 bytes public key modulus
//	key-exchange-failed
//		reason(string)
//  login-ok
//		nothing
//  login-failed
//		reason(string)
//  command-ok
//		command result(string)
//  command-failed
//		reason(string)
//  encryption-ok
//		nothing
//  encryption-failed
//		reason(string)
//	no_operation-ok
//		nothing
//	message
//		message(string)
//  error
//		message(string)
//

#include "otsystem.h"
#include "player.h"
#include <string>
#include <map>

class NetworkMessage;
class RSA;

enum eLogType
{
	LOGTYPE_EVENT,
	LOGTYPE_WARNING,
	LOGTYPE_ERROR,
};

enum
{
	AP_MSG_LOGIN = 1,
	AP_MSG_ENCRYPTION = 2,
	AP_MSG_KEY_EXCHANGE = 3,
	AP_MSG_COMMAND = 4,
	AP_MSG_PING = 5,

	AP_MSG_HELLO = 1,
	AP_MSG_KEY_EXCHANGE_OK = 2,
	AP_MSG_KEY_EXCHANGE_FAILED = 3,
	AP_MSG_LOGIN_OK = 4,
	AP_MSG_LOGIN_FAILED = 5,
	AP_MSG_COMMAND_OK = 6,
	AP_MSG_COMMAND_FAILED = 7,
	AP_MSG_ENCRYPTION_OK = 8,
	AP_MSG_ENCRYPTION_FAILED = 9,
	AP_MSG_PING_OK = 10,
	AP_MSG_MESSAGE = 11,
	AP_MSG_ERROR = 12,
};

enum
{
	CMD_BROADCAST = 1,
	CMD_CLOSE_SERVER = 2,
	CMD_PAY_HOUSES = 3,
	CMD_OPEN_SERVER = 4,
	CMD_SHUTDOWN_SERVER = 5,
	//CMD_RELOAD_SCRIPTS = 6,
	//CMD_PLAYER_INFO = 7,
	//CMD_GETONLINE = 8,
	CMD_KICK = 9,
	//CMD_BAN_MANAGER = 10,
	//CMD_SERVER_INFO = 11,
	//CMD_GETHOUSE = 12,
	CMD_SETOWNER = 13
};


enum
{
	REQUIRE_LOGIN = 1,
	REQUIRE_ENCRYPTION = 2,
};

enum
{
	ENCRYPTION_RSA1024XTEA = 1,
};

class Logger
{
	public:
		virtual ~Logger();
		static Logger* getInstance()
		{
			static Logger instance;
			return &instance;
		}

		void logMessage(const char* channel, eLogType type, int32_t level, std::string message, const char* func);

	private:
		FILE* m_file;
		Logger();
};

#define LOG_MESSAGE(channel, type, level, message) \
	Logger::getInstance()->logMessage(channel, type, level, message, __OTSERV_PRETTY_FUNCTION__);

class Admin
{
	public:
		Admin();
		virtual ~Admin();

		bool loadXMLConfig();

		bool addConnection();
		void removeConnection();

		uint16_t getProtocolPolicy();
		uint32_t getProtocolOptions();

		bool isEnabled() const {return m_enabled;}
		bool onlyLocalHost() const {return m_onlyLocalHost;}

		bool requireLogin() const {return m_requireLogin;}
		bool requireEncryption() const {return m_requireEncryption;}

		bool passwordMatch(std::string& password);
		RSA* getRSAKey(uint8_t type);

		bool allowIP(uint32_t ip);

	protected:
		int32_t m_maxConnections, m_currrentConnections;
		bool m_enabled, m_onlyLocalHost, m_requireLogin, m_requireEncryption;

		std::string m_password;
		RSA* m_key_RSA1024XTEA;
};


class ProtocolAdmin : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolAdminCount;
#endif
		ProtocolAdmin(Connection* connection);
		virtual ~ProtocolAdmin();

		virtual int32_t getProtocolId() {return 0xFE;}

		virtual void parsePacket(NetworkMessage& msg);

		virtual void onRecvFirstMessage(NetworkMessage& msg);

	protected:
		virtual void deleteProtocolTask();

		void adminCommandPayHouses();
		void adminCommandKickPlayer(const std::string& name);
		void adminCommandSetOwner(const std::string& param);

		enum ConnectionState_t
		{
			NO_CONNECTED,
			ENCRYPTION_NO_SET,
			ENCRYPTION_OK,
			NO_LOGGED_IN,
			LOGGED_IN,
		};

	private:
		int32_t m_loginTries;
		ConnectionState_t m_state;
		uint32_t m_lastCommand, m_startTime;
};

#endif
#endif
