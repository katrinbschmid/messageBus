#pragma once
/// @file eventApi.h
/// This file contains an api for Event system
/// It is implemented using constructs from C++14 standard.
#ifndef MESSAGEBUSAPI_H
#define MESSAGEBUSAPI_H

#include <iostream>
#include <future>
#include <memory>
#include <stdexcept>

#include "eventFramework.h"

namespace
{
	/// simple api

	/// @brief Add function as described in the use cases
	/// @details Provides an ability to add callbacks and  run them
	/// @param eventHandling::EventBus object to use
	/// @return true in case the bus was started
	std::shared_ptr<eventHandling::EventBus> g_eventBusPtr;
	static bool startBus(eventHandling::EventBus & eventBus);

	/// @brief Add function as described in the use cases
	/// @details Provides an ability to add callbacks and run them
	/// @param eventHandling::EventBus object to use
	/// @param callBackName identifier for callback
	/// @param async optional string argument, default  true
	/// @param verbose optional argument, default 1
	/// @return true in case the handler was successfully posted
	template<typename T>
	static bool addEvent(eventHandling::EventBus & eventBus,
		std::string callBackName, std::function<void(T)> functionObject,
		bool async = true, int verbose = 1);

	/// @brief Implements basic event type 
	/// @details Provides an ability to add callbacks and  run them
	/// @param eventHandling::EventBus object to use
	/// @param callBackName identifier for callback
	/// @param functionArgument optional string argument, default empty string
	/// @param async optional string argument, default  true
	/// @param verbose optional argument, default 1
	/// @return number of sucesssful calls in the invocation
	//setting function argument back to simple default, cannot properly detect void now
	static int invokeEvent(eventHandling::EventBus & eventBus,
		const std::string & callBackName, const std::string functionArgument = "",
		bool async = true, int verbose = 1);

	/// @brief Add function as described in the use cases
	/// @details Provides an ability to add callbacks and  run them
	/// @param callBackName identifier for callback
	/// @param functionObject std::function<void(T)>() to store
	/// only supports string at the moment no return values 
	/// @return true in case the handler was successfully posted
	template<typename T>
	static bool add(const std::string & callBackName,
		std::function<void(T)> functionObject)
	{
		if (!g_eventBusPtr.get())
		{
			new eventHandling::EventBus;
			g_eventBusPtr = std::shared_ptr<eventHandling::EventBus>(new eventHandling::EventBus);
			if (g_eventBusPtr.get())
			{
				startBus(*(g_eventBusPtr.get()));
			}
			else
			{
				return false;
			}
		}
		return addEvent(*(g_eventBusPtr.get()), callBackName, functionObject);
	}
	/// @brief Implements basic event type 
	/// @details Provides an ability to add callbacks and  run them
	/// @param callBackName identifier for callback
	/// @param functionArgument optional string argument, default empty string
	/// @param verbose optional argument, default 1
	/// @return number of sucesssful calls in the invocation
	// Should be using template <class... Args>
	static int invoke(const std::string & callBackName, 
		const std::string &functionArgument="", int verbose=1)
	{
		if (!g_eventBusPtr.get())
		{
			if (verbose > 0)
			{
				std::cout << ("Please add event first\n");
			}
			return false;
		}
		return invokeEvent(*(g_eventBusPtr.get()), callBackName, functionArgument);
	}

	/// implementation

	static bool startBus(eventHandling::EventBus & eventBus)
	{
		std::thread mainthr(&eventHandling::EventBus::run, &eventBus);
		mainthr.detach();// there must be something better
		return true;
	}

	template<typename T>
	static bool addEvent(eventHandling::EventBus & eventBus,
		std::string callBackName,
		std::function<void(T)> functionObject, bool async, int verbose)
	{
		if (true)//if (!async) //TODO
		{
			if (verbose > 0)
			{
				std::cout << "\n__ADD__ " << callBackName << "\n";
			}
			return eventBus.add(callBackName, functionObject);
		}
	}

	static int invokeEvent(eventHandling::EventBus & eventBus,
			const std::string & callBackName, const std::string functionArgument,
			bool async, int verbose)
	{
		if (true)//if (!async) //TODO
		{
			if (verbose > 0)
			{ 
				std::cout << "\n__INVOKE__ " << callBackName << 
					" "<< functionArgument <<"\n";
			}
			return eventBus.invokeEvent(callBackName, functionArgument);
		}
	}

	static bool reStartBus(eventHandling::EventBus & eventBus)
	{
		eventBus.reset();
		return startBus(eventBus);
	}

	/// @brief Stop the main loop
	/// @details can take some time for the loop to terminate
	/// @param eventHandling::EventBus to stop
	/// @return true in case the bus was stopped
	static bool stopBus(eventHandling::EventBus & eventBus)
	{
		eventBus.stop();
		return true;//check state after some wait TODO
	}


}//namespace
#endif