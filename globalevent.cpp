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

#include "tools.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "globalevent.h"

GlobalEvents::GlobalEvents() :
m_scriptInterface("GlobalEvent Interface")
{
	m_scriptInterface.initState();
}

GlobalEvents::~GlobalEvents()
{
	clear();
}

void GlobalEvents::clear()
{
	GlobalEventList::iterator it = eventsMap.begin();
	while(it != eventsMap.end())
	{
		delete it->second;
		eventsMap.erase(it);
		it = eventsMap.begin();
	}

	m_scriptInterface.reInitState();
}

LuaScriptInterface& GlobalEvents::getScriptInterface()
{
	return m_scriptInterface;
}

std::string GlobalEvents::getScriptBaseName()
{
	return "globalevents";
}

Event* GlobalEvents::getEvent(const std::string& nodeName)
{
	if(asLowerCaseString(nodeName) == "globalevent")
		return new GlobalEvent(&m_scriptInterface);
	else
		return NULL;
}

bool GlobalEvents::registerEvent(Event* event, xmlNodePtr p)
{
	GlobalEvent* globalEvent = dynamic_cast<GlobalEvent*>(event);
	if(!globalEvent)
		return false;

	eventsMap.push_back(std::make_pair(globalEvent->getName(), globalEvent));
	return true;
}

void GlobalEvents::startup()
{
	Scheduler::getScheduler().addEvent(createSchedulerTask(GLOBAL_THINK_INTERVAL,
		boost::bind(&GlobalEvents::onThink, this, GLOBAL_THINK_INTERVAL)));
}

void GlobalEvents::onThink(uint32_t interval)
{
	uint32_t timeNow = time(NULL);
	for(GlobalEventList::iterator it = eventsMap.begin(); it != eventsMap.end(); ++it)
	{
		GlobalEvent* globalEvent = it->second;
		if(timeNow > (globalEvent->getLastExecution() + globalEvent->getInterval()))
		{
			globalEvent->setLastExecution(timeNow);
			if(!globalEvent->executeThink(interval, timeNow))
				std::cout << "[Error - GlobalEvents::onThink] Couldn't execute event: " << globalEvent->getName() << std::endl;
		}
	}

	Scheduler::getScheduler().addEvent(createSchedulerTask(interval,
		boost::bind(&GlobalEvents::onThink, this, interval)));
}

GlobalEvent::GlobalEvent(LuaScriptInterface* _interface) : Event(_interface)
{
	m_lastExecution = time(NULL);
}

bool GlobalEvent::configureEvent(xmlNodePtr p)
{
	std::string strValue;
	if(readXMLString(p, "name", strValue))
		m_name = strValue;
	else
	{
		std::cout << "[Error - GlobalEvent::configureEvent] No name for GlobalEvent." << std::endl;
		return false;
	}

	int32_t intValue;
	if(readXMLInteger(p, "interval", intValue))
		m_interval = intValue;
	else
	{
		std::cout << "[Error - GlobalEvent::configureEvent] No interval for GlobalEvent." << std::endl;
		return false;
	}

	return true;
}

std::string GlobalEvent::getScriptEventName()
{
	return "onThink";
}

int32_t GlobalEvent::executeThink(uint32_t interval, uint32_t lastExecution)
{
	//onThink(interval, lastExecution)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[125];
		sprintf(desc, "%s - %i (%i)", getName().c_str(), interval, lastExecution);
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, interval);
		lua_pushnumber(L, lastExecution);

		int32_t result = m_scriptInterface->callFunction(2);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. GlobalEvent::executeThink" << std::endl;
		return 0;
	}
}
