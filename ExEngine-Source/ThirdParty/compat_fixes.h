#pragma once

// Compatibility fixes for C++26 with macOS SDK
#ifdef __APPLE__
#ifndef _RSIZE_T
#define _RSIZE_T
typedef __SIZE_TYPE__ rsize_t;
#endif
#endif


// LUA ADJUSTMENTS TO WORK WITH C++ 26
// C++26 fix: optional<T&> doesn't have construct() method, use direct assignment
// this->construct(std::forward<Args>(args)...); replaced by
// m_value = std::addressof(args...);
// return *m_value;