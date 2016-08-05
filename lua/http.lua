
function on_messageBegin(parser)
	print("on_messageBegin");
end

function on_url(parser, str)
	print("on_url: " .. str);
end

function on_status(parser, str)
	print("on_status: " .. str);
end

function on_headersComplete(parser)
	print("on_headersComplete ");
	print("	getHttpVersion: ", string.format("%d.%d", http.getHttpVersion(parser)));
	print("	getHttpMethod: " .. http.getHttpMethod(parser));
	print("	getHttpStatusCode: " .. http.getHttpStatusCode(parser));
	print("	getHttpUpgrade: " .. tostring(http.getHttpUpgrade(parser)));
	print("	getHttpAlive: " .. tostring(http.getHttpAlive(parser)));
	-- headers
	local t = http.getHttpHeaders(parser);
	print("	getHttpHeaders: " .. #t);
	for i,v in pairs(t) do 
		print("		" .. i .. " = " .. v);
	end
end

function on_body(parser, str)
	print("on_body: " .. str);
end

function on_messageComplete(parser)
	print("on_messageComplete");
end

function proc(c) 
	local ip, port = net.getraddr(c);
	print("-------------accept a client: " .. ip .. ":" .. port);
	local parser = http.create(c, "request");
	http.setMessageBegin(parser, on_messageBegin);
	http.setUrl(parser, on_url);
	http.setStatus(parser, on_status);
	http.setHeadersComplete(parser, on_headersComplete);
	http.setBody(parser, on_body);
	http.setMessageComplete(parser, on_messageComplete);
	local r = http.execute(parser);
	if r ~= 0 then
		print(r);  --error
	end
	http.clear(parser);
end

function main() 
	local s = net.listen(8090, nil, "reuseaddr=true");
	
	while true do
		local c = net.accept(s);
		st.spawn(proc, c);
	end
	net.close(s);
end

main();
