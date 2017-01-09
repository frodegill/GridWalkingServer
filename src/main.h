#ifndef _MAIN_H_
#define _MAIN_H_

#include "poco_glue.h"

#define WORKER_THREADS (4)
#define REST_PORT      (1416)

#define MAX_HIGHSCORE_ENTRIES (10)


extern gridwalking::PocoGlue DB;

void log(const std::string& msg);
int crc(const std::string s);

bool persist(const std::string& guid, Poco::UInt32* levels, Poco::UInt32 score, const std::string& name);
bool render_highscore_list(Poco::UInt32* user_levels, Poco::UInt32 user_score, const std::string& user_name, std::string& result);
void append_result(Poco::UInt32 position, Poco::UInt32* levels, Poco::UInt32 score, const std::string& name, std::string& result);

int main(int argc, char** argv);

#endif // _MAIN_H_
