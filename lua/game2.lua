
local tid = st.spawn(function ()
	while true do
		local t, m = st.recv();
		print("$$$$$$$$$$$$$$$$$$$$=", st.tostring(t))
		st.send(t, "haha");
	end
	end);
	
st.register("game2", tid);