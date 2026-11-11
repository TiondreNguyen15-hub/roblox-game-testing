-- BLOCKLINE: authoritative menu, profiles, rewards, modes, and combat.
local Players=game:GetService('Players')
local RS=game:GetService('ReplicatedStorage')
local DSS=game:GetService('DataStoreService')
local Teams=game:GetService('Teams')
local Http=game:GetService('HttpService')
local Run=game:GetService('RunService')
local Debris=game:GetService('Debris')
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
 Rifle={Name='AR-24',Price=0,Magazine=30,Damage=24,Head=48,Interval=.11,Reload=2,Range=500,Spread=.6,AimSpread=.1,Recoil=.7},
 SMG={Name='VX-9',Price=300,Magazine=32,Damage=18,Head=32,Interval=.08,Reload=1.7,Range=300,Spread=1.1,AimSpread=.3,Recoil=.5},
 Marksman={Name='DMR-7',Price=600,Magazine=10,Damage=45,Head=80,Interval=.36,Reload=2.4,Range=650,Spread=.2,AimSpread=.02,Recoil=1.5},
}
local modes={
 Arena={Name='TEAM DEATHMATCH',Offset=Vector3.new(),Limit=30,Duration=300,Active=false,Text='Waiting for 2 players'},
 FFA={Name='FREE FOR ALL',Offset=Vector3.new(0,0,800),Limit=20,Duration=300,Active=false,Text='Waiting for 2 players'},
 Duel={Name='1V1 DUEL',Offset=Vector3.new(0,0,1600),Limit=5,Duration=180,Active=false,Text='Waiting for opponent'},
}
for _,cfg in pairs(modes) do cfg.Scores={Azure=0,Ember=0}; cfg.Round=0 end
modes.Arena.Map=arena
for _,part in ipairs(arena:GetDescendants()) do if part:IsA('SpawnLocation') then part.Enabled=false end end
local function ensureArena(id)
 local cfg=modes[id]; if cfg.Map then return end
 local map=arena:Clone(); map.Name='Arena_'..id
 for _,part in ipairs(map:GetDescendants()) do
  if part:IsA('BasePart') then part.Position=part.Position+cfg.Offset end
  if part:IsA('SpawnLocation') then part.Enabled=false end
 end
 map.Parent=workspace; cfg.Map=map
end
local function competitive(s) return s and modes[s.Mode]~=nil end
local function participants(id)
 local list={}; for p,s in pairs(profiles) do if p.Parent and s.Mode==id then table.insert(list,p) end end
 table.sort(list,function(a,b)return a.UserId<b.UserId end); return list
end
local function trainingPacket(s)
 local t=s.Training or {Shots=0,Hits=0,Eliminations=0,ReactionTotal=0,Reactions=0,Drill='Stationary'}
 return {Drill=t.Drill,Shots=t.Shots,Hits=t.Hits,Eliminations=t.Eliminations,Accuracy=t.Shots>0 and t.Hits/t.Shots*100 or 0,ReactionMs=t.Reactions>0 and t.ReactionTotal/t.Reactions*1000 or 0}
end
local scores={Azure=0,Ember=0}; local roundActive=false
local metrics={AcceptedShots=0,RejectedShots=0,Raycasts=0,Hits=0}
local function finite(n) return type(n)=='number' and n==n and math.abs(n)<math.huge end
for i,name in ipairs({'Azure','Ember'}) do
 local team=Instance.new('Team',Teams); team.Name=name; team.TeamColor=BrickColor.new(i==1 and 'Bright blue' or 'Bright orange'); team.AutoAssignable=false
end
local lobby=Instance.new('SpawnLocation',workspace); lobby.Name='Menu spawn'; lobby.Size=Vector3.new(40,2,40); lobby.Position=Vector3.new(0,101,350); lobby.Anchored=true; lobby.Neutral=true; lobby.Transparency=1; lobby.CanCollide=true; lobby.Duration=0
local function defaults()
 return {Coins=0,XP=0,Kills=0,Deaths=0,Matches=0,Wins=0,LastDaily=0,Equipped='Rifle',Owned={Rifle=true},Settings={FOV=80,Sensitivity=1,ReducedMotion=false,Crosshair=true,LowGraphics=false}}
