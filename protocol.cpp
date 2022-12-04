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

#include "definitions.h"

#if defined WIN32
#include <winerror.h>
#endif

#include "protocol.h"
#include "connection.h"
#include "outputmessage.h"
#include "scheduler.h"
#include "rsa.h"

void Protocol::onSendMessage(OutputMessage* msg)
{
	#ifdef __DEBUG_NET_DETAIL__
	std::cout << "Protocol::onSendMessage" << std::endl;
	#endif
	if(!m_rawMessages)
	{
		msg->writeMessageLength();
		if(m_encryptionEnabled)
		{
			#ifdef __DEBUG_NET_DETAIL__
			std::cout << "Protocol::onSendMessage - encrypt" << std::endl;
			#endif
			XTEA_encrypt(*msg);
			msg->addCryptoHeader(m_checksumEnabled);
		}
	}

	if(msg == m_outputBuffer)
		m_outputBuffer = NULL;
}

void Protocol::onRecvMessage(NetworkMessage& msg)
{
	#ifdef __DEBUG_NET_DETAIL__
	std::cout << "Protocol::onRecvMessage" << std::endl;
	#endif
	if(m_encryptionEnabled)
	{
		#ifdef __DEBUG_NET_DETAIL__
		std::cout << "Protocol::onRecvMessage - decrypt" << std::endl;
		#endif
		XTEA_decrypt(msg);
	}

	parsePacket(msg);
}

OutputMessage* Protocol::getOutputBuffer()
{
	if(m_outputBuffer)
		return m_outputBuffer;

	else if(m_connection)
	{
		m_outputBuffer = OutputMessagePool::getInstance()->getOutputMessage(this);
		return m_outputBuffer;
	}
	else
		return NULL;
}

void Protocol::releaseProtocol()
{
	if(m_refCount > 0)
		Scheduler::getScheduler().addEvent(createSchedulerTask(SCHEDULER_MINTICKS, boost::bind(&Protocol::releaseProtocol, this)));
	else
		deleteProtocolTask();
		
}

void Protocol::deleteProtocolTask()
{
	//dispather thread
	assert(m_refCount == 0);
	setConnection(NULL);
	if(m_outputBuffer)
		OutputMessagePool::getInstance()->releaseMessage(m_outputBuffer);

	delete this;
}

void Protocol::XTEA_encrypt(OutputMessage& msg)
{
	uint32_t k[4];
	for(uint8_t i = 0; i < 4; i++)
		k[i] = m_key[i];

	int32_t messageLength = msg.getMessageLength();
	//add bytes until reach 8 multiple
	uint32_t n;
	if((messageLength % 8) != 0)
	{
		n = 8 - (messageLength % 8);
		msg.AddPaddingBytes(n);
		messageLength = messageLength + n;
	}

	int32_t readPos = 0;
	uint32_t* buffer = (uint32_t*)msg.getOutputBuffer();
	while(readPos < messageLength / 4)
	{
		uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
		uint32_t delta = 0x61C88647;
		uint32_t sum = 0;
		for(int32_t i = 0; i < 32; i++)
		{
			v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
			sum -= delta;
			v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
		}

		buffer[readPos] = v0;
		buffer[readPos + 1] = v1;
		readPos += 2;
	}
}

bool Protocol::XTEA_decrypt(NetworkMessage& msg)
{
	if((msg.getMessageLength() - 6) % 8 != 0)
	{
		std::cout << "Failure: [Protocol::XTEA_decrypt]. Not valid encrypted message size" << std::endl;
		return false;
	}

	uint32_t k[4];
	for(uint8_t i = 0; i < 4; i++)
		k[i] = m_key[i];

	int32_t messageLength = msg.getMessageLength() - 6;
	uint32_t* buffer = (uint32_t*)(msg.getBuffer() + msg.getReadPos());
	int32_t readPos = 0;
	while(readPos < messageLength / 4)
	{
		uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
		uint32_t delta = 0x61C88647;
		uint32_t sum = 0xC6EF3720;
		for(int32_t i = 0; i < 32; i++)
		{
			v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
			sum += delta;
			v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
		}

		buffer[readPos] = v0;
		buffer[readPos + 1] = v1;
		readPos += 2;
	}
	//

	int32_t tmp = msg.GetU16();
	if(tmp > msg.getMessageLength() - 8)
	{
		std::cout << "Failure: [Protocol::XTEA_decrypt]. Not valid unencrypted message size" << std::endl;
		return false;
	}

	msg.setMessageLength(tmp);
	return true;
}

bool Protocol::RSA_decrypt(RSA* rsa, NetworkMessage& msg)
{
	if(msg.getMessageLength() - msg.getReadPos() != 128)
	{
		std::cout << "Warning: [Protocol::RSA_decrypt]. Not valid packet size" << std::endl;
		return false;
	}

	rsa->decrypt((char*)(msg.getBuffer() + msg.getReadPos()), 128);
	if(msg.GetByte() != 0)
	{
		std::cout << "Warning: [Protocol::RSA_decrypt]. First byte != 0" << std::endl;
		return false;
	}

	return true;
}

uint32_t Protocol::getIP() const
{
	if(getConnection())
		return getConnection()->getIP();

	return 0;
}
