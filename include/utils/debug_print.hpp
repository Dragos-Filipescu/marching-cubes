#pragma once
#ifndef MARCHING_CUBES_UTILS_DEBUG_PRINT_HPP
#define MARCHING_CUBES_UTILS_DEBUG_PRINT_HPP

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

 /*
  * Concatenate preprocessor tokens A and B after macro-expanding them.
  */
#define PPCAT(A, B) PPCAT_NX(A, B)

  /*
   * Turn A into a string literal without expanding macro definitions
   * (however, if invoked from a macro, macro arguments are expanded).
   */
#define STRINGIZE_NX(A) #A

   /*
    * Turn A into a string literal after macro-expanding it.
    */
#define STRINGIZE(A) STRINGIZE_NX(A)

#ifdef ENABLE_DEBUG_PRINT
#include <print> // C++23
#include <string_view>

#define DEBUG_PRINT(Category, Fmt, ...) \
        std::println(STRINGIZE(PPCAT("[{} Debug] ", Fmt)), Category, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

#endif // !MARCHING_CUBES_UTILS_DEBUG_PRINT_HPP

