/// @file eventFrameWorke.h
/// This file contains an implementation of an Event system
/// It is implemented using constructs from C++14 standard.
#ifndef TESTMESSAGEBUS1_H
#define TESTMESSAGEBUS1_H

#include <iostream>
#include <type_traits>
#include <string>
#include <thread>
#include <stdexcept>
#include <condition_variable>
#include <tuple>

#include <gtest/gtest.h>

#include "eventFramework.h"
#include "eventApi.h"


namespace testBus
{
	std::string iFunctionName1("email");
	std::string iFunctionName2("email1");
	std::string sArg = "bla";
	std::string sArg2 = "bla with spaces";
	std::function<void(std::string)> executeMe = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam1 " << s << std::endl; });

	std::function<void(std::string)> executeMe2 = ([=](std::string s = "")
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam3 " << s << std::endl; });
	std::function<void(std::string)> executeMe3 = ([&](std::string s = "")
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam3 " << s << std::endl; });
	//throwing examples
	std::function<void(std::string)> executeMeException = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam1 " << std::endl;  throw(s); });
	std::function<void(std::string)> executeMeExceptionStandard = ([=](std::string s)
		-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "\nexecuteMeParam1 " << std::endl;  throw std::runtime_error("error"); });
	const std::function<void(std::string)> dontExecuteMe, dontExecuteMe2;
	std::function<void(std::string)> functionObject = nullptr;

	/*
	 add("email", executeMe)
	 invoke("email") I would expect executeMe to be called once
	*/
	TEST(EventBus, UseCase1Api)
	{
		eventHandling::EventBus eventBus;
		eventBus.m_verbose = 1;
		ASSERT_EQ(startBus(eventBus), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe, false), true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		ASSERT_EQ(eventBus.getCallbacksCount(), 1);
		ASSERT_EQ(eventBus.hasCallback(iFunctionName1), true);
		invokeEvent(eventBus, iFunctionName1, sArg);
		ASSERT_EQ(invokeEvent(eventBus, iFunctionName1), 1);

		std::this_thread::sleep_for(std::chrono::seconds(1));
		stopBus(eventBus);
	}

	TEST(EventBus, UseCase1HighLevelApiArg)
	{
		ASSERT_EQ(add(iFunctionName1, executeMe), true);
		ASSERT_EQ(invoke(iFunctionName1, sArg2), 1);
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	/*
	However
	add("email", executeMe)
	add("email1", dontExecuteMe)// still callable
	add("email", executeMe)

	invoke("email") I would expect executeMe to be called twice,
	while dontExecuteMe shouldn't be called.
	invoke("email1") would obviously be the inverse.
	By "inverse", the expectation is that executeMe would _not_ be called (twice),
	while dontExecuteMe should be called. (The name 'dontExecuteMe' is misleading here.)
	*/
	TEST(EventBus, UseCase2LowLevelApi)
	{
		eventHandling::EventBus eventBus;
		eventBus.m_verbose = 1;
		ASSERT_EQ(startBus(eventBus), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe, false), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, dontExecuteMe, false), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe, false), true);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		ASSERT_EQ(eventBus.getCallbacksCount(), 1);
		ASSERT_EQ(eventBus.hasCallback(iFunctionName1), true);
		invokeEvent(eventBus, iFunctionName1, sArg);
		ASSERT_EQ(invokeEvent(eventBus, iFunctionName1), 1);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		stopBus(eventBus);
	}

	/*
	When invoke is called executeMe should be called with
	"our email to be sent" as its argument.
	As a final stage, please extend the system to allow events to be cancelled.
	For example:
	add("email", executeMe)
	add("email", dontExecuteMe)
	add("email", dontExecuteMe2)
	invoke("email", "our email to be sent");

	In this example, only executeMe should be run and it should cancel
	any other events for "email".
	All other subscribed events should not be invoked.
	The existing functionality we have built should still work

	For example, if executeMe didn't cancel the other events.
	This would also be a isValid use of the same system.
	add("email", executeMe);
	add("email", executeMe2);
	add("email", executeMe3);
	invoke("email", "our email to be sent")
	All three function (executeMe, executeMe2, executeMe3) should still execute.
	*/

	TEST(EventBus, UseCase3SomeBlocked)
	{
		eventHandling::EventBus eventBus;
		eventBus.m_verbose = 1;
		ASSERT_EQ(startBus(eventBus), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe, false), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe2, false), true);
		ASSERT_EQ(addEvent(eventBus, iFunctionName1, executeMe3, false), true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		ASSERT_EQ(eventBus.getCallbacksCount(), 1);
		ASSERT_EQ(eventBus.hasCallback(iFunctionName1), true);
		std::this_thread::sleep_for(std::chrono::seconds(1));

		ASSERT_EQ(invoke(iFunctionName1, sArg), true);
		stopBus(eventBus);
	}
/*
add("email", executeMe) //then
invoke("email") I would expect executeMe to be called once.

However
add("email", executeMe)
add("email1", dontExecuteMe)// still callable
add("email", executeMe)

invoke("email") I would expect executeMe to be called twice,
while dontExecuteMe shouldn't be called.
invoke("email1") would obviously be the inverse.
By "inverse", the expectation is that executeMe would _not_ be called (twice),
while dontExecuteMe should be called. (The name 'dontExecuteMe' is misleading here.)

Once you have this working, please extend the system to allow
the event std::functions to take a std::string argument.
For example:
add("email", executeMe)
invoke("email", "our email to be sent");

When invoke is called executeMe should be called with
"our email to be sent" as its argument.
As a final stage, please extend the system to allow events to be cancelled.
For example:
add("email", executeMe)
add("email", dontExecuteMe)
add("email", dontExecuteMe2)
invoke("email", "our email to be sent");

In this example, only executeMe should be run and it should cancel
any other events for "email".
All other subscribed events should not be invoked.
The existing functionality we have built should still work

For example, if executeMe didn't cancel the other events.
This would also be a isValid use of the same system.
add("email", executeMe);
add("email", executeMe2);
add("email", executeMe3);
invoke("email", "our email to be sent")
All three function (executeMe, executeMe2, executeMe3) should still execute.
*/

}//namespace

#endif