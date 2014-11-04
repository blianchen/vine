
function secondthread()
	print("---------in lua secondthread********");
end


function threadStart(i, j, s)
	print("---------in lua threadStart " .. i .. "**" .. j .. "**" .. s);

	--st.create_thread(secondthread);
end

function do_calc(cn) 
--	local j = 5;
	while true do
		print("---------in do_calc start " .. (cn*100));
		st.msleep(cn*100);
--		print("---------in do_calc end " .. (cn*1000));
--		j = j - 1;
	end
end

function luafun()
	print("---------lua luafun start !!");
--	st.create_thread(threadStart, 400, 500, "ertyu");
	for i=1,3 do
		print("---------create_thread " .. i);

		st.create_thread("do_calc", i);
	end
--	st.stop_thread();
	print("---------lua luafun end !!");
end
