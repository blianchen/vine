
#ifndef POSTGRESQLCONNECTION_INCLUDED
#define POSTGRESQLCONNECTION_INCLUDED

#define T dbconn_delegate_t
T postgresqlconn_new(uri_t url);
void postgresqlconn_free(T *C);
int postgresqlconn_connstate(T C);
int postgresqlconn_getsocket(T C);
void postgresqlconn_setQueryTimeout(T C, int ms);
void postgresqlconn_setMaxRows(T C, int max);
int postgresqlconn_ping(T C);
int postgresqlconn_beginTransaction(T C);
int postgresqlconn_commit(T C);
int postgresqlconn_rollback(T C);
long long postgresqlconn_lastRowId(T C);
long long postgresqlconn_rowsChanged(T C);
//int postgresqlconn_execute(T C, const char *sql, va_list ap);
int postgresqlconn_execute(T C, const char *sql);
//dbrs_t postgresqlconn_executeQuery(T C, const char *sql, va_list ap);
dbpst_t postgresqlconn_prepareStatement(T C, const char *sql);
const char *postgresqlconn_getLastError(T C);
dbrs_t postgresqlconn_getrs(T C);
/* Event handlers */
void  postgresqlconn_onconn(T C);
void  postgresqlconn_onstop(T C);

#undef T
#endif

