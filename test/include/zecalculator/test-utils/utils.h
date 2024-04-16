#pragma once

#include <chrono>

using namespace std::chrono;

/// @brief call 'f' for a given amount of time and return the number of times it got called
template <class F>
size_t loop_call_for(nanoseconds duration, F&& f)
{
  size_t iterations = 0;
  auto begin = high_resolution_clock::now();
  while (high_resolution_clock::now() - begin < duration)
  {
    f();
    iterations++;
  }

  return iterations;
}
