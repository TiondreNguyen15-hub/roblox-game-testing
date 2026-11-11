-- BLOCKLINE: original urban team arena. All combat decisions run on server.
local Players = game:GetService('Players')
local Teams = game:GetService('Teams')
local RS = game:GetService('ReplicatedStorage')
local Debris = game:GetService('Debris')
local Lighting = game:GetService('Lighting')
local remote = Instance.new('RemoteEvent', RS); remote.Name = 'BlocklineAction'
local status = Instance.new('StringValue', RS); status.Name = 'BlocklineStatus'
local arena = workspace:WaitForChild('BlocklineCity', 10)
assert(arena, 'BlocklineCity map missing. Open the updated Blockline.rbxlx place.')
local states = {}
local scores = {Azure = 0, Ember = 0}
local active = false
local gray = Color3.fromRGB(59,65,76)
local orange = Color3.fromRGB(255,122,65)
Lighting.ClockTime = 17; Lighting.Brightness = 2
Lighting.Ambient = Color3.fromRGB(125,132,152)
workspace.FallenPartsDestroyHeight = -60
local function part(name, size, pos, color)
 local p = Instance.new('Part'); p.Name=name; p.Size=size; p.Position=pos
 p.Anchored=true; p.Color=color; p.Parent=arena
 return p
end
for i,name in ipairs({'Azure','Ember'}) do
 local team=Teams:FindFirstChild(name) or Instance.new('Team',Teams)
 team.Name=name; team.TeamColor=BrickColor.new(i==1 and 'Bright blue' or 'Bright orange'); team.AutoAssignable=false
 local spawn=arena:FindFirstChild(name..' spawn',true)
 assert(spawn, name..' spawn missing from city')
 spawn.TeamColor=team.TeamColor; spawn.Neutral=false
end
status.Value='CITY READY'
-- Practice targets respawn and do not award match points.
for _,z in ipairs({-28,28}) do
 local model=Instance.new('Model',arena); model.Name='Practice target'
 local body=part('Torso',Vector3.new(3,4,1.5),Vector3.new(0,3,z),orange); body.Parent=model
 local head=part('Head',Vector3.new(2,2,2),Vector3.new(0,6,z),orange); head.Parent=model
 local function resetTarget()
  local hum=Instance.new('Humanoid',model); hum.RequiresNeck=false; hum.BreakJointsOnDeath=false; hum.MaxHealth=100; hum.Health=100
  hum.Died:Connect(function() task.delay(3,function() if model.Parent then hum:Destroy(); resetTarget() end end) end)
 end
 resetTarget()
end
local function send(p)
 local s=states[p]; if s then remote:FireClient(p,'Ammo',s.ammo,s.reloading) end
end
local function armed(p)
 local char=p.Character; local hum=char and char:FindFirstChildOfClass('Humanoid')
 return char and hum and hum.Health>0 and char:FindFirstChild('AR-24'), hum
end
local function equip(p,char)
 local s={ammo=30,last=0,reloading=false,sprint=false}; states[p]=s
 local hum=char:WaitForChild('Humanoid'); hum.WalkSpeed=16
 local root=char:WaitForChild('HumanoidRootPart')
 local spawn=p.Team and arena:FindFirstChild(p.Team.Name..' spawn',true)
 if spawn then
  p.RespawnLocation=spawn
  local position=spawn.Position+Vector3.new(0,4,0)
  char:PivotTo(CFrame.lookAt(position,Vector3.new(0,position.Y,0)))
 end
 local tool=Instance.new('Tool'); tool.Name='AR-24'; tool.CanBeDropped=false
 local handle=Instance.new('Part',tool); handle.Name='Handle'; handle.Size=Vector3.new(0.45,0.7,2.7)
 handle.Color=Color3.fromRGB(32,38,47); handle.Material=Enum.Material.Metal; handle.CanCollide=false; handle.Massless=true
 local barrel=Instance.new('Part',tool); barrel.Name='Barrel'; barrel.Size=Vector3.new(0.2,0.2,1.5)
 barrel.Color=gray; barrel.Material=Enum.Material.Metal; barrel.CanCollide=false; barrel.Massless=true
 barrel.CFrame=handle.CFrame*CFrame.new(0,0,-1.7)
 local weld=Instance.new('WeldConstraint',handle); weld.Part0=handle; weld.Part1=barrel
 tool.Parent=p:WaitForChild('Backpack'); hum:EquipTool(tool)
 hum.Died:Connect(function() local d=p:FindFirstChild('leaderstats'); if d then d.Deaths.Value=d.Deaths.Value+1 end end)
 send(p)
