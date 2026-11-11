-- BLOCKLINE: authoritative menu, profiles, rewards, modes, and combat.
local Players=game:GetService('Players')
local RS=game:GetService('ReplicatedStorage')
local DSS=game:GetService('DataStoreService')
local Teams=game:GetService('Teams')
local Debris=game:GetService('Debris')
local Http=game:GetService('HttpService')
local store=DSS:GetDataStore('Blockline_Profile_v1')
local folder=Instance.new('Folder'); folder.Name='Blockline'; folder.Parent=RS
local request=Instance.new('RemoteFunction',folder); request.Name='Request'
local event=Instance.new('RemoteEvent',folder); event.Name='Event'
local action=Instance.new('RemoteEvent',folder); action.Name='Combat'
local status=Instance.new('StringValue',folder); status.Name='MatchStatus'; status.Value='Waiting for players'
local arena=workspace:WaitForChild('BlocklineArena')
local profiles={}; local combat={}; local locks={}; local lastRequest={}; local ranges={}; local slots={}
local session=game.JobId~='' and game.JobId or Http:GenerateGUID(false)
local weapons={
 Rifle={Name='AR-24',Price=0,Magazine=30,Damage=24,Head=48,Interval=.11,Reload=2,Range=500},
 SMG={Name='VX-9',Price=300,Magazine=32,Damage=18,Head=32,Interval=.08,Reload=1.7,Range=300},
 Marksman={Name='DMR-7',Price=600,Magazine=10,Damage=45,Head=80,Interval=.36,Reload=2.4,Range=650},
}
local scores={Azure=0,Ember=0}; local roundActive=false
for i,name in ipairs({'Azure','Ember'}) do
 local team=Instance.new('Team',Teams); team.Name=name; team.TeamColor=BrickColor.new(i==1 and 'Bright blue' or 'Bright orange'); team.AutoAssignable=false
end
local lobby=Instance.new('SpawnLocation',workspace); lobby.Name='Menu spawn'; lobby.Size=Vector3.new(40,2,40); lobby.Position=Vector3.new(0,101,350); lobby.Anchored=true; lobby.Neutral=true; lobby.Transparency=1; lobby.CanCollide=true; lobby.Duration=0
local function defaults()
 return {Coins=0,XP=0,Kills=0,Deaths=0,Matches=0,Wins=0,LastDaily=0,Equipped='Rifle',Owned={Rifle=true},Settings={FOV=80,Sensitivity=1,ReducedMotion=false,Crosshair=true}}
end
local function normalize(value)
 local p=defaults(); if type(value)~='table' then return p end
 for _,key in ipairs({'Coins','XP','Kills','Deaths','Matches','Wins','LastDaily'}) do if type(value[key])=='number' and value[key]==value[key] then p[key]=math.max(0,math.floor(value[key])) end end
 if type(value.Owned)=='table' then for id in pairs(weapons) do if value.Owned[id]==true then p.Owned[id]=true end end end
 if weapons[value.Equipped] and p.Owned[value.Equipped] then p.Equipped=value.Equipped end
 if type(value.Settings)=='table' then
  for _,key in ipairs({'FOV','Sensitivity'}) do local n=value.Settings[key]; if type(n)=='number' and n==n then p.Settings[key]=math.clamp(n,key=='FOV' and 60 or .2,key=='FOV' and 110 or 2) end end
  for _,key in ipairs({'ReducedMotion','Crosshair'}) do if type(value.Settings[key])=='boolean' then p.Settings[key]=value.Settings[key] end end
 end
 return p
end
local function save(p,release)
 local state=profiles[p]; if not state or not state.Persistent then return false end
 if state.Saving then return false end
 state.Saving=true
 local snapshot=Http:JSONDecode(Http:JSONEncode(state.Data))
 local ok,result=pcall(function()
  return store:UpdateAsync('u'..p.UserId,function(old)
   if type(old)~='table' or old.Session~=session then return nil end
   return {Data=snapshot,Session=release and '' or session,Expires=release and 0 or os.time()+150}
  end)
 end)
 state.Saving=false
 if not ok or not result then
  state.Persistent=false; state.Notice='Cloud saving unavailable. This session will not overwrite saved data.'
  if p.Parent then event:FireClient(p,'Notice',state.Notice) end
  return false
 end
 return true
