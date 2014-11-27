

struct tm *time_toDateTime(const char *s, struct tm *t) {
	assert(t);
	assert(s);
	struct tm tm = {.tm_isdst = -1}; 
	int has_date = 0, has_time = 0;
	const char *limit = s + strlen(s), *marker, *token, *cursor = s;
	while (1) {
		if (cursor >= limit) {
			if (has_date || has_time) {
				*(struct tm*)t = tm;
				return t;
			}
			THROW(sys_exception, "Invalid date or time [s%]", s);
		}
		token = cursor;
		/*!re2c
		 re2c:define:YYCTYPE  = "unsigned char";
		 re2c:define:YYCURSOR = cursor;
		 re2c:define:YYLIMIT  = limit;
		 re2c:define:YYMARKER = marker;
		 re2c:yyfill:enable   = 0;
		 
		 any    = [\000-\377];
		 x      = [^0-9];
		 dd     = [0-9][0-9];
		 yyyy   = [0-9]{4};
		 tz     = [-+]dd(.? dd)?;
		 frac   = [.,][0-9]+;
		 
		 yyyy x dd x dd
		 { // Date: YYYY-MM-DD
				tm.tm_year  = a2i(token, 4);
				tm.tm_mon   = a2i(token + 5, 2) - 1;
				tm.tm_mday  = a2i(token + 8, 2);
				has_date = 1;
				continue;
		 }
		 yyyy dd dd
		 { // Compressed Date: YYYYMMDD
				tm.tm_year  = a2i(token, 4);
				tm.tm_mon   = a2i(token + 4, 2) - 1;
				tm.tm_mday  = a2i(token + 6, 2);
				has_date = 1;
				continue;
		 }
		 dd x dd x dd frac?
		 { // Time: HH:MM:SS
				tm.tm_hour = a2i(token, 2);
				tm.tm_min  = a2i(token + 3, 2);
				tm.tm_sec  = a2i(token + 6, 2);
				has_time = 1;
				continue;
		 }
		 dd dd dd frac?
		 { // Compressed Time: HHMMSS
				tm.tm_hour = a2i(token, 2);
				tm.tm_min  = a2i(token + 2, 2);
				tm.tm_sec  = a2i(token + 4, 2);
				has_time = 1;
				continue;
		 }
		 tz
		 { // Timezone: +-HH:MM, +-HH or +-HHMM is offset from UTC in seconds
				if (has_time) { // Only set timezone if time has been seen
						tm.TM_GMTOFF = a2i(token + 1, 2) * 3600;
						if (isdigit(token[3]))
								tm.TM_GMTOFF += a2i(token + 3, 2) * 60;
						else if (isdigit(token[4]))
								tm.TM_GMTOFF += a2i(token + 4, 2) * 60;
						if (token[0] == '-')
								tm.TM_GMTOFF *= -1;
				}
				continue;
		 }
		 any
		 {
				continue;
		 }
		 */
    }
	return NULL;
}
