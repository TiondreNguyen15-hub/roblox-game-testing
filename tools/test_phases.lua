dofile('tools/roblox_mock.lua')
local source=assert(io.open('Server.luau')):read('*a')
local cut=assert(source:find('\nwhile true do\n roundActive=false',1,true))
local api=assert(load(source:sub(1,cut-1)..'\n'..[[
return {load=loadPlayer,enter=enter,profiles=profiles,modes=modes,states=combat,action=action,request=request,finish=finishRound,training=trainingPacket,broadcast=broadcastMatch,metrics=metrics,run=runMode}
]]))()
local a=newPlayer(31,'Alpha');local b=newPlayer(32,'Bravo');local c=newPlayer(33,'Charlie')
for _,p in ipairs({a,b,c})do api.load(p)end
local function clearProtection(p)local f=p.Character:FindFirstChildOfClass('ForceField');if f then f:Destroy()end end
local function shoot(p,target)
 advance(.5);workspace.NextHit=target and {Instance=target,Position=target.Position} or nil
 api.action.OnServerEvent:Fire(p,'Shoot',Vector3.new(0,0,-1))
end
-- Duel capacity, offsets, neutral player assignment and spawn protection.
assert(api.enter(a,'Duel'));assert(api.enter(b,'Duel'));assert(not api.enter(c,'Duel'))
assert(a.Character:FindFirstChildOfClass('ForceField'))
assert(a.Character:FindFirstChild('HumanoidRootPart').Position.Z==1600)
assert(a.Neutral and b.Neutral and not a.Team and not b.Team)
api.modes.Duel.Active=true;clearProtection(a);clearProtection(b)
local hum=b.Character:FindFirstChildOfClass('Humanoid');local body=Instance.new('Part',b.Character);body.Name='Torso';body.Position=b.Character:FindFirstChild('HumanoidRootPart').Position
shoot(a,body);assert(hum.Health==76)
for i=1,4 do shoot(a,body)end
assert(api.profiles[a].RoundKills==1 and api.profiles[b].RoundDeaths==1)
api.profiles[a].RoundParticipant=true;api.profiles[b].RoundParticipant=true
local coins=api.profiles[a].Data.Coins;api.finish('Duel',false)
assert(api.profiles[a].Data.Wins==1 and api.profiles[b].Data.Wins==0)
assert(api.profiles[a].Data.Coins==coins+75)
assert(not api.modes.Duel.Active)
-- FFA allows damage between neutral players, but no damage across modes.
hum.Health=100;api.enter(a,'FFA');api.enter(b,'FFA');clearProtection(a);clearProtection(b);api.modes.FFA.Active=true
assert(a.Character:FindFirstChild('HumanoidRootPart').Position.Z>=746)
shoot(a,body);assert(hum.Health==76,'Neutral FFA players incorrectly treated as teammates')
api.enter(b,'Arena');clearProtection(b);hum.Health=100;shoot(a,body);assert(hum.Health==100,'Cross-mode damage')
-- Crouch/sprint/jump are server controlled, with stance restored on mode changes.
local ah=a.Character:FindFirstChildOfClass('Humanoid')
api.action.OnServerEvent:Fire(a,'Crouch',true);assert(ah.WalkSpeed==9 and ah.JumpPower==0 and ah.HipHeight==1.2)
api.action.OnServerEvent:Fire(a,'Sprint',true);assert(ah.WalkSpeed==9)
api.action.OnServerEvent:Fire(a,'Crouch',false);assert(ah.WalkSpeed==16 and ah.HipHeight==2 and ah.JumpPower==50)
api.action.OnServerEvent:Fire(a,'Sprint',true);assert(ah.WalkSpeed==23)
api.enter(a,'Practice');assert(ah.WalkSpeed==16 and ah.HipHeight==2)
local career=api.profiles[a].Data.Kills
local range=workspace:FindFirstChild('Alpha practice');local target=range:FindFirstChild('Practice target');local torso=target:FindFirstChild('Torso')
shoot(a,nil);shoot(a,torso)
local t=api.training(api.profiles[a]);assert(t.Shots==2 and t.Hits==1 and t.Accuracy==50)
assert(api.profiles[a].Data.Kills==career,'Practice changed career stats')
-- Foreign owner and hidden reaction target reject hits while still counting the shot.
target:SetAttribute('Owner',999);local health=target:FindFirstChildOfClass('Humanoid').Health;shoot(a,torso);assert(target:FindFirstChildOfClass('Humanoid').Health==health)
target:SetAttribute('Owner',a.UserId);target:SetAttribute('Active',false);shoot(a,torso);assert(target:FindFirstChildOfClass('Humanoid').Health==health)
for _,drill in ipairs({'Stationary','Moving','Tracking','Flick','Reaction'})do
 advance(1);local r=api.request.OnServerInvoke(a,'Drill',drill);assert(r.OK and r.Profile.Training.Drill==drill)
 assert(r.Profile.Training.Shots==0)
end
-- A reaction hit samples the server-side target appearance time.
range=workspace:FindFirstChild('Alpha practice');target=range:FindFirstChild('Practice target');target:SetAttribute('Active',true);target:SetAttribute('ReadyAt',os.clock()-.25)
local th=target:FindFirstChildOfClass('Humanoid');th.Health=1;shoot(a,target:FindFirstChild('Torso'))
t=api.training(api.profiles[a]);assert(t.ReactionMs>=250 and t.Reactions==nil and t.Eliminations==1)
-- Cancelled rounds do not grant completion rewards.
api.enter(a,'FFA');api.profiles[a].RoundParticipant=true
coins=api.profiles[a].Data.Coins;api.finish('FFA',true);assert(api.profiles[a].Data.Coins==coins)
print('PASS: Duel capacity/round rewards, FFA neutral combat, mode isolation, server crouch/sprint/jump restoration, spawn protection, independent training stats, all five drill entries, reaction timing and cancellation rewards.')

-- Execute the real round controller under a cooperative clock.
assert(api.enter(a,'Duel'));assert(api.enter(b,'Duel'))
local originalWait=task.wait
task.wait=function(n)advance(n or .1);coroutine.yield(n)end
local cycle=coroutine.create(function()api.run('Duel')end)
for i=1,9 do local ok,message=coroutine.resume(cycle);assert(ok,message)end
assert(api.modes.Duel.Active and api.modes.Duel.Remaining==180,'Countdown did not start round')
api.profiles[a].RoundKills=5
local ok,message=coroutine.resume(cycle);assert(ok,message)
assert(not api.modes.Duel.Active,'Score limit did not finish round')
local result
for _,m in ipairs(a._messages)do if m[1]=='Results'then result=m[2]end end
assert(result and result.Outcome=='VICTORY' and result.Kills==5 and result.Reward==75)
task.wait=originalWait
print('PASS: actual duel countdown, automatic score-limit ending and server-generated victory results under a cooperative test clock.')
