/// @file eventFrameWorke.h
/// This file contains an implementation of an Event system
/// It is implemented using constructs from C++14 standard.
#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include <iostream>
#include <sstream>  
#include <type_traits>
#include <string>
#include <functional>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <future>

#include "containerWrapper.h"

namespace eventHandling
{
	static std::exception_ptr g_excPtr = nullptr;
	/// @brief TODO
	enum class ArgsTypes
	{
		voidType = 0,
		stringType = 1,
		intType = 2,
		unkown = 3
	};
	/// @brief run states of objects
	enum class RunState
	{
		running = 0,
		interrupt = 1,
		stopped = 2,
		blocked = 3,
		notRunning = 4,
		ready = 5,
		waiting = 6
	};
	/// @brief result states of objects
	enum class ResultState
	{
		created = 0,
		invalid = 1,
		failed = 2,
		success = 3
	};
	
	/// @brief make state human readable and atomic friendly
	/// @details Provides an ability to add callbacks and  run them
	//from stackoverflow, make state human readable and atomic friendly
	///https://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
	namespace utils
	{
		namespace details
		{
			template< typename E >
			using enable_enum_t = typename std::enable_if< std::is_enum<E>::value,
				typename std::underlying_type<E>::type
			>::type;

		}   // namespace details

		template< typename E >
		constexpr inline details::enable_enum_t<E> underlying_value(E e)noexcept
		{
			return static_cast<typename std::underlying_type<E>::type>(e);
		}

		template< typename E, typename T>
		constexpr inline typename std::enable_if< std::is_enum<E>::value &&
			std::is_integral<T>::value, E
		>::type
			to_enum(T value) noexcept
		{
			return static_cast<E>(value);
		}
	} // namespace utils


	// leaving these in the header so I can use them in tests
	static ResultState getResultStateAsEnum(int val)
	{
		return utils::to_enum<ResultState>(static_cast<int>(val));
	}
	static RunState getRunStateAsEnum(int val)
	{
		return utils::to_enum<RunState>(static_cast<int>(val));
	}
	static int valueFromEnum(ArgsTypes val)
	{
		return utils::underlying_value(val);
	}
	static int valueFromEnum(RunState val)
	{
		return utils::underlying_value(val);
	}
	static int valueFromEnum(ResultState val)
	{
		return utils::underlying_value(val);
	}

	/// @brief Base for ArgumentContainer for different types
	/// @details Special case for void
	class ArgumentContainerBase
	{
	public:
		ArgumentContainerBase() : m_isVoid(true)
		{
			m_argsType = ArgsTypes::unkown;
		}
		bool m_isVoid;//TODO use enum instead
		ArgsTypes m_argsType;
		virtual ~ArgumentContainerBase() {};
	};


	/// @briefArgumentContainer  for different types
	/// @details Special case for void
	template <class T>
	class ArgumentContainer : public ArgumentContainerBase
	{
	public:
		ArgumentContainer(bool isVoid = false) : ArgumentContainerBase() {}
		T m_Argument;
	};
	//runner

	template <class T>
	static bool setContainerArgument(ArgumentContainerBase * functionArgumentContainerPtr,
		T val, ArgsTypes argType = ArgsTypes::unkown)
	{
		if (functionArgumentContainerPtr)
		{
			bool isVoid = (argType == ArgsTypes::voidType);
			ArgumentContainer<T> * argPtr =
				dynamic_cast<ArgumentContainer<T> *> (functionArgumentContainerPtr);
			if (argPtr)
			{
				if (!isVoid)
				{
					argPtr->m_Argument = val;
				}
				else
				{
					argPtr->m_isVoid = true;
				}
				return true;
			}
		}
		return false;
	}

	template <class T>
	static bool setContainerArgumentString(T dummy,
		 ArgumentContainerBase * functionArgumentContainerPtr,
		T val,  ArgsTypes argType = eventHandling::ArgsTypes::stringType)
	{
		return  setContainerArgument(functionArgumentContainerPtr, val, argType);
	}
	template <class T>
	static bool getContainerArgument(ArgumentContainerBase * functionArgumentContainerPtr,
		 T & val, bool & isVoid)
	{
		if (functionArgumentContainerPtr)
		{
			ArgumentContainer<T> * argPtr =
				dynamic_cast<ArgumentContainer<T> *> (functionArgumentContainerPtr);
			if (argPtr)
			{
				isVoid = argPtr->m_isVoid;
				if (!isVoid)
				{
					val = argPtr->m_Argument;
				}
			}
			return true;
		}
		return false;
	}

