// Minimal stand-ins for the few engine symbols the SQF evaluator needs, so the
// persistence tests can link engine/Evaluator/express.cpp without the whole engine.
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <strings.h>
namespace Poseidon { namespace Foundation {
void ErrorMessage(const char* format, ...) { va_list a; va_start(a, format); fprintf(stderr, "ERROR: "); vfprintf(stderr, format, a); fprintf(stderr, "\n"); va_end(a); }
} }
int strcmpi(const char* a, const char* b) { return strcasecmp(a, b); }
char* strlwr(char* s) { for (char* p = s; *p; p++) *p = (char)tolower((unsigned char)*p); return s; }
namespace Poseidon { namespace Foundation { unsigned GlobalTickCount() { return 0; } } }
