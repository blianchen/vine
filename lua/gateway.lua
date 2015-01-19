
--sessionId = 0;

sessionDic = {};

function connProxyStart()
	print("client proxy start...");
	local srvsock = net.listen(10011, nil, "reuseaddr=true, package=2");
	print("gateway listen at " .. 10011);
	
	while true do
--		sessionId = sessionId + 1;
		local clisock = net.accept(srvsock);
		local ip, port = net.getraddr(clisock);
		print("accept a client: " .. ip .. ":" .. port);
		sessionDic[clisock] = {ip=ip, port=port, sock=clisock};

		local clientThread = st.create_thread(proclient);
		st.run_thread(clientThread, clisock);
	end
	net.close(srvsock);
end


function proclient(clientSock)
	while true do
		local n, s = net.read(clientSock);
		if n < 0 then
			net.close(clientSock);
			sessionDic[clientSock] = nil;
			return;
		end
		print("---------read from socket byte n==" .. n .. ", s=" .. s);
--		net.write(clientSock, resp);
		
	end
end


function innerProxyStart() 
	print("inner server proxy start...");
	local srvsock = net.listen(10012, nil, "reuseaddr=true, package=2");
	print("gateway listen at " .. 10012);
	
	while true do
--		sessionId = sessionId + 1;
		local srvsock = net.accept(srvsock);
		local ip, port = net.getraddr(srvsock);
		print("accept a inner server: " .. ip .. ":" .. port);
		sessionDic[srvsock] = {ip=ip, port=port, sock=srvsock};

		local srvThread = st.create_thread(prosrv);
		st.run_thread(srvThread, srvsock);
	end
	net.close(srvsock);
end


connProxyStart();