end
local function public(p)
 local s=profiles[p]; if not s then return {Loading=true} end
 local d=s.Data; local roster={}
 for _,q in ipairs(Players:GetPlayers()) do local qs=profiles[q]; if qs then table.insert(roster,{Username=q.Name,Team=q.Team and q.Team.Name or 'Lobby',Mode=qs.Mode}) end end
 return {Data=d,Level=1+math.floor(d.XP/250),Mode=s.Mode,Persistent=s.Persistent,Notice=s.Notice,Weapons=weapons,Roster=roster,DailyReady=d.LastDaily+86400<=os.time(),DailyRemaining=math.max(0,d.LastDaily+86400-os.time()),Match=status.Value}
end
local function push(p) if p.Parent then event:FireClient(p,'Profile',public(p)) end end
local function roster() for p in pairs(profiles) do push(p) end end
local function block(name,size,pos,parent,color)
 local b=Instance.new('Part'); b.Name=name; b.Size=size; b.Position=pos; b.Anchored=true; b.Color=color or Color3.fromRGB(14,15,18); b.Material=Enum.Material.SmoothPlastic; b.Parent=parent; return b
end
local function clearRange(p)
 if ranges[p] then ranges[p]:Destroy(); ranges[p]=nil end
end
local function createRange(p)
 clearRange(p)
 local slot=slots[p]; local origin=Vector3.new(600+slot*90,0,400)
 local range=Instance.new('Model',workspace); range.Name=p.Name..' practice'; ranges[p]=range
 block('Practice floor',Vector3.new(64,4,100),origin+Vector3.new(0,-2,0),range,Color3.fromRGB(48,50,54))
 for _,x in ipairs({-32,32}) do block('Range wall',Vector3.new(2,24,100),origin+Vector3.new(x,10,0),range) end
 block('Backstop',Vector3.new(64,24,2),origin+Vector3.new(0,10,-50),range)
 block('Rear wall',Vector3.new(64,24,2),origin+Vector3.new(0,10,50),range)
 for i,x in ipairs({-18,0,18}) do
  local model=Instance.new('Model',range); model.Name='Practice target'; model:SetAttribute('Owner',p.UserId)
  local z=-12-i*8
  block('Torso',Vector3.new(3,4,1.5),origin+Vector3.new(x,3,z),model,Color3.fromRGB(220,28,44))
  block('Head',Vector3.new(2,2,2),origin+Vector3.new(x,6,z),model,Color3.fromRGB(255,50,60))
  local function reset()
   if not model.Parent then return end
   local h=Instance.new('Humanoid',model); h.RequiresNeck=false; h.BreakJointsOnDeath=false
   h.Died:Connect(function() task.delay(1.5,function() if model.Parent then h:Destroy(); reset() end end) end)
  end
  reset()
 end
 return origin+Vector3.new(0,4,35)
end
local function sendAmmo(p)
 local c=combat[p]; if c then event:FireClient(p,'Ammo',{Ammo=c.Ammo,Reloading=c.Reloading,Name=weapons[c.Weapon].Name,Magazine=weapons[c.Weapon].Magazine}) end
end
local function clearWeapon(p)
 for _,parent in ipairs({p.Character,p:FindFirstChildOfClass('Backpack')}) do
  if parent then for _,t in ipairs(parent:GetChildren()) do if t:IsA('Tool') then t:Destroy() end end end
 end
 combat[p]=nil
end
local function equip(p)
 clearWeapon(p)
 local s=profiles[p]; local char=p.Character; local hum=char and char:FindFirstChildOfClass('Humanoid')
 if not s or s.Mode=='Lobby' or not hum or hum.Health<=0 then return end
 local id=s.Data.Equipped; local w=weapons[id]
 local t=Instance.new('Tool'); t.Name=w.Name; t.CanBeDropped=false; t:SetAttribute('BlocklineWeapon',true)
 local handle=Instance.new('Part',t); handle.Name='Handle'; handle.Size=Vector3.new(.4,.7,id=='SMG' and 1.8 or 2.8); handle.CanCollide=false; handle.Massless=true; handle.Color=Color3.fromRGB(20,22,26)
 local barrel=Instance.new('Part',t); barrel.Name='Barrel'; barrel.Size=Vector3.new(.18,.18,id=='Marksman' and 2.5 or 1.3); barrel.CanCollide=false; barrel.Massless=true; barrel.Color=Color3.fromRGB(230,24,44); barrel.CFrame=handle.CFrame*CFrame.new(0,.1,-1.7)
 local weld=Instance.new('WeldConstraint',handle); weld.Part0=handle; weld.Part1=barrel
 t.Parent=p:WaitForChild('Backpack'); hum:EquipTool(t)
 combat[p]={Weapon=id,Ammo=w.Magazine,Last=0,Reloading=false,Sprint=false}; sendAmmo(p)
