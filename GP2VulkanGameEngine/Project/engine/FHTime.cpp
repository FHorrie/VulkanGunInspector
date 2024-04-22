#include "FHTime.h"

void FH::Time::UpdateTime()
{
	const std::chrono::duration<double, std::milli> deltaMillis{(std::chrono::steady_clock::now() - PREVTIME)};
	PREVTIME = std::chrono::steady_clock::now();

	DELTATIME = static_cast<float>(deltaMillis.count() / 1000.0f);

	if (DELTATIME >= MAX_DELTATIME)
		DELTATIME = MAX_DELTATIME;

	ELAPSEDTIME += DELTATIME;
}