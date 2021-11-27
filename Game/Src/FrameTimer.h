#pragma once

#include <chrono>

class FrameTimer
{
private:
	class InternalTimer
	{
		friend FrameTimer;
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end;
		std::chrono::microseconds m_duration;

		static InternalTimer s_timer;
		static uint64_t m_frameCount;

	private:
		InternalTimer();
		~InternalTimer() = default;

		InternalTimer(const InternalTimer&) = delete;
		InternalTimer& operator=(const InternalTimer&) = delete;

		void start();
		void stop();
	};



public:
	FrameTimer() = default;
	~FrameTimer() = default;

	enum class Duration
	{
		SECONDS,
		MILLISECONDS,
		MICROSECONDS
	};


	static double NewFrame();

	// Print duration in milliseconds and seconds
	static void print(std::wstring headerMsg);

	// Get duration: MILLISECONDS, SECONDS
	static double getTime(Duration duration);

	// Shorthand for get duration seconds
	static double dt();
	static uint64_t frame();
};

