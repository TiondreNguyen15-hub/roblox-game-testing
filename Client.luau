-- Native ScreenGui. Mouse/touch Activated events and controller navigation.
local Players=game:GetService('Players')
local RS=game:GetService('ReplicatedStorage')
local Tween=game:GetService('TweenService')
local UIS=game:GetService('UserInputService')
local CAS=game:GetService('ContextActionService')
local GuiService=game:GetService('GuiService')
local Run=game:GetService('RunService')
local Content=game:GetService('ContentProvider')
local p=Players.LocalPlayer
local network=RS:WaitForChild('Blockline')
local request=network:WaitForChild('Request'); local event=network:WaitForChild('Event')
local combat=network:WaitForChild('Combat'); local matchStatus=network:WaitForChild('MatchStatus')
local C={BG=Color3.fromRGB(12,13,16),Panel=Color3.fromRGB(22,24,29),Hover=Color3.fromRGB(39,42,49),Text=Color3.fromRGB(242,243,246),Muted=Color3.fromRGB(155,161,174),Red=Color3.fromRGB(240,32,54)}
local profile; local current='PLAY'; local menuOpen=true; local introDone=false; local busy=false; local mobile=false
local settings={FOV=80,Sensitivity=1,ReducedMotion=false,Crosshair=true,LowGraphics=false}
local modeNames={Arena='TEAM DEATHMATCH',FFA='FREE FOR ALL',Duel='1V1 DUEL',Practice='AIM TRAINING'}
local selectedDrill='Stationary'
local selectedMode='Arena'; local firing=false; local aiming=false; local ammo=0; local reloading=false; local weaponId='Rifle'; local lastShot=0
p:SetAttribute('BLMenuOpen',true)
local aimParams=RaycastParams.new(); aimParams.FilterType=Enum.RaycastFilterType.Exclude
p.CharacterAdded:Connect(function(char) aimParams.FilterDescendantsInstances={char} end)
if p.Character then aimParams.FilterDescendantsInstances={p.Character} end
local gui=Instance.new('ScreenGui'); gui.Name='BlocklineInterface'; gui.ResetOnSpawn=false; gui.ZIndexBehavior=Enum.ZIndexBehavior.Sibling; gui.DisplayOrder=5; gui.Parent=p:WaitForChild('PlayerGui')
local function create(cls,parent,props)
 local o=Instance.new(cls); for k,v in pairs(props or {}) do o[k]=v end; o.Parent=parent; return o
end
local function round(o,r) create('UICorner',o,{CornerRadius=UDim.new(0,r or 8)}) end
local function text(parent,value,size,color)
 return create('TextLabel',parent,{Text=value,Font=Enum.Font.GothamMedium,TextSize=size or 16,TextColor3=color or C.Text,BackgroundTransparency=1,TextXAlignment=Enum.TextXAlignment.Left,TextWrapped=true,Size=UDim2.new(1,0,0,(size or 16)+12)})
end
local function tween(o,props,time)
 Tween:Create(o,TweenInfo.new(settings.ReducedMotion and 0 or time or .18,Enum.EasingStyle.Quad,Enum.EasingDirection.Out),props):Play()
end
local function button(parent,label,callback,accent)
 local b=create('TextButton',parent,{Text=label,Font=Enum.Font.GothamBold,TextSize=15,TextColor3=C.Text,BackgroundColor3=accent and C.Red or C.Panel,BorderSizePixel=0,AutoButtonColor=false,Size=UDim2.new(1,0,0,46),Selectable=true}); round(b)
 local normal=accent and C.Red or C.Panel
 local function enter() tween(b,{BackgroundColor3=accent and Color3.fromRGB(255,58,78) or C.Hover}) end
 local function leave() tween(b,{BackgroundColor3=normal}) end
 b.MouseEnter:Connect(enter); b.MouseLeave:Connect(leave); b.SelectionGained:Connect(enter); b.SelectionLost:Connect(leave)
 b.Activated:Connect(function() if callback then callback() end end)
 return b
