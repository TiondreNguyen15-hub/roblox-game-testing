local First=game:GetService('ReplicatedFirst')
local ok,loader=pcall(function()return require(First:WaitForChild('BlocklineLoading'))end)
if ok then loader.Start();First:RemoveDefaultLoadingScreen()else warn('Blockline cinematic loader failed:',loader)end