end
local function spawnPoint(p)
 local s=profiles[p]; if not s or s.Mode=='Lobby' then return lobby.Position+Vector3.new(0,4,0) end
 if s.Mode=='Practice' then return s.PracticePosition end
 local x=p.Team and p.Team.Name=='Azure' and -143 or 143
 return Vector3.new(x,4,math.random(0,1)==0 and -54 or 54)
end
local function positionCharacter(p,char)
 local hum=char:WaitForChild('Humanoid',10); local root=char:WaitForChild('HumanoidRootPart',10)
 if not hum or not root or p.Character~=char then return end
 local s=profiles[p]; if not s then return end
 local pos=spawnPoint(p); local look=s.Mode=='Practice' and pos+Vector3.new(0,0,-1) or Vector3.new(0,pos.Y,0)
 char:PivotTo(CFrame.lookAt(pos,look)); root.Anchored=s.Mode=='Lobby'; hum.WalkSpeed=16
 if s.Mode~='Lobby' then equip(p) end
 if not hum:GetAttribute('BlocklineDeathHook') then
  hum:SetAttribute('BlocklineDeathHook',true)
  hum.Died:Connect(function()
   if profiles[p] and s.Mode=='Arena' then s.Data.Deaths=s.Data.Deaths+1; push(p) end
  end)
 end
end
local function enter(p,mode)
 local s=profiles[p]; clearWeapon(p); clearRange(p); s.Mode=mode; s.RoundParticipant=false
 p.Neutral=true; p.Team=nil; p.RespawnLocation=lobby
 if mode=='Practice' then s.PracticePosition=createRange(p)
 elseif mode=='Arena' then
  local a,b=0,0
  for _,q in ipairs(Players:GetPlayers()) do if q.Team then if q.Team.Name=='Azure' then a=a+1 else b=b+1 end end end
  p.Team=Teams:FindFirstChild(a<=b and 'Azure' or 'Ember'); p.Neutral=false; p.RespawnLocation=nil; s.RoundParticipant=roundActive
 end
 if p.Character then positionCharacter(p,p.Character) else p:LoadCharacter() end
 roster()
end
local function loadPlayer(p)
 local slot=1
 while true do local used=false; for _,n in pairs(slots) do if n==slot then used=true end end; if not used then break end; slot=slot+1 end
 slots[p]=slot; p.RespawnLocation=lobby
 local stored; local locked=false
 local ok,result=pcall(function()
  return store:UpdateAsync('u'..p.UserId,function(old)
   if type(old)=='table' and old.Session and old.Session~='' and old.Session~=session and (old.Expires or 0)>os.time() then locked=true; return nil end
   stored=type(old)=='table' and (old.Data or old) or nil
   return {Data=normalize(stored),Session=session,Expires=os.time()+150}
  end)
 end)
 if not p.Parent then
  if ok and result then pcall(function() store:UpdateAsync('u'..p.UserId,function(old) if old and old.Session==session then old.Session=''; old.Expires=0; return old end; return nil end) end) end
  return
 end
 if locked then p:Kick('Your profile is open in another server. Please rejoin in a moment.'); return end
 profiles[p]={Data=normalize(ok and result and result.Data or nil),Persistent=ok and result~=nil,Mode='Lobby',Notice=ok and result and '' or 'Cloud data unavailable. Session only; publish and enable Studio API access to test saving.'}
 p.CharacterAdded:Connect(function(char) positionCharacter(p,char) end)
 if p.Character then task.spawn(positionCharacter,p,p.Character) end
 push(p); roster()
