#ifndef _SECRETS_H_
#define _SECRETS_H_

#define CRC_SEED1      (0)
#define CRC_SEED2      (0)

#ifdef USE_ODBC_CONNECTION
# define DB_CONNECTION_STRING "DSN=gridwalking"
#else
# define DB_CONNECTION_STRING "host=localhost;port=3306;db=gridwalking;user=gridwalking;password=gridwalking;compress=true;auto-reconnect=true"
#endif

#endif // _SECRETS_H_
