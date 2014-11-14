
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
		print("---------in do_calc start ==== llcc " .. (cn));
		st.msleep(cn*1000);
		print("---------in do_calc end " .. (cn));
		j = j - 1;
	end
end

function luafun()
	print("---------lua luafun start !!");

	local cos = {};

--	st.create_thread(threadStart, 400, 500, "ertyu");
print("time11111===" .. string.format("%d", st.mstime()))
	for i=1,4 do
		--print("---------create_thread " .. i);

		cos[i] = st.create_thread(function (cn) 	
										local j = 3;
										while j>0 do
											print("---------in do_calc start ==== llcc " .. (cn));
											st.msleep(cn*1000);
										--	print("---------in do_calc end " .. (cn));
											j = j - 1;
										end
								end);
	end
print("time22222===" .. string.format("%d", st.mstime()))

print("time33333===" .. string.format("%d", st.mstime()))
	for i=1,4 do
		st.run_thread(cos[i], i);
	end
print("time44444===" .. string.format("%d", st.mstime()))
--	st.stop_thread();
	print("---------lua luafun end !!");
end
