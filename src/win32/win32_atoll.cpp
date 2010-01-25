#include <stdlib.h>
#include "win32_atoll.h"

__int64 atoll(const char* _String) {
	return _atoi64(_String);
}

