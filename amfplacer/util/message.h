/**
 * @file   message.h
 * @author Yibo Lin
 * @date   Mar 2019
 */

#ifndef OPENPARF_UTIL_MESSAGE_H_
#define OPENPARF_UTIL_MESSAGE_H_

#include "util/namespace.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

OPENPARF_BEGIN_NAMESPACE


/// message type for print functions
enum MessageType { kNone = 0, kDebug = 1, kInfo = 2, kWarn = 3, kError = 4, kAssert = 5 };

void setMessageLevel(MessageType message_level);

/// print to screen (stderr)
int amfplacerPrint(MessageType m, const char *format, ...);
/// print to stream
int amfplacerPrintStream(MessageType m, FILE *stream, const char *format, ...);
/// core function to print formatted data from variable argument list
int amfplacerVPrintStream(MessageType m, FILE *stream, const char *format, va_list args);
/// format to a buffer
int amfplacerSPrint(MessageType m, char *buf, const char *format, ...);
/// core function to format a buffer
int amfplacerVSPrint(MessageType m, char *buf, const char *format, va_list args);
/// format prefix
int amfplacerSPrintPrefix(MessageType m, char *buf);

/// assertion
void amfplacerPrintAssertMsg(const char *expr, const char *fileName,
                            unsigned lineNum, const char *funcName,
                            const char *format, ...);
void amfplacerPrintAssertMsg(const char *expr, const char *fileName,
                            unsigned lineNum, const char *funcName);

#define amfplacerAssertMsg(condition, args...)                                  \
  do {                                                                         \
    if (!(condition)) {                                                        \
      ::OPENPARF_NAMESPACE::amfplacerPrintAssertMsg(                            \
          #condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, args);          \
      abort();                                                                 \
    }                                                                          \
  } while (false)
#define amfplacerAssert(condition)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      ::OPENPARF_NAMESPACE::amfplacerPrintAssertMsg(                            \
          #condition, __FILE__, __LINE__, __PRETTY_FUNCTION__);                \
      abort();                                                                 \
    }                                                                          \
  } while (false)

/// static assertion
template <bool> struct amfplacerStaticAssert;
template <> struct amfplacerStaticAssert<true> {
  explicit amfplacerStaticAssert(const char * = NULL) {}
};

#define amfplacerDebugMileStone()                                                                                       \
  do {                                                                                                                 \
    ::OPENPARF_NAMESPACE::amfplacerPrint(kDebug, "%s:%d\n", __FILE__, __LINE__);                                        \
  } while (false)

// Q: What does `##__VA_ARGS__` mean?
// A: https://stackoverflow.com/questions/52891546/what-does-va-args-mean
//    Some compilers offer an extension that allows ## to appear after a
//    comma and before __VA_ARGS__, in which case the ## does nothing when the
//    variable arguments are present, but removes the comma when the variable
//    arguments are not present: this makes it possible to define macros such as
//    fprintf (stderr, format, ##__VA_ARGS__)
#define amfplacerDebugMileStoneMsg(format, ...)                                                                         \
  do {                                                                                                                 \
    ::OPENPARF_NAMESPACE::amfplacerPrint(kDebug, "%s:%d: " format "\n", __FILE__, __LINE__, ##__VA_ARGS__);             \
  } while (false)

OPENPARF_END_NAMESPACE

#endif   // OPENPARF_UTIL_MESSAGE_H_
