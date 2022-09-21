--$using model transform

function interpolate(v1, v2, t)
	return v1 * (1 - t) + v2 * t
end


function init()
	up = true
	myTable = {
		name = "Tabel1",
		count = 0,
		myFunc = function(self)
			self.count = self.count + 1
		end
	}
	print("system1: ", transform, _G)
	
end

function update(dt)
	
	
	if up then
		transform.position.y = interpolate(transform.position.y, -2, dt)
		
		if transform.position.y + 2 < 0.1 then
			up = false
			myTable:myFunc()
		end
	else
		transform.position.y = interpolate(transform.position.y, 0, dt)
		
		if transform.position.y > -0.1 then
			up = true
		end
	end
	
end