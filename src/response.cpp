#include <stdio.h>
#include <cassert>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "common.h"
#include "response.h"

namespace aws { 
	
Response::Response()
	: theIsSuccessful(false)
{
}

Response::~Response()
{
}

} // end namespace
