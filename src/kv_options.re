
kv_option_t parse_kv_option(char *option) {
	char *s = str_dup(option);
	kv_option_t param = NULL;
	kv_option_t nextParam = NULL;
	char *marker, *cursor = s, *ctx, *token;
	while (1) {
		if (*cursor == 0) {
			return param;
		}
		token = cursor;
	
		/*!re2c
		re2c:define:YYCTYPE  = "unsigned char";
		re2c:define:YYCURSOR = cursor;
		re2c:define:YYLIMIT  = limit;
		re2c:define:YYMARKER = marker;
		re2c:define:YYCTXMARKER = ctx;
		re2c:yyfill:enable   = 0;

		any				= [\000-\377];
		parameterkey	= ([\040-\377]\[=])+;
		parametervalue	= ([\040-\377]\[,])*;
         
         
		parameterkey/[=] 
		{
			NEW(param);
			param->name = token;
			param->next = nextParam;
			nextParam = param;
			continue;
		}
         
		[=]parametervalue[,]?
		{
		 	*token++ = 0;
			if (*(cursor - 1) == ',')
		 		*(cursor - 1) = 0;
			if (! param) // format error
				return NULL;
			param->value = token;
			
			param->name = str_trim(param->name);
			param->value = str_trim(param->value);
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