	/// @brief Base for ArgumentContainer for different types
	/// @details Special case for void
	class ObjectBase
	{
	private:
		ResultState m_ResultState;
		RunState m_RunState;
	public:
		// reset to 0 for a lot of debug output
		ObjectBase(std::string name = "") : m_name(name), m_verbose(0) {}
		virtual ~ObjectBase() {};
		std::string m_name;
		int m_verbose;
		ArgsTypes argtype;
		virtual bool isValid() = 0;
	};
	/// @brief Base for ArgumentContainer for different types
/// @details Special case for void
	class EventBase : public ObjectBase
	{
	public:
		EventBase(std::string name = "") : ObjectBase(name) {}
		virtual ~EventBase() {};
		ArgsTypes m_argtype;
	};

	/// @brief Base for ArgumentContainer for different types
	/// @details Special case for void
	template <class T>
	class Event : public EventBase
	{
		mutable std::mutex m_dataMtx; //m_Callback
		std::atomic<int> m_aResultState, m_aRunState;
	protected:
		std::function<void(T)> m_Callback;
		void setRunState(const RunState & val);
		void setResultState(const ResultState & val);
		//inital implementation should use argument containers
		bool invokeInternal(T functionArgument, bool isVoid = false);
		bool dispatch();
		template <class A>
		bool dispatch(A functionArgument);
	public:
		bool invokeWithContainerArg(std::shared_ptr<ArgumentContainerBase> argConPtr);
		Event(std::string eventName = "") : EventBase(eventName), m_aRunState(0),
			m_aResultState(0)
		{
			m_aResultState = 0;
			m_aRunState = 0;
			m_argtype = ArgsTypes::unkown;
		}
		ResultState getResultState();
		RunState getRunState();
		bool isValid() override
		{
			return getResultState() != ResultState::invalid;
		}
		bool setCallback(std::function<void(T)> callback);
		template <class... Args>
		auto invoke(Args... args) -> decltype(dispatch(args...))
		{
			return dispatch(args...);
		}
	};

	/// @brief Group of events identified by same id
	/// @details TODO
	class EventHandler : public EventBase
	{
		std::atomic<int> m_aResultState, m_aRunState;
	protected:
		std::string m_callbackId;
		VectorWrapper<std::shared_ptr<EventBase>> m_eventsPtrs;
		void setRunState(const RunState & val);
		void setResultState(const ResultState & val);

	public:
		EventHandler(const std::string &callbackId="") : m_callbackId(callbackId), m_aResultState(0) {}
		bool isValid() override
		{
			return !m_eventsPtrs.data.empty();//TODO
		}
		std::string getCallbackId()
		{
			return m_callbackId;
		}
		ResultState getResultState();
		bool getEventCount();
		void setBlockState(const bool setToState);
		bool addEvent(std::shared_ptr <EventBase> eventObjectPtr);
		int dispatchAllCalls(std::shared_ptr<ArgumentContainerBase> argContainer);
	};

	/// @brief //Job, keep state and result
	/// @details TODO
	class EventCall : public ObjectBase
	{
		mutable std::mutex m_dataMtx, m_argMtx;
		std::atomic<int> m_aResultState, m_aRunState;
	protected:
		std::shared_ptr < EventHandler> m_EventHandlerPtr;
		std::shared_ptr<ArgumentContainerBase> m_functionArgumentContainerPtr;
		std::chrono::system_clock::time_point m_startTime;
		void setRunState(const RunState & val);
		void setResultState(const ResultState & val);
	public:
		EventCall() : m_aRunState(0), m_aResultState(0), m_startTime(std::chrono::system_clock::now()) {}
		std::shared_ptr<ArgumentContainerBase> getArgument()
		{
			return m_functionArgumentContainerPtr;
		}
		bool isValid() override
		{
			std::lock_guard<std::mutex> lk(m_dataMtx);
			return m_EventHandlerPtr.get() && m_EventHandlerPtr->isValid();
		}
		std::string getCallbackId();
		RunState getRunState();
		ResultState getResultState();
		void setEventHandler(std::shared_ptr < EventHandler> eventHandlerPtr);
		void setBlockState();
		void setUpdateTimeStamp();
		int dispatchAllCalls();
	};

