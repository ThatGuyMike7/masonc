#ifndef _MASON_TEXT_H_
#define _MASON_TEXT_H_

// Returns a value different from zero (i.e. true) if an ASCII character is ' ' (whitespace), '\n' (new line), '\t' (horizontal tab) or '\r' (return).
// Otherwise returns zero (i.e. false).
extern int text_is_space(int c);

#endif