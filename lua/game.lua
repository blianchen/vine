
httpstr1 = "HTTP/1.0 200 OK\r\nContent-Type:text/html;charset=utf-8\r\nServer:vine/1.0\r\nConnection:keep-alive\r\n\r\n<html><head><title>vine Homepage</title></head><body>"
httpstr2 = "</body></html>\r\n\r\n"

recv1 = 0;

function logictest()
	local t = st.spawn(function ()
			local ftid, msg;
			local n, cmdcode, str;
			while true do
				ftid, msg = st.recv();
				recv1 = recv1 + 1;
				print("-------------recv a msg....." .. recv1)
				n, cmdcode, str = pack.unpack(msg, "bp");
				st.send(ftid, httpstr1 .. "hello" .. httpstr2);
			end
		end);
		
	print("----------------", st.tostring(t));
	st.register("logicsrv", t);
end

logictest();
