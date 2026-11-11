dofile('tools/roblox_mock.lua')
local source=assert(io.open('Server.luau')):read('*a')
local cut=assert(source:find('\nwhile true do\n roundActive=false',1,true))
local api=assert(load(source:sub(1,cut-1)..'\nreturn {load=loadPlayer,enter=enter,profiles=profiles,combat=action,states=combat,metrics=metrics,normalize=normalize,save=save,request=request,round=function(v)modes.Arena.Active=v end}'))()
local a=newPlayer(21,'Shooter');local b=newPlayer(22,'Target');api.load(a);api.load(b);api.enter(a,'Arena');api.enter(b,'Arena');api.round(true)
local function shoot(part,dir)
 advance(.2);workspace.NextHit=part and {Instance=part,Position=part.Position} or nil
 api.combat.OnServerEvent:Fire(a,'Shoot',dir or Vector3.new(0,0,-1))
end
for _,q in ipairs({a,b}) do local f=q.Character:FindFirstChildOfClass('ForceField'); if f then f:Destroy() end end
local hum=b.Character:FindFirstChildOfClass('Humanoid');local body=Instance.new('Part',b.Character);body.Name='Torso';body.Position=Vector3.new()
local params=api.states[a].Params
shoot(body);assert(hum.Health==76)
assert(api.states[a].Params==params)
local nested=Instance.new('Model',b.Character);nested.Name='Nested head rig';local head=b.Character:FindFirstChild('Head');head.Parent=nested
shoot(head);assert(hum.Health==28,'Nested headshot missed')
shoot(body);shoot(body);assert(hum.Health==0 and api.profiles[a].Data.Kills==1)
shoot(body);assert(api.profiles[a].Data.Kills==1,'Double reward')
hum.Health=100;b.Team=a.Team;shoot(body);assert(hum.Health==100,'Friendly fire');b.Team=game:GetService('Teams'):FindFirstChild('Ember')
local ff=Instance.new('ForceField',b.Character);shoot(body);assert(hum.Health==100);ff:Destroy()
local wall=Instance.new('Part',workspace);wall.Position=Vector3.new();shoot(wall);assert(hum.Health==100)
local ammo=api.states[a].Ammo;shoot(body,Vector3.new(0/0,0,0));shoot(body,Vector3.new(math.huge,0,0));shoot(body,Vector3.new(0,0,2));assert(api.states[a].Ammo==ammo)
a.Character:FindFirstChild('Head').Position=Vector3.new(999,4,999);shoot(body);assert(api.states[a].Ammo==ammo,'Outside arena fired')
a.Character:FindFirstChild('Head').Position=a.Character:FindFirstChild('HumanoidRootPart').Position
api.states[a].Ammo=0;shoot(body);assert(api.states[a].Ammo==0)
-- Equip changes preserve magazine state; repeated same-weapon equip never refills.
api.states[a].Ammo=7;advance(1);local r=api.request.OnServerInvoke(a,'Loadout','Rifle');assert(r.OK and api.states[a].Ammo==7)
api.profiles[a].Data.Owned.SMG=true;advance(3);r=api.request.OnServerInvoke(a,'Loadout','SMG');assert(r.OK)
advance(3);r=api.request.OnServerInvoke(a,'Loadout','Rifle');assert(r.OK and api.states[a].Ammo==7)
-- A stale session token cannot replace the current owner's data.
FakeStore.Data.u21.Session='new-owner';assert(not api.save(a,false));assert(FakeStore.Data.u21.Session=='new-owner')
local normalized=api.normalize({Coins=math.huge,Settings={FOV=math.huge,Sensitivity=0/0}});assert(normalized.Coins==0 and normalized.Settings.FOV==80)
-- Stress rate validation: thousands of same-timestamp requests produce one raycast at most.
api.enter(a,'Practice');local before=api.metrics.Raycasts
for i=1,10000 do api.combat.OnServerEvent:Fire(a,'Shoot',Vector3.new(0,0,-1))end
assert(api.metrics.Raycasts-before<=1)
assert(not workspace:FindFirstChild('Tracer'),'Server tracer allocation returned')
print('PASS: two-player body/head hits, nested rigs, kill deduplication, friendly fire, forcefields, walls, invalid vectors, map bounds, empty ammo, cached raycast params, magazine exploit and stale profile ownership.')
print('PASS: 10,000 same-timestamp fire requests yield at most one raycast; zero server tracer Parts.')
-- Use the existing UI harness, adding layout and pool visibility tests.
local source=assert(io.open('tools/test_menu.lua')):read('*a')
source=source:gsub('page=setPage,menu=showMenu,ready=', 'page=setPage,menu=showMenu,layout=layout,ready=')
source=source..[[
local function pixels(u,axis,parent)
 local k=axis=='X' and 1 or 3;return (u[k] or 0)*parent+(u[k+1] or 0)
end
local camera=workspace.CurrentCamera
for _,size in ipairs({{320,568},{390,844},{844,390},{768,1024},{960,600},{1920,1080},{2560,1440}})do
 camera.ViewportSize=Vector2.new(size[1],size[2]);client.layout()
 local panel=gui:WaitForChild('Menu'):WaitForChild('Content')
 local x=pixels(panel.Position,'X',size[1]);local y=pixels(panel.Position,'Y',size[2])
 local w=pixels(panel.Size,'X',size[1]);local h=pixels(panel.Size,'Y',size[2])
 assert(w>=270 and h>=220 and x>=0 and y>=0 and x+w<=size[1] and y+h<=size[2],'Clipped panel')
end
client.page('SETTINGS')
local page=gui:WaitForChild('Menu'):WaitForChild('Content'):WaitForChild('Page')
local plus
for _,o in ipairs(page:GetDescendants())do if o.ClassName=='TextButton' and o.Text=='+'then plus=o;break end end
assert(plus);plus.Activated:Fire()
local settingsRow=plus.Parent
local draftLabel;for _,o in ipairs(settingsRow:GetChildren())do if o.ClassName=='TextLabel'then draftLabel=o end end
local draftText=draftLabel.Text
event.OnClientEvent:Fire('Profile',api.public(a));assert(draftLabel.Parent and draftLabel.Text==draftText,'Settings draft replaced')
game:GetService('UserInputService').GamepadEnabled=true
client.page('LOADOUT');assert(game:GetService('GuiService').SelectedObject==nav:FindFirstChild('TextButton') or game:GetService('GuiService').SelectedObject.Parent==nav)
client.menu(false)
for i=1,100 do event.OnClientEvent:Fire('Shot',{Origin=Vector3.new(),Finish=Vector3.new(0,0,-100)})end
local tracers=0;for _,o in ipairs(workspace:GetChildren())do if o.Name=='LocalTracer'then tracers=tracers+1;assert(not o.CanCollide and not o.CanQuery and not o.CanTouch and not o.CastShadow)end end
assert(tracers==32)
client.menu(true)
print('PASS: menu bounds at seven phone/tablet/desktop resolutions; 100 tracer events reuse 32 non-physical client Parts.')
]]
assert(load(source,'UI regressions'))()
