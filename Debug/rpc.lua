
rp = {}
rp.rp12 = {}

function rpctest(a, b, c, d) 
	print("rpctest: a=" .. a .. ", b=" .. b .. ", c=" .. c .. ", d=" .. d);
	return 10;
end

rp.rp12.stest = rpctest;

--function getNumberPackType(v)
--	if (v < 0) then
--		if v >= -128 then
--			return "c";
--		elseif v >= -32768 then
--			return "h";
--		elseif v >= -2147483648 then
--			return "i";
--		else
--			return "l";
--		end
--	else
--		if v < 256 then 
--			return "b";
--		elseif v < 65536 then
--			return "H";
--		elseif v < 4294967296 then
--			return "I";
--		else
--			return "L";
--		end
--	end
--end

function getPackFormat(table)
	local f = ">";
	local n = #table;
	local t, v;
	for i=1,n do
		v = table[i];
		t = type(v);
		if t == "number" then
			f = f .. "n";
		elseif t == "string" then
--				local strLen = string.len(v);
			f = f .. "P";
		elseif t == "boolean" then
			f = f .. "o";
--			elseif t == "table" then
--			elseif t == "function" then
--			elseif t == "thread" then
--			elseif t == "userdata" then
		else
			print("error arg type " .. t);
		end
	end
	return f;
end

function rpc_call(node, funName, ...)
	local f = ">PP";
	local msg;
	local n = arg.n;
	if n == 0 then 
		msg = pack.pack(f, funName);
	else
		local argFormat = getPackFormat(arg);
		msg = pack.pack(f .. argFormat, funName, argFormat, unpack(arg));
	end
	
	st.send(node, "rpcsrv", msg);
	
	-- fun result
	local fromTid, rmsg = st.recv();
	if fromTid > 0 then
		local pos, argFormat = pack.unpack(rmsg, ">P");
		return pack.unpack(rmsg, argFormat, false, pos);
	end
end

function rpcsrv_main() 
	local tid = st.spawn(function () 
		while (true) do
			local fromTid, msg = st.recv();
			if fromTid > 0 then
				local rt;
				local pos, funName, argFormat = pack.unpack(msg, ">PP");
	
				local fun = _G;
				local mod;
				for mod in string.gmatch(funName, "([^%.]+)%.*") do
					fun = fun[mod];
				end
				
				if (argFormat == nil) then
					rt = {pcall(fun)};
				else
					rt = {pcall(fun, pack.unpack(msg, argFormat, false, pos))};
				end
				
				-- send result
				local f = getPackFormat(rt);
				local rb = pack.pack(">P" .. f, f, unpack(rt));
				st.send(fromTid, rb);
			end
		end
	end);
	st.register("rpcsrv", tid);
end

--print(_G["st"]["spawn"].toString)
rpcsrv_main();
--print( rpc_call("rpca@127.0.0.1", "rpctest", 1,2,3,5) );
--print( rpc_call("rpca@127.0.0.1", "rp.stest", 2,3,4,5) )
--rpcsrv_main();
--print(_G.rpctest);
--pcall(_G.rpctest, 1, 2, 3, 4);

