
#ifndef POSTGRESQLPREPAREDSTATEMENT_INCLUDED
#define POSTGRESQLPREPAREDSTATEMENT_INCLUDED
#define T dbpst_delegate_t
T postgresqlpst_new(PGconn *db, int maxRows, char *stmt, int paramCount);
void postgresqlpst_free(T *P);
void postgresqlpst_setString(T P, int parameterIndex, const char *x);
void postgresqlpst_setInt(T P, int parameterIndex, int x);
void postgresqlpst_setLLong(T P, int parameterIndex, long long x);
void postgresqlpst_setDouble(T P, int parameterIndex, double x);
void postgresqlpst_setTimestamp(T P, int parameterIndex, time_t x);
void postgresqlpst_setBlob(T P, int parameterIndex, const void *x, int size);
int postgresqlpst_execute(T P);
//dbrs_t postgresqlpst_executeQuery(T P);
dbrs_t postgresqlpst_getrs(T P);
long long postgresqlpst_rowsChanged(T P);
#undef T
#endif
