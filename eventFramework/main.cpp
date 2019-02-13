/// @file eventFrameWorke.h
/// This file contains an implementation of an Event system
/// It is implemented using constructs from C++14 standard.

#include <iostream>
#include <type_traits>
#include <string>
#include <thread>
#include <stdexcept>
#include <condition_variable>

#include <gtest/gtest.h>

#include "eventApi.h"

#include "testComponents.h"
#include "testBus.h"

namespace test
{
	/// please see test headers for more examples
	static void verySimple()
	{
		const std::string iFunctionName1("email");
		const std::string iFunctionArgument("argument");

		std::function<void(std::string)> executeMe1 = ([=](std::string s)
			-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << "\nexecuteMeParam test2 " << s << std::endl; });

		std::function<void(std::string)> executeMe = ([=](std::string s)
			-> void {  std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << "\nexecuteMeParam test2 " << s << std::endl; });

		add(iFunctionName1, executeMe1);
		invoke(iFunctionName1, iFunctionArgument);
		return;
	}

	static void testRun(int argc, char ** argv)
	{
		std::cout << std::this_thread::get_id() << " from thread " << std::endl;
		testing::InitGoogleTest(&argc, argv);

		RUN_ALL_TESTS();
		return;
	}
}//namespace


int main(int argc, char ** argv)
{
	test::verySimple();
	test::testRun(argc, argv);

	return 0;
}