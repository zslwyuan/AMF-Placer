/**
 * @file   message.cpp
 * @author Yibo Lin
 * @date   Mar 2019
 */

#include "util/message.h"

OPENPARF_BEGIN_NAMESPACE

static MessageType message_level_ = MessageType::kDebug;

void setMessageLevel(MessageType message_level){
    message_level_ = message_level;
}

int amfplacerPrint(MessageType m, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = amfplacerVPrintStream(m, stderr, format, args);
  va_end(args);

  return ret;
}

int amfplacerPrintStream(MessageType m, FILE *stream, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = amfplacerVPrintStream(m, stream, format, args);
  va_end(args);

  return ret;
}

int amfplacerVPrintStream(MessageType m, FILE *stream, const char *format,
                         va_list args) {
    if (m < message_level_) { return 0; }
    // print prefix
  char prefix[16];
  amfplacerSPrintPrefix(m, prefix);
  fprintf(stream, "%s", prefix);

  // print message
  int ret = vfprintf(stream, format, args);

  return ret;
}

int amfplacerSPrint(MessageType m, char *buf, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = amfplacerVSPrint(m, buf, format, args);
  va_end(args);

  return ret;
}

int amfplacerVSPrint(MessageType m, char *buf, const char *format,
                    va_list args) {
    if (m < message_level_) { return 0; }
    // print prefix
    char prefix[16];
    amfplacerSPrintPrefix(m, prefix);
    sprintf(buf, "%s", prefix);

    // print message
    int ret = vsprintf(buf + strlen(prefix), format, args);

    return ret;
}

int amfplacerSPrintPrefix(MessageType m, char *prefix) {
  switch (m) {
  case kNone:
    return sprintf(prefix, "%c", '\0');
  case kInfo:
    return sprintf(prefix, "[INFO   ] ");
  case kWarn:
    return sprintf(prefix, "[WARNING] ");
  case kError:
    return sprintf(prefix, "[ERROR  ] ");
  case kDebug:
    return sprintf(prefix, "[DEBUG  ] ");
  case kAssert:
    return sprintf(prefix, "[ASSERT ] ");
  default:
    amfplacerAssertMsg(0, "unknown message type");
  }
}

void amfplacerPrintAssertMsg(const char *expr, const char *fileName,
                            unsigned lineNum, const char *funcName,
                            const char *format, ...) {
  // construct message
  char buf[1024];
  va_list args;
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  // print message
  amfplacerPrintStream(kAssert, stderr, "%s:%u: %s: Assertion `%s' failed: %s\n",
                      fileName, lineNum, funcName, expr, buf);
}

void amfplacerPrintAssertMsg(const char *expr, const char *fileName,
                            unsigned lineNum, const char *funcName) {
  // print message
  amfplacerPrintStream(kAssert, stderr, "%s:%u: %s: Assertion `%s' failed\n",
                      fileName, lineNum, funcName, expr);
}

OPENPARF_END_NAMESPACE