end
local function normalize(value)
 local p=defaults(); if type(value)~='table' then return p end
 for _,key in ipairs({'Coins','XP','Kills','Deaths','Matches','Wins','LastDaily'}) do if finite(value[key]) then p[key]=math.max(0,math.floor(value[key])) end end
 if type(value.Owned)=='table' then for id in pairs(weapons) do if value.Owned[id]==true then p.Owned[id]=true end end end
 if weapons[value.Equipped] and p.Owned[value.Equipped] then p.Equipped=value.Equipped end
 if type(value.Settings)=='table' then
  for _,key in ipairs({'FOV','Sensitivity'}) do local n=value.Settings[key]; if finite(n) then p.Settings[key]=math.clamp(n,key=='FOV' and 60 or .2,key=='FOV' and 110 or 2) end end
  for _,key in ipairs({'ReducedMotion','Crosshair','LowGraphics'}) do if type(value.Settings[key])=='boolean' then p.Settings[key]=value.Settings[key] end end
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
   if type(old)~='table' or old.Session~=state.Token then return nil end
   return {Data=snapshot,Session=release and '' or state.Token,Expires=release and 0 or os.time()+150}
  end)
 end)
 state.Saving=false
 if not ok or not result then
  state.Persistent=false; state.Notice='Cloud saving unavailable. This session will not overwrite saved data.'
  if p.Parent then event:FireClient(p,'Persistence',state.Notice) end
  return false
 end
 return true
end
local cachedRoster={}
local function public(p)
 local s=profiles[p]; if not s then return {Loading=true} end
 local d=s.Data
 return {Data=d,Level=1+math.floor(d.XP/250),Mode=s.Mode,Persistent=s.Persistent,Notice=s.Notice,Weapons=weapons,Roster=cachedRoster,DailyReady=d.LastDaily+86400<=os.time(),DailyRemaining=math.max(0,d.LastDaily+86400-os.time()),Match=modes[s.Mode] and modes[s.Mode].Text or status.Value,Training=trainingPacket(s),RoundKills=s.RoundKills or 0,RoundDeaths=s.RoundDeaths or 0}
end
local function push(p) if p.Parent then event:FireClient(p,'Profile',public(p)) end end
local function roster()
 cachedRoster={}
 for _,q in ipairs(Players:GetPlayers()) do local qs=profiles[q]; if qs then table.insert(cachedRoster,{Username=q.Name,Team=q.Team and q.Team.Name or 'Lobby',Mode=qs.Mode}) end end
 for p in pairs(profiles) do if p.Parent then event:FireClient(p,'Roster',cachedRoster) end end
end
local function block(name,size,pos,parent,color)
 local b=Instance.new('Part'); b.Name=name; b.Size=size; b.Position=pos; b.Anchored=true; b.CanTouch=false; b.Color=color or Color3.fromRGB(14,15,18); b.Material=Enum.Material.SmoothPlastic; b.Parent=parent; return b
end
local function clearRange(p)
 if ranges[p] then ranges[p]:Destroy(); ranges[p]=nil end