end
Players.PlayerAdded:Connect(loadPlayer)
for _,p in ipairs(Players:GetPlayers()) do task.spawn(loadPlayer,p) end
Players.PlayerRemoving:Connect(function(p)
 local s=profiles[p]; if s then while s.Saving do task.wait(.1) end; save(p,true) end
 clearRange(p); slots[p]=nil; profiles[p]=nil; combat[p]=nil; locks[p]=nil; lastRequest[p]=nil; roster()
end)
request.OnServerInvoke=function(p,kind,payload)
 local s=profiles[p]; if not s then return {OK=false,Loading=true,Message='Loading profile'} end
 if kind=='Fetch' then return {OK=true,Profile=public(p)} end
 if locks[p] then return {OK=false,Message='Please wait for the current request.'} end
 if os.clock()-(lastRequest[p] or 0)<.2 then return {OK=false,Message='Please wait a moment.'} end
 lastRequest[p]=os.clock(); locks[p]=true
 local ok,result=pcall(function()
  if kind=='Enter' then
   if payload~='Arena' and payload~='Practice' and payload~='Lobby' then return {OK=false,Message='Unknown mode'} end
   enter(p,payload); return {OK=true,Profile=public(p)}
  elseif kind=='Settings' then
   if type(payload)~='table' then return {OK=false,Message='Invalid settings'} end
   local copy={Settings=payload}; s.Data.Settings=normalize(copy).Settings
   local persisted=save(p,false)
   return {OK=true,Profile=public(p),Message=persisted and 'Settings saved to your account.' or 'Settings applied for this session. Cloud saving is unavailable.'}
  elseif kind=='Loadout' then
   if not weapons[payload] then return {OK=false,Message='Unknown weapon'} end
   if not s.Data.Owned[payload] then return {OK=false,Message='Unlock this weapon first.'} end
   s.Data.Equipped=payload; if s.Mode~='Lobby' then equip(p) end
   return {OK=true,Profile=public(p),Message='Weapon equipped.'}
  elseif kind=='Purchase' then
   if not weapons[payload] then return {OK=false,Message='Unknown weapon'} end
   if not s.Persistent then return {OK=false,Message='Cloud saving must be available to purchase unlocks.'} end
   if s.Data.Owned[payload] then return {OK=false,Message='Already owned.'} end
   local price=weapons[payload].Price
   if s.Data.Coins<price then return {OK=false,Message='Not enough credits. Earn credits in arena matches.'} end
   s.Data.Coins=s.Data.Coins-price; s.Data.Owned[payload]=true
   if not save(p,false) then return {OK=false,Profile=public(p),Message='Saving failed. Unlock is session-only; your cloud balance was not changed.'} end
   return {OK=true,Profile=public(p),Message='Weapon unlocked.'}
  elseif kind=='Daily' then
   if not s.Persistent then return {OK=false,Message='Cloud saving must be available to claim rewards.'} end
   if os.time()-s.Data.LastDaily<86400 then return {OK=false,Message='Your daily reward is not ready yet.'} end
   s.Data.LastDaily=os.time(); s.Data.Coins=s.Data.Coins+100
   if not save(p,false) then return {OK=false,Profile=public(p),Message='Cloud save failed. Reward is session-only; please rejoin when saving is available.'} end
   return {OK=true,Profile=public(p),Message='Daily reward claimed: +100 credits.'}
  end
  return {OK=false,Message='Unknown request'}
 end)
 locks[p]=nil
 if not ok then warn('Blockline request failed:',result); return {OK=false,Message='Request failed. Please try again.'} end
 return result
end
local function armed(p)
 local s=profiles[p]; local c=combat[p]; local char=p.Character; local hum=char and char:FindFirstChildOfClass('Humanoid'); local t=char and char:FindFirstChildOfClass('Tool')
 return s and c and s.Mode~='Lobby' and hum and hum.Health>0 and t and t:GetAttribute('BlocklineWeapon'),s,c,char,hum