end
local menu=create('Frame',gui,{Name='Menu',Size=UDim2.fromScale(1,1),BackgroundColor3=C.BG,BackgroundTransparency=.23,BorderSizePixel=0})
local shade=create('Frame',menu,{Size=UDim2.fromScale(1,1),BackgroundColor3=C.BG,BackgroundTransparency=.15,BorderSizePixel=0})
create('UIGradient',shade,{Transparency=NumberSequence.new({NumberSequenceKeypoint.new(0,.02),NumberSequenceKeypoint.new(.55,.45),NumberSequenceKeypoint.new(1,.08)})})
local header=create('Frame',menu,{BackgroundTransparency=1,Position=UDim2.fromOffset(28,14),Size=UDim2.new(1,-56,0,54)})
local logo=text(header,'B /  BLOCKLINE',26); logo.Font=Enum.Font.GothamBlack; logo.Size=UDim2.new(.6,0,0,34)
local tagline=text(header,'COMPETITIVE ARENA',11,C.Red); tagline.Position=UDim2.fromOffset(0,36)
local identity=text(header,p.Name,14,C.Muted); identity.TextXAlignment=Enum.TextXAlignment.Right; identity.Position=UDim2.new(.55,0,0,6); identity.Size=UDim2.new(.45,0,0,40)
local nav=create('ScrollingFrame',menu,{Name='Navigation',BackgroundTransparency=1,BorderSizePixel=0,ScrollBarThickness=0,AutomaticCanvasSize=Enum.AutomaticSize.Y,CanvasSize=UDim2.new()})
local navLayout=create('UIListLayout',nav,{Padding=UDim.new(0,10),SortOrder=Enum.SortOrder.LayoutOrder})
local panel=create('Frame',menu,{Name='Content',BackgroundColor3=C.BG,BackgroundTransparency=.12,BorderSizePixel=0}); round(panel,12)
local pageTitle=text(panel,'PLAY',27); pageTitle.Font=Enum.Font.GothamBlack; pageTitle.Position=UDim2.fromOffset(22,16); pageTitle.Size=UDim2.new(1,-44,0,40)
local content=create('ScrollingFrame',panel,{Name='Page',Position=UDim2.fromOffset(22,66),Size=UDim2.new(1,-44,1,-82),BackgroundTransparency=1,BorderSizePixel=0,ScrollBarThickness=3,ScrollBarImageColor3=C.Red,CanvasSize=UDim2.new(),AutomaticCanvasSize=Enum.AutomaticSize.Y})
create('UIListLayout',content,{Padding=UDim.new(0,12),SortOrder=Enum.SortOrder.LayoutOrder})
create('UIPadding',content,{PaddingBottom=UDim.new(0,10)})
local side=create('Frame',menu,{Name='ProfilePanel',BackgroundColor3=C.BG,BackgroundTransparency=.1,BorderSizePixel=0}); round(side,12)
local sideBody=create('ScrollingFrame',side,{Position=UDim2.fromOffset(18,16),Size=UDim2.new(1,-36,1,-30),BackgroundTransparency=1,BorderSizePixel=0,CanvasSize=UDim2.new(),AutomaticCanvasSize=Enum.AutomaticSize.Y,ScrollBarThickness=2})
create('UIListLayout',sideBody,{Padding=UDim.new(0,10),SortOrder=Enum.SortOrder.LayoutOrder})
local portrait=create('ViewportFrame',sideBody,{Name='Character',Size=UDim2.new(1,0,0,210),BackgroundColor3=C.Panel,BorderSizePixel=0,Ambient=Color3.fromRGB(185,185,195),LightColor=Color3.fromRGB(255,190,190),LightDirection=Vector3.new(-1,-1,-1)}); round(portrait)
local charWorld=create('WorldModel',portrait,{Name='CharacterPreview'}); local charCam=create('Camera',portrait,{}); portrait.CurrentCamera=charCam
local profileText=text(sideBody,'Loading profile…',16); profileText.Size=UDim2.new(1,0,0,90)
local rosterText=text(sideBody,'SERVER ROSTER',13,C.Muted); rosterText.Size=UDim2.new(1,0,0,150)
local footer=text(menu,'CONNECTING TO SERVER',12,C.Muted); footer.TextXAlignment=Enum.TextXAlignment.Center
local toast=create('TextLabel',gui,{Name='Notice',Text='',BackgroundColor3=C.Panel,BackgroundTransparency=.05,TextColor3=C.Text,Font=Enum.Font.GothamMedium,TextSize=14,TextWrapped=true,AnchorPoint=Vector2.new(.5,1),Position=UDim2.new(.5,0,1,-18),Size=UDim2.new(.8,0,0,56),Visible=false,ZIndex=30}); round(toast)
local toastVersion=0
local function notify(message)
 toastVersion=toastVersion+1; local v=toastVersion; toast.Text=message; toast.Visible=true
 task.delay(5,function() if v==toastVersion then toast.Visible=false end end)
end
local currentMatch; local trainingStats
local render; local showMenu; local refreshProfile
local editingSettings=false
local function acceptProfile(data)
 if profile and profile.Mode~=data.Mode then currentMatch=nil; trainingStats=nil end
 profile=data; if not editingSettings then settings=table.clone(data.Data.Settings) end
 weaponId=data.Data.Equipped; refreshProfile()
end
local function call(kind,payload)
 if busy then notify('Please wait for the current request.'); return nil end
 busy=true
 local ok,result=pcall(function() return request:InvokeServer(kind,payload) end)
 busy=false
 if not ok then notify('Server unavailable. Please try again.'); return nil end
 if result.Profile then
  if kind=='Settings' and result.OK then editingSettings=false end
  acceptProfile(result.Profile)
 end
 if result.Message then notify(result.Message) end
 return result
end
local function rowText(value,size,color,height)
 local l=text(content,value,size,color); if height then l.Size=UDim2.new(1,0,0,height) end; return l
end
local function section(value) return rowText(value,12,C.Red) end
local function weaponPreview(id)
 local viewport=create('ViewportFrame',content,{Name='WeaponPreview',Size=UDim2.new(1,0,0,120),BackgroundColor3=C.Panel,BorderSizePixel=0,Ambient=Color3.fromRGB(190,190,200),LightColor=Color3.fromRGB(255,150,155)}); round(viewport)
 local world=create('WorldModel',viewport,{})
 local function block(size,pos,col)
  return create('Part',world,{Size=size,Position=pos,Anchored=true,Color=col,Material=Enum.Material.Metal})
 end
 -- Uses the same dimensions and colors as the equipped server Tool.
 local barrelLength=id=='Marksman' and 2.5 or 1.3
 block(Vector3.new(.4,.7,id=='SMG' and 1.8 or 2.8),Vector3.new(),Color3.fromRGB(20,22,26))
 block(Vector3.new(.18,.18,barrelLength),Vector3.new(0,.1,-1.7),C.Red)
 local camera=create('Camera',viewport,{CFrame=CFrame.lookAt(Vector3.new(5,2,3),Vector3.new(0,0,-.7))}); viewport.CurrentCamera=camera
 return viewport
end
local navButtons={}
local function setPage(name)
 if name~='SETTINGS' then editingSettings=false end
 current=name; render(); if UIS.GamepadEnabled then GuiService.SelectedObject=navButtons[name] end
end
local function enterMode(mode)
 local r=call('Enter',mode)
 if r and r.OK then showMenu(false) end
end
local function profileSummary()
 local d=profile.Data
 rowText(string.format('%s  /  LEVEL %d\n%d CREDITS  •  %d XP',p.Name,profile.Level,d.Coins,d.XP),18,C.Text,68)