end
local function createRange(p)
 clearRange(p)
 local slot=slots[p]; local origin=Vector3.new(600+slot*90,0,400)
 local state=profiles[p]; local drill=state.Training and state.Training.Drill or 'Stationary'
 local range=Instance.new('Model',workspace); range.Name=p.Name..' practice'; ranges[p]=range
 block('Practice floor',Vector3.new(64,4,100),origin+Vector3.new(0,-2,0),range,Color3.fromRGB(48,50,54))
 for _,x in ipairs({-32,32}) do block('Range wall',Vector3.new(2,24,100),origin+Vector3.new(x,10,0),range) end
 block('Backstop',Vector3.new(64,24,2),origin+Vector3.new(0,10,-50),range)
 block('Rear wall',Vector3.new(64,24,2),origin+Vector3.new(0,10,50),range)
 for i,x in ipairs({-18,0,18}) do
  local model=Instance.new('Model',range); model.Name='Practice target'; model:SetAttribute('Owner',p.UserId); model:SetAttribute('TargetIndex',i)
  model:SetAttribute('BaseX',origin.X+x); model:SetAttribute('BaseZ',origin.Z-12-i*8)
  local z=-12-i*8
  block('Torso',Vector3.new(3,4,1.5),origin+Vector3.new(x,3,z),model,Color3.fromRGB(220,28,44))
  block('Head',Vector3.new(2,2,2),origin+Vector3.new(x,6,z),model,Color3.fromRGB(255,50,60))
  local function reset()
   if not model.Parent then return end
   model:SetAttribute('ReadyAt',os.clock()); model:SetAttribute('Active',drill~='Reaction')
   for _,part in ipairs(model:GetChildren()) do if part:IsA('BasePart') then part.CanCollide=false; part.CanQuery=drill~='Reaction'; part.Transparency=drill=='Reaction' and 1 or 0 end end
   if drill=='Reaction' then
    task.delay(1+math.random()*2,function()
     if not model.Parent then return end
     model:SetAttribute('Active',true); model:SetAttribute('ReadyAt',os.clock())
     for _,part in ipairs(model:GetChildren()) do if part:IsA('BasePart') then part.Transparency=0; part.CanQuery=true end end
    end)
   end
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
 local s=profiles[p]; local c=combat[p]
 if s and c then s.Magazines=s.Magazines or {}; s.Magazines[c.Weapon]=c.Ammo end
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
 local handle=Instance.new('Part',t); handle.Name='Handle'; handle.Size=Vector3.new(.4,.7,id=='SMG' and 1.8 or 2.8); handle.CanCollide=false; handle.CanTouch=false; handle.CanQuery=false; handle.Massless=true; handle.Color=Color3.fromRGB(20,22,26)
 local barrel=Instance.new('Part',t); barrel.Name='Barrel'; barrel.Size=Vector3.new(.18,.18,id=='Marksman' and 2.5 or 1.3); barrel.CanCollide=false; barrel.CanTouch=false; barrel.CanQuery=false; barrel.Massless=true; barrel.Color=Color3.fromRGB(230,24,44); barrel.CFrame=handle.CFrame*CFrame.new(0,.1,-1.7)
 local weld=Instance.new('WeldConstraint',handle); weld.Part0=handle; weld.Part1=barrel
 t.Parent=p:WaitForChild('Backpack'); hum:EquipTool(t)
 local params=RaycastParams.new(); params.FilterType=Enum.RaycastFilterType.Exclude; params.FilterDescendantsInstances={char}
 combat[p]={Weapon=id,Ammo=s.Magazines and s.Magazines[id] or w.Magazine,Last=0,Reloading=false,Sprint=false,Params=params,Tokens=60,TokenTime=os.clock()}; sendAmmo(p)
end
local function spawnPoint(p)
 local s=profiles[p]; if not s or s.Mode=='Lobby' then return lobby.Position+Vector3.new(0,4,0) end
 if s.Mode=='Practice' then return s.PracticePosition end
 local cfg=modes[s.Mode]; local list=participants(s.Mode); local index=1
 for i,q in ipairs(list) do if q==p then index=i end end
 local x=(s.Mode=='Arena' and p.Team and p.Team.Name=='Azure') and -143 or 143
 local z=math.random(0,1)==0 and -54 or 54
 if s.Mode~='Arena' then x=index%2==1 and -143 or 143; z=s.Mode=='Duel' and 0 or (index%4<2 and -54 or 54) end
 return cfg.Offset+Vector3.new(x,4,z)
end
local function positionCharacter(p,char)
 local hum=char:WaitForChild('Humanoid',10); local root=char:WaitForChild('HumanoidRootPart',10)
 if not hum or not root or p.Character~=char then return end
 local s=profiles[p]; if not s then return end
 local pos=spawnPoint(p); local offset=modes[s.Mode] and modes[s.Mode].Offset or Vector3.new()
 local look=s.Mode=='Practice' and pos+Vector3.new(0,0,-1) or Vector3.new(offset.X,pos.Y,offset.Z)
 char:PivotTo(CFrame.lookAt(pos,look)); root.Anchored=s.Mode=='Lobby'; hum.WalkSpeed=16
 local hip=hum:GetAttribute('BlocklineBaseHip'); if hip==nil then hip=hum.HipHeight; hum:SetAttribute('BlocklineBaseHip',hip) end
 hum.HipHeight=hip; hum.UseJumpPower=true; hum.JumpPower=50; hum.AutoRotate=s.Mode=='Lobby'
 if s.Mode~='Lobby' then
  equip(p)
  if competitive(s) then
   local old=char:FindFirstChildOfClass('ForceField'); if old then old:Destroy() end
   local force=Instance.new('ForceField',char); force.Visible=true; Debris:AddItem(force,3)
  end
 end
 if not hum:GetAttribute('BlocklineDeathHook') then
  hum:SetAttribute('BlocklineDeathHook',true)
  hum.Died:Connect(function()
   if profiles[p] and competitive(s) and modes[s.Mode].Active then
    s.Data.Deaths=s.Data.Deaths+1; s.RoundDeaths=(s.RoundDeaths or 0)+1
    event:FireClient(p,'Death','Respawning…'); push(p)
   end
  end)
 end
