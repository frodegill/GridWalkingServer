#include <memory>
#include <cstdlib>
#include <restbed>

#include "main.h"
#include "secrets.h"


// Database
gridwalking::PocoGlue DB;

void log(const std::string& /*msg*/)
{
}

int crc(const std::string s)
{
	int sum1 = CRC_SEED1;
	int sum2 = CRC_SEED2;
	size_t i;
	for (i=0; i<s.length(); i++) { /* https://en.wikipedia.org/wiki/Fletcher's_checksum */
			sum1 = (sum1 + s[i]) % 255;
			sum2 = (sum2 + sum1) % 255;
	}
	return (sum2 << 8) | sum1;
}

bool persist(const std::string& guid, Poco::UInt32* levels, Poco::UInt32 score, const std::string& name)
{
	Poco::Data::Session* session_in_transaction;
	if (!DB.CreateSession(session_in_transaction))
		return false;
	
	int exist_count = 0;
	DEBUG_TRY_CATCH(*session_in_transaction << "SELECT COUNT(*) FROM user WHERE id=?",
		Poco::Data::Keywords::into(exist_count),
		Poco::Data::Keywords::useRef(guid),
		Poco::Data::Keywords::now;)

	if (exist_count == 0)
	{
		DEBUG_TRY_CATCH(*session_in_transaction << "INSERT INTO user (id, username, score, l13, l12, l11, l10, l9, l8, l7, l6, l5, "\
		                                                             "l4, l3, l2, l1, l0) VALUE (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
			Poco::Data::Keywords::useRef(guid),
			Poco::Data::Keywords::useRef(name),
			Poco::Data::Keywords::use(score),
			Poco::Data::Keywords::use(levels[13]),
			Poco::Data::Keywords::use(levels[12]),
			Poco::Data::Keywords::use(levels[11]),
			Poco::Data::Keywords::use(levels[10]),
			Poco::Data::Keywords::use(levels[9]),
			Poco::Data::Keywords::use(levels[8]),
			Poco::Data::Keywords::use(levels[7]),
			Poco::Data::Keywords::use(levels[6]),
			Poco::Data::Keywords::use(levels[5]),
			Poco::Data::Keywords::use(levels[4]),
			Poco::Data::Keywords::use(levels[3]),
			Poco::Data::Keywords::use(levels[2]),
			Poco::Data::Keywords::use(levels[1]),
			Poco::Data::Keywords::use(levels[0]),
			Poco::Data::Keywords::now;)
	}
	else
	{
		DEBUG_TRY_CATCH(*session_in_transaction << "UPDATE user SET username=?, score=?, l13=?, l12=?, l11=?, l10=?, l9=?, l8=?, "\
		                                                           "l7=?, l6=?, l5=?, l4=?, l3=?, l2=?, l1=?, l0=? WHERE id=?",
			Poco::Data::Keywords::useRef(name),
			Poco::Data::Keywords::use(score),
			Poco::Data::Keywords::use(levels[13]),
			Poco::Data::Keywords::use(levels[12]),
			Poco::Data::Keywords::use(levels[11]),
			Poco::Data::Keywords::use(levels[10]),
			Poco::Data::Keywords::use(levels[9]),
			Poco::Data::Keywords::use(levels[8]),
			Poco::Data::Keywords::use(levels[7]),
			Poco::Data::Keywords::use(levels[6]),
			Poco::Data::Keywords::use(levels[5]),
			Poco::Data::Keywords::use(levels[4]),
			Poco::Data::Keywords::use(levels[3]),
			Poco::Data::Keywords::use(levels[2]),
			Poco::Data::Keywords::use(levels[1]),
			Poco::Data::Keywords::use(levels[0]),
			Poco::Data::Keywords::useRef(guid),
			Poco::Data::Keywords::now;)
	}

	DB.ReleaseSession(session_in_transaction, gridwalking::PocoGlue::COMMIT);
	return true;
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

void post_method_handler(const std::shared_ptr<restbed::Session> session)
{
	const std::shared_ptr<const restbed::Request> request = session->get_request();

	std::string param_guid = request->get_path_parameter("guid");

	Poco::UInt32 score = 0;
	Poco::UInt32 level[14];
	int i;
	char path_param_name[3];
	path_param_name[0] = 'l';
	path_param_name[2] = 0;
	for (i=0; i<14; i++)
	{
		path_param_name[1] = 'a'+i;
		level[i] = std::stoi(request->get_path_parameter(path_param_name));
		
		score += level[i]<<(2*i);
	}

	std::string param_name = request->get_path_parameter("name");

	int param_crc = 0;
	if (request->has_query_parameter("crc"))
	{
		param_crc = std::stoi(request->get_query_parameter("crc"));
	}

	std::string path = request->get_path();
	int calculated_crc = crc(path);

	int response_status;
	std::string response_body;
	if (calculated_crc != param_crc)
	{
		response_status = restbed::NOT_ACCEPTABLE;
		response_body = "CRC error";
#ifdef DBG
		response_body += ". Expected "+std::to_string(calculated_crc);
#endif
	}
	else
	{
		if (persist(param_guid, level, score, param_name) &&
		    render_highscore_list(level, score, param_name, response_body))
		{
			response_status = restbed::OK;
		}
		else
		{
			response_status = restbed::SERVICE_UNAVAILABLE;
			response_body = "";
		}
	}
	session->close(response_status, response_body, {{"Content-Length", std::to_string(response_body.size())}});
}

int main(int /*argc*/, char** /*argv*/)
{
	if (!DB.Initialize(DB_CONNECTION_STRING))
		return false;

	auto resource = std::make_shared<restbed::Resource>();
	resource->set_path("highscore/{guid: .*}/{ln: [0-9]*}/{lm: [0-9]*}/{ll: [0-9]*}/{lk: [0-9]*}/{lj: [0-9]*}"\
	                                       "/{li: [0-9]*}/{lh: [0-9]*}/{lg: [0-9]*}/{lf: [0-9]*}/{le: [0-9]*}"\
																				 "/{ld: [0-9]*}/{lc: [0-9]*}/{lb: [0-9]*}/{la: [0-9]*}/{name: .*}");
	resource->set_method_handler("POST", post_method_handler);

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
	service.publish(resource);
	service.start(settings);
	
	return EXIT_SUCCESS;
}
