
function secondthread()
	print("---------in lua secondthread********");
end


function threadStart(i, j, s)
	print("---------in lua threadStart " .. i .. "**" .. j .. "**" .. s);

	--st.create_thread(secondthread);
end

local llcc;

function do_calc(cn) 
llcc = cn
	local j = 3;
	while j>0 do
		print("---------in do_calc start ==== llcc " .. (llcc));
		st.msleep(cn*1000);
		print("---------in do_calc end " .. (llcc));
		j = j - 1;
	end
end

function luafun()
	print("---------lua luafun start !!");
--	st.create_thread(threadStart, 400, 500, "ertyu");
	for i=1,4 do
		print("---------create_thread " .. i);

		st.create_thread("do_calc", i);
	end
--	st.stop_thread();
	print("---------lua luafun end !!");
end
