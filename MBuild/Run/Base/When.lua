MBuild.When = MBuild.When or {};
local When  = MBuild.When;

function When:new(conditions, callback)
	when = {
		conditions = conditions,
		callback   = callback
	};
	setmetatable(when, self);
	self.__index = self;
	return when;
end

function When:Evaluate()
	local result = true;
	for _, condition in ipairs(self.conditions) do
		local str = "return " .. condition;
		local suc, res = pcall(loadstring(str));
		if not suc or not res then
			result = false;
			break;
		end
	end
	if result then
		self.callback();
	end
end