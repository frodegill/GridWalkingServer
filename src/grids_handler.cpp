#include "grids_handler.h"


void grids_handler(const std::shared_ptr<restbed::Session> session)
{
	const std::shared_ptr<const restbed::Request> request = session->get_request();

	std::string param_guid = request->get_path_parameter("guid");
    
	int response_status = restbed::OK;
	std::string response_body;

    Poco::Data::Session* db_session;
	if (!DB.CreateSession(db_session))
    {
        response_status = restbed::SERVICE_UNAVAILABLE;
        response_body = "";
    }
    else
    {
        std::ostringstream sb;
        
        int i;
        for (i=0; i<14; i++)
        {
            Poco::UInt32 grid;
            Poco::Data::Statement statement(*db_session);
            statement << "SELECT g.grid FROM user u, grid g WHERE u.guid=? AND u.id=g.owner AND g.level=?",
                Poco::Data::Keywords::into(grid),
                Poco::Data::Keywords::use(param_guid),
                Poco::Data::Keywords::use(i),
                Poco::Data::Keywords::range<Poco::Data::Limit::SizeT>(0,1);

            while (!statement.done() && 0<statement.execute())
            {
                append_uint32(sb, grid);
            }

            append_uint32(sb, 0xFFFFFFFF);
        }
        response_body = sb.str();
    }
	DB.ReleaseSession(db_session, gridwalking::PocoGlue::COMMIT);
	session->close(response_status, response_body, {{"Content-Length", std::to_string(response_body.size())}});
}
