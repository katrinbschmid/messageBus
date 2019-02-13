
#ifndef CONTAINER_WRAPPER_H
#define CONTAINER_WRAPPER_H

#include <iostream>
#include <type_traits>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <stdexcept>

namespace
{
	/// @brief minimal wrapper
	/// @details Provides an ability to add callbacks and  run them
	/// A bit of a can of worms probably wouldnt try again
	template<class T>
	class QueueWrapper
	{
	private:
		std::mutex m_dataMtx;
	public:
		std::queue<T> data;//make private
		QueueWrapper() = default;
		QueueWrapper& operator = (QueueWrapper&) = delete;
		void push(T value);
		void pop();
		bool empty();
		size_t size();
		void get(T& value);
	};

	template<class T>
	class VectorWrapper
	{
	private:
		std::mutex m_dataMtx;
	public:
		std::vector<T> data;//make private
		VectorWrapper() = default;
		VectorWrapper& operator = (VectorWrapper&) = delete;
		void emplace_back(T value);
		void set(int i, T value);
		void get(int i, T & value);
		bool empty();
		size_t size();
		void resize(int val);
	};

	template<class S, class T>
	class UnordMapWrapper
	{
	private:
		std::mutex m_dataMtx;
	public:
		std::unordered_map<S, T> data;////make private
		UnordMapWrapper() = default;
		UnordMapWrapper& operator = (UnordMapWrapper&) = delete;
		void push(S key, T value);
		void get(S key, T& value);
		bool empty();
		bool has(T value);
		size_t size();
	};
	template<class S, class T>
	void UnordMapWrapper<S, T>::get(S key, T & value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		value = this->data[key];
		return;
	}
	template<class S, class T>
	void UnordMapWrapper<S, T>::push(S key, T value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data[key] = value;
		return;
	}

	template<class S, class T>
	bool UnordMapWrapper<S, T>::has(T value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.find(value) == this->data.end();
	}
	template<class S, class T>
	bool UnordMapWrapper<S, T>::empty()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.empty();
	}
	template<class S, class T>
	size_t UnordMapWrapper<S, T>::size()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.size();
	}

	template<class T>
	void VectorWrapper<T>::get(int i, T &value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		value = this->data[i];
		return;
	}

	template<class T>
	void VectorWrapper<T>::emplace_back(T value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data.emplace_back(value);
		return;
	}

	template<class T>
	void VectorWrapper<T>::set(int i, T value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data[i] = value;
		return;
	}
	template<class T>
	bool VectorWrapper<T>::empty()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.empty();
	}

	template<class T>
	void VectorWrapper<T>::resize(int val)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data.resize(val);
		return;
	}
	template<class T>
	size_t VectorWrapper<T>::size()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.size();
	}

	template<class T>
	void QueueWrapper<T>::pop() {
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data.pop();
	}
	template<class T>
	void QueueWrapper<T>::push(T value)
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		this->data.push(value);
	}
	template<class T>
	void QueueWrapper<T>::get(T& value) {
		std::lock_guard<std::mutex> lk(m_dataMtx);
		value = this->data.front();
		this->data.pop();
	}
	template<class T>
	bool QueueWrapper<T>::empty()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.empty();
	}
	template<class T>
	size_t QueueWrapper<T>::size()
	{
		std::lock_guard<std::mutex> lk(m_dataMtx);
		return this->data.size();
	}

}
#endif