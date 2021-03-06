#include "sync_handler.h"

#include "db.h"

static const char HEX[] = "0123456789ABCDEF";


void sync_handler(const std::shared_ptr<restbed::Session> session)
{
	const std::shared_ptr<const restbed::Request> request = session->get_request();

	std::string param_guid = request->get_path_parameter("guid");

	Poco::UInt32 score = 0;
	Poco::UInt32 bonus = std::stoi(request->get_path_parameter("bonus"));
	Poco::UInt32 level[14];
	int i;
	char path_param_name[3];
	path_param_name[0] = 'l';
	path_param_name[2] = 0;
	for (i=0; i<14; i++)
	{
		path_param_name[1] = 'a'+i;
		level[i] = std::stoi(request->get_path_parameter(path_param_name));
		
		score += (level[i]<<(2*i)) * (i+1);
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
		closeConnection(session, response_status, response_body);
	}
	else
	{
		size_t length = request->get_header("Content-Length", 0);
		if (length > 0)
		{
			::log(stdout, "Provides %ld bytes\n", length);

			session->fetch(length, [param_guid,level,score,bonus,param_name](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body)
			{
				::log(stdout, "Within Lambda: %ld bytes\n", body.size());

				size_t i;
				for (i=0; i<body.size(); i++)
				{
					fputc(HEX[(body.at(i)&0xF0)>>4], stdout);
					fputc(HEX[body.at(i)&0x0F], stdout);
					if (39==(i%40))
					{
						fputc('\n', stdout);
					} else if (3==(i%4))
					{
						fputc(' ', stdout);
					}
				}
				fputc('*', stdout);
				fputc('\n', stdout);
				
				int response_status;
				std::string response_body;
				if (persist(param_guid, level, score, bonus, param_name, body) &&
				    render_highscore_list(level, score, true, param_guid, param_name, response_body))
				{
					response_status = restbed::OK;
				}
				else
				{
					response_status = restbed::SERVICE_UNAVAILABLE;
					response_body = "";
				}

				::log(stdout, "Response: %d %lu %s\n", response_status, response_body.size(), response_body.c_str());
				
				closeConnection(session, response_status, response_body);
			} );
		}
		else
		{
			::log(stdout, "Provided no body\n");

			if (render_highscore_list(level, score, true, param_guid, param_name, response_body)) {
				response_status = restbed::OK;
			}
			else
			{
				response_status = restbed::SERVICE_UNAVAILABLE;
				response_body = "";
			}

			closeConnection(session, response_status, response_body);
		}
	}
}
