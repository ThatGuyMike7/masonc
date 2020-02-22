#include <text.h>

int text_is_space(int c)
{
	return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}