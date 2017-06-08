#include "main.h"
#include "secrets.h"

#include "grids_handler.h"
#include "highscore_handler.h"
#include "sync_handler.h"
#include "restbed/custom_logger.hpp"

#include "db.h"


void log_arg(FILE* stream, const char* format, va_list& arguments)
{
	char date_str[100];
	nowAsString(date_str, sizeof(date_str)/sizeof(date_str[0]));
	fprintf(stream, "%s: ", date_str);

	vfprintf(stream, format, arguments);
	fprintf(stream, "\n");
	fflush(stream);
}

void log(FILE* stream, const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	
	log_arg(stream, format, arguments);

	va_end(arguments);
}

void append_byte(restbed::Bytes& bytes, const uint8_t b)
{
	bytes.push_back(b);
}

void append_uint32(restbed::Bytes& bytes, uint32_t i)
{
	append_byte(bytes, (i&0xFF000000)>>24);
	append_byte(bytes, (i&0x00FF0000)>>16);
	append_byte(bytes, (i&0x0000FF00)>>8);
	append_byte(bytes, i&0x000000FF);
}

bool fetch_uint32(restbed::Bytes::const_iterator& iter, const restbed::Bytes::const_iterator& end, uint32_t& grid)
{
	grid = 0;
	uint8_t i;
	for (i=0; i<4; i++)
	{
		if (iter==end)
			return false;
		else
			grid = (grid<<8)|(*iter++);
	}
	return true;
}

int crc(const uint8_t* buffer, size_t length)
{
	int sum1 = CRC_SEED1;
	int sum2 = CRC_SEED2;
	size_t i;
	for (i=0; i<length; i++) { /* https://en.wikipedia.org/wiki/Fletcher's_checksum */
			sum1 = (sum1 + buffer[i]) % 255;
			sum2 = (sum2 + sum1) % 255;
	}
	return (sum2 << 8) | sum1;
}

