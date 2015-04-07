
function rcvfun()
	while true do
		local fromtid, msg = st.recv();
		if (fromtid > 0) then
			local pos, str1, i1, i2,  str2 = pack.unpack(msg, ">P2ip");
			print(str1 .. "," .. str2 .. "," .. i1 .. "," .. i2 .. ", from tid:" .. string.format("%d", fromtid));
		end
	end
end

function main()
	local tid = st.spawn(rcvfun);
	st.register("trcv", tid);
	print("trcv strat ...")
end

main();
