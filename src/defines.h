#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <memory>
#include <string>
#include <stdint.h>


namespace gridwalking
{

#define SERVER_LOCALE "nb_NO"

#define IdType   Poco::UInt32
#define TimeType Poco::Int64

#define UNUSED(x) /* make compiler happy */

#define EQUAL (0)

#define INVALID_ID (static_cast<Poco::UInt32>(0))

} // namespace gridwalking

#endif // _DEFINES_H_
