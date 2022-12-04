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
#include "otpch.h"

#include "protocollogin.h"
#include "resources.h"

#include "outputmessage.h"
#include "connection.h"
#include "rsa.h"

#include "configmanager.h"
#include "tools.h"
#include "iologindata.h"
#include "ioban.h"
#include <iomanip>
#include "game.h"
#ifndef __CONSOLE__
#include "gui.h"
#endif

extern RSA* g_otservRSA;
extern ConfigManager g_config;
extern IPList serverIPs;
extern Game g_game;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolLogin::protocolLoginCount = 0;
#endif

#ifdef __DEBUG_NET_DETAIL__
void ProtocolLogin::deleteProtocolTask()
{
	std::cout << "Deleting ProtocolLogin" << std::endl;
	Protocol::deleteProtocolTask();
}
#endif

void ProtocolLogin::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage* output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
	if(output)
	{
		TRACK_MESSAGE(output);
		output->AddByte(error);
		output->AddString(message);
		OutputMessagePool::getInstance()->send(output);
	}

	getConnection()->closeConnection();
}

bool ProtocolLogin::parseFirstPacket(NetworkMessage& msg)
{
	if(
#ifndef __CONSOLE__
		!GUI::getInstance()->m_connections ||
#endif
		g_game.getGameState() == GAME_STATE_SHUTDOWN)
	{
		getConnection()->closeConnection();
		return false;
	}

	uint32_t clientIP = getConnection()->getIP();
	/*uint16_t operatingSystem = */msg.GetU16();
	uint16_t version  = msg.GetU16();
	msg.SkipBytes(12);

	if(!RSA_decrypt(g_otservRSA, msg))
	{
		getConnection()->closeConnection();
		return false;
	}

	uint32_t key[4] = {msg.GetU32(), msg.GetU32(), msg.GetU32(), msg.GetU32()};
	enableXTEAEncryption();
	setXTEAKey(key);

	#ifndef __LOGIN_SERVER__
	if(g_config.getBool(ConfigManager::LOGIN_ONLY_LOGINSERVER))
	{
		disconnectClient(0x0A, "Please re-connect using port 7171.");
		return false;
	}
	#endif

	std::string name = msg.GetString();
	toLowerCaseString(name);
	std::string password = msg.GetString();
	uint32_t id = 1;

	if(!name.length())
	{
		if(g_config.getBool(ConfigManager::ACCOUNT_MANAGER))
			password = "1";
		else
		{
			disconnectClient(0x0A, "You must enter your account name.");
			return false;
		}
	}

	if(version < CLIENT_VERSION_MIN || version > CLIENT_VERSION_MAX)
	{
		disconnectClient(0x0A, CLIENT_VERSION_STRING);
		return false;
	}

	if(g_game.getGameState() == GAME_STATE_STARTUP)
	{
		disconnectClient(0x0A, "Gameworld is starting up. Please wait.");
		return false;
	}

	if(g_game.getGameState() == GAME_STATE_MAINTAIN)
	{
		disconnectClient(0x0A, "Gameworld is under maintenance. Please re-connect in a while.");
		return false;
	}

	if(ConnectionManager::getInstance()->isDisabled(clientIP))
	{
		disconnectClient(0x0A, "Too many connections attempts from this IP. Please try again later.");
		return false;
	}

	if(IOBan::getInstance()->isIpBanished(clientIP))
	{
		disconnectClient(0x0A, "Your IP is banished!");
		return false;
	}

	uint32_t serverIP = serverIPs[0].first;
	for(uint32_t i = 0; i < serverIPs.size(); i++)
	{
		if((serverIPs[i].first & serverIPs[i].second) == (clientIP & serverIPs[i].second))
		{
			serverIP = serverIPs[i].first;
			break;
		}
	}

	Account account;
	if(IOLoginData::getInstance()->getAccountId(name, id) || (!name.length() && g_config.getBool(ConfigManager::ACCOUNT_MANAGER)))
	{
		account = IOLoginData::getInstance()->loadAccount(id);
		if(id < 1 || id != account.number || !passwordTest(password, account.password))
			account.number = 0;
	}

	if(!account.number)
	{
		ConnectionManager::getInstance()->addAttempt(clientIP, false);
		disconnectClient(0x0A, "Account name or password is not correct.");
		return false;
	}

	//Remove premium days
	IOLoginData::getInstance()->removePremium(account);
	if(!g_config.getBool(ConfigManager::ACCOUNT_MANAGER) && !account.charList.size())
	{
		disconnectClient(0x0A, std::string("This account does not contain any character yet.\nCreate a new character on the " + g_config.getString(ConfigManager::SERVER_NAME) + " website at " + g_config.getString(ConfigManager::URL) + ".").c_str());
		return false;
	}

	ConnectionManager::getInstance()->addAttempt(clientIP, true);
	if(OutputMessage* output = OutputMessagePool::getInstance()->getOutputMessage(this, false))
	{
		TRACK_MESSAGE(output);

		//Add MOTD
		output->AddByte(0x14);
		char motd[1300];
		sprintf(motd, "%d\n%s", g_game.getMotdNum(), g_config.getString(ConfigManager::MOTD).c_str());
		output->AddString(motd);

		//Add char list
		output->AddByte(0x64);
		if(g_config.getBool(ConfigManager::ACCOUNT_MANAGER) && id != 1)
		{
			output->AddByte((uint8_t)account.charList.size() + 1);
			output->AddString("Account Manager");
			output->AddString(g_config.getString(ConfigManager::SERVER_NAME));
			output->AddU32(serverIP);
			output->AddU16(g_config.getNumber(ConfigManager::PORT));
		}
		else
			output->AddByte((uint8_t)account.charList.size());

		for(Characters::iterator it = account.charList.begin(); it != account.charList.end(); it++)
		{
			#ifndef __LOGIN_SERVER__
			output->AddString((*it));
			if(g_config.getBool(ConfigManager::ON_OR_OFF_CHARLIST))
			{
				if(g_game.getPlayerByName((*it)))
					output->AddString("Online");
				else
					output->AddString("Offline");
			}
			else
				output->AddString(g_config.getString(ConfigManager::SERVER_NAME));

			output->AddU32(serverIP);
			output->AddU16(g_config.getNumber(ConfigManager::PORT));
			#else
			output->AddString(it->first);
			output->AddString(it->second->getName());
			output->AddU32(inet_addr(it->second->getAddress().c_str()));
			output->AddU16(it->second->getPort());
			#endif
		}

		//Add premium days
		if(g_config.getBool(ConfigManager::FREE_PREMIUM))
			output->AddU16(65535); //client displays free premium
		else
			output->AddU16(account.premiumDays);

		OutputMessagePool::getInstance()->send(output);
	}

	getConnection()->closeConnection();
	return true;
}

void ProtocolLogin::onRecvFirstMessage(NetworkMessage& msg)
{
	parseFirstPacket(msg);
}
