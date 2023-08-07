local _M = { }

local function tostringvalue(value)
	if type(value) == "table" then
		return _M.tabletostr(value)
	elseif type(value) == "string" then
		return """ .. value .. """
	end
	return tostring(value)
end

function _M.tabletostr(t)
	local retstr = "{"
	local i, first = 1, true
	for k,v in pairs(t) do
		local signal = ","
		if first then
			signal = ""
			first = false
		end

		if k == i then
			retstr = retstr .. signal .. tostringvalue(v)
			i = i + 1
		else
			if type(k) == "number" then
				retstr = retstr .. signal .. "[" .. k .. "]=" .. tostringvalue(v)
			elseif type(k) == "string" then
				retstr = retstr .. signal .. k .. "=" .. tostringvalue(v)
			end
		end
	end
	retstr = retstr .. "}"
	return retstr
end

function _M.toluastring(t)
	if t == nil then return "" end
	return _M.tabletostr(t)
end

return _M