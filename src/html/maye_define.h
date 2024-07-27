#pragma once

#define isspace _isspace
#define isalnum _isalnum
#define isalpha _isalpha

inline bool _isspace(int c)
{
	if (c == ' ' || c == '\t'|| c=='\v' || c=='\r' || c=='\n' ||
		c == '\f')
	{
		return true;
	}
	return false;
}

inline bool _isalnum(int c)
{
	if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9') {
		return true;
	}
	return false;
}

inline bool _isalpha(int c)
{
	return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z';
}


