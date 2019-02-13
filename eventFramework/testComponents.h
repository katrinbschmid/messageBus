/// @file eventFrameWorke.h
/// This file contains an implementation of an Event system
/// It is implemented using constructs from C++14 standard.
#ifndef TESTMESSAGEBUS2_H
#define TESTMESSAGEBUS2_H

#include <iostream>
#include <type_traits>
#include <string>
#include <thread>
#include <stdexcept>
#include <condition_variable>

#include <gtest/gtest.h>

#include "eventFramework.h"


static std::exception_ptr g_excPtr = nullptr;
namespace testCom
{
	const std::string iFunctionName1("email");
	const std::string iFunctionName2("email1");
	std::string s = "bla";
	std::string argS = "arg";
	std::function<void(std::string)> executeMe = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam test2 " << s << std::endl; });
	std::function<void(std::string)> executeMeException = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam2 " << std::endl;  throw(s); });
	std::function<void(std::string)> executeMeExceptionStandard = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam12" << std::endl;  throw std::runtime_error("error"); });
	std::function<void(std::string)> executeMeEmpty = ([=](std::string s)
		-> void {  });

	/*
	std::function<void(std::string)> executeMe2 = ([=]()
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam2 " << s << std::endl; });
	*/
	auto executeMe3 = ([=](std::string s = "")
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam3 " << s << std::endl; });
	auto executeMeNoString = ([=]()
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam4 " << std::endl; });
	const std::function<void(std::string)> dontExecuteMe, dontExecuteMe2;
	std::function<void(std::string)> functionObject = nullptr;


	TEST(Event, Invoke)
	{
		eventHandling::Event<std::string> testEvent("test");
		testEvent.m_verbose = 1;
		testEvent.setCallback(executeMe);
		ASSERT_EQ(testEvent.isValid(), true);
		std::shared_ptr<eventHandling::ArgumentContainerBase> argConPtr(
			new eventHandling::ArgumentContainer<std::string>());
		setContainerArgumentString(std::string(), argConPtr.get(), argS);
		ASSERT_EQ((argConPtr->m_argsType != eventHandling::ArgsTypes::voidType), true);
		ASSERT_EQ(testEvent.invokeWithContainerArg(argConPtr), true);
	}

	TEST(Event, Blocked)
	{
		eventHandling::Event<std::string> testEvent("testBlocked");
		testEvent.m_verbose = 1;;
		ASSERT_EQ(testEvent.setCallback(dontExecuteMe), true);
		ASSERT_EQ((testEvent.getRunState() == eventHandling::RunState::blocked), true);
		ASSERT_EQ(testEvent.isValid(), true);
		std::shared_ptr<eventHandling::ArgumentContainerBase> argConPtr(
			new eventHandling::ArgumentContainer<std::string>());
		ASSERT_EQ(testEvent.invokeWithContainerArg(argConPtr), false);
		ASSERT_EQ((testEvent.getRunState() == eventHandling::RunState::blocked), true);
	}

	TEST(EventHandler, Invoke)
	{
		eventHandling::EventHandler testHandler(iFunctionName1);
		std::shared_ptr <eventHandling::Event<std::string>> eventObjectPtr(
			new eventHandling::Event<std::string>("test"));
		testHandler.m_verbose = 1;
		eventObjectPtr->m_verbose = 1;
		eventObjectPtr->setCallback(executeMe);
		ASSERT_EQ(testHandler.isValid(), false);
		ASSERT_EQ(eventObjectPtr->isValid(), true);
		ASSERT_EQ(testHandler.addEvent(eventObjectPtr), true);
		ASSERT_EQ(testHandler.isValid(), true);
		std::shared_ptr<eventHandling::ArgumentContainerBase> argConPtr(
			new eventHandling::ArgumentContainer<std::string>());
		eventHandling::setContainerArgumentString(std::string(),
			argConPtr.get(), iFunctionName1, eventHandling::ArgsTypes::stringType);
		ASSERT_EQ(testHandler.dispatchAllCalls(argConPtr), 1);
	}

	TEST(EventCall, Invoke)
	{
		eventHandling::EventCall testCall;
		testCall.m_verbose = 1;
		std::shared_ptr <eventHandling::EventHandler> eventHandlerPtr(
			new eventHandling::EventHandler(iFunctionName1));
		std::shared_ptr <eventHandling::Event<std::string>> eventObjectPtr(
			new eventHandling::Event<std::string>("test"));
		eventObjectPtr->setCallback(executeMe);
		eventObjectPtr->m_verbose = 1;
		eventHandlerPtr->m_verbose = 1;
		testCall.m_verbose = 1;
		//setup
		std::shared_ptr<eventHandling::ArgumentContainerBase> argBasePtr
			= testCall.getArgument();
		if (!argBasePtr.get())
		{
			argBasePtr = std::make_shared<
				eventHandling::ArgumentContainer<std::string>>(new 
					eventHandling::ArgumentContainer<std::string>());
		}
		ASSERT_EQ((argBasePtr.get() != 0), true);
		if (argBasePtr.get())
		{
			eventHandling::setContainerArgumentString(std::string(),
				argBasePtr.get(), iFunctionName1, eventHandling::ArgsTypes::stringType);
		}
		ASSERT_EQ((argBasePtr.get()!= 0), true);
		ASSERT_EQ(eventHandlerPtr->addEvent(eventObjectPtr), true);
		ASSERT_EQ(testCall.isValid(), false);
		testCall.setEventHandler(eventHandlerPtr);
		ASSERT_EQ(testCall.isValid(), true);
		//call
		ASSERT_EQ(testCall.dispatchAllCalls(), 1);
		testCall.setBlockState();
		ASSERT_EQ(testCall.getRunState() == eventHandling::RunState::blocked, true);
	}
	/*
	add("email", executeMe) //then
	invoke("email") I would expect executeMe to be called once.
	*/
	TEST(EventBus, UseCase1)
	{
	
		eventHandling::EventBus eventBus;
		eventBus.m_verbose = 1;
		ASSERT_EQ(eventBus.add(iFunctionName1, executeMe), true);
		ASSERT_EQ(eventBus.getCallbacksCount(), 1);
		ASSERT_EQ(eventBus.hasCallback(iFunctionName1), true);
		ASSERT_EQ(eventBus.invokeEvent(iFunctionName1, s), 1);
		ASSERT_EQ(eventBus.getCallsCount(), 1);
	}

	// incredible basic
	TEST(QueueWrapper, Basic)
	{
		QueueWrapper<int> q;
		q.push(1);
		ASSERT_EQ(q.size(), 1);
		ASSERT_EQ(q.empty(), false);
	}

	TEST(MapWrapper, Basic)
	{
		UnordMapWrapper <std::string, int> map;
		map.push("la", 1);
		ASSERT_EQ(map.size(), 1);
	}
	//		UnordMapWrapper <std::string, std::shared_ptr<EventHandler>>  m_EventHandlerMap;
	TEST(VectorWrapper, Basic)
	{
		VectorWrapper<int> v;
		v.emplace_back(1);
		ASSERT_EQ(v.size(), 1);
		ASSERT_EQ(v.empty(), false);
	}

}
#endif