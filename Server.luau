-- BLOCKLINE: original urban team arena. All combat decisions run on server.
local Players = game:GetService('Players')
local Teams = game:GetService('Teams')
local RS = game:GetService('ReplicatedStorage')
local Debris = game:GetService('Debris')
local Lighting = game:GetService('Lighting')
local remote = Instance.new('RemoteEvent', RS); remote.Name = 'BlocklineAction'
local status = Instance.new('StringValue', RS); status.Name = 'BlocklineStatus'
local arena = Instance.new('Folder', workspace); arena.Name = 'BlocklineCity'
local states = {}
local scores = {Azure = 0, Ember = 0}
local active = false
local function part(name, size, pos, color, material)
 local p = Instance.new('Part'); p.Name = name; p.Size = size; p.Position = pos
 p.Anchored = true; p.Color = color; p.Material = material or Enum.Material.Concrete; p.Parent = arena
 return p
end
local gray = Color3.fromRGB(59,65,76)
local blue = Color3.fromRGB(53,182,255)
local orange = Color3.fromRGB(255,122,65)
Lighting.ClockTime = 17.4; Lighting.Brightness = 2
Lighting.Ambient = Color3.fromRGB(125,132,152)
workspace.FallenPartsDestroyHeight = -50
part('City foundation', Vector3.new(220,3,180), Vector3.new(0,-2,0), gray)
part('Main road', Vector3.new(200,0.3,32), Vector3.new(0,-0.3,0), Color3.fromRGB(30,33,40), Enum.Material.Asphalt)
for x=-90,90,18 do
 part('Road stripe',Vector3.new(8,0.1,0.4),Vector3.new(x,-0.1,0),Color3.fromRGB(244,207,105))
end
for _,z in ipairs({-90,90}) do part('Boundary',Vector3.new(224,25,3),Vector3.new(0,10,z),gray) end
for _,x in ipairs({-110,110}) do part('Boundary',Vector3.new(3,25,180),Vector3.new(x,10,0),gray) end
local function sign(parent, text, color)
 local gui=Instance.new('SurfaceGui',parent); gui.Face=Enum.NormalId.Front
 local label=Instance.new('TextLabel',gui); label.Size=UDim2.fromScale(1,1); label.BackgroundTransparency=1
 label.Text=text; label.TextColor3=color; label.Font=Enum.Font.GothamBold; label.TextScaled=true
end
for _,z in ipairs({-55,55}) do
 for i,x in ipairs({-68,-23,23,68}) do
  local h = 23 + (i%3)*7
  part('City block',Vector3.new(33,h,35),Vector3.new(x,h/2,z),Color3.fromRGB(69+i*9,73+i*7,87+i*7))
  for y=8,h-3,8 do
   for dx=-10,10,10 do
    part('Window',Vector3.new(4,4,0.2),Vector3.new(x+dx,y,z+(z<0 and 17.6 or -17.6)),Color3.fromRGB(168,203,213),Enum.Material.Glass)
   end
  end
  local awning=part('Shop sign',Vector3.new(26,4,0.6),Vector3.new(x,5,z-17.9),gray)
  sign(awning,({'METRO','NIGHT MARKET','BLOCKLINE','NOODLE BAR'})[i],blue)
 end
end
for _,pos in ipairs({Vector3.new(-40,2,9),Vector3.new(35,2,-9),Vector3.new(0,2,24),Vector3.new(0,2,-24)}) do
 part('Street cover',Vector3.new(10,4,5),pos,Color3.fromRGB(91,111,103))
end
for _,x in ipairs({-60,60}) do
 local car=part('Parked car',Vector3.new(13,3,7),Vector3.new(x,1.5,12),x<0 and blue or orange,Enum.Material.Metal)
 part('Car roof',Vector3.new(7,2,6),car.Position+Vector3.new(0,2.5,0),gray,Enum.Material.Metal)
end
for _,x in ipairs({-88,0,88}) do
 for _,z in ipairs({-22,22}) do
  part('Lamp post',Vector3.new(0.5,17,0.5),Vector3.new(x,8,z),gray,Enum.Material.Metal)
  local bulb=part('Street light',Vector3.new(3,0.4,2),Vector3.new(x,16.7,z),Color3.fromRGB(255,224,160),Enum.Material.Neon)
  local light=Instance.new('PointLight',bulb); light.Range=24; light.Brightness=1.5
 end
end
for i,name in ipairs({'Azure','Ember'}) do
 local team=Instance.new('Team',Teams); team.Name=name; team.TeamColor=BrickColor.new(i==1 and 'Bright blue' or 'Bright orange'); team.AutoAssignable=false
 local spawn=Instance.new('SpawnLocation',arena); spawn.Name=name..' spawn'; spawn.Size=Vector3.new(16,1,22)
 spawn.Position=Vector3.new(i==1 and -96 or 96,0.5,0); spawn.Anchored=true; spawn.Neutral=false
 spawn.TeamColor=team.TeamColor; spawn.Color=i==1 and blue or orange; spawn.Duration=3
 part('Spawn cover',Vector3.new(3,10,17),Vector3.new(i==1 and -83 or 83,4.5,0),gray)
end
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
Players.PlayerAdded:Connect(function(p)
 local a,b=0,0
 for _,other in ipairs(Players:GetPlayers()) do if other.Team and other.Team.Name=='Azure' then a=a+1 elseif other.Team then b=b+1 end end
 p.Team=Teams:FindFirstChild(a<=b and 'Azure' or 'Ember')
 local stats=Instance.new('Folder',p); stats.Name='leaderstats'
 for _,name in ipairs({'Kills','Deaths'}) do local v=Instance.new('IntValue',stats); v.Name=name end
 p.CharacterAdded:Connect(function(char) equip(p,char) end)
 if p.Character then equip(p,p.Character) end
end)
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
