#ifndef _MAIN_H_
#define _MAIN_H_

#include <memory>
#include <cstdlib>
#include <restbed>

#include "poco_glue.h"

#define WORKER_THREADS (4)
#define REST_PORT      (1416)
#ifndef DBG
 #define SECURE
#endif

#define MAX_HIGHSCORE_ENTRIES (20)


extern gridwalking::PocoGlue DB;

void log_arg(FILE* stream, const char* format, va_list& arguments);
void log(FILE* stream, const char* format, ...);

void append_uint32(restbed::Bytes& bytes, uint32_t i);
bool fetch_uint32(restbed::Bytes::const_iterator& iter, const restbed::Bytes::const_iterator& end, uint32_t& grid);

int crc(const std::string s);

void nowAsString(char* buf, const size_t& buf_len);

bool render_highscore_list(const Poco::UInt32* user_levels, Poco::UInt32 user_score,
													 bool include_guid, const std::string& user_guid, const std::string& user_name, std::string& result);
void append_result(Poco::UInt32 position, const Poco::UInt32* levels, Poco::UInt32 score,
									 bool include_guid, const std::string& guid, const std::string& name, std::string& result);

void closeConnection(const std::shared_ptr<restbed::Session> session, int response_status, const std::string& response_body);
void closeConnection(const std::shared_ptr<restbed::Session> session, int response_status, const restbed::Bytes& response_bytes);

int main(int argc, char** argv);

#endif // _MAIN_H_