	/// @brief Processing Queue
	/// @details runs in a infinite loop unless stopped
	class EventBus : public ObjectBase
	{
		std::mutex m_runMtx;
		std::condition_variable m_cond;
	protected:
		int m_stopped = 0;// 0 running, 1 interrupt, 2 stop processing like RunState enum
		QueueWrapper<std::unique_ptr<EventCall>> m_eventCallPtrs; // 0…*
		UnordMapWrapper <std::string, std::shared_ptr<EventHandler>>  m_EventHandlerMap;

		void setState(int val);

		/// @brief intern
		/// @details make a new EventCall object and queue it
		template<typename T>
		bool invokeEventInternal(std::string pFunctionName, 
			T functionArgument, bool isVoid = false)//TODO use enum
		{
			bool exists = hasCallback(pFunctionName);
			if (getCallsCount() > m_maxCapacity || !exists)
			{
				if (m_verbose > 0 && !exists)
				{
					std::cout << "EventBus::invokeEvent No such callback " << pFunctionName
						<< " " << std::this_thread::get_id() << "\n";
				}
				return false;
			}
			std::unique_ptr <EventCall> evCallPtr(new EventCall);
			//add handler
			evCallPtr->setEventHandler(m_EventHandlerMap.data[pFunctionName]);
			std::shared_ptr <ArgumentContainerBase> argConPtr(new ArgumentContainer<T>());
			/*
			//TODO use a string for now, out of time unfortunately
			*/
			std::string argString;
			std::stringstream ss(functionArgument);
			if (std::istringstream(functionArgument))
			{
				argString = ss.str();
			}
			std::shared_ptr<ArgumentContainerBase> argBasePtr = evCallPtr->getArgument();
			argBasePtr = std::shared_ptr<eventHandling::ArgumentContainerBase> 
				(new eventHandling::ArgumentContainer<std::string>());
			if (argBasePtr.get())
			{
				setContainerArgumentString(std::string(), 
					 argBasePtr.get(), argString, ArgsTypes::stringType);
			}
			else if (m_verbose > 0)
			{
				std::cout << "EventBus::invokeEvent No argument container " << pFunctionName
						<< " " << std::this_thread::get_id() << "\n";
			}

			if (evCallPtr->isValid())
			{ 
				m_eventCallPtrs.data.push(std::move(evCallPtr));//TODO
				setState(1);
				m_cond.notify_all(); // notify the condition variable 
			}
			else if (m_verbose > 0)
			{
				std::cout << "EventBus::invokeEvent Invalid callback in " << pFunctionName
					<< " " << std::this_thread::get_id() << "\n";
			}

			return true;
		}

	public:
		int m_maxCapacity;
		EventBus(int maxCapacity = 100) : m_maxCapacity(maxCapacity) {}
		bool isValid() override
		{
			return !m_EventHandlerMap.empty();
		}
		int getRunState();
		int getCallbacksCount();
		int getCallsCount();
		//make a new event call object and add to queue
		bool blockEvent(std::string pFunctionName, bool val=true);
		void stop();
		bool reset();
		bool hasCallback(const std::string & pFunctionName);

		template<typename T>
		int add(T dummy, std::string pFunctionName, std::function<void(T)> functionObject);
		//addEventHandler if needed, also used to block objects
		template<typename T>
		int add(std::string pFunctionName, std::function<void(T)> functionObject);
		//back to simple, varaiadic templated on my reading list
		bool invokeEvent(const std::string & callBackName)
		{
			if (m_verbose > 0)
			{
				std::cout << " EventBus::invokeEvent dispatch no args" << std::this_thread::get_id() << "\n";
			}
			return invokeEventInternal(callBackName, false, true);
		}
		bool invokeEvent(const std::string & callBackName, const std::string functionArgument="")
		{
			if (m_verbose > 0)
			{
				std::cout << "EventBus::invokeEvent dispatch args" << functionArgument
					<< std::this_thread::get_id() << "\n";
			}
			return invokeEventInternal(callBackName, functionArgument, false);
		}