end
render=function()
 for _,child in ipairs(content:GetChildren()) do if child:IsA('GuiObject') then child:Destroy() end end
 pageTitle.Text=current
 if not profile then rowText('Loading account data…',16,C.Muted); return end
 local d=profile.Data
 if current=='PLAY' then
  section('DEPLOY / CURRENT SERVER')
  rowText('ENTER THE ARENA',30,C.Text,70)
  rowText('Red edges. Clear sightlines. Team competition on the new minimalist arena.',15,C.Muted,54)
  button(content,'MODE  /  '..modeNames[selectedMode],function() local order={'Arena','FFA','Duel','Practice'}; for i,id in ipairs(order) do if selectedMode==id then selectedMode=order[i%#order+1]; break end end; render() end)
  rowText(selectedMode=='Practice' and 'Private drills • Server-scored accuracy and reaction time\nPractice never changes your career statistics.' or (selectedMode=='Duel' and '1v1 • First to 5 • 3-minute rounds\nTwo slots, separate duel arena.' or (selectedMode=='FFA' and 'Everyone versus everyone • First to 20\n5-minute rounds in a separate arena.' or 'Balanced teams • First to 30 • 5 minutes\nTwo players required to start.')),14,C.Muted,64)
  button(content,'PLAY / '..modeNames[selectedMode],function() enterMode(selectedMode) end,true)
  if profile.Mode~='Lobby' then
   button(content,'RESUME '..string.upper(profile.Mode),function() showMenu(false) end)
   button(content,'LEAVE MODE / RETURN TO LOBBY',function() local r=call('Enter','Lobby'); if r and r.OK then render() end end)
  end
  section('EQUIPPED / '..profile.Weapons[d.Equipped].Name); weaponPreview(d.Equipped)
  section('DAILY REWARD / 100 CREDITS')
  local ready=profile.DailyReady
  rowText(ready and 'Your daily reward is ready.' or string.format('Available in %dh %dm',math.floor(profile.DailyRemaining/3600),math.floor(profile.DailyRemaining%3600/60)),14,C.Muted,36)
  button(content,ready and 'CLAIM DAILY REWARD' or 'CHECK DAILY REWARD',function() local r=call('Daily'); if r and r.Profile then render() end end)
  if mobile then
   local preview=portrait:Clone(); preview.Size=UDim2.new(1,0,0,190); preview.Parent=content; preview.CurrentCamera=preview:FindFirstChildOfClass('Camera')
   profileSummary(); rowText(rosterText.Text,13,C.Muted,150)
  end
 elseif current=='PRACTICE' then
  section('AIM TRAINING / PRIVATE RANGE')
  rowText('BUILD YOUR AIM',28,C.Text,68)
  rowText('Three targets with head and body hit zones. Targets reset after elimination. Your range is isolated from other players.',16,C.Muted,100)
  weaponPreview(d.Equipped)
  button(content,'DRILL / '..selectedDrill:upper(),function() local order={'Stationary','Moving','Tracking','Flick','Reaction'}; for i,id in ipairs(order) do if id==selectedDrill then selectedDrill=order[i%#order+1]; break end end; render() end)
  rowText('Stationary: precision. Moving: lateral targets. Tracking: continuous motion. Flick: random target positions. Reaction: targets appear after a random delay.',14,C.Muted,100)
  button(content,'START '..selectedDrill:upper(),function() local r=call('Drill',selectedDrill); if r and r.OK then showMenu(false) end end,true)
  if profile.Training then local t=profile.Training; rowText(string.format('LAST DRILL / %d hits / %d shots / %.1f%% accuracy / %.0f ms reaction',t.Hits,t.Shots,t.Accuracy,t.ReactionMs),14,C.Muted,70) end
 elseif current=='LOADOUT' then
  section('ARSENAL / EARN CREDITS BY PLAYING')
  for _,id in ipairs({'Rifle','SMG','Marksman'}) do
   local w=profile.Weapons[id]
   rowText(w.Name..(d.Equipped==id and '  /  EQUIPPED' or ''),22)
   weaponPreview(id)
   rowText(string.format('%d body damage  •  %d rounds  •  %.2fs fire interval',w.Damage,w.Magazine,w.Interval),13,C.Muted,40)
   button(content,d.Owned[id] and ('EQUIP '..w.Name) or ('UNLOCK / '..w.Price..' CREDITS'),function()
    local r=call(d.Owned[id] and 'Loadout' or 'Purchase',id); if r and r.Profile then render() end
   end,d.Equipped==id)
  end
 elseif current=='SETTINGS' then
  section('PREFERENCES / ACCOUNT SETTINGS')
  rowText('Changes apply immediately. Press save to store them.',14,C.Muted,42)
  local function adjust(label,key,step,lo,hi)
   local holder=create('Frame',content,{Size=UDim2.new(1,0,0,68),BackgroundColor3=C.Panel,BorderSizePixel=0}); round(holder)
   local l=text(holder,label..': '..string.format(key=='Sensitivity' and '%.1f' or '%d',settings[key]),15); l.Position=UDim2.fromOffset(12,0); l.Size=UDim2.new(1,-24,0,28)
   local function change(sign)
    editingSettings=true; settings[key]=math.clamp(math.floor((settings[key]+step*sign)*10+.5)/10,lo,hi)
    l.Text=label..': '..string.format(key=='Sensitivity' and '%.1f' or '%d',settings[key])
   end
   local minus=button(holder,'−',function() change(-1) end); minus.Size=UDim2.new(.5,-18,0,30); minus.Position=UDim2.fromOffset(12,32)
   local plus=button(holder,'+',function() change(1) end); plus.Size=UDim2.new(.5,-18,0,30); plus.Position=UDim2.new(.5,6,0,32)
  end
  adjust('Field of view','FOV',5,60,110); adjust('Mouse sensitivity','Sensitivity',.1,.2,2)
  for _,setting in ipairs({{'Reduced motion','ReducedMotion'},{'Show crosshair','Crosshair'},{'Low graphics (shadows)','LowGraphics'}}) do
   local b
   b=button(content,setting[1]..' / '..(settings[setting[2]] and 'ON' or 'OFF'),function() editingSettings=true; settings[setting[2]]=not settings[setting[2]]; b.Text=setting[1]..' / '..(settings[setting[2]] and 'ON' or 'OFF') end)
  end
  button(content,'SAVE SETTINGS',function() call('Settings',settings) end,true)
  rowText(profile.Persistent and 'Cloud profile connected.' or profile.Notice,13,C.Muted,80)
 elseif current=='STATISTICS' then
  section('CAREER / SERVER-VERIFIED STATS'); profileSummary()
  rowText(string.format('ELIMINATIONS    %d\nDEATHS    %d\nK/D    %.2f\nMATCHES    %d\nWINS    %d\nWIN RATE    %.0f%%',d.Kills,d.Deaths,d.Kills/math.max(1,d.Deaths),d.Matches,d.Wins,d.Matches>0 and d.Wins/d.Matches*100 or 0),20,C.Text,230)
  rowText('50 XP + 20 credits per elimination.\n100 XP + 25 credits per completed match.\n50 extra credits for a win.\nLevel up every 250 XP.',14,C.Muted,110)
 elseif current=='TEAM' then
  section('TEAM / CURRENT SERVER'); rowText(rosterText.Text,16,C.Text,300)
  rowText('Teams are balanced on arena entry. This roster shows players in this server.',14,C.Muted,70)
  button(content,'REFRESH ROSTER',function() local r=call('Fetch'); if r and r.OK then render() end end)
 end
 content.CanvasPosition=Vector2.new()
 if introDone and not settings.ReducedMotion then
  content.Position=UDim2.fromOffset(30,66)
  tween(content,{Position=UDim2.fromOffset(22,66)},.18)
 end
 for name,b in pairs(navButtons) do b.TextColor3=name==current and C.Red or C.Text end
 if UIS.GamepadEnabled and menuOpen and (not GuiService.SelectedObject or not GuiService.SelectedObject.Parent) then GuiService.SelectedObject=navButtons[current] end
end
for i,name in ipairs({'PLAY','PRACTICE','LOADOUT','SETTINGS','STATISTICS','TEAM'}) do
 local b=button(nav,name,function() setPage(name) end); b.LayoutOrder=i; navButtons[name]=b
end
refreshProfile=function()
 if not profile then return end
 local d=profile.Data; identity.Text=string.format('%s  /  LV %d  /  %d CR',p.Name,profile.Level,d.Coins)
 profileText.Text=string.format('%s\nLEVEL %d   /   %d CREDITS\n%s\nK %d / D %d / W %d',p.Name,profile.Level,d.Coins,profile.Mode:upper(),d.Kills,d.Deaths,d.Wins)
 local lines={'TEAM / SERVER ROSTER'}
 for _,r in ipairs(profile.Roster) do table.insert(lines,r.Username..'  /  '..(r.Mode=='Arena' and r.Team or r.Mode)) end
 rosterText.Text=table.concat(lines,'\n'); rosterText.Size=UDim2.new(1,0,0,math.max(150,#lines*25))
 footer.Text=profile.Persistent and matchStatus.Value or 'SESSION DATA / CLOUD SAVING UNAVAILABLE'
 UIS.MouseDeltaSensitivity=1
 p:SetAttribute('BLSensitivity',settings.Sensitivity); p:SetAttribute('BLFOV',settings.FOV); p:SetAttribute('BLReducedMotion',settings.ReducedMotion)
 game:GetService('Lighting').GlobalShadows=not settings.LowGraphics
end
local function layout()
 local camera=workspace.CurrentCamera; if not camera then return end
 local size=camera.ViewportSize; mobile=size.X<950 or size.Y<580
 if mobile then
  header.Position=UDim2.fromOffset(16,4); header.Size=UDim2.new(1,-32,0,52); logo.TextSize=21; identity.TextSize=11
  logo.Size=UDim2.new(1,0,0,30); identity.Position=UDim2.fromOffset(0,30); identity.Size=UDim2.new(1,0,0,24); identity.TextXAlignment=Enum.TextXAlignment.Right
  tagline.Visible=false
  nav.Position=UDim2.fromOffset(12,66); nav.Size=UDim2.new(1,-24,0,46); nav.AutomaticCanvasSize=Enum.AutomaticSize.X
  navLayout.FillDirection=Enum.FillDirection.Horizontal
  for _,b in pairs(navButtons) do b.Size=UDim2.fromOffset(112,40); b.TextSize=13 end
  panel.Position=UDim2.fromOffset(12,120); panel.Size=UDim2.new(1,-24,1,-156); side.Visible=false
 else
  header.Position=UDim2.fromOffset(28,14); header.Size=UDim2.new(1,-56,0,54); logo.TextSize=26; identity.TextSize=14
  logo.Size=UDim2.new(.6,0,0,34); identity.Position=UDim2.new(.55,0,0,6); identity.Size=UDim2.new(.45,0,0,40); tagline.Visible=true
  nav.Position=UDim2.fromOffset(28,112); nav.Size=UDim2.new(0,180,1,-168); nav.AutomaticCanvasSize=Enum.AutomaticSize.Y
  navLayout.FillDirection=Enum.FillDirection.Vertical
  for _,b in pairs(navButtons) do b.Size=UDim2.new(1,0,0,48); b.TextSize=15 end
  panel.Position=UDim2.fromOffset(230,100); panel.Size=UDim2.new(1,-548,1,-152)
  side.Position=UDim2.new(1,-296,0,100); side.Size=UDim2.new(0,268,1,-152); side.Visible=true
 end
 footer.Position=UDim2.new(0,16,1,-30); footer.Size=UDim2.new(1,-32,0,24)
end
local function avatarPreview(char)
 task.delay(.8,function()
  if not char.Parent then return end
  local was=char.Archivable; char.Archivable=true
  local ok,clone=pcall(function() return char:Clone() end); char.Archivable=was
  if not ok or not clone then return end
  for _,x in ipairs(charWorld:GetChildren()) do x:Destroy() end
  for _,x in ipairs(clone:GetDescendants()) do if x:IsA('Script') or x:IsA('LocalScript') or x:IsA('Tool') then x:Destroy()
   elseif x:IsA('BasePart') then x.Anchored=true; x.CanCollide=false; x.CanTouch=false; x.CanQuery=false end end
  clone.Parent=charWorld; clone:PivotTo(CFrame.new(0,0,0)*CFrame.Angles(0,math.rad(160),0))
  charCam.CFrame=CFrame.lookAt(Vector3.new(0,1,8),Vector3.new(0,.5,0))
  if introDone and menuOpen and mobile and current=='PLAY' then render() end
 end)
end
p.CharacterAdded:Connect(avatarPreview); if p.Character then avatarPreview(p.Character) end
local hud=create('Frame',gui,{Name='GameplayHUD',BackgroundTransparency=1,Size=UDim2.fromScale(1,1),Visible=false})
local modeStatus=text(hud,'',15); modeStatus.TextXAlignment=Enum.TextXAlignment.Center; modeStatus.Position=UDim2.new(.1,0,0,12); modeStatus.Size=UDim2.new(.8,0,0,36)
local ammoLabel=text(hud,'',19); ammoLabel.TextXAlignment=Enum.TextXAlignment.Right; ammoLabel.AnchorPoint=Vector2.new(1,1); ammoLabel.Position=UDim2.new(1,-18,1,-24); ammoLabel.Size=UDim2.fromOffset(240,38)
local hp=text(hud,'100 HP',19); hp.Position=UDim2.new(0,18,1,-62); hp.Size=UDim2.fromOffset(140,38)
local cross=text(hud,'+',25); cross.AnchorPoint=Vector2.new(.5,.5); cross.Position=UDim2.fromScale(.5,.5); cross.Size=UDim2.fromOffset(30,30); cross.TextXAlignment=Enum.TextXAlignment.Center
local hitLabel=text(hud,'',14,C.Red); hitLabel.Position=UDim2.new(.5,-80,.5,26); hitLabel.Size=UDim2.fromOffset(160,30); hitLabel.TextXAlignment=Enum.TextXAlignment.Center
local back=button(hud,'MENU',function() showMenu(true) end); back.Position=UDim2.fromOffset(14,14); back.Size=UDim2.fromOffset(80,34)
local function gameAction(name,state)
 if menuOpen then return Enum.ContextActionResult.Pass end
 local down=state==Enum.UserInputState.Begin
 if name:sub(1,6)=='BLSlot' and down then
  local id=({'Rifle','SMG','Marksman'})[tonumber(name:sub(7))]; call('Loadout',id)
 elseif name=='BLNextWeapon' and down then
  local order={'Rifle','SMG','Marksman'}; local index=1; for i,id in ipairs(order) do if weaponId==id then index=i end end
  for n=1,3 do local id=order[(index+n-1)%3+1]; if profile.Data.Owned[id] then call('Loadout',id); break end end
 elseif name=='BLFire' then firing=down elseif name=='BLAim' then aiming=down; combat:FireServer('Aim',down); p:SetAttribute('BLAiming',down) elseif name=='BLReload' and down then combat:FireServer('Reload') elseif name=='BLSprint' then combat:FireServer('Sprint',down) end
 return Enum.ContextActionResult.Sink
end
local function bindCombat(on)
 for _,name in ipairs({'BLFire','BLAim','BLReload','BLSprint','BLSlot1','BLSlot2','BLSlot3','BLNextWeapon'}) do CAS:UnbindAction(name) end
 if not on then return end
 CAS:BindAction('BLFire',gameAction,true,Enum.UserInputType.MouseButton1,Enum.KeyCode.ButtonR2)
 CAS:BindAction('BLAim',gameAction,true,Enum.UserInputType.MouseButton2,Enum.KeyCode.ButtonL2)
 CAS:BindAction('BLReload',gameAction,true,Enum.KeyCode.R,Enum.KeyCode.ButtonX)
 CAS:BindAction('BLSlot1',gameAction,false,Enum.KeyCode.One)
 CAS:BindAction('BLSlot2',gameAction,false,Enum.KeyCode.Two)
 CAS:BindAction('BLSlot3',gameAction,false,Enum.KeyCode.Three)
 CAS:BindAction('BLNextWeapon',gameAction,false,Enum.KeyCode.ButtonR1)
 CAS:BindAction('BLSprint',gameAction,true,Enum.KeyCode.LeftShift,Enum.KeyCode.ButtonL3)
 for name,title in pairs({BLFire='FIRE',BLAim='AIM',BLReload='RELOAD',BLSprint='RUN'}) do CAS:SetTitle(name,title) end
 CAS:SetPosition('BLFire',UDim2.new(1,-150,1,-160)); CAS:SetPosition('BLAim',UDim2.new(1,-230,1,-210)); CAS:SetPosition('BLReload',UDim2.new(1,-235,1,-125)); CAS:SetPosition('BLSprint',UDim2.new(0,110,1,-170))
end
showMenu=function(on)
 menuOpen=on; p:SetAttribute('BLMenuOpen',on); firing=false; aiming=false; p:SetAttribute('BLAiming',false); combat:FireServer('Aim',false); combat:FireServer('Sprint',false)
 menu.Visible=on; hud.Visible=not on; UIS.MouseIconEnabled=on; UIS.MouseBehavior=on and Enum.MouseBehavior.Default or Enum.MouseBehavior.LockCenter
 p.CameraMode=Enum.CameraMode.Classic
 if on then
  bindCombat(false); render(); layout(); if UIS.GamepadEnabled then GuiService.SelectedObject=navButtons[current] end
 else
  GuiService.SelectedObject=nil; workspace.CurrentCamera.CameraType=Enum.CameraType.Scriptable
  local hum=p.Character and p.Character:FindFirstChildOfClass('Humanoid'); if hum then workspace.CurrentCamera.CameraSubject=hum end
  bindCombat(true)
 end
end
CAS:BindAction('BLMenu',function(_,state)
 if state==Enum.UserInputState.Begin and introDone and profile then
  if menuOpen and profile.Mode=='Lobby' then return Enum.ContextActionResult.Pass end
  showMenu(not menuOpen)
 end
 return Enum.ContextActionResult.Sink
end,false,Enum.KeyCode.M,Enum.KeyCode.ButtonY)
CAS:BindAction('BLBack',function(_,state)
 if state==Enum.UserInputState.Begin and menuOpen and introDone and profile then
  if current~='PLAY' then setPage('PLAY') elseif profile.Mode~='Lobby' then showMenu(false) end
  return Enum.ContextActionResult.Sink
 end
 return Enum.ContextActionResult.Pass
end,false,Enum.KeyCode.ButtonB)
UIS.WindowFocusReleased:Connect(function() firing=false; aiming=false; combat:FireServer('Sprint',false) end)
UIS.GamepadConnected:Connect(function() if menuOpen and introDone then GuiService.SelectedObject=navButtons[current] end end)
local career=text(hud,'0 K / 0 D',14,C.Muted); career.Position=UDim2.fromOffset(18,52); career.Size=UDim2.fromOffset(180,28)
local healthTrack=create('Frame',hud,{Position=UDim2.new(0,18,1,-25),Size=UDim2.fromOffset(130,5),BackgroundColor3=C.Panel,BorderSizePixel=0})
local healthFill=create('Frame',healthTrack,{Size=UDim2.fromScale(1,1),BackgroundColor3=C.Red,BorderSizePixel=0})
local weaponButton=button(hud,'WEAPONS',function() showMenu(true); setPage('LOADOUT') end); weaponButton.Position=UDim2.fromOffset(14,84); weaponButton.Size=UDim2.fromOffset(100,32)
local trainingLabel=text(hud,'',13); trainingLabel.Position=UDim2.new(.12,0,0,100); trainingLabel.Size=UDim2.new(.76,0,0,60); trainingLabel.TextXAlignment=Enum.TextXAlignment.Center
local feed=text(hud,'',13,C.Muted); feed.Position=UDim2.new(1,-275,0,70); feed.Size=UDim2.fromOffset(260,135); feed.TextXAlignment=Enum.TextXAlignment.Right
local elimination=text(hud,'',20,C.Red); elimination.Position=UDim2.new(.25,0,.32,0); elimination.Size=UDim2.new(.5,0,0,36); elimination.TextXAlignment=Enum.TextXAlignment.Center
local damage=create('Frame',hud,{Size=UDim2.fromScale(1,1),BackgroundTransparency=1,BackgroundColor3=C.Red,BorderSizePixel=0,ZIndex=1})
local directionLabel=text(hud,'',15,C.Red); directionLabel.Position=UDim2.new(.25,0,.65,0); directionLabel.Size=UDim2.new(.5,0,0,28); directionLabel.TextXAlignment=Enum.TextXAlignment.Center
local results=create('Frame',gui,{Visible=false,Position=UDim2.fromScale(.08,.12),Size=UDim2.fromScale(.84,.76),BackgroundColor3=C.BG,BorderSizePixel=0,ZIndex=35}); round(results)
local resultTitle=text(results,'ROUND COMPLETE',26,C.Red); resultTitle.Position=UDim2.fromOffset(18,20); resultTitle.Size=UDim2.new(1,-36,0,60)
local resultBody=text(results,'',18); resultBody.Position=UDim2.fromOffset(18,90); resultBody.Size=UDim2.new(1,-36,1,-165)
local closeResults=button(results,'CONTINUE TO MENU',function() results.Visible=false; showMenu(true) end,true); closeResults.Position=UDim2.new(.05,0,1,-65); closeResults.Size=UDim2.new(.9,0,0,46)
local feedLines={}
local feedbackVersion=0
local function feedback(message)
 feedbackVersion=feedbackVersion+1; local v=feedbackVersion; elimination.Text=message
 task.delay(2,function() if feedbackVersion==v then elimination.Text='' end end)
end
local tracerPool={}; local tracerCursor=0
local function shotVisual(data)
 if menuOpen then return end
 tracerCursor=tracerCursor%32+1
 local t=tracerPool[tracerCursor]
 if not t then
  local part=create('Part',workspace,{Name='LocalTracer',Anchored=true,CanCollide=false,CanTouch=false,CanQuery=false,CastShadow=false,Material=Enum.Material.Neon,Color=C.Red,Transparency=1})
  t={Part=part,Until=0}; tracerPool[tracerCursor]=t
 end
 local distance=(data.Finish-data.Origin).Magnitude
 if distance<.01 then return end
 t.Part.Size=Vector3.new(.06,.06,distance); t.Part.CFrame=CFrame.lookAt((data.Origin+data.Finish)/2,data.Finish)
 t.Part.Transparency=0; t.Until=os.clock()+.06
end
local hitVersion=0
event.OnClientEvent:Connect(function(kind,data)
 if kind=='Profile' then
  acceptProfile(data)
  if menuOpen and introDone and not busy and not editingSettings then render() end
 elseif kind=='Shot' then shotVisual(data)
 elseif kind=='Roster' then
  if profile then profile.Roster=data; refreshProfile(); if menuOpen and current=='TEAM' and not busy then render() end end
 elseif kind=='Persistence' then
  if profile then profile.Persistent=false; profile.Notice=data; refreshProfile() end
  notify(data)
 elseif kind=='Notice' then notify(data)
 elseif kind=='Ammo' then p:SetAttribute('BLReloading',data.Reloading); ammo=data.Ammo; reloading=data.Reloading; ammoLabel.Text=data.Name..' / '..(reloading and 'RELOADING' or (data.Ammo..' | '..data.Magazine))
 elseif kind=='Training' then trainingStats=data; if profile then profile.Training=data end
 elseif kind=='Match' then
  currentMatch=data
  if data.Remaining==8 and results.Visible then results.Visible=false end
 elseif kind=='Damage' then
  damage.BackgroundTransparency=.82; tween(damage,{BackgroundTransparency=1},.35)
  local root=p.Character and p.Character:FindFirstChild('HumanoidRootPart')
  if root then
   local d=data.From-root.Position; local cam=workspace.CurrentCamera
   local side=d:Dot(cam.CFrame.RightVector); local front=d:Dot(cam.CFrame.LookVector)
   directionLabel.Text=math.abs(side)>math.abs(front) and (side>0 and 'DAMAGE / RIGHT' or 'DAMAGE / LEFT') or (front>0 and 'DAMAGE / FRONT' or 'DAMAGE / BEHIND')
   task.delay(1,function() directionLabel.Text='' end)
  end
 elseif kind=='Elimination' then feedback('ELIMINATED / '..data)
 elseif kind=='Death' then firing=false; aiming=false; p:SetAttribute('BLAiming',false); feedback(data)
 elseif kind=='KillFeed' then
  table.insert(feedLines,1,{Text=data.Killer..' > '..data.Victim..' / '..data.Weapon,Expires=os.clock()+6}); while #feedLines>4 do table.remove(feedLines) end
 elseif kind=='Results' then
  firing=false; aiming=false; results.Visible=true; showMenu(true)
  resultTitle.Text=data.Outcome..' / '..data.Mode
  resultBody.Text=string.format('ELIMINATIONS  %d\nDEATHS  %d\nMATCH CREDITS  +%d',data.Kills,data.Deaths,data.Reward)
  if UIS.GamepadEnabled then GuiService.SelectedObject=closeResults end
 elseif kind=='Hit' then hitVersion=hitVersion+1; local v=hitVersion; hitLabel.Text=data and 'ELIMINATED' or 'HIT'; task.delay(.4,function() if hitVersion==v then hitLabel.Text='' end end)
 end
end)
matchStatus:GetPropertyChangedSignal('Value'):Connect(function() if profile then refreshProfile() end end)
local lastViewport=Vector2.new()
local hudElapsed=0; local lastHealth,lastMode
local layoutCombatHUD
layoutCombatHUD=function()
 local narrow=workspace.CurrentCamera.ViewportSize.X<600
 ammoLabel.Size=UDim2.fromOffset(narrow and 180 or 240,38); ammoLabel.TextSize=narrow and 16 or 19
 hp.Size=UDim2.fromOffset(narrow and 85 or 140,38); hp.TextSize=narrow and 16 or 19
 modeStatus.Position=UDim2.new(0,104,0,12); modeStatus.Size=UDim2.new(1,-116,0,36); modeStatus.TextSize=narrow and 12 or 15
 feed.Position=narrow and UDim2.new(.5,4,0,60) or UDim2.new(1,-275,0,70)
 feed.Size=narrow and UDim2.new(.5,-20,0,100) or UDim2.fromOffset(260,135); feed.TextSize=narrow and 11 or 13
 trainingLabel.Position=UDim2.new(0,14,0,narrow and 140 or 100); trainingLabel.Size=UDim2.new(1,-28,0,56)
 resultTitle.TextSize=narrow and 18 or 26
end
Run.RenderStepped:Connect(function(dt)
 local camera=workspace.CurrentCamera; if not camera then return end
 local now=os.clock()
 local light=game:GetService('Lighting'); if light.GlobalShadows==settings.LowGraphics then light.GlobalShadows=not settings.LowGraphics end
 for _,t in ipairs(tracerPool) do if t.Until>0 and now>=t.Until then t.Part.Transparency=1; t.Until=0 end end
 if camera.ViewportSize~=lastViewport then lastViewport=camera.ViewportSize; local before=mobile; layout(); layoutCombatHUD(); if before~=mobile and introDone then render() end end
 if menuOpen then
  camera.CameraType=Enum.CameraType.Scriptable
  local angle=settings.ReducedMotion and .7 or .7+math.sin(os.clock()*.08)*.08
  camera.CFrame=CFrame.lookAt(Vector3.new(math.cos(angle)*165,85,math.sin(angle)*165),Vector3.new(0,4,0))
  camera.FieldOfView=70
 else
  p:SetAttribute('BLFOV',settings.FOV); p:SetAttribute('BLSensitivity',settings.Sensitivity); p:SetAttribute('BLReducedMotion',settings.ReducedMotion)
  UIS.MouseDeltaSensitivity=1
  cross.Visible=settings.Crosshair
  local hum=p.Character and p.Character:FindFirstChildOfClass('Humanoid')
  hudElapsed=hudElapsed+dt
  if hudElapsed>=.1 then
   hudElapsed=0; local health=hum and math.ceil(hum.Health)..' HP' or 'RESPAWNING'
   local mode=profile and profile.Mode=='Practice' and 'AIM TRAINING' or (currentMatch and currentMatch.Text or (profile and profile.Match or matchStatus.Value))
   if currentMatch and profile and profile.Mode=='Arena' then mode=string.format('AZURE %d / %s / %d EMBER',currentMatch.Scores.Azure,mode,currentMatch.Scores.Ember)
   elseif currentMatch and currentMatch.Board[1] then mode=mode..' / LEADER '..currentMatch.Board[1].Name..' '..currentMatch.Board[1].Kills end
   healthFill.Size=UDim2.fromScale(hum and math.clamp(hum.Health/hum.MaxHealth,0,1) or 0,1)
   career.Text=profile and string.format('%d K / %d D',profile.RoundKills or 0,profile.RoundDeaths or 0) or ''
   local lines={}; for _,line in ipairs(feedLines) do if line.Expires>now then table.insert(lines,line.Text) end end; feed.Text=table.concat(lines,'\n'); feed.Visible=profile and profile.Mode~='Practice' or false
   local t=trainingStats or (profile and profile.Training)
   trainingLabel.Visible=profile and profile.Mode=='Practice' or false
   if trainingLabel.Visible and t then trainingLabel.Text=string.format('%s / %d HITS / %d SHOTS / %.1f%% / REACTION %.0f MS',t.Drill:upper(),t.Hits,t.Shots,t.Accuracy,t.ReactionMs) end
   if health~=lastHealth then hp.Text=health; lastHealth=health end
   if mode~=lastMode then modeStatus.Text=mode; lastMode=mode end
  end
  local t=p.Character and p.Character:FindFirstChildOfClass('Tool'); local w=profile and profile.Weapons[weaponId]
  if firing and t and t:GetAttribute('BlocklineWeapon') and hum and hum.Health>0 and ammo>0 and not reloading and w and os.clock()-lastShot>=w.Interval then
   lastShot=os.clock(); local ray=camera:ViewportPointToRay(camera.ViewportSize.X/2,camera.ViewportSize.Y/2)
   local hit=workspace:Raycast(ray.Origin,ray.Direction*w.Range,aimParams)
   local target=hit and hit.Position or ray.Origin+ray.Direction*w.Range
   local head=p.Character:FindFirstChild('Head'); if head then combat:FireServer('Shoot',(target-head.Position).Unit) end
   p:SetAttribute('BLShotAt',os.clock()); p:SetAttribute('BLRecoil',settings.ReducedMotion and 0 or (w.Recoil or .7))
  end
 end
end)
-- Real progress: account response, then each distinct character/arena content asset.
local intro=create('Frame',gui,{Name='Introduction',Size=UDim2.fromScale(1,1),BackgroundColor3=C.BG,BackgroundTransparency=.08,BorderSizePixel=0,ZIndex=50})
local mark=text(intro,'B /',72,C.Red); mark.AnchorPoint=Vector2.new(.5,.5); mark.Position=UDim2.fromScale(.5,.34); mark.Size=UDim2.fromOffset(180,100); mark.TextXAlignment=Enum.TextXAlignment.Center; mark.Font=Enum.Font.GothamBlack; mark.ZIndex=51
local title=text(intro,'BLOCKLINE',38); title.AnchorPoint=Vector2.new(.5,.5); title.Position=UDim2.fromScale(.5,.47); title.Size=UDim2.new(.9,0,0,60); title.TextXAlignment=Enum.TextXAlignment.Center; title.Font=Enum.Font.GothamBlack; title.ZIndex=51
local loading=text(intro,'CONNECTING / LOADING ACCOUNT',13,C.Muted); loading.TextXAlignment=Enum.TextXAlignment.Center; loading.AnchorPoint=Vector2.new(.5,.5); loading.Position=UDim2.fromScale(.5,.6); loading.Size=UDim2.new(.9,0,0,42); loading.ZIndex=51
local track=create('Frame',intro,{Position=UDim2.fromScale(.2,.68),Size=UDim2.new(.6,0,0,3),BackgroundColor3=C.Panel,BorderSizePixel=0,ZIndex=51})
local fill=create('Frame',track,{Size=UDim2.fromScale(0,1),BackgroundColor3=C.Red,BorderSizePixel=0,ZIndex=52})
layout(); render()
task.spawn(function()
 repeat
  local ok,r=pcall(function() return request:InvokeServer('Fetch') end)
  if ok and r.OK and r.Profile then profile=r.Profile; settings=profile.Data.Settings; weaponId=profile.Data.Equipped; break end
  loading.Text=ok and 'LOADING ACCOUNT DATA…' or 'CONNECTION INTERRUPTED / RETRYING…'
  task.wait(1)
 until not gui.Parent
 if not gui.Parent then return end
 refreshProfile(); fill.Size=UDim2.fromScale(.25,1)
 local assets={}; local seen={}
 local sources={workspace:WaitForChild('BlocklineArena')}; if p.Character then table.insert(sources,p.Character) end
 for _,source in ipairs(sources) do for _,o in ipairs(source:GetDescendants()) do
  local asset
  if o:IsA('MeshPart') then asset=o.MeshId elseif o:IsA('Decal') or o:IsA('Texture') then asset=o.Texture elseif o:IsA('Shirt') then asset=o.ShirtTemplate elseif o:IsA('Pants') then asset=o.PantsTemplate end
  if asset and asset~='' and not seen[asset] then seen[asset]=true; table.insert(assets,asset) end
 end end
 local done,failed=0,0
 if #assets>0 then
  local preloadOK=pcall(function() Content:PreloadAsync(assets,function(_,assetStatus)
   done=done+1; if assetStatus~=Enum.AssetFetchStatus.Success then failed=failed+1 end
   local progress=.25+.75*math.min(1,done/#assets)
   loading.Text=string.format('LOADING ASSETS / %d%%',math.floor(progress*100)); tween(fill,{Size=UDim2.fromScale(progress,1)},.1)
  end) end)
  if not preloadOK then failed=failed+1 end
 end
 loading.Text=failed>0 and 'READY / SOME ASSETS UNAVAILABLE' or 'READY / 100%'; fill.Size=UDim2.fromScale(1,1)
 render(); showMenu(true); introDone=true
 if not settings.ReducedMotion then
  tween(intro,{BackgroundTransparency=1},.45)
  for _,o in ipairs(intro:GetDescendants()) do if o:IsA('TextLabel') then tween(o,{TextTransparency=1},.4) elseif o:IsA('Frame') then tween(o,{BackgroundTransparency=1},.4) end end
  task.wait(.5)
 end
 intro:Destroy()
 if profile.Notice~='' then notify(profile.Notice) elseif failed>0 then notify('Some character assets could not load; the arena is ready.') end
end)