end
local function enter(p,mode)
 local s=profiles[p]
 if mode=='Duel' and s.Mode~='Duel' and #participants('Duel')>=2 then return false,'Duel is full. Choose another mode or wait for a slot.' end
 if modes[mode] then ensureArena(mode) end
 clearWeapon(p); clearRange(p); s.Mode=mode; s.RoundParticipant=false
 p.Neutral=true; p.Team=nil; p.RespawnLocation=lobby
 if mode=='Practice' then
  s.Training={Drill=s.Drill or 'Stationary',Shots=0,Hits=0,Eliminations=0,ReactionTotal=0,Reactions=0}; s.PracticePosition=createRange(p)
 elseif mode=='Arena' then
  local a,b=0,0
  for _,q in ipairs(Players:GetPlayers()) do if q.Team then if q.Team.Name=='Azure' then a=a+1 else b=b+1 end end end
  p.Team=Teams:FindFirstChild(a<=b and 'Azure' or 'Ember'); p.Neutral=false; p.RespawnLocation=nil; s.RoundParticipant=modes.Arena.Active
 elseif modes[mode] then s.RoundParticipant=modes[mode].Active end
 s.RoundKills=0; s.RoundDeaths=0
 if p.Character then positionCharacter(p,p.Character) else p:LoadCharacter() end
 roster(); return true
end
local function loadPlayer(p)
 local slot=1
 while true do local used=false; for _,n in pairs(slots) do if n==slot then used=true end end; if not used then break end; slot=slot+1 end
 slots[p]=slot; p.RespawnLocation=lobby
 local stored; local locked=false
 local token=session..':'..Http:GenerateGUID(false)
 local ok,result=pcall(function()
  return store:UpdateAsync('u'..p.UserId,function(old)
   if type(old)=='table' and old.Session and old.Session~='' and old.Session~=token and (old.Expires or 0)>os.time() then locked=true; return nil end
   stored=type(old)=='table' and (old.Data or old) or nil
   return {Data=normalize(stored),Session=token,Expires=os.time()+150}
  end)
 end)
 if not p.Parent then
  if ok and result then pcall(function() store:UpdateAsync('u'..p.UserId,function(old) if old and old.Session==token then old.Session=''; old.Expires=0; return old end; return nil end) end) end
  return
 end
 if locked then p:Kick('Your profile is open in another server. Please rejoin in a moment.'); return end
 profiles[p]={Token=token,Data=normalize(ok and result and result.Data or nil),Persistent=ok and result~=nil,Mode='Lobby',Notice=ok and result and '' or 'Cloud data unavailable. Session only; publish and enable Studio API access to test saving.'}
 p.CharacterAdded:Connect(function(char)
  clearWeapon(p); if profiles[p] then profiles[p].Magazines={} end
  positionCharacter(p,char)
 end)
 if p.Character then task.spawn(positionCharacter,p,p.Character) end
 roster(); push(p)
