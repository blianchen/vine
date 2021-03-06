
--sessionId = 0;

--sessionDic = {};
playeridDic = {};

function proclient(clientSock)
	local ip, port = net.getraddr(clientSock);
	--print("-------------accept a client: " .. ip .. ":" .. port);
--	sessionDic[clientSock] = {ip=ip, port=port, sock=clientSock};
	local selfTid = st.tid();
	local playerid;
	local n, msg;
	
--	process player login message
--	n, msg = net.read(clientSock);
--	n, playerid = pack.unpack(msg, ">P");
	
	-- create rcv game server message thread
	local rcvTid = st.spawn(function ()
			local ftid, srvmsg;
			while true do 
				ftid, srvmsg = st.recv();
				if (ftid[1] == selfTid[1]) then
					if srvmsg == "close" then
--						print("----srvmsg close");
						break;
					end
				else 
--					print(srvmsg);
					net.write(clientSock, srvmsg);
--					net.close(clientSock)
--					return;
				end
			end
		end);
--	print("-------------rcvTid: ", st.tostring(rcvTid));
--	playeridDic[playerid] = {playerid=playerid, sock=clientSock, tid=selfTid, recvTid=rcvTid};
	
	-- rcv client message
	local cmd;
	while true do
		n, msg = net.read(clientSock);
		if n <= 0 then
		--	print("------------- close......");
--			sessionDic[clientSock] = nil;
			net.close(clientSock);
--			playeridDic[playerid] = nil;
			-- close recv thread
--			print("-------------send rcvTid: ", st.tostring(rcvTid));
			st.send(rcvTid, "close");
			return;
		end
--		print("---------read from socket byte n==" .. n .. ", msg=" .. msg);
--		net.write(clientSock, resp);
--		n, cmd = pack.unpack(msg, "b");
--		if cmd < 128 then 
			st.send("game1@127.0.0.1", "logicsrv", "test", rcvTid);
--			print("---------send game1");
--    net.write(clientSock, resp);
--		elseif cmd < 160 then
--			st.send("game1@127.0.0.1", "chatsrv", msg);
--		end
	end
end

function connProxyStart()
	print("-----------client proxy start...");
--	local srvsock = net.listen(10011, nil, "reuseaddr=true,package=2");
	local srvsock = net.listen(10011, nil, "reuseaddr=true");
	print("-----------gateway listen at " .. 10011);
	
	while true do
--		sessionId = sessionId + 1;
		local clisock = net.accept(srvsock);
		st.spawn(proclient, clisock);
	end
	net.close(srvsock);
	print("-----------client proxy stop...")
end


--function innerProxyStart()
--	print("inner server proxy start...");
--	local srvsock = net.listen(10012, nil, "reuseaddr=true, package=2");
--	print("gateway listen at " .. 10012);
--	
--	while true do
--		local srvsock = net.accept(srvsock);
--		local ip, port = net.getraddr(srvsock);
--		print("accept a inner server: " .. ip .. ":" .. port);
--		sessionDic[srvsock] = {ip=ip, port=port, sock=srvsock};
--
--		local srvThread = st.spawn(prosrv, srvsock);
--	end
--	net.close(srvsock);
--end


connProxyStart();
