
static int parse_uri(T U) {
        param_t param = NULL;
	/*!re2c
         re2c:define:YYCTYPE      = "unsigned char";
         re2c:define:YYCURSOR     = U->buffer;
         re2c:define:YYLIMIT      = U->limit;
         re2c:define:YYMARKER     = U->marker;
         re2c:define:YYCTXMARKER  = U->ctx;
         re2c:yyfill:enable       = 0;

         ws                       = [ \t\r\n];
         any		          = [\000-\377];
         protocol                 = [a-zA-Z0-9]+"://";
         auth                     = ([\040-\077\101-\132\134\136-\377])+[@];
         host                     = ([a-zA-Z0-9\-]+)([.]([a-zA-Z0-9\-]+))*;
         host6                    = '[' [0-9a-zA-Z:%]+ ']';
         port                     = [:][0-9]+;
         path                     = [/]([\041-\377]\[?#;])*;
         query                    = ([\040-\377]\[#])*;
         parameterkey             = ([\041-\377]\[=])+;
         parametervalue           = ([\040-\377]\[&])*;
	*/
proto:
	if (YYCURSOR >= YYLIMIT)
		return 0;
	YYTOKEN = YYCURSOR;
	/*!re2c
         ws         
         {
                goto proto;
         }
         "redis://"
         {
                SET_PROTOCOL(REDIS_DEFAULT_PORT);
         }
         "mysql://"
         {
                SET_PROTOCOL(MYSQL_DEFAULT_PORT);
         }
         "postgresql://" 
         {
                SET_PROTOCOL(POSTGRESQL_DEFAULT_PORT);
         }
         "oracle://"
         {
                SET_PROTOCOL(ORACLE_DEFAULT_PORT);
         }
         "http://"
         {
                SET_PROTOCOL(HTTP_DEFAULT_PORT);
         }
         protocol
         {
                SET_PROTOCOL(UNKNOWN_PORT);
         }
         any        
         {
                goto proto;
         }
	*/
authority:
	if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN = YYCURSOR;
	/*!re2c
         ws         
         {
                goto authority;
         }
         auth       
         {
                *(YYCURSOR - 1) = 0;
                U->user = YYTOKEN;
                char *p = strchr(U->user, ':');
                if (p) {
                        *(p++) = 0;
                        U->password = uri_unescape(p);
                }
                uri_unescape(U->user);
                goto authority;
         }
         host6
         {
                U->ip6 = 1;
                U->host = str_ndup(YYTOKEN + 1, (int)(YYCURSOR - YYTOKEN - 2));
                goto authority;
         }
         host
         {
                U->host = str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                goto authority;
         }
         port
         {
                U->portStr = YYTOKEN + 1; // read past ':'
                U->port = str_parseInt(U->portStr);
                goto authority;
         }
         path       
         {
                *YYCURSOR = 0;
                U->path = uri_unescape(YYTOKEN);
                return 1;
         }
         path[?]    
         {
                *(YYCURSOR-1) = 0;
                U->path = uri_unescape(YYTOKEN);
                goto query;
         }
         any         
         {
                return 1;
         }
	*/
query:
        if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN =  YYCURSOR;
	/*!re2c
         query      
         {
                *YYCURSOR = 0;
                U->query = str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                YYCURSOR = YYTOKEN; // backtrack to start of query string after terminating it and
                goto params;
         }
         any        
         {
                return 1;
         }
	*/
params:
	if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN =  YYCURSOR;
	/*!re2c
         parameterkey/[=] 
         {
                NEW(param);
                param->name = YYTOKEN;
                param->next = U->params;
                U->params = param;
                goto params;
         }
         [=]parametervalue[&]?
         {
                *YYTOKEN++ = 0;
                if (*(YYCURSOR - 1) == '&')
                        *(YYCURSOR - 1) = 0;
                if (! param) // format error
                        return 1; 
                param->value = uri_unescape(YYTOKEN);
                goto params;
         }
         any 
         {
                return 1;
         }
        */
        return 0;
}