end
Players.PlayerAdded:Connect(loadPlayer)
for _,p in ipairs(Players:GetPlayers()) do task.spawn(loadPlayer,p) end
Players.PlayerRemoving:Connect(function(p)
 local s=profiles[p]; if s then while s.Saving do task.wait(.1) end; save(p,true) end
 clearRange(p); slots[p]=nil; profiles[p]=nil; combat[p]=nil; locks[p]=nil; lastRequest[p]=nil; roster()
end)
request.OnServerInvoke=function(p,kind,payload)
 local s=profiles[p]; if not s then return {OK=false,Loading=true,Message='Loading profile'} end
  if locks[p] then return {OK=false,Message='Please wait for the current request.'} end
 if os.clock()-(lastRequest[p] or 0)<.2 then return {OK=false,Message='Please wait a moment.'} end
 lastRequest[p]=os.clock()
 if kind=='Fetch' then return {OK=true,Profile=public(p)} end
 locks[p]=true
 local ok,result=pcall(function()
  if kind=='Enter' then
   if not modes[payload] and payload~='Practice' and payload~='Lobby' then return {OK=false,Message='Unknown mode'} end
   local entered,message=enter(p,payload); return {OK=entered,Message=message,Profile=public(p)}
  elseif kind=='Drill' then
   local valid={Stationary=true,Moving=true,Tracking=true,Flick=true,Reaction=true}
   if not valid[payload] then return {OK=false,Message='Unknown drill'} end
   s.Drill=payload; enter(p,'Practice'); return {OK=true,Profile=public(p)}
  elseif kind=='Settings' then
   if type(payload)~='table' then return {OK=false,Message='Invalid settings'} end
   local copy={Settings=payload}; s.Data.Settings=normalize(copy).Settings
   local persisted=save(p,false)
   return {OK=true,Profile=public(p),Message=persisted and 'Settings saved to your account.' or 'Settings applied for this session. Cloud saving is unavailable.'}
  elseif kind=='Loadout' then
   if not weapons[payload] then return {OK=false,Message='Unknown weapon'} end
   if not s.Data.Owned[payload] then return {OK=false,Message='Unlock this weapon first.'} end
   if s.Data.Equipped==payload then return {OK=true,Profile=public(p),Message='Weapon already equipped.'} end
   if os.clock()-(s.LastEquip or 0)<2 then return {OK=false,Message='Wait before switching weapons.'} end
   s.LastEquip=os.clock(); s.Data.Equipped=payload; if s.Mode~='Lobby' then equip(p) end
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
 local now=os.clock(); c.Tokens=math.min(60,c.Tokens+(now-c.TokenTime)*40); c.TokenTime=now
 if c.Tokens<1 then return end; c.Tokens=c.Tokens-1
 local w=weapons[c.Weapon]
 if kind=='Aim' then c.Aim=direction==true; return end
 if kind=='Crouch' then
  c.Crouch=direction==true; c.Sprint=false; c.BaseHip=hum:GetAttribute('BlocklineBaseHip') or hum.HipHeight
  hum.HipHeight=c.BaseHip-(c.Crouch and .8 or 0); hum.WalkSpeed=c.Crouch and 9 or 16; hum.JumpPower=c.Crouch and 0 or 50; return
 end
 if kind=='Sprint' then local sprint=direction==true and not c.Crouch; if c.Sprint~=sprint then c.Sprint=sprint; hum.WalkSpeed=c.Crouch and 9 or (sprint and 23 or 16) end; return end
 if kind=='Reload' then
  if c.Reloading or c.Ammo==w.Magazine then return end
  c.Reloading=true; sendAmmo(p)
  task.delay(w.Reload,function() if combat[p]==c then c.Ammo=w.Magazine; c.Reloading=false; sendAmmo(p) end end); return
 end
 if kind~='Shoot' then return end
 if c.Reloading or c.Sprint or c.Ammo<=0 or now-c.Last<w.Interval then metrics.RejectedShots=metrics.RejectedShots+1; return end
 if competitive(s) and not modes[s.Mode].Active then return end
 if typeof(direction)~='Vector3' then return end
 local magnitude=direction.Magnitude; if magnitude~=magnitude or magnitude<.9 or magnitude>1.1 then return end
 local head=char:FindFirstChild('Head'); if not head then return end
 local pos=head.Position
 if not finite(pos.X) or not finite(pos.Y) or not finite(pos.Z) then return end
 if competitive(s) then
  local localPos=pos-modes[s.Mode].Offset
  if math.abs(localPos.X)>160 or math.abs(localPos.Z)>100 or pos.Y < -5 or pos.Y>42 then return end
 elseif (pos-s.PracticePosition).Magnitude>115 then return end
 c.Last=now; c.Ammo=c.Ammo-1; sendAmmo(p)
 metrics.AcceptedShots=metrics.AcceptedShots+1; metrics.Raycasts=metrics.Raycasts+1
 local spread=math.rad(c.Aim and w.AimSpread or w.Spread)
 local shot=(direction.Unit+Vector3.new((math.random()-.5)*spread,(math.random()-.5)*spread,(math.random()-.5)*spread)).Unit
 local origin=head.Position; local hit=workspace:Raycast(origin,shot*w.Range,c.Params); local finish=hit and hit.Position or origin+shot*w.Range
 -- Cosmetic shot endpoints only; clients reuse a bounded tracer pool.
 for viewer,vs in pairs(profiles) do
  if viewer==p or (competitive(s) and vs.Mode==s.Mode) then
   event:FireClient(viewer,'Shot',{Origin=origin,Finish=finish})
  end
 end
 if s.Mode=='Practice' then s.Training.Shots=s.Training.Shots+1; event:FireClient(p,'Training',trainingPacket(s)) end
 if not hit then return end
 local model=hit.Instance.Parent; local target
 while model and model~=workspace do
  if model:IsA('Model') then target=model:FindFirstChildOfClass('Humanoid'); if target then break end end
  model=model.Parent
 end
 if not target or target.Health<=0 or model:FindFirstChildOfClass('ForceField') then return end
 local victim=Players:GetPlayerFromCharacter(model)
 if s.Mode=='Practice' then if victim or model:GetAttribute('Owner')~=p.UserId or model:GetAttribute('Active')==false then return end
 else
  if not victim or not profiles[victim] or profiles[victim].Mode~=s.Mode or (s.Mode=='Arena' and victim.Team==p.Team) then return end
 end
 metrics.Hits=metrics.Hits+1
 if victim then event:FireClient(victim,'Damage',{From=origin,Amount=hit.Instance.Name=='Head' and w.Head or w.Damage}) end
 target:TakeDamage(hit.Instance.Name=='Head' and w.Head or w.Damage); event:FireClient(p,'Hit',target.Health<=0)
 if s.Mode=='Practice' then
  local t=s.Training; t.Hits=t.Hits+1
  if target.Health<=0 then
   t.Eliminations=t.Eliminations+1
   if t.Drill=='Reaction' then t.Reactions=t.Reactions+1; t.ReactionTotal=t.ReactionTotal+math.max(0,now-(model:GetAttribute('ReadyAt') or now)) end
   if t.Drill=='Flick' then
    local x=s.PracticePosition.X+math.random(-24,24); local z=s.PracticePosition.Z-math.random(35,75)
    model:SetAttribute('BaseX',x); model:SetAttribute('BaseZ',z)
    for _,part in ipairs(model:GetChildren()) do if part:IsA('BasePart') then part.Position=Vector3.new(x,part.Name=='Head' and 6 or 3,z) end end
   end
  end
  event:FireClient(p,'Training',trainingPacket(s))
 end
 if target.Health<=0 and victim then
  s.Data.Kills=s.Data.Kills+1; s.Data.XP=s.Data.XP+50; s.Data.Coins=s.Data.Coins+20
  s.RoundKills=(s.RoundKills or 0)+1
  if s.Mode=='Arena' then local scores=modes.Arena.Scores; scores[p.Team.Name]=scores[p.Team.Name]+1 end
  for viewer,vs in pairs(profiles) do if vs.Mode==s.Mode then event:FireClient(viewer,'KillFeed',{Killer=p.Name,Victim=victim.Name,Weapon=w.Name}) end end
  event:FireClient(p,'Elimination',victim.Name); push(p)
 end
end)
task.spawn(function()
 while true do
  task.wait(5)
  for name,value in pairs(metrics) do folder:SetAttribute(name,value) end
 end
end)
task.spawn(function()
 while true do task.wait(35); for p,s in pairs(profiles) do task.wait(.15); if not locks[p] then task.spawn(function() if profiles[p] and not locks[p] then locks[p]=true; save(p,false); locks[p]=nil end end) end end end
end)
game:BindToClose(function()
 local pending=0
 for p in pairs(profiles) do pending=pending+1; task.spawn(function() local s=profiles[p]; while s and s.Saving do task.wait(.1) end; save(p,true); pending=pending-1 end) end
 local start=os.clock(); while pending>0 and os.clock()-start<25 do task.wait(.1) end
end)
-- 20 Hz anchored target motion; no per-target physics bodies or frame connections.
local motionElapsed=0
Run.Heartbeat:Connect(function(dt)
 motionElapsed=motionElapsed+dt; if motionElapsed<.05 then return end; motionElapsed=0
 local now=os.clock()
 for p,range in pairs(ranges) do
  local s=profiles[p]; local t=s and s.Training
  if t and (t.Drill=='Moving' or t.Drill=='Tracking') then
   for _,model in ipairs(range:GetChildren()) do
    if model:IsA('Model') and model:GetAttribute('Owner')==p.UserId then
     local index=model:GetAttribute('TargetIndex') or 1
     local x=model:GetAttribute('BaseX')+math.sin(now*(t.Drill=='Tracking' and 1.1 or 2.2)+index)*8
     local z=model:GetAttribute('BaseZ')
     for _,part in ipairs(model:GetChildren()) do if part:IsA('BasePart') then part.Position=Vector3.new(x,part.Name=='Head' and 6 or 3,z) end end
    end
   end
  end
 end
end)
local function broadcastMatch(id)
 local cfg=modes[id]; local board={}
 for _,q in ipairs(participants(id)) do local s=profiles[q]; table.insert(board,{Name=q.Name,Kills=s.RoundKills or 0,Deaths=s.RoundDeaths or 0}) end
 table.sort(board,function(a,b)if a.Kills==b.Kills then return a.Name<b.Name end; return a.Kills>b.Kills end)
 for _,q in ipairs(participants(id)) do event:FireClient(q,'Match',{Mode=id,Text=cfg.Text,Remaining=cfg.Remaining or 0,Scores=cfg.Scores,Board=board}) end
