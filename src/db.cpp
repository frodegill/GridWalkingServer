#include "db.h"


// Database
gridwalking::PocoGlue DB;


bool persistGrids(Poco::Data::Session* session_in_transaction, const std::string& guid, const restbed::Bytes& body)
{
	if (body.empty())
		return true;

	uint32_t user_id = 0;
	DEBUG_TRY_CATCH(*session_in_transaction << "SELECT id FROM user WHERE guid=?",
		Poco::Data::Keywords::into(user_id),
		Poco::Data::Keywords::useRef(guid),
		Poco::Data::Keywords::now;)

	restbed::Bytes::const_iterator iter = body.begin(), end=body.end();
	uint32_t grid;
	while (fetch_uint32(iter, end, grid) && 0xFFFFFFFF!=grid)
	{
		DEBUG_TRY_CATCH(*session_in_transaction << "DELETE FROM grid WHERE owner=? AND grid=?",
			Poco::Data::Keywords::use(user_id),
			Poco::Data::Keywords::use(grid),
			Poco::Data::Keywords::now;)
	}

	int i;
	for (i=0; i<14; i++)
	{
		while (fetch_uint32(iter, end, grid) && 0xFFFFFFFF!=grid)
		{
			DEBUG_TRY_CATCH(*session_in_transaction << "DELETE FROM grid WHERE owner=? AND grid=?",
				Poco::Data::Keywords::use(user_id),
				Poco::Data::Keywords::use(grid),
				Poco::Data::Keywords::now;)

			DEBUG_TRY_CATCH(*session_in_transaction << "INSERT INTO grid (owner, level, grid) VALUE (?,?,?)",
				Poco::Data::Keywords::use(user_id),
				Poco::Data::Keywords::use(i),
				Poco::Data::Keywords::use(grid),
				Poco::Data::Keywords::now;)
		}
	}

	return true;
}

bool persist(const std::string& guid, Poco::UInt32* levels, Poco::UInt32 score, Poco::UInt32 bonus, const std::string& name, const restbed::Bytes& body)
{
	if (0 == score)
		return true;

	Poco::Data::Session* session_in_transaction;
	if (!DB.CreateSession(session_in_transaction))
		return false;
	
	int exist_count = 0;
	DEBUG_TRY_CATCH(*session_in_transaction << "SELECT COUNT(*) FROM user WHERE guid=?",
		Poco::Data::Keywords::into(exist_count),
		Poco::Data::Keywords::useRef(guid),
		Poco::Data::Keywords::now;)

	if (exist_count == 0)
	{
		DEBUG_TRY_CATCH(*session_in_transaction << "INSERT INTO user (guid, username, score, l13, l12, l11, l10, l9, l8, l7, l6, l5, "\
		                                                             "l4, l3, l2, l1, l0, bonus) "\
		                                                             "VALUE (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
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
			Poco::Data::Keywords::use(bonus),
			Poco::Data::Keywords::now;)
	}
	else
	{
		DEBUG_TRY_CATCH(*session_in_transaction << "UPDATE user SET username=?, score=?, l13=?, l12=?, l11=?, l10=?, l9=?, l8=?, "\
		                                                           "l7=?, l6=?, l5=?, l4=?, l3=?, l2=?, l1=?, l0=?, bonus=? "\
		                                                           "WHERE guid=?",
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
			Poco::Data::Keywords::use(bonus),
			Poco::Data::Keywords::useRef(guid),
			Poco::Data::Keywords::now;)
	}

	bool success = persistGrids(session_in_transaction, guid, body);
	DB.ReleaseSession(session_in_transaction, success ? gridwalking::PocoGlue::COMMIT : gridwalking::PocoGlue::ROLLBACK);
	return success;
}

bool persist(const std::string& guid, Poco::UInt32* levels, Poco::UInt32 score, const std::string& name)
{
	restbed::Bytes empty_body;
	return persist(guid, levels, score, 0, name, empty_body);
}
