#include "timer.h"

Timer::Timer()
{
    // Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void Timer::start()
{
    // Start the timer
    mStarted = true;

    // Unpause the timer
    mPaused = false;

    // Get the current clock time
    mStartTicks = SDL_GetTicks64();
    mPausedTicks = 0;
}

void Timer::stop()
{
    // Stop the timer
    mStarted = false;

    // Unpause the timer
    mPaused = false;

    // Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void Timer::pause()
{
    // If the timer is running and isn't already paused
    if (mStarted && !mPaused)
    {
        // Pause the timer
        mPaused = true;

        // Calculate the paused ticks
        mPausedTicks = SDL_GetTicks64() - mStartTicks;
        mStartTicks = 0;
    }
}

void Timer::unpause()
{
    // If the timer is running and paused
    if (mStarted && mPaused)
    {
        // Unpause the timer
        mPaused = false;

        // Reset the starting ticks
        mStartTicks = SDL_GetTicks64() - mPausedTicks;

        // Reset the paused ticks
        mPausedTicks = 0;
    }
}

uint32_t Timer::getTicks()
{
    // The actual timer time
    uint32_t time = 0;

    // If the timer is running
    if (mStarted)
    {
        // If the timer is paused
        if (mPaused)
        {
            // Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            // Return the current time minus the start time
            time = SDL_GetTicks64() - mStartTicks;
        }
    }

    return time;
}

bool Timer::isStarted()
{
    // Timer is running and paused or unpaused
    return mStarted;
}

bool Timer::isPaused()
{
    // Timer is running and paused
    return mPaused && mStarted;
}
