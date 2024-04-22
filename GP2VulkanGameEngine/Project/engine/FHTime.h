#pragma once
#include <chrono>

namespace FH
{
	class Time final
	{
	public:
		static float GetDeltaTime() { return DELTATIME; }
		static float GetElapsedTime() { return ELAPSEDTIME; }

		static void UpdateTime();

		Time() = default;
		~Time() = default;
		Time(const Time& other) = delete;
		Time(Time&& other) = delete;
		Time& operator=(const Time& other) = delete;
		Time& operator=(Time&& other) = delete;

	private:
		static inline float DELTATIME{ 0.01f };
		static inline float MAX_DELTATIME{ 0.2f };
		static inline float ELAPSEDTIME{ 0.01f };

		static inline std::chrono::steady_clock::time_point PREVTIME{ std::chrono::steady_clock::now() };
	};
}