int crc(const std::string s)
{
    return crc(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
}

bool render_highscore_list(const Poco::UInt32* user_levels, Poco::UInt32 user_score,
													 bool include_guid, const std::string& user_guid, const std::string& user_name, std::string& result)
{
	Poco::Data::Session* session;
	if (!DB.CreateSession(session))
		return false;
	
	Poco::UInt32 player_count = 0;
	DEBUG_TRY_CATCH(*session << "SELECT COUNT(*) FROM user",
		Poco::Data::Keywords::into(player_count),
		Poco::Data::Keywords::now;)

	Poco::UInt32 position = 0;
	DEBUG_TRY_CATCH(*session << "SELECT COUNT(*) FROM user WHERE score>?",
		Poco::Data::Keywords::into(position),
		Poco::Data::Keywords::use(user_score),
		Poco::Data::Keywords::now;)

	position++;

	result = std::to_string(position);
	result.append(";");
	result.append(std::to_string(player_count));
	result.append("\n");

	std::string guid;
	std::string name;
	Poco::UInt32 score;
	Poco::UInt32 levels[14];
	size_t max_highscore_entries = MAX_HIGHSCORE_ENTRIES;
	Poco::Data::Statement statement(*session);
	statement << "SELECT guid, username, score, l13, l12, l11, l10, l9, l8, l7, l6, l5, l4, l3, l2, l1, l0 "\
	             "FROM user ORDER BY score DESC LIMIT ?",
		Poco::Data::Keywords::into(guid),
		Poco::Data::Keywords::into(name),
		Poco::Data::Keywords::into(score),
		Poco::Data::Keywords::into(levels[13]),
		Poco::Data::Keywords::into(levels[12]),
		Poco::Data::Keywords::into(levels[11]),
		Poco::Data::Keywords::into(levels[10]),
		Poco::Data::Keywords::into(levels[9]),
		Poco::Data::Keywords::into(levels[8]),
		Poco::Data::Keywords::into(levels[7]),
		Poco::Data::Keywords::into(levels[6]),
		Poco::Data::Keywords::into(levels[5]),
		Poco::Data::Keywords::into(levels[4]),
		Poco::Data::Keywords::into(levels[3]),
		Poco::Data::Keywords::into(levels[2]),
		Poco::Data::Keywords::into(levels[1]),
		Poco::Data::Keywords::into(levels[0]),
		Poco::Data::Keywords::use(max_highscore_entries),
		Poco::Data::Keywords::range<Poco::Data::Limit::SizeT>(0,1);

	Poco::UInt32 old_score = 0;
	Poco::UInt32 old_position = 1;
	position = 0;
	while (!statement.done() && 0<statement.execute())
	{
		position++;
		if (old_score != score)
		{
			old_score = score;
			old_position = position;
		}
		append_result(old_position, levels, score, include_guid, guid, name, result);
	}

	if (position > MAX_HIGHSCORE_ENTRIES)
	{
		append_result(position, user_levels, user_score, include_guid, user_guid, user_name, result);
	}

	DB.ReleaseSession(session, gridwalking::PocoGlue::IGNORE);
	return true;
}

void append_result(Poco::UInt32 position, const Poco::UInt32* levels, Poco::UInt32 score,
									 bool include_guid, const std::string& guid, const std::string& username, std::string& result)
{
	result.append(std::to_string(position));
	result.append(";");
	int i;
	for (i=13; i>=0; i--)
	{
		result.append(std::to_string(levels[i]));
		result.append(";");
	}
	result.append(std::to_string(score));
	result.append(";");
	if (include_guid)
	{
		result.append(guid);
		result.append(";");
	}
	result.append(username);
	result.append("\n");
}

void printGreeting()
{
	Poco::Data::Session* session;
	if (!DB.CreateSession(session))
	{
		::log(stdout, "Creating session failed\n");
		return;
	}
	
	Poco::UInt32 highscore = 0;
	DEBUG_TRY_CATCH(*session << "SELECT MAX(score) FROM user",
		Poco::Data::Keywords::into(highscore),
		Poco::Data::Keywords::now;)

	::log(stdout, "GridWalkingServer operational. Current highscore:%d\n", highscore);
	DB.ReleaseSession(session, gridwalking::PocoGlue::IGNORE);
}

void nowAsString(char* buf, const size_t& buf_len)
{
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	::strftime(buf, buf_len-1, "%a, %d %b %Y %H:%M:%S %Z", &tm);
}

void closeConnection(const std::shared_ptr<restbed::Session> session, int response_status, const std::string& response_body)
{
	char date_str[100];
	nowAsString(date_str, sizeof(date_str)/sizeof(date_str[0]));
	session->close(response_status, response_body,
								 {{"Server", "Grid Walking Server"},
								  {"Date", date_str},
								  {"Content-Type", "text/plain; charset=utf-8"},
								  {"Content-Length", std::to_string(response_body.size())}});
}

void closeConnection(const std::shared_ptr<restbed::Session> session, int response_status, const restbed::Bytes& response_bytes)
{
	char date_str[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date_str, sizeof(date_str)/sizeof(date_str[0]), "%a, %d %b %Y %H:%M:%S %Z", &tm);

	session->close(response_status, response_bytes,
								 {{"Server", "Grid Walking Server"},
								  {"Date", date_str},
								  {"Content-Type", "text/plain; charset=utf-8"},
								  {"Content-Length", std::to_string(response_bytes.size())}});
}

int main(int /*argc*/, char** /*argv*/)
{
	if (!DB.Initialize(DB_CONNECTION_STRING))
		return EXIT_FAILURE;

	auto highscore_resource = std::make_shared<restbed::Resource>();
	highscore_resource->set_path("highscore/{guid: .*}/{ln: [0-9]*}/{lm: [0-9]*}/{ll: [0-9]*}/{lk: [0-9]*}/{lj: [0-9]*}"\
	                                       "/{li: [0-9]*}/{lh: [0-9]*}/{lg: [0-9]*}/{lf: [0-9]*}/{le: [0-9]*}"\
																				 "/{ld: [0-9]*}/{lc: [0-9]*}/{lb: [0-9]*}/{la: [0-9]*}/{name: .*}");
	highscore_resource->set_method_handler("POST", highscore_handler);

	auto sync_resource = std::make_shared<restbed::Resource>();
	sync_resource->set_path("sync/{guid: .*}/{bonus: [0-9]*}/{ln: [0-9]*}/{lm: [0-9]*}/{ll: [0-9]*}/{lk: [0-9]*}/{lj: [0-9]*}"\
	                                       "/{li: [0-9]*}/{lh: [0-9]*}/{lg: [0-9]*}/{lf: [0-9]*}/{le: [0-9]*}"\
																				 "/{ld: [0-9]*}/{lc: [0-9]*}/{lb: [0-9]*}/{la: [0-9]*}/{name: .*}");
	sync_resource->set_method_handler("POST", sync_handler);

	auto grids_resource = std::make_shared<restbed::Resource>();
	grids_resource->set_path("grids/{guid: .*}");
	grids_resource->set_method_handler("GET", grids_handler);

#ifdef SECURE
	auto ssl_settings = std::make_shared<restbed::SSLSettings>();
	ssl_settings->set_port(REST_PORT);
	ssl_settings->set_http_disabled(true);
	ssl_settings->set_sslv2_enabled(false);
	ssl_settings->set_sslv3_enabled(false);
	ssl_settings->set_tlsv1_enabled(false);
	ssl_settings->set_tlsv11_enabled(true);
	ssl_settings->set_tlsv12_enabled(true);
	ssl_settings->set_private_key(restbed::Uri("file://gill-roxrud.dyndns.org.key"));
	ssl_settings->set_certificate_chain(restbed::Uri("file://fullchain.cer"));
#endif

	auto settings = std::make_shared<restbed::Settings>();
	settings->set_root("gridwalking");
	settings->set_connection_timeout(std::chrono::seconds(10));
	settings->set_worker_limit(WORKER_THREADS);
#ifdef SECURE
	settings->set_ssl_settings(ssl_settings);
#else
    settings->set_port(REST_PORT);
#endif
	settings->set_default_header("Connection", "close");

	restbed::Service service;
	service.publish(highscore_resource);
	service.publish(sync_resource);
	service.publish(grids_resource);
  service.set_logger(make_shared<CustomLogger>());

	printGreeting();

	service.start(settings);
	
	return EXIT_SUCCESS;
}
