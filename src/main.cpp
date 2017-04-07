#include "main.h"
#include "secrets.h"

#include "grids_handler.h"
#include "highscore_handler.h"
#include "sync_handler.h"

#include "db.h"


void log(const std::string& /*msg*/)
{
}

void append_byte(std::ostringstream& sb, const uint8_t b)
{
	sb << b;
}

void append_uint32(std::ostringstream& sb, uint32_t i)
{
	append_byte(sb, (i&0xFF000000)>>24);
	append_byte(sb, (i&0x00FF0000)>>16);
	append_byte(sb, (i&0x0000FF00)>>8);
	append_byte(sb, i&0x000000FF);
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

bool render_highscore_list(Poco::UInt32* user_levels, Poco::UInt32 user_score, const std::string& user_name, std::string& result)
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

	std::string name;
	Poco::UInt32 score;
	Poco::UInt32 levels[14];
	size_t max_highscore_entries = MAX_HIGHSCORE_ENTRIES;
	Poco::Data::Statement statement(*session);
	statement << "SELECT username, score, l13, l12, l11, l10, l9, l8, l7, l6, l5, l4, l3, l2, l1, l0 "\
	             "FROM user ORDER BY score DESC LIMIT ?",
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
		append_result(old_position, levels, score, name, result);
	}

	if (position > MAX_HIGHSCORE_ENTRIES)
	{
		append_result(position, user_levels, user_score, user_name, result);
	}

	DB.ReleaseSession(session, gridwalking::PocoGlue::IGNORE);
	return true;
}

void append_result(Poco::UInt32 position, Poco::UInt32* levels, Poco::UInt32 score, const std::string& username, std::string& result)
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
	result.append(username);
	result.append("\n");
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
	sync_resource->set_path("highscore/{guid: .*}/{bonus: [0-9]*}/{ln: [0-9]*}/{lm: [0-9]*}/{ll: [0-9]*}/{lk: [0-9]*}/{lj: [0-9]*}"\
	                                       "/{li: [0-9]*}/{lh: [0-9]*}/{lg: [0-9]*}/{lf: [0-9]*}/{le: [0-9]*}"\
																				 "/{ld: [0-9]*}/{lc: [0-9]*}/{lb: [0-9]*}/{la: [0-9]*}/{name: .*}");
	sync_resource->set_method_handler("POST", sync_handler);

	auto grids_resource = std::make_shared<restbed::Resource>();
	grids_resource->set_path("grids/{guid: .*}");
	grids_resource->set_method_handler("GET", grids_handler);

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

	auto settings = std::make_shared<restbed::Settings>();
	settings->set_root("gridwalking");
	settings->set_connection_timeout(std::chrono::seconds(10));
	settings->set_worker_limit(WORKER_THREADS);
	settings->set_ssl_settings(ssl_settings);
	settings->set_default_header("Connection", "close");

	restbed::Service service;
	service.publish(highscore_resource);
	service.publish(sync_resource);
	service.publish(grids_resource);
	service.start(settings);
	
	return EXIT_SUCCESS;
}
