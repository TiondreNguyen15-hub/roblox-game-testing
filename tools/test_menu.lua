dofile('tools/roblox_mock.lua')
local source=assert(io.open('Server.luau')):read('*a')
local cut=assert(source:find('\nwhile true do\n roundActive=false',1,true))
source=source:sub(1,cut-1)..'\nreturn {load=loadPlayer,profiles=profiles,request=request,enter=enter,save=save,normalize=normalize,public=public,weapons=weapons,combat=action,states=combat}'
local api=assert(load(source,'MenuServer'))()
local a=newPlayer(1,'Alpha');api.load(a)
local function request(p,kind,payload)advance(1);return api.request.OnServerInvoke(p,kind,payload)end
local r=request(a,'Fetch');assert(r.OK and r.Profile.Persistent and r.Profile.Level==1)
assert(r.Profile.Data.Owned.Rifle and not r.Profile.Data.Owned.SMG)
r=request(a,'Settings',{FOV=999,Sensitivity=-2,ReducedMotion=true,Crosshair=false});assert(r.OK)
assert(r.Profile.Data.Settings.FOV==110 and r.Profile.Data.Settings.Sensitivity==.2)
assert(FakeStore.Data.u1.Data.Settings.ReducedMotion==true)
r=request(a,'Daily');assert(r.OK and r.Profile.Data.Coins==100)
r=request(a,'Daily');assert(not r.OK and api.profiles[a].Data.Coins==100)
r=request(a,'Purchase','SMG');assert(not r.OK and api.profiles[a].Data.Coins==100)
api.profiles[a].Data.Coins=350
r=request(a,'Purchase','SMG');assert(r.OK and r.Profile.Data.Coins==50 and r.Profile.Data.Owned.SMG)
r=request(a,'Loadout','SMG');assert(r.OK and r.Profile.Data.Equipped=='SMG')
r=request(a,'Loadout','Marksman');assert(not r.OK)
r=request(a,'Enter','BadMode');assert(not r.OK)
r=request(a,'Enter','Practice');assert(r.OK and r.Profile.Mode=='Practice')
assert(a.Character:FindFirstChildOfClass('Tool').Name=='VX-9')
assert(api.states[a].Ammo==32 and a.Character:FindFirstChild('HumanoidRootPart').Anchored==false)
local range=workspace:FindFirstChild('Alpha practice')
local target=range:FindFirstChild('Practice target');workspace.NextHit={Instance=target:FindFirstChild('Torso'),Position=target:FindFirstChild('Torso').Position}
local ammoBefore=api.states[a].Ammo
api.combat.OnServerEvent:Fire(a,'Shoot',Vector3.new(0,0,-1));assert(api.states[a].Ammo==ammoBefore-1)
assert(target:FindFirstChildOfClass('Humanoid').Health==82)
api.combat.OnServerEvent:Fire(a,'Shoot',Vector3.new(0,0,-1));assert(api.states[a].Ammo==ammoBefore-1)
advance(1);api.combat.OnServerEvent:Fire(a,'Sprint',true);api.combat.OnServerEvent:Fire(a,'Shoot',Vector3.new(0,0,-1));assert(api.states[a].Ammo==ammoBefore-1)
api.combat.OnServerEvent:Fire(a,'Sprint',false)
api.combat.OnServerEvent:Fire(a,'Reload');assert(api.states[a].Reloading)
advance(1);api.combat.OnServerEvent:Fire(a,'Shoot',Vector3.new(0,0,-1));assert(api.states[a].Ammo==ammoBefore-1)
print('PASS: practice shooting applies damage and consumes ammo; server rejects rate violations, sprint fire and firing during reload.')
local b=newPlayer(2,'Bravo');api.load(b)
r=request(b,'Enter','Arena');assert(r.OK and b.Team.Name=='Azure')
r=request(a,'Enter','Arena');assert(r.OK and a.Team.Name=='Ember')
r=request(a,'Enter','Lobby');assert(r.OK and not a.Character:FindFirstChildOfClass('Tool'))
assert(a.Character:FindFirstChild('HumanoidRootPart').Anchored==true)
-- Re-entering modes must not accumulate death handlers.
assert(#a.Character:FindFirstChildOfClass('Humanoid').Died.Callbacks==1)
FakeStore.Fail=true
r=request(a,'Settings',{FOV=80,Sensitivity=1,ReducedMotion=false,Crosshair=true});assert(r.OK and not r.Profile.Persistent)
assert(r.Message:find('session'))
r=request(a,'Daily');assert(not r.OK)
local c=newPlayer(3,'Offline');api.load(c);assert(not api.profiles[c].Persistent)
FakeStore.Fail=false
local d=newPlayer(4,'Locked');FakeStore.Data.u4={Data={},Session='another-server',Expires=os.time()+100};api.load(d);assert(d.Kicked and not api.profiles[d])
print('PASS: profile load, defaults, settings persistence/clamps, daily cooldown, unlock prices, owned equip, real modes, balanced teams, cleanup, death hooks, cloud failures and session locks.')
-- Build the actual ScreenGui in a headless hierarchy and exercise native callbacks.
game:GetService('Players').LocalPlayer=a
local rs=game:GetService('ReplicatedStorage');local network=rs:WaitForChild('Blockline')
local event=network:WaitForChild('Event');local rf=network:WaitForChild('Request');function rf:InvokeServer(kind,payload) return request(a,kind,payload)end
function network:WaitForChild(n)return self:FindFirstChild(n)end
local combat=network:WaitForChild('Combat');function combat:FireServer(kind,payload)self.OnServerEvent:Fire(a,kind,payload)end
local first=game:GetService('ReplicatedFirst');local module=Instance.new('ModuleScript',first);module.Name='BlocklineLoading'
local loader=assert(loadfile('LoadingScreen.luau'))()
local nativeRequire=require;require=function(m)if m==module then return loader end;return nativeRequire(m)end
local clientSource=assert(io.open('Client.luau')):read('*a')
local client=assert(load(clientSource..'\nreturn {page=setPage,menu=showMenu,ready=function()introDone=true end}','MenuClient'))()
local gui=a:WaitForChild('PlayerGui'):WaitForChild('BlocklineInterface')
local nav=gui:WaitForChild('Menu'):WaitForChild('Navigation')
assert(#nav:GetChildren()==7)
local count=0
for _,o in ipairs(nav:GetChildren())do if o.ClassName=='TextButton'then assert(#o.Activated.Callbacks==1);count=count+1 end end
assert(count==6)
assert(a:WaitForChild('PlayerGui'):FindFirstChild('BlocklineLoading') and gui:FindFirstChild('GameplayHUD'))
print('PASS: ScreenGui construction, six navigation actions, intro, viewport hierarchy, gameplay HUD and controller bindings load under test doubles.')

event.OnClientEvent:Fire('Profile',api.public(a))
client.ready()
for _,name in ipairs({'PLAY','PRACTICE','LOADOUT','SETTINGS','STATISTICS','TEAM'})do
 client.page(name)
 local page=gui:WaitForChild('Menu'):WaitForChild('Content'):WaitForChild('Page')
 assert(#page:GetChildren()>2,'Empty page '..name)
 for _,o in ipairs(page:GetDescendants())do if o.ClassName=='TextButton'then assert(#o.Activated.Callbacks==1,'Unwired button '..o.Text)end end
end
client.menu(false);assert(game:GetService('ContextActionService').Actions.BLFire)
client.menu(true);assert(not game:GetService('ContextActionService').Actions.BLFire)
print('PASS: every page renders with wired buttons; menu transitions bind/unbind combat controls.')