		/// @brief Event loop execution function. 
		/// @details When a new eventCall is added to queue the condition variable
		///          signals and the thread will continue execution of the event loop. 
		///          This function exits when stop() is called.
		///          After stopping use reset() member function to enable the state for it
		///          to execute again
		void run()
		{
			while (true)
			{
				//	lock
				while (!m_eventCallPtrs.empty())
				{
					if (m_stopped == 1)
					{
						std::cout << "EventBus::run  m_stoppedthread " << std::this_thread::get_id() << "\n";
						break;
					}
					else if (m_stopped == 2)
					{
						std::cout << " EventBus::run  quit " << std::this_thread::get_id() << "\n";
						return;
					}
					if (m_verbose > 0)
					{
						std::cout << "EventBus::run  processing " <<
							m_eventCallPtrs.data.front()->isValid() << " : " <<
							m_eventCallPtrs.data.front()->m_name << m_stopped <<
							" " << std::this_thread::get_id() << "\n";
						m_eventCallPtrs.data.front()->m_verbose = 1;//DEBUG
					}
					//
					if (m_eventCallPtrs.data.front()->isValid() &&
						m_eventCallPtrs.data.front()->getRunState() != RunState::blocked)
					{
						//bool res = m_eventCallPtrs.data.front()->dispatchAll();
						bool res = m_eventCallPtrs.data.front()->dispatchAllCalls();
					}
					m_eventCallPtrs.pop();
				}//while
				//wait
				std::unique_lock<std::mutex> lk(m_runMtx);//locked?
				m_cond.wait(lk, [&] {return m_stopped == 1; });//true: wake up
				if (m_stopped == 2)
				{
					return;
				}
				//run
				m_stopped = 0;
				lk.unlock();
				if (m_verbose > 0)
				{
					std::cout << " EventBus::run :: wakeup queue " <<
						m_eventCallPtrs.size()<<" "
						<< m_stopped << " " << std::this_thread::get_id() << "\n";
				}
			}
			return;
		}
	};
		//UNfortunately could not  get this to properly work  and run out of time, TODO
		/*
		template <class... Args>
		auto invoke(const std::string & callBackName, Args... args) -> decltype(dispatch(args...))
		{
			return dispatch(callBackName, args...);
		}
		bool invoke(const std::string & callBackName)
		{
			return invokeEventInternal(callBackName, false, true);
		}
		bool invoke(const std::string & callBackName,
			const std::string & functionArgument="")
		{
			return invokeEventInternal(callBackName, functionArgument, false);
		}*/
	
	
	//addEventHandler if needed, can also block invalid objects
		//last minute hack to fix the type, sorry
	// 0 if disabled 1 if added -1 failed
	template<typename T>
	int EventBus::add(std::string pFunctionName, std::function<void(T)> functionObject)
	{
		bool handlerExists = hasCallback(pFunctionName);
		if (!handlerExists)
		{
			m_EventHandlerMap.push(pFunctionName,
				std::shared_ptr<EventHandler>(new EventHandler(pFunctionName)));
		}
		if (!hasCallback(pFunctionName))
		{
			return false;
		}
		if (m_verbose > 1)
		{
			std::cout << " EventBus::add :: pFunctionName " <<
				pFunctionName << " " << std::this_thread::get_id() << "\n";
		}
		//addEvent to handler
		bool res = blockEvent(pFunctionName, false);
		std::shared_ptr <EventBase> eventBasePtr(new Event<T>(pFunctionName));
		if (!eventBasePtr)
		{
			return -1;
		}
		Event<T> * eventPtr = dynamic_cast<Event<T> *>(eventBasePtr.get());
		if (!eventPtr)
		{
			if (m_verbose > 0)
			{
				std::cout << " EventBus::add :: failed " <<
					pFunctionName << " " << std::this_thread::get_id() << "\n";
			}
			return -1;
		}

		//will set blocked state for invalid objects
		eventPtr->setCallback(functionObject);
		if (eventPtr->isValid())
		{
			auto it = m_EventHandlerMap.data.find(pFunctionName);
			if ((*it).second.get())
			{
				(*it).second->addEvent(eventBasePtr);
			}
			else
			{
				if (m_verbose > 0)
				{
					std::cout << " EventBus::add :: failed2 " <<
						pFunctionName << " " << std::this_thread::get_id() << "\n";
				}
				return -1;
			}
		}
		else
		{
			if (m_verbose > 0)
			{
				std::cout << " EventBus::add :: failed2 " <<
					pFunctionName << " " << std::this_thread::get_id() << "\n";
			}
			return -1;
		}
		return 1;
	}
	/*
	template <class T>
	int EventBus:::add(T dummy, std::string pFunctionName, std::function<void(T)> functionObject)
	{
		return EventBus::add(pFunctionName, functionObject);
	}
	*/
	//Event imple TODO move has trouble namespace
	template <class T>
	bool Event<T>::dispatch()
	{
		return invokeInternal(false, true);
	}
	template <class T>
	template <class A>
	bool Event<T>::dispatch(A functionArgument)
	{
		return invokeInternal(functionArgument, false);
	}
	template <class T>
	RunState Event<T>::getRunState()
	{
		return getRunStateAsEnum(m_aRunState);
	}
	template <class T>
	void Event<T>::setResultState(const ResultState & val)
	{
		m_aResultState = valueFromEnum(val);
	}
	template <class T>
	void Event<T>::setRunState(const RunState & val)
	{
		m_aRunState = valueFromEnum(val);
	}
	template <class T>
	ResultState Event<T>::getResultState()
	{
		return getResultStateAsEnum(m_aResultState);
	}

