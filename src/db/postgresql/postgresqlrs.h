
#ifndef POSTGRESQLRESULTSET_INCLUDED
#define POSTGRESQLRESULTSET_INCLUDED

#define T dbresultset_delegate_t
T postgresqlrs_new(void *stmt, int maxRows);
void postgresqlrs_free(T *R);
int postgresqlrs_getColumnCount(T R);
const char *postgresqlrs_getColumnName(T R, int columnIndex);
long postgresqlrs_getColumnSize(T R, int columnIndex);
int postgresqlrs_next(T R);
int postgresqlrs_isnull(T R, int columnIndex);
const char *postgresqlrs_getString(T R, int columnIndex);
const void *postgresqlrs_getBlob(T R, int columnIndex, int *size);

#undef T
#endif
