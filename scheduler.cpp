//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Scheduler-Objects for OpenTibia
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

#include "scheduler.h"
#include <iostream>

#if defined __EXCEPTION_TRACER__
#include "exception.h"
#endif

Scheduler::SchedulerState Scheduler::m_threadState = Scheduler::STATE_TERMINATED;

Scheduler::Scheduler()
{
	m_lastEventId = 0;
	Scheduler::m_threadState = STATE_RUNNING;
	OTSYS_THREAD_LOCKVARINIT(m_eventLock);
	OTSYS_THREAD_SIGNALVARINIT(m_eventSignal);
	OTSYS_CREATE_THREAD(Scheduler::schedulerThread, NULL);
}

OTSYS_THREAD_RETURN Scheduler::schedulerThread(void* p)
{
	#if defined __EXCEPTION_TRACER__
	ExceptionHandler schedulerExceptionHandler;
	schedulerExceptionHandler.InstallHandler();
	#endif
	srand((uint32_t)OTSYS_TIME());

	while(Scheduler::m_threadState != Scheduler::STATE_TERMINATED)
	{
		SchedulerTask* task = NULL;
		bool runTask = false;
		int32_t ret;

		// check if there are events waiting...
		OTSYS_THREAD_LOCK(getScheduler().m_eventLock, "schedulerThread()")
		if(getScheduler().m_eventList.empty())
		{
			// unlock mutex and wait for signal
			ret = OTSYS_THREAD_WAITSIGNAL(getScheduler().m_eventSignal, getScheduler().m_eventLock);
		}
		else
		{
			// unlock mutex and wait for signal or timeout
			ret = OTSYS_THREAD_WAITSIGNAL_TIMED(getScheduler().m_eventSignal, getScheduler().m_eventLock, getScheduler().m_eventList.top()->getCycle());
		}

		// the mutex is locked again now...
		if(ret == OTSYS_THREAD_TIMEOUT && Scheduler::m_threadState != Scheduler::STATE_TERMINATED)
		{
			// ok we had a timeout, so there has to be an event we have to execute...
			task = getScheduler().m_eventList.top();
			getScheduler().m_eventList.pop();

			// check if the event was stopped
			EventIdSet::iterator it = getScheduler().m_eventIds.find(task->getEventId());
			if(it != getScheduler().m_eventIds.end())
			{
				// was not stopped so we should run it
				runTask = true;
				getScheduler().m_eventIds.erase(it);
			}
		}

		OTSYS_THREAD_UNLOCK(getScheduler().m_eventLock, "schedulerThread()");
		// add task to dispatcher
		if(task)
		{
			// if it was not stopped
			if(runTask)
				Dispatcher::getDispatcher().addTask(task);
			else
				delete task; // was stopped, have to be deleted here
		}
	}

	#if defined __EXCEPTION_TRACER__
	schedulerExceptionHandler.RemoveHandler();
	#endif
	#if not defined(__USE_BOOST_THREAD__) && not defined(WIN32)
	return NULL;
	#endif
}

uint32_t Scheduler::addEvent(SchedulerTask* task)
{
	bool signal = false;
	if(Scheduler::m_threadState == Scheduler::STATE_RUNNING)
	{
		OTSYS_THREAD_LOCK(m_eventLock, "");
		// check if the event has a valid id
		if(task->getEventId() == 0)
		{
			// if not generate one
			if(m_lastEventId >= 0xFFFFFFFF)
				m_lastEventId = 0;

			++m_lastEventId;
			task->setEventId(m_lastEventId);
		}

		// insert the eventid in the list of active events
		m_eventIds.insert(task->getEventId());
		// add the event to the queue
		m_eventList.push(task);

		// if the list was empty or this event is the top in the list
		// we have to signal it
		signal = (task == m_eventList.top());
		OTSYS_THREAD_UNLOCK(m_eventLock, "");
	}
#ifdef __DEBUG_SCHEDULER__
	else
		std::cout << "[Error - Scheduler::addTask] Scheduler thread is terminated." << std::endl;
#endif

	if(signal)
		OTSYS_THREAD_SIGNAL_SEND(m_eventSignal);

	return task->getEventId();
}

bool Scheduler::stopEvent(uint32_t eventid)
{
	if(eventid == 0)
		return false;

	OTSYS_THREAD_LOCK(m_eventLock, "")
	// search the event id...
	EventIdSet::iterator it = m_eventIds.find(eventid);
	if(it != m_eventIds.end())
	{
		// if it is found erase from the list
		m_eventIds.erase(it);
		OTSYS_THREAD_UNLOCK(m_eventLock, "");
		return true;
	}
	else
	{
		// this eventid is not valid
		OTSYS_THREAD_UNLOCK(m_eventLock, "");
		return false;
	}
}

void Scheduler::stop()
{
	OTSYS_THREAD_LOCK(m_eventLock, "");
	m_threadState = Scheduler::STATE_CLOSING;
	OTSYS_THREAD_UNLOCK(m_eventLock, "");
}

void Scheduler::shutdown()
{
	OTSYS_THREAD_LOCK(m_eventLock, "");
	m_threadState = Scheduler::STATE_TERMINATED;
	//this list should already be empty
	while(!m_eventList.empty())
		m_eventList.pop();

	m_eventIds.clear();
	OTSYS_THREAD_UNLOCK(m_eventLock, "");
}
