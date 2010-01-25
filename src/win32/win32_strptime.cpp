/*
 * Copyright 2010 Forrest Heller
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* implements most of strptime on Windows - definitely copy and pastable */
/*
Big time specifiers not supported:
-W,U,V (weeks in the year)
-X,x (local date/time)

Possible problems for you [someone who wants to use this]:
-This is a rather rigid implementation and DOES NOT account for locales
-It has undefined behavior for non-exact matching abbreviations (matches 3-letter and full day names)
*/
/* references:
http://www.opengroup.org/onlinepubs/009695399/functions/strptime.html
http://linux.die.net/man/3/strptime
http://www.gnu.org/software/hello/manual/libc/Formatting-Calendar-Time.html
*/
#include <windows.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "win32_strptime.h"

#ifndef ARRAYSIZE
#	define ARRAYSIZE(x) (sizeof(A)/sizeof((A)[0]))
#endif
static const char* kDaysFull[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static const char* kDaysAbbr[] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"
};
static const char* kMonthsFull[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};
static const char* kMonthsAbbr[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char* kAMPM[] = {
	"AM","a.m.","PM","p.m"
};
const int kYearBase = 1900;

static int get_string_match(const char **s, const char *strings[], int strings_count);
static int fill_member(int* value, const char **s, int min, int max);
static int fill_time(time_t* value, const char **s);

#ifdef WIN32
/* Windows calls it's version of gmtime_r gmtime_s */
static void gmtime_r();
static void gmtime_r(tm *_Tm, const time_t *_Time) {
	gmtime_s(_Tm, _Time);
}
#endif
char* strptime(const char* buf, const char* format, struct tm* tm) {
	const char* s = buf;
	int i;
	time_t t;
	/* strategy: look through the format until we find a '%'
	Then, determine the format string, then parse the number */

	while(*format != NULL && *s != NULL) {

		if (*format == '%') {
format_table: /* jumps here from 'O' and 'E' -- but nothing else*/
			switch(*++format) {
				/* "specification modifiers" E and O */
					case 'O':
					case 'E':
						/* remember that format pointer gets incremented inside switch() */
						goto format_table;
						break;
					case '%':
						if (*s != '%') {
							return NULL;
						}
						s++;
						break;
						/* day of week */
					case 'a':
					case 'A':
						/* match full name FIRST */
						tm->tm_wday = get_string_match(&s,kDaysFull,ARRAYSIZE(kDaysFull));
						if (tm->tm_wday == -1) {
							/* match abbr names */
							tm->tm_wday = get_string_match(&s,kDaysAbbr,ARRAYSIZE(kDaysAbbr));
							/* couldn't find a day */
							if (tm->tm_wday == -1) {
								return NULL;
							}
						}
						break;
						/* months */
					case 'b':
					case 'B':
					case 'h':
						/* match full name FIRST */
						tm->tm_mon = get_string_match(&s,kMonthsFull,ARRAYSIZE(kMonthsFull));
						if (tm->tm_mon == -1) {
							/* match abbr names */
							tm->tm_mon = get_string_match(&s,kMonthsAbbr,ARRAYSIZE(kMonthsAbbr));
							/* couldn't find a month */
							if (tm->tm_mon == -1) {
								return NULL;
							}
						}
						break;
						/* century */
					case 'C':
						if (fill_member(&i,&s,0,99) != 0) {
							return NULL;
						}
						tm->tm_year += i*100 - kYearBase;
						break;
						/* day of month */
					case 'e':
					case 'd':
						if (fill_member(&tm->tm_mday,&s,1,31) != 0) {
							return NULL;
						}
						break;
						/* compound: %m/%d/%y */
					case 'D':
						s = strptime(s,"%m/%d/%y",tm);
						if (s == NULL) {
							return NULL;
						}
						s++;
						break;
						/* 24 - hour [0,23] */
					case 'H':
					case 'k':
						if (fill_member(&tm->tm_hour,&s,0,23) != 0) {
							return NULL;
						}
						break;
						/* 1 - 12 hour */
					case 'I':
						if (fill_member(&tm->tm_hour,&s,1,12) != 0) {
							return NULL;
						}
						if (tm->tm_hour == 12) {
							tm->tm_hour = 0;
						}
						break;
						/* day of year */
					case 'j':
						if (fill_member(&tm->tm_yday,&s,1,366) != 0) {
							return NULL;
						}
						tm->tm_yday--;
						break;
						/* month */
					case 'm':
						if (fill_member(&tm->tm_mon,&s,1,12) != 0) {
							return NULL;
						}
						tm->tm_mon--;
						break;
						/* minute */
					case 'M':
						if (fill_member(&tm->tm_min,&s,0,59) != 0) {
							return NULL;
						}
						break;
						/* any whitespace */
					case 'n':
						while (*s != NULL && isspace(*s)) {
							s++;
						}
						s--;
						break;
						/* am/pm */
					case 'p':
						i = get_string_match(&s,kAMPM,ARRAYSIZE(kAMPM));
						if (i == -1) {
							return NULL;
						}
						/* PM string variations are stored in the upper-half of the array */
						if (i >= (ARRAYSIZE(kAMPM)/2)) {
							tm->tm_hour += 12;
						} else if (tm->tm_hour == 0) {
							tm->tm_hour = 12;
						}
						break;
						/* 12-hour clock time using the AM/PM notation */
					case 'r':
						s = strptime(s,"%I:%M:%S %p",tm);
						if (s == NULL) {
							return NULL;
						}
						s++;
						break;
						/* compound - %H:%M */
					case 'R':
						s = strptime(s,"%H:%M",tm);
						if (s == NULL) {
							return NULL;
						}
						s++;
						break;
						/* seconds - 61 for double leap seconds */
					case 'S':
						if (fill_member(&tm->tm_sec,&s,0,61) != 0) {
							return NULL;
						}
						break;
						/* compound: %H:%M:%S */
					case 'T':
						s = strptime(s,"%H:%M:%S",tm);
						if (s == NULL) {
							return NULL;
						}
						s++;
						break;
						/* dow */
					case 'w':
						if (fill_member(&tm->tm_wday,&s,0,6) != 0) {
							return NULL;
						}
						break;
					case 'Y':
						if (fill_member(&tm->tm_year,&s,0,INT_MAX) != 0) {
							return NULL;
						}
						tm->tm_year -= kYearBase;
						break;
					case 'y':
						if (fill_member(&tm->tm_year,&s,0,99) != 0) {
							return NULL;
						}
						/* The standard(s) mandates that all years  <= 68 
						are in the next century */
						if (tm->tm_year <= 68){
							tm->tm_year += 100;
						}
						break;
						/* common extensions */
						/* dow [1,7] */
					case 'u':
						if (fill_member(&tm->tm_wday,&s,1,7) != 0) {
							return NULL;
						}
						tm->tm_wday--;
						break;
						/* GNU extension - seconds since the epoc */
					case 's':
						if (fill_time(&t,&s) != 0) {
							return NULL;
						}
						gmtime_r(tm,&t);
						break;
						/* POSIX.2-1992 and by ISO C99.  */
					case 't':
						if (*s != '\t') {
							return NULL;
						}
						s++;
						break;
					default:
						return NULL;

			}
		} else if (*format != *s) {
			return NULL;
		} else {
			s++;
		}
		format++;
	}
	/* still not at the end of the date ?*/
	if (*format != NULL) {
		return NULL;
	}
	return const_cast<char*>(--s);
}
/**
 * Parses and advances the buffer s
 * @param value - pointer to the value of the parsed value of s
 * @param min - value must be >= min
 * @param max - value must be <= max
 * @return - 0 if success, non-0 if failure
 */
static int fill_member(int* value, const char **s, int min, int max) {
	*value = 0;
	char found_number = 0;
	for(; **s != NULL && isdigit(**s); (*s)++) {
		found_number = 1;
		*value *= 10;
		*value += **s - '0';
	}
	if (found_number == 1 && *value >= min && *value <= max) {
		return 0;
	}
	return 1;
}
/**
 * Works like fill_member, but with time_t--note that time_t is 64-bit on Windows
 */
static int fill_time(time_t* value, const char **s) {
	*value = 0;
	char found_number = 0;
	for(; **s != NULL && isdigit(**s); (*s)++) {
		found_number = 1;
		*value *= 10;
		*value += **s - '0';
	}
	if (found_number == 1) {
		return 0;
	}
	return 1;
}
/**
 * Checks to see if the next N characters of s match one of strings,
 * where N is the length of the particular strings[i]
 * @return - index of the string in the strings[] array that matches OR
 * -1 if no suitable match is found
 */
static int get_string_match(const char **s, const char *strings[], int strings_count) {
	int i;
	for(i = 0; i < strings_count && strings[i] != NULL; i++) {
		int string_size = strlen(strings[i]);
		if (_strnicmp(*s,strings[i],string_size) == 0) {
			*s += (string_size*sizeof(char));
			return i;
		}
	}
	return -1;
}
