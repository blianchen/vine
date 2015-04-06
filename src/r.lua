
function rcvfun()
	while true do
		local fromtid, msg = st.recv();
		if (fromtid > 0) then
			print(msg .. ", from tid:" .. string.format("%d", fromtid));
		end
	end
end

function main()
	local tid = st.spawn(rcvfun);
	st.register("trcv", tid);
	print("trcv strat ...")
end

main();
