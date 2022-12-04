#ifdef __OTSERV_OTCP_H__
#error "Precompiled header should be included only once."
#endif

#define __OTSERV_OTCP_H__
// Definitions should be global.
#include "definitions.h"

#ifdef __USE_OTPCH__

#if defined __WINDOWS__ || defined WIN32
#include <winerror.h>
#endif

//libxml
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/threads.h>
//boost
#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
//std
#include <list>
#include <vector>
#include <map>
#include <string>
//otserv
#include "thing.h"

#endif