end
action.OnServerEvent:Connect(function(p,kind,direction)
 local valid,s,c,char,hum=armed(p); if not valid then return end
 local w=weapons[c.Weapon]
 if kind=='Sprint' then c.Sprint=direction==true; hum.WalkSpeed=c.Sprint and 23 or 16; return end
 if kind=='Reload' then
  if c.Reloading or c.Ammo==w.Magazine then return end
  c.Reloading=true; sendAmmo(p)
  task.delay(w.Reload,function() if combat[p]==c then c.Ammo=w.Magazine; c.Reloading=false; sendAmmo(p) end end); return
 end
 if kind~='Shoot' or c.Reloading or c.Sprint or c.Ammo<=0 or os.clock()-c.Last<w.Interval*.95 then return end
 if s.Mode=='Arena' and not roundActive then return end
 if typeof(direction)~='Vector3' then return end
 local magnitude=direction.Magnitude; if magnitude~=magnitude or magnitude<.9 or magnitude>1.1 then return end
 local head=char:FindFirstChild('Head'); if not head then return end
 local expected=spawnPoint(p); if (head.Position-expected).Magnitude>420 then return end
 c.Last=os.clock(); c.Ammo=c.Ammo-1; sendAmmo(p)
 local params=RaycastParams.new(); params.FilterType=Enum.RaycastFilterType.Exclude; params.FilterDescendantsInstances={char}
 local origin=head.Position; local hit=workspace:Raycast(origin,direction.Unit*w.Range,params); local finish=hit and hit.Position or origin+direction.Unit*w.Range
 local tracer=block('Tracer',Vector3.new(.06,.06,(finish-origin).Magnitude),(origin+finish)/2,workspace,Color3.fromRGB(255,75,85)); tracer.CanCollide=false; tracer.CanQuery=false; tracer.CanTouch=false; tracer.Material=Enum.Material.Neon; tracer.CFrame=CFrame.lookAt((origin+finish)/2,finish); Debris:AddItem(tracer,.06)
 if not hit then return end
 local model=hit.Instance:FindFirstAncestorOfClass('Model'); local target=model and model:FindFirstChildOfClass('Humanoid')
 if not target or target.Health<=0 or model:FindFirstChildOfClass('ForceField') then return end
 local victim=Players:GetPlayerFromCharacter(model)
 if s.Mode=='Practice' then if victim or model:GetAttribute('Owner')~=p.UserId then return end
 else
  if not victim or not profiles[victim] or profiles[victim].Mode~='Arena' or victim.Team==p.Team then return end
 end
 target:TakeDamage(hit.Instance.Name=='Head' and w.Head or w.Damage); event:FireClient(p,'Hit',target.Health<=0)
 if target.Health<=0 and victim then
  s.Data.Kills=s.Data.Kills+1; s.Data.XP=s.Data.XP+50; s.Data.Coins=s.Data.Coins+20
  scores[p.Team.Name]=scores[p.Team.Name]+1; push(p)
 end
end)
task.spawn(function()
 while true do task.wait(35); for p,s in pairs(profiles) do if not locks[p] then task.spawn(function() if profiles[p] and not locks[p] then locks[p]=true; save(p,false); locks[p]=nil end end) end end end
end)
game:BindToClose(function()
 local pending=0
 for p in pairs(profiles) do pending=pending+1; task.spawn(function() local s=profiles[p]; while s and s.Saving do task.wait(.1) end; save(p,true); pending=pending-1 end) end
 local start=os.clock(); while pending>0 and os.clock()-start<25 do task.wait(.1) end
end)
while true do
 roundActive=false
 local function arenaCount()
  local count=0; for _,s in pairs(profiles) do if s.Mode=='Arena' then count=count+1 end end; return count
 end
 if arenaCount()<2 then status.Value='ARENA / Waiting for 2 players'; task.wait(1)
 else
  local ready=true
  for n=8,1,-1 do
   if arenaCount()<2 then ready=false; break end
   status.Value='MATCH STARTS / '..n; task.wait(1)
  end
  if ready and arenaCount()>=2 then
   scores.Azure=0; scores.Ember=0; roundActive=true
   for p,s in pairs(profiles) do if s.Mode=='Arena' then s.RoundParticipant=true; p:LoadCharacter() end end
   local abandoned=false
   for left=300,0,-1 do
    if arenaCount()<2 then abandoned=true; break end
    status.Value=string.format('AZURE %02d  /  %d:%02d  /  %02d EMBER',scores.Azure,math.floor(left/60),left%60,scores.Ember)
    if left==0 or scores.Azure>=30 or scores.Ember>=30 then break end
    task.wait(1)
   end
   roundActive=false
   local winner=scores.Azure==scores.Ember and 'DRAW' or (scores.Azure>scores.Ember and 'Azure' or 'Ember')
   status.Value=abandoned and 'MATCH ENDED / Not enough players' or (winner=='DRAW' and 'MATCH DRAW' or string.upper(winner)..' WINS')
   if not abandoned then
    for p,s in pairs(profiles) do if s.Mode=='Arena' and s.RoundParticipant then
     s.Data.Matches=s.Data.Matches+1; s.Data.XP=s.Data.XP+100; s.Data.Coins=s.Data.Coins+25
     if p.Team and p.Team.Name==winner then s.Data.Wins=s.Data.Wins+1; s.Data.Coins=s.Data.Coins+50 end
     push(p)
    end end
   end
   task.wait(7)
  end
 end
end