end
local function finishRound(id,abandoned)
 local cfg=modes[id]; cfg.Active=false
 local list=participants(id); local best=-1; local winners={}
 if id=='Arena' then
  if cfg.Scores.Azure~=cfg.Scores.Ember then winners[cfg.Scores.Azure>cfg.Scores.Ember and 'Azure' or 'Ember']=true end
 else
  local tied=false; local winner
  for _,q in ipairs(list) do local k=profiles[q].RoundKills or 0; if k>best then best=k; winner=q.UserId; tied=false elseif k==best then tied=true end end
  if winner and not tied then winners[winner]=true end
 end
 for _,q in ipairs(list) do
  local s=profiles[q]; local won=winners[id=='Arena' and q.Team and q.Team.Name or q.UserId]==true
  if not abandoned and s.RoundParticipant then
   s.Data.Matches=s.Data.Matches+1; s.Data.XP=s.Data.XP+100; s.Data.Coins=s.Data.Coins+25
   if won then s.Data.Wins=s.Data.Wins+1; s.Data.Coins=s.Data.Coins+50 end
  end
  event:FireClient(q,'Results',{Mode=cfg.Name,Outcome=abandoned and 'MATCH CANCELLED' or (next(winners)==nil and 'DRAW' or (won and 'VICTORY' or 'DEFEAT')),Kills=s.RoundKills or 0,Deaths=s.RoundDeaths or 0,Reward=(abandoned or not s.RoundParticipant) and 0 or (won and 75 or 25)})
  push(q)
 end
 cfg.Text=abandoned and 'MATCH CANCELLED / Need 2 players' or 'ROUND COMPLETE'; broadcastMatch(id)
