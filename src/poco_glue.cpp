#include "main.h"

#include "poco_glue.h"


using namespace gridwalking;

PocoGlue::PocoGlue()
: m_pool(nullptr)
{
}

PocoGlue::~PocoGlue()
{
	if (m_pool)
	{
		delete m_pool;
		m_pool=nullptr;
#ifdef USE_ODBC_CONNECTION
		Poco::Data::ODBC::Connector::unregisterConnector();
#else
		Poco::Data::MySQL::Connector::unregisterConnector();
#endif
	}
}

bool PocoGlue::Initialize(const std::string& connection_string)
{
	if (m_pool) //Already initialized
		return true;

#ifdef USE_ODBC_CONNECTION
	Poco::Data::ODBC::Connector::registerConnector();
#else
	Poco::Data::MySQL::Connector::registerConnector();
#endif
	m_pool = new Poco::Data::SessionPool(DB_CONNECTION_TYPE, connection_string);

	return m_pool!=nullptr;
}

bool PocoGlue::CreateSession(Poco::Data::Session*& session)
{
	try
	{
		session = new Poco::Data::Session(m_pool->get());
		if (!session)
			return false;

		*session << "set autocommit = 0", Poco::Data::Keywords::now;
		session->begin();

		return true;
	}
	catch (Poco::Exception& e)
	{
		::log(stderr, e.displayText().c_str());
	}
	return false;
}

void PocoGlue::ReleaseSession(Poco::Data::Session* session, TransactionCommand command)
{
	if (!session)
		return;

	switch(command)
	{
		case COMMIT: session->commit(); break;
		case ROLLBACK: session->rollback(); break;
		default: break;
	}

	delete session;
}

bool PocoGlue::CommitTransaction(Poco::Data::Session* session)
{
	if (!session)
		return false;

	//*session << "COMMIT AND CHAIN", Poco::Data::now;
	session->commit();
	session->begin();
	return true;
}

bool PocoGlue::RollbackTransaction(Poco::Data::Session* session)
{
	if (!session)
		return false;

	//*session << "ROLLBACK AND CHAIN", Poco::Data::now;
	session->rollback();
	session->begin();
	return true;
}
