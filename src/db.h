#ifndef _DB_H_
#define _DB_H_

#include "main.h"


bool persistGrids(Poco::Data::Session* session_in_transaction, const std::string& guid, const restbed::Bytes& body);

bool persist(const std::string& guid, const Poco::UInt32* levels, Poco::UInt32 score, Poco::UInt32 bonus, const std::string& name, const restbed::Bytes& body);

bool persist(const std::string& guid, const Poco::UInt32* levels, Poco::UInt32 score, const std::string& name);

#endif // _DB_H_
