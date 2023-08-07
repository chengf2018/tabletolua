local _M = { }
local trans_char = {
	['\"'] = "\\\"",
	['\t'] = "\\t",
	['\n'] = "\\n",
	['\a'] = "\\a",
	['\b'] = "\\b",
	['\f'] = "\\f",
	['\r'] = "\\r",
	['\v'] = "\\v",
	['\\'] = "\\\\",
	['\''] = "\\\'"
}
local function trans2escapechar(str)
	local newstr = ""
	for i=1, #str do
		local c = str[i]
		if trans_char[c] then
			newstr = newstr .. trans_char[c]
		else
			newstr = newstr .. c
		end
	end
end

local function tostringvalue(value)
	if type(value) == "table" then
		return _M.tabletostr(value)
	elseif type(value) == "string" then
		return "\"" .. value .. "\""
	end
	return tostring(value)
end

function _M.tabletostr(t)
	local retstr = "{"
	local i, first = 1, true
	for k,v in pairs(t) do
		local comma = ","
		if first then
			comma = ""
			first = false
		end

		if k == i then
			retstr = retstr .. comma .. tostringvalue(v)
			i = i + 1
		elseif type(k) == "number" then
			retstr = retstr .. comma .. "[" .. k .. "]=" .. tostringvalue(v)
		elseif type(k) == "string" then
			retstr = retstr .. comma .. k .. "=" .. tostringvalue(v)
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