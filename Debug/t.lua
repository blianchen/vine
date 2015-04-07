
function secondthread()
	print("---------in lua secondthread********");
end


function threadStart(i, j, s)
	print("---------in lua threadStart " .. i .. "**" .. j .. "**" .. s);
	--st.spawn(secondthread);
end

local llcc;

function do_calc(cn) 
	local j = 3;
	while j>0 do
		print("---------in do_calc start ==== llcc " .. (cn) .. " ttt=" .. string.format("%d", st.ustime()));
		st.usleep(cn*1000);
		print("---------in do_calc end " .. (cn) .. " ttt=" .. string.format("%d", st.ustime()));
		j = j - 1;
	end
end

resp = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n<H2>It worked!</H2>\n";

function process_session(clisock)
--	while (true) do
	local n, s = net.read(clisock);
	print("---------read from socket byte n==" .. (n) .. ", s=" .. s)
	net.write(clisock, resp); 
	net.close(clisock);
	print("---------process_session")
--	end
end

function servsock()
	local srvsock = net.listen(10011, nil, "reuseaddr=true,package=2");
--	local srvsock = net.listen(10011, nil, nil);
	while(true) do
		local clisock = net.accept(srvsock);
		
--		print("--------- socket ip=" .. net.getraddr(clisock))

		print("-------------create session thread start.....");
		local player = st.spawn(process_session, clisock);
--		 print(debug.traceback())  
--		st.run_thread(player, clisock);
		print("-------------create session thread end.....");
	end
	net.close(srvsock);
end


function sockcon()
	local sock = net.connect("127.0.0.1:17408", "reuseaddr=true,package=2");
	local n,s = net.read(sock);
	print("---------read from socket byte n==" .. (n) .. ", sss==" .. s);
--	net.write(sock, "test haha!");
	net.close(sock);
end

function dbtest1(i)
	print("---------" .. i .. "db_connect start 111 " .. string.format("%d", st.ustime()));
	
	local conn = db.getConntion(dbpool);
	local rs = db.executeQuery(conn, "select * from tb");
--	local rs = db.executeQuery(conn, "strlen  123.4");
	while db.next(rs)==1 do
		local colnum = db.getColumnCount(rs);
		--print("---------db_query colnum=" .. colnum);
		print("---------" .. i .. "db_query colnum=" .. colnum .. ",colname=" .. db.getColumnName(rs, 2) .. ",value=" .. db.getString(rs, 2));
--		print("---------" .. i .. "db_query colnum=" .. colnum .. ",value=" .. db.getString(rs, 2) );
	end
	db.close(conn);
	print("---------" .. i .. "db_connect end 111 " .. string.format("%d", st.ustime()));
--	print("-------------- get key " .. db.command(c));
--	db.set(dbpool, "qwe", 1234.567934556789)
--    print("-------------- get key " .. db.get(dbpool, "qwe"));
--	print("---------" .. i .. "db_connect end 111 " .. string.format("%d", st.ustime()));
end

function dbtest2()
	print("---------db_connect start 222 " .. string.format("%d", st.ustime()));
	local c = db.connect();
	print("---------db_connect end 222 " .. string.format("%d", st.ustime()));
	print("-------------- get key " .. db.command(c));
end


function luafun()
	print("---------lua luafun start !!");
	
--	sockcon();

	cos = {};

	dbpool = db.newPool("postgresql://dbuser:dbuser@127.0.0.1:5432/test?application-name=corelib");
--	dbpool = db.newPool("redis://dbuser:dbuser@127.0.0.1:6379/");
	db.setInitialConn(dbpool, 4);
	db.setMaxConn(dbpool, 8);
	db.setSqlTimeout(dbpool, 3);
	--db.start(dbpool);

	print("time11111===" .. string.format("%d", st.ustime()))
	
--	st.send("tta@bogon", "trcv", "a hhahaha");
--st.send("tta@127.0.0.1", "trcv", "a hhahaha");
local ps = pack.pack(">P2ip", "b hahaha!!", 23, 456, "hello ==");
st.send("tta@127.0.0.1", "trcv", ps);
	
--	for i=1,1 do
--		--print("---------spawn " .. i);
--		cos[i] = st.spawn(servsock, i);
--	end
	--cos[5] = st.spawn(dbtest1);
	--cos[6] = st.spawn(dbtest2);
	
--	local lt = 2;
--	
--	st.spawn(function ()
--		lt = lt + 1;
--		print("------------ lt=" .. lt);
--	end);

--	local toTid = st.spawn(function () 
--		local i = 0;
--		while true do
--			local tid, msg = st.recv();
--			if (tid > 0) then
--				print("recv msg: " .. msg .. ", from tid:" .. string.format("%d", tid));
--				i = i+1;
----				if i==1 then
--					st.send(tid, "resp >> " .. msg);
----				end
--			end
--		end
--	end);
--	
--	st.spawn(function ()
--		print("send from tid:" .. string.format("%d", st.tid()) .. ", to tid:" .. string.format("%d", toTid));
--		st.send(toTid, "a haha!!");
--		st.send(toTid, "b 77777777777");
--		st.send(toTid, "cccvvv");
--		
--		while true do
--			local fromtid, msg = st.recv();
--			if (fromtid > 0) then
--				print(msg .. ", from tid:" .. string.format("%d", fromtid));
----				st.send(tid, "234");
--			end
--		end
--	end)

--	for i=1, 5 do
--		cos[i] = st.spawn(function () 
--			while true do
--				print("thread " .. i .. ", start recv ...")
--				local fromtid, msg = st.recv();
--				if fromtid > 0 then
--					print("recv id=" .. i .. ", msg=" .. msg);
--				end
--			end
--		end);
--	end
--	
--	st.spawn(function () 
--		st.send_multi(cos, "multi send msg!!!!");
--	end);
	
	print("time22222===" .. string.format("%d", st.ustime()))


--	print("time33333===" .. string.format("%d", st.ustime()))
--	for i=1,19 do
--		st.run_thread(cos[i], i);
--	end


	print("time44444===" .. string.format("%d", st.ustime()))
	--	st.stop_thread();
	print("---------lua luafun end !!");
end


luafun();
