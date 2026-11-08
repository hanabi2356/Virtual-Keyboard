#pragma once
#include<Windows.h>
#include<typeinfo>

template<typename T>
class MonoSingleton
{
protected:
	MonoSingleton() = default;
	virtual ~MonoSingleton() = default;

	MonoSingleton(const MonoSingleton&) = delete;
	MonoSingleton& operator=(const MonoSingleton&) = delete;

private:
	static T* instance;
public:
	static T* GetInstance()
	{
		if (!instance)
		{
			instance = new T();
			static struct Deleter {
				~Deleter() {
					MonoSingleton<T>::DestroyManager();
				}
			} deleter;
		}
		return instance;
	}

	static void DestroyManager()
	{
		if (instance)
		{
			auto& id = typeid(*instance);

			delete instance;
			instance = nullptr;
		}
	}
};

template <typename T>
T* MonoSingleton<T>::instance = nullptr;