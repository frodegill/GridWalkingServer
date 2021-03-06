#include "highscore_handler.h"

#include "db.h"


//DEPRECATED New clients calls sync_handler
void highscore_handler(const std::shared_ptr<restbed::Session> session)
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
	}
	else
	{
		if (persist(param_guid, level, score, param_name) &&
		    render_highscore_list(level, score, false, param_guid, param_name, response_body))
		{
			response_status = restbed::OK;
		}
		else
		{
			response_status = restbed::SERVICE_UNAVAILABLE;
			response_body = "";
		}
	}
	
	closeConnection(session, response_status, response_body);
}