	template <class T>
	bool Event<T>::invokeWithContainerArg(
		std::shared_ptr<ArgumentContainerBase> argConPtr)
	{
		if (argConPtr.get())
		{ 
			std::cout << "Event<T>::invokeWithContainerArg No valid argument container "
				<< m_name << " " << std::this_thread::get_id() << "\n";
		}
		//skipping blocked objects
		if (getRunState() == RunState::blocked 
			|| getResultState() == ResultState::invalid)
		{
			return false;
		}
		setResultState(ResultState::failed);
		setRunState(RunState::running);

		std::lock_guard<std::mutex> l(m_dataMtx);
		if (argConPtr.get() && argConPtr->m_argsType != ArgsTypes::voidType)
		{
			T functionArgument;
			bool isVoid = false;
			bool res = getContainerArgument(argConPtr.get(), functionArgument, isVoid);//TODO change to enum
			//TODO use return invokeInternal(functionArgument, isVoid);
			if (m_verbose > 0)
			{
				std::cout << "EventBus::invokeEvent No argument container " << m_name
					<< " "<< functionArgument << std::this_thread::get_id() << "\n";
			}
			try
			{
				m_Callback(functionArgument);
				setRunState(RunState::notRunning);
				setResultState(ResultState::success);
				return true;
			}
			catch (...)
			{
				g_excPtr = std::current_exception();
			}
		}
		else //if (argConPtr.get())
		{
			if (m_verbose > 0)
			{
				std::cout << "Event<T>:: dispatching void " << m_name
					<< " " << std::this_thread::get_id() << "\n";
			}
			try
			{
				m_Callback;
				setRunState(RunState::notRunning);
				setResultState(ResultState::success);
				return true;
			}
			catch (...)
			{
				g_excPtr = std::current_exception();
			}
		}
		setRunState(RunState::notRunning);
		return false;
	}

	//Inital implementation, should use container as argument
	template <class T>
	bool Event<T>::invokeInternal(T functionArgument, bool isVoid)
	{
		if (m_verbose > 0)
		{
			std::cout << "Event<T>::dispatching "
				<< isValid() << " from " << m_name << " with argument " <<
				functionArgument << " " << std::this_thread::get_id() << "\n";
		}
		//skipping blocked objects
		if (getRunState() == RunState::blocked || getResultState() == ResultState::invalid)
		{
			if (m_verbose > 0)
			{
				std::cout << " Event<T>::invokeInternal _Skipping blocked object " << m_name
					<< " " << std::this_thread::get_id() << "\n";
			}
			return false;
		}
		//set state
		setResultState(ResultState::failed);
		setRunState(RunState::running);
		std::lock_guard<std::mutex> l(m_dataMtx);
		if (isVoid)
		{
			try
			{
				m_Callback;
			}
			catch (...)
			{
				g_excPtr = std::current_exception();
			}
		}
		else
		{
			try
			{
				m_Callback(functionArgument);
				setRunState(RunState::notRunning);
				setResultState(ResultState::success);
				return true;
			}
			catch (...)
			{
				g_excPtr = std::current_exception();
			}
		}
		setRunState(RunState::notRunning);
		return false;
	}

	//will set blocked state for invalid objects, can be void now
	template <class T>
	bool Event<T>::setCallback(std::function<void(T)> callback)
	{
		setRunState(RunState::ready);
		//not possible to distinguish invalid object and nullptr here
		if (!callback)
		{
			setRunState(RunState::blocked);
		}
		if (std::is_same<T, void>::value)//??? TODO
		{
			m_argtype = ArgsTypes::voidType;
		}
		else if (typeid(T) == typeid(std::string))
		{
			m_argtype = ArgsTypes::stringType;
		}
		else if (typeid(T) == typeid(int))
		{
			m_argtype = ArgsTypes::intType;
		}
		else
		{
			//unsupported
		}
		std::lock_guard<std::mutex> l(m_dataMtx);
		m_Callback = callback;
		return true;
	}
}//namespace

#endif