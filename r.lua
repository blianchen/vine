
function rcvfun()
	while true do
		local fromtid, msg = st.recv();
		if (fromtid > 0) then
			local pos, str1, i1, i2, str2, str3 = pack.unpack(msg, ">P2ipi", 1);
			print(str1 .. "," .. pos .. "," .. i1 .. "," .. i2  .. ", from tid:" .. string.format("%d", fromtid));
			print(str3);
			st.send(fromtid, "recv hahahahaha..")
		end
	end
end

function main()
	local tid = st.spawn(rcvfun);
	st.register("trcv", tid);
	print("trcv strat ...")
end

main();
