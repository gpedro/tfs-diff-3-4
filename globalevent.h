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


#ifndef __GLOBALEVENT_H__
#define __GLOBALEVENT_H__
#endif

#include <list>
#include <string>
#include "luascript.h"
#include "baseevents.h"
#include "const.h"
#include "scheduler.h"

#define GLOBAL_THINK_INTERVAL 1000
class GlobalEvent;

class GlobalEvents : public BaseEvents
{
	public:
		GlobalEvents();
		virtual ~GlobalEvents();

		void startup();
		void onThink(uint32_t interval);

	protected:
		virtual LuaScriptInterface& getScriptInterface();
		virtual std::string getScriptBaseName();
		virtual Event* getEvent(const std::string& nodeName);
		virtual bool registerEvent(Event* event, xmlNodePtr p);
		virtual void clear();

		typedef std::list< std::pair<std::string, GlobalEvent* > > GlobalEventList;
		GlobalEventList eventsMap;

		LuaScriptInterface m_scriptInterface;
};

class GlobalEvent : public Event
{
	public:
		GlobalEvent(LuaScriptInterface* _interface);
		virtual ~GlobalEvent() {}

		virtual bool configureEvent(xmlNodePtr p);

		std::string getName() const {return m_name;}
		uint32_t getInterval() const {return m_interval;}

		uint32_t getLastExecution() const {return m_lastExecution;}
		void setLastExecution(uint32_t time) {m_lastExecution = time;}

		//scripting
		int32_t executeThink(uint32_t interval, uint32_t lastExecution);

	protected:
		virtual std::string getScriptEventName();

		std::string m_name;
		uint32_t m_interval;

		uint32_t m_lastExecution;
};