end
local function runMode(id)
 local cfg=modes[id]
 while true do
  cfg.Active=false
  if #participants(id)<2 then cfg.Text=cfg.Name..' / Waiting for 2 players'; broadcastMatch(id); task.wait(1)
  else
   local ready=true
   for n=8,1,-1 do if #participants(id)<2 then ready=false; break end; cfg.Text=cfg.Name..' / STARTING '..n; cfg.Remaining=n; broadcastMatch(id); task.wait(1) end
   if ready and #participants(id)>=2 then
    cfg.Round=cfg.Round+1; cfg.Scores={Azure=0,Ember=0}; cfg.Active=true
    -- Rebalance TDM on each new round, including players who joined an uneven lobby.
    for i,q in ipairs(participants(id)) do
     local s=profiles[q]; s.RoundKills=0; s.RoundDeaths=0; s.RoundParticipant=true
     if id=='Arena' then q.Team=Teams:FindFirstChild(i%2==1 and 'Azure' or 'Ember'); q.Neutral=false end
     q:LoadCharacter()
    end
    roster(); local abandoned=false
    for left=cfg.Duration,0,-1 do
     if #participants(id)<2 then abandoned=true; break end
     cfg.Remaining=left; cfg.Text=string.format('%s / %d:%02d',cfg.Name,math.floor(left/60),left%60); broadcastMatch(id)
     local top=0
     if id=='Arena' then top=math.max(cfg.Scores.Azure,cfg.Scores.Ember) else for _,q in ipairs(participants(id)) do top=math.max(top,profiles[q].RoundKills or 0) end end
     if left==0 or top>=cfg.Limit then break end
     task.wait(1)
    end
    finishRound(id,abandoned); task.wait(8)
   end
  end
 end
end
for id in pairs(modes) do task.spawn(runMode,id) end
while true do
 roundActive=false
 task.wait(60)
end