end
local function setupPlayer(p)
 if p:FindFirstChild('leaderstats') then return end
 local a,b=0,0
 for _,other in ipairs(Players:GetPlayers()) do if other.Team and other.Team.Name=='Azure' then a=a+1 elseif other.Team then b=b+1 end end
 p.Team=Teams:FindFirstChild(a<=b and 'Azure' or 'Ember')
 local stats=Instance.new('Folder',p); stats.Name='leaderstats'
 for _,name in ipairs({'Kills','Deaths'}) do local v=Instance.new('IntValue',stats); v.Name=name end
 p.CharacterAdded:Connect(function(char) equip(p,char) end)
 if p.Character then task.spawn(equip,p,p.Character) end
end
Players.PlayerAdded:Connect(setupPlayer)
for _,p in ipairs(Players:GetPlayers()) do setupPlayer(p) end
Players.PlayerRemoving:Connect(function(p) states[p]=nil end)
remote.OnServerEvent:Connect(function(p,action,direction)
 local s=states[p]; if not s then return end
 if action=='Sync' then send(p); return end
 local tool,hum=armed(p); if not tool then return end
 if action=='Sprint' then
  s.sprint=direction==true; hum.WalkSpeed=s.sprint and 23 or 16; return
 end
 if action=='Reload' then
  if s.reloading or s.ammo==30 then return end
  s.reloading=true; send(p)
  task.delay(2,function() if states[p]==s then s.ammo=30; s.reloading=false; send(p) end end)
  return
 end
 if action~='Shoot' or not active or s.reloading or s.sprint or s.ammo<=0 or os.clock()-s.last<0.105 then return end
 if typeof(direction)~='Vector3' then return end
 local magnitude=direction.Magnitude
 if magnitude~=magnitude or magnitude<0.9 or magnitude>1.1 then return end
 local char=p.Character; local head=char and char:FindFirstChild('Head'); if not head then return end
 s.last=os.clock(); s.ammo=s.ammo-1; send(p)
 local params=RaycastParams.new(); params.FilterType=Enum.RaycastFilterType.Exclude; params.FilterDescendantsInstances={char}
 local origin=head.Position
 local result=workspace:Raycast(origin,direction.Unit*500,params)
 local finish=result and result.Position or origin+direction.Unit*500
 local tracer=Instance.new('Part'); tracer.Name='Tracer'; tracer.Anchored=true; tracer.CanCollide=false; tracer.CanQuery=false; tracer.CanTouch=false
 tracer.Material=Enum.Material.Neon; tracer.Color=Color3.fromRGB(255,220,135)
 tracer.Size=Vector3.new(0.06,0.06,(finish-origin).Magnitude); tracer.CFrame=CFrame.lookAt((origin+finish)/2,finish); tracer.Parent=workspace; Debris:AddItem(tracer,0.06)
 if not result then return end
 local model=result.Instance:FindFirstAncestorOfClass('Model'); local target=model and model:FindFirstChildOfClass('Humanoid')
 if not target or target.Health<=0 or model:FindFirstChildOfClass('ForceField') then return end
 local victim=Players:GetPlayerFromCharacter(model)
 if victim and victim.Team==p.Team then return end
 local wasAlive=target.Health>0
 target:TakeDamage(result.Instance.Name=='Head' and 48 or 24)
 remote:FireClient(p,'Hit',target.Health<=0)
 if victim and wasAlive and target.Health<=0 then
  p.leaderstats.Kills.Value=p.leaderstats.Kills.Value+1
  scores[p.Team.Name]=scores[p.Team.Name]+1
 end
end)
while true do
 active=false
 for t=10,1,-1 do status.Value='NEXT MATCH  /  '..t; task.wait(1) end
 scores.Azure=0; scores.Ember=0
 for _,p in ipairs(Players:GetPlayers()) do p:LoadCharacter() end
 active=true
 for remaining=300,0,-1 do
  status.Value=string.format('AZURE %02d   /   %d:%02d   /   %02d EMBER',scores.Azure,math.floor(remaining/60),remaining%60,scores.Ember)
  if remaining==0 or scores.Azure>=30 or scores.Ember>=30 then break end
  task.wait(1)
 end
 active=false
 status.Value=scores.Azure==scores.Ember and 'DRAW' or ((scores.Azure>scores.Ember and 'AZURE' or 'EMBER')..' WINS')
 task.wait(6)
end
