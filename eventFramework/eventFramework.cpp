/// @file eventFrameWorke.h
/// This file contains an implementation of an Event system
/// It is implemented using constructs from C++14 standard.
//check windows TODO
//#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
//#include "pch.h";
//#endif

//#include "stdafx.h"  
#include <iostream>
#include <utility>
#include <functional>

#include "eventFrameWork.h"


namespace eventHandling
{

	// Event


	// EventHandler
	void EventHandler::setBlockState(const bool setToState)
	{
		if (setToState)
		{
			m_aRunState = valueFromEnum(RunState::blocked);
		}
		else
		{
			m_aRunState = valueFromEnum(RunState::ready);
		}
	}

	ResultState EventHandler::getResultState()
	{
		return getResultStateAsEnum(m_aResultState);
	}
	bool EventHandler::getEventCount()
	{
		return m_eventsPtrs.data.size();//TODO
	}
	void EventHandler::setRunState(const RunState & val)
	{
		m_aRunState = valueFromEnum(val);
	}
	void EventHandler::setResultState(const ResultState & val)
	{
		m_aResultState = valueFromEnum(val);
	}

	// returns number of successful calls
	int EventHandler::dispatchAllCalls(
		std::shared_ptr<ArgumentContainerBase> argContainerPtr)
	{
		int i = 0;
		size_t arry_size = m_eventsPtrs.data.size();
		if (m_verbose > 0)
		{
			std::cout << "EventHandler::dispatching num of events: "
				<< arry_size << " from " << m_name << " with "
				<< " " << std::this_thread::get_id() << "\n";
		}

		for (auto & baseEventPtr : m_eventsPtrs.data)//TODO
		{
			if (!baseEventPtr.get())
			{
				continue;
			}
			//TODO hard coded to string for now to string
			Event<std::string> * eventPtr = 
				dynamic_cast<Event<std::string> *>(baseEventPtr.get());
			if (eventPtr && eventPtr->isValid())
			{
				if (m_verbose > 0)
				{
					eventPtr->m_verbose = 1;
				}
				if (eventPtr->invokeWithContainerArg(argContainerPtr))
				{
					++i;
				}
				else
				{
					if (m_verbose > 0)
					{
						std::cout << "EventHandler:: failed call  from " << baseEventPtr->m_name << " with "
							<< " " << std::this_thread::get_id() << "\n";
					}
				}
			}
			else if (m_verbose > 0)
			{
				std::cout << "EventHandler:: not dispatching bad call  from " << baseEventPtr->m_name << " with "
						<< " " << std::this_thread::get_id() << "\n";
			}
		}
		return i;
	}

	bool EventHandler::addEvent(std::shared_ptr <EventBase> eventObjectPtr)
	{
		if (!eventObjectPtr.get())
		{
			setResultState(ResultState::invalid);//TODO
			return false;
		}
		m_eventsPtrs.data.emplace_back(eventObjectPtr);//TODO
		return true;
	}

	// EventCall
	std::string EventCall::getCallbackId()
	{
		if (m_EventHandlerPtr.get())
			return m_EventHandlerPtr->getCallbackId();
		return std::string();
	}
	void EventCall::setRunState(const RunState & val)
	{
		m_aRunState = valueFromEnum(val);
	}
	void EventCall::setResultState(const ResultState & val)
	{
		m_aResultState = valueFromEnum(val);
	}
	RunState EventCall::getRunState()
	{
		return getRunStateAsEnum(m_aRunState);
	}
	ResultState EventCall::getResultState()
	{
		return getResultStateAsEnum(m_aResultState);
	}
	void EventCall::setBlockState()
	{
		setRunState(RunState::blocked);
	}
	void EventCall::setEventHandler(std::shared_ptr < EventHandler> eventHandlerPtr)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		m_EventHandlerPtr = eventHandlerPtr;
	}

	void EventCall::setUpdateTimeStamp()
	{
		m_startTime = std::chrono::system_clock::now();
	}

	int EventCall::dispatchAllCalls()
	{
		if (!isValid() || !m_EventHandlerPtr.get())
		{
			setResultState(ResultState::invalid);
			return -1;
		}
		setRunState(RunState::running);
		setResultState(ResultState::failed);
		if (m_verbose > 0)
		{
			std::cout << "EventCall:: dispatchAllCalls():" << m_EventHandlerPtr->m_name
				<< " " << std::this_thread::get_id() << "\n";
		}
		int res = -1;
		std::lock_guard<std::mutex> lk(m_dataMtx);
		try
		{
			res = m_EventHandlerPtr->dispatchAllCalls(m_functionArgumentContainerPtr);
		}
		catch (...)
		{
			g_excPtr = std::current_exception();
		}
		setRunState(RunState::notRunning);
		setResultState(ResultState::success);
		return res;
	}

	//EventBus
	bool EventBus::hasCallback(const std::string & pFunctionName)
	{
		return m_EventHandlerMap.data.find(pFunctionName) != m_EventHandlerMap.data.end();//TODO
	}
	void EventBus::setState(int val)
	{
		std::unique_lock<std::mutex> lk(m_runMtx);
		m_stopped = val;
		return;
	}
	int EventBus::getCallbacksCount()
	{
		return static_cast<int>(m_EventHandlerMap.data.size());//TODO
	}
	int EventBus::getCallsCount()
	{
		return static_cast<int>(m_eventCallPtrs.data.size());//TODO
	}
	void EventBus::stop()
	{
		setState(2);
		m_cond.notify_all();
	}
	bool EventBus::reset()
	{
		setState(0);
		m_cond.notify_all();
		return true;
	}

	int EventBus::getRunState()
	{
		std::unique_lock<std::mutex> lk(m_runMtx);
		return m_stopped;
	}

	bool EventBus::blockEvent(std::string pFunctionName, bool val)
	{
		bool exists = hasCallback(pFunctionName) && m_EventHandlerMap.data[pFunctionName].get();
		if (exists)
		{
			m_EventHandlerMap.data[pFunctionName]->setBlockState(val);//TODO
			return true;
		}
		return false;
	}
}//namespace

