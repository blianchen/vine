
function secondthread()
	print("---------in lua secondthread********");
end


function threadStart(i, j, s)
	print("---------in lua threadStart " .. i .. "**" .. j .. "**" .. s);
	--st.create_thread(secondthread);
end

local llcc;

function do_calc(cn) 
	local j = 1;
	while j>0 do
		print("---------in do_calc start ==== llcc " .. (cn) .. " ttt=" .. string.format("%d", st.mstime()));
		st.msleep(cn*1);
		print("---------in do_calc end " .. (cn) .. " ttt=" .. string.format("%d", st.mstime()));
		j = j - 1;
	end
end

resp = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n<H2>It worked!</H2>\n";

function process_session(clisock)
--	while (true) do
	local n, s = net.read(clisock);
	net.write(clisock, resp);
	net.close(clisock);
--	print("---------read from socket byte n==, " .. (n) .. "sss==" .. s)
--	end
end



function servsock()
	local srvsock = net.server_socket("", 10001);
	while(true) do
		local clisock = net.accept(srvsock);

		local player = st.create_thread(process_session);
		st.run_thread(player, clisock);
	--	print("-------------create session thread.....");
	end
end


function sockcon()
	local sock = net.socket();
	net.connect(sock, "127.0.0.1:6379");
	local n,s = net.read(sock);
	print("---------read from socket byte n==, " .. (n) .. "sss==" .. s);
	net.write(sock, "test haha!");
end

function dbtest1(i)
	print("---------" .. i .. "db_connect start 111 " .. string.format("%d", st.mstime()));
	
--	local conn = db.getConntion(dbpool);
--	local rs = db.executeQuery(conn, "select * from tb");
--	local rs = db.executeQuery(conn, "strlen  123.4");
--	while db.next(rs)==1 do
--		local colnum = db.getColumnCount(rs);
--		--print("---------db_query colnum=" .. colnum);
--		--print("---------" .. i .. "db_query colnum=" .. colnum .. ",colname=" .. db.getColumnName(rs, 2) .. ",value=" .. db.getString(rs, 2));
--		print("---------" .. i .. "db_query colnum=" .. colnum .. ",value=" .. db.getString(rs, 2) );
--	end
--	db.close(conn);
--	print("---------" .. i .. "db_connect end 111 " .. string.format("%d", st.mstime()));
--	print("-------------- get key " .. db.command(c));
--	db.set(dbpool, "qwe", 1234.567934556789)
    print("-------------- get key " .. db.get(dbpool, "qwe"));
	print("---------" .. i .. "db_connect end 111 " .. string.format("%d", st.mstime()));
end

function dbtest2()
	print("---------db_connect start 222 " .. string.format("%d", st.mstime()));
	local c = db.connect();
	print("---------db_connect end 222 " .. string.format("%d", st.mstime()));
	print("-------------- get key " .. db.command(c));
end


function luafun()
	print("---------lua luafun start !!");

	local cos = {};

--	dbpool = db.newPool("postgresql://dbuser:dbuser@127.0.0.1:5432/test?application-name=corelib");
	dbpool = db.newPool("redis://dbuser:dbuser@127.0.0.1:6379/");
	db.setInitialConn(dbpool, 4);
	db.setMaxConn(dbpool, 8);
	db.setSqlTimeout(dbpool, 3);
	--db.start(dbpool);

	print("time11111===" .. string.format("%d", st.mstime()))
	for i=1,19 do
		--print("---------create_thread " .. i);
		cos[i] = st.create_thread(dbtest1);
	end
	--cos[5] = st.create_thread(dbtest1);
	--cos[6] = st.create_thread(dbtest2);
	print("time22222===" .. string.format("%d", st.mstime()))


	print("time33333===" .. string.format("%d", st.mstime()))
	for i=1,19 do
		st.run_thread(cos[i], i);
	end


	print("time44444===" .. string.format("%d", st.mstime()))
	--	st.stop_thread();
	print("---------lua luafun end !!");
end
