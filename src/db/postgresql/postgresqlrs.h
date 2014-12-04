/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.
 */
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
