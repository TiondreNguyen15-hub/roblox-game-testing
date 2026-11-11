-- Cinematic loading presentation. All progress is supplied by completed asset requests.
local Loading={}
local Players=game:GetService('Players')
local Run=game:GetService('RunService')
local Tween=game:GetService('TweenService')
local Content=game:GetService('ContentProvider')
local SoundService=game:GetService('SoundService')
local UIS=game:GetService('UserInputService')
local GuiService=game:GetService('GuiService')
-- Replace with a music asset your experience has permission to play if Roblox denies this track.
local MUSIC_ID='rbxassetid://1843529606'
local gui,root,bar,status,counter,tip,musicLabel,volumeLabel,showcase,camera,model
local connection,music; local reduced=false; local closed=false; local elapsed=0; local tipIndex=1; local volume=.25; local previousVolume=.25
local tips={
 'HOLD YOUR ANGLE  /  Use cover to break long sightlines.',
 'CONTROL THE BURST  /  Aim down sights for a tighter spread.',
 'STAY MOBILE  /  Sprint between cover. Stop sprinting before firing.',
 'TRAIN WITH INTENT  /  Practice tracking before entering a match.',
 'CHECK YOUR AMMO  /  Reload from safety, not in an open lane.',
}
local function new(class,parent,properties)
 local o=Instance.new(class); for k,v in pairs(properties or {})do o[k]=v end;o.Parent=parent;return o
end
local function label(parent,name,value,size,color)
 return new('TextLabel',parent,{Name=name,Text=value,Font=Enum.Font.GothamMedium,TextSize=size,TextColor3=color or Color3.fromRGB(237,239,244),BackgroundTransparency=1,TextXAlignment=Enum.TextXAlignment.Left,TextWrapped=true})
end
local function fade(o,properties,duration)
 Tween:Create(o,TweenInfo.new(reduced and 0 or duration,Enum.EasingStyle.Quad,Enum.EasingDirection.Out),properties):Play()
end
local function button(parent,name,value,position,size,callback)
 local b=new('TextButton',parent,{Name=name,Text=value,Position=position,Size=size,BackgroundColor3=Color3.fromRGB(32,35,42),TextColor3=Color3.fromRGB(239,240,244),Font=Enum.Font.GothamBold,TextSize=14,BorderSizePixel=0,AutoButtonColor=false,Selectable=true})
 new('UICorner',b,{CornerRadius=UDim.new(0,5)})
 local function focus()fade(b,{BackgroundColor3=Color3.fromRGB(168,24,42)},.15)end
 local function blur()fade(b,{BackgroundColor3=Color3.fromRGB(32,35,42)},.15)end
 b.MouseEnter:Connect(focus);b.MouseLeave:Connect(blur);b.SelectionGained:Connect(focus);b.SelectionLost:Connect(blur)
 b.Activated:Connect(callback);return b
end
local function setVolume(value)
 volume=math.clamp(math.floor(value*100+.5)/100,0,1)
 if music then music.Volume=volume end
 if volumeLabel then volumeLabel.Text=string.format('MUSIC  %d%%',math.floor(volume*100+.5))end
end
function Loading.Start()
 if gui then return Loading end
 local p=Players.LocalPlayer
 gui=new('ScreenGui',p:WaitForChild('PlayerGui'),{Name='BlocklineLoading',ResetOnSpawn=false,IgnoreGuiInset=true,DisplayOrder=100,ZIndexBehavior=Enum.ZIndexBehavior.Sibling})
 root=new('CanvasGroup',gui,{Name='Cinematic',Size=UDim2.fromScale(1,1),BackgroundColor3=Color3.fromRGB(8,10,14),BorderSizePixel=0})
 new('UIGradient',root,{Color=ColorSequence.new(Color3.fromRGB(6,8,11),Color3.fromRGB(28,32,40)),Rotation=20})
 local accent=new('Frame',root,{Position=UDim2.fromScale(.055,.12),Size=UDim2.new(0,4,.12,0),BackgroundColor3=Color3.fromRGB(240,30,52),BorderSizePixel=0})
 local logo=label(root,'Wordmark','B /  BLOCKLINE',44);logo.Font=Enum.Font.GothamBlack;logo.Position=UDim2.fromScale(.075,.12);logo.Size=UDim2.fromScale(.84,.09)
 local eyebrow=label(root,'Eyebrow','TACTICAL  /  COMPETITIVE  /  PVP',12,Color3.fromRGB(240,50,68));eyebrow.Position=UDim2.fromScale(.078,.22);eyebrow.Size=UDim2.fromScale(.85,.04)
 local title=label(root,'Title','EVERY ANGLE\nMATTERS.',48);title.Font=Enum.Font.GothamBlack;title.Position=UDim2.fromScale(.075,.33);title.Size=UDim2.fromScale(.4,.23)
 local sub=label(root,'Subtitle','PREPARE. ADAPT. TAKE CONTROL.',12,Color3.fromRGB(139,147,162));sub.Position=UDim2.fromScale(.078,.57);sub.Size=UDim2.fromScale(.8,.045)
 showcase=new('ViewportFrame',root,{Name='WeaponShowcase',Position=UDim2.fromScale(.44,.18),Size=UDim2.fromScale(.55,.48),BackgroundTransparency=1,Ambient=Color3.fromRGB(115,126,145),LightColor=Color3.fromRGB(255,218,210),LightDirection=Vector3.new(-1,-.5,-1)})
 local world=new('WorldModel',showcase,{Name='DisplayWorld'});model=new('Model',world,{Name='OriginalRifleShowcase'})
 local dark=Color3.fromRGB(28,32,38);local metal=Color3.fromRGB(88,96,105);local red=Color3.fromRGB(220,27,46)
 local function part(name,size,pos,color)
  return new('Part',model,{Name=name,Size=size,Position=pos,Color=color or dark,Material=Enum.Material.Metal,Anchored=true,CanCollide=false,CanTouch=false,CanQuery=false,CastShadow=false})
 end
 part('Receiver',Vector3.new(.55,.8,2.7),Vector3.new())
 part('Upper receiver',Vector3.new(.44,.3,2.6),Vector3.new(0,.52,0),metal)
 part('Handguard',Vector3.new(.5,.6,2.2),Vector3.new(0,.05,-2.4))
 part('Barrel',Vector3.new(.2,.2,1.6),Vector3.new(0,.08,-4.25),metal)
 part('Muzzle',Vector3.new(.3,.3,.4),Vector3.new(0,.08,-5.15))
 part('Stock',Vector3.new(.5,.65,1.8),Vector3.new(0,.06,2.1))
 part('Buttpad',Vector3.new(.65,.95,.2),Vector3.new(0,.06,3.05),metal)
 part('Pistol grip',Vector3.new(.4,1.1,.55),Vector3.new(0,-.85,.65))
 part('Magazine',Vector3.new(.4,1.25,.75),Vector3.new(0,-.95,-.5),metal)
 part('Red magazine band',Vector3.new(.43,.18,.78),Vector3.new(0,-1.25,-.5),red)
 part('Trigger guard',Vector3.new(.16,.12,.7),Vector3.new(.29,-.6,.15),metal)
 for i=1,9 do part('Rail segment',Vector3.new(.56,.12,.11),Vector3.new(0,.76,1.1-i*.42),metal)end
 for i=1,5 do for _,x in ipairs({-.26,.26})do part('Handguard vent',Vector3.new(.02,.16,.2),Vector3.new(x,.08,-1.7-i*.28),Color3.fromRGB(7,9,11))end end
 part('Rear sight',Vector3.new(.4,.32,.22),Vector3.new(0,.92,.9))
 part('Front sight',Vector3.new(.28,.42,.2),Vector3.new(0,.66,-3.25))
 camera=new('Camera',showcase,{FieldOfView=36});showcase.CurrentCamera=camera
 camera.CFrame=CFrame.lookAt(Vector3.new(10,3.6,7),Vector3.new(0,-.1,-.8))
 status=label(root,'Status','CONNECTING TO THE SERVER',13);status.Position=UDim2.fromScale(.075,.71);status.Size=UDim2.fromScale(.7,.045)
 counter=label(root,'Progress','WAITING',13,Color3.fromRGB(160,170,186));counter.TextXAlignment=Enum.TextXAlignment.Right;counter.Position=UDim2.fromScale(.75,.71);counter.Size=UDim2.fromScale(.175,.045)
 local track=new('Frame',root,{Position=UDim2.fromScale(.075,.77),Size=UDim2.new(.85,0,0,4),BackgroundColor3=Color3.fromRGB(50,55,65),BorderSizePixel=0})
 bar=new('Frame',track,{Name='CompletedAssets',Size=UDim2.fromScale(0,1),BackgroundColor3=Color3.fromRGB(240,30,52),BorderSizePixel=0})
 tip=label(root,'GameplayTip',tips[1],13,Color3.fromRGB(160,170,186));tip.Position=UDim2.fromScale(.075,.80);tip.Size=UDim2.fromScale(.85,.06)
 musicLabel=label(root,'MusicStatus','LOADING MUSIC',10,Color3.fromRGB(125,133,148));musicLabel.Position=UDim2.fromScale(.075,.94);musicLabel.Size=UDim2.fromScale(.85,.035)
 volumeLabel=label(root,'Volume','MUSIC  25%',12);volumeLabel.Position=UDim2.fromScale(.075,.885);volumeLabel.Size=UDim2.fromScale(.25,.045)
 button(root,'LowerVolume','−',UDim2.fromScale(.49,.875),UDim2.new(.12,0,0,44),function()setVolume(volume-.1)end)
 button(root,'RaiseVolume','+',UDim2.fromScale(.63,.875),UDim2.new(.12,0,0,44),function()setVolume(volume+.1)end)
 local mute=button(root,'Mute','MUTE',UDim2.fromScale(.77,.875),UDim2.new(.155,0,0,44),function()
  if volume>0 then previousVolume=volume;setVolume(0)else setVolume(previousVolume)end
 end)
 if UIS.GamepadEnabled then GuiService.SelectedObject=mute end
 music=new('Sound',SoundService,{Name='BlocklineLoadingMusic',SoundId=MUSIC_ID,Looped=true,Volume=volume})
 task.spawn(function()
  local ok,loaded=false,false
  ok=pcall(function()Content:PreloadAsync({music},function(_,fetch)loaded=fetch==Enum.AssetFetchStatus.Success end)end)
  if closed then return end
  if ok and music.IsLoaded then music:Play();musicLabel.Text='LOADING SCREEN MUSIC'
  else musicLabel.Text='MUSIC UNAVAILABLE / CHECK EXPERIENCE AUDIO PERMISSIONS' end
 end)
 local previousSize=Vector2.new()
 connection=Run.RenderStepped:Connect(function(dt)
  if closed then return end
  elapsed=elapsed+dt;local cam=workspace.CurrentCamera
  if cam and cam.ViewportSize~=previousSize then
   previousSize=cam.ViewportSize;local narrow=previousSize.X<800
   title.TextSize=narrow and 26 or 48;logo.TextSize=narrow and 26 or 44
   title.Position=UDim2.fromScale(.075,narrow and .30 or .33); title.Size=UDim2.fromScale(narrow and .85 or .4,narrow and .12 or .23)
   sub.Position=UDim2.fromScale(.078,narrow and .65 or .57)
   showcase.Position=UDim2.fromScale(narrow and .25 or .44,narrow and .42 or .18)
   showcase.Size=UDim2.fromScale(narrow and .72 or .55,narrow and .22 or .48)
   status.TextSize=narrow and 10 or 13;counter.TextSize=narrow and 10 or 13;tip.TextSize=narrow and 11 or 13
  end
  if not reduced then model:PivotTo(CFrame.Angles(0,math.sin(elapsed*.24)*.16,0))end
  local index=math.floor(elapsed/5)%#tips+1
  if index~=tipIndex then tipIndex=index;tip.Text=tips[index]end
 end)
 return Loading
end
function Loading.Status(message)if status and not closed then status.Text=message end end
function Loading.Progress(done,total,failed)
 if not bar or closed then return end
 local fraction=total>0 and math.clamp(done/total,0,1)or 1
 counter.Text=string.format('%d / %d  ·  %d%%',done,total,math.floor(fraction*100))
 fade(bar,{Size=UDim2.fromScale(fraction,1)},.18)
 if failed and failed>0 then Loading.Status('ASSETS PROCESSED / '..failed..' UNAVAILABLE')end
end
function Loading.ReducedMotion(value)reduced=value==true end
function Loading.Finish()
 if closed then return end;closed=true
 if connection then connection:Disconnect()end
 if music then fade(music,{Volume=0},.5)end
 if root then fade(root,{GroupTransparency=1},.6)end
 task.delay(reduced and 0 or .65,function()
  if music then music:Stop();music:Destroy()end
  if gui then gui:Destroy()end
 end)
end
return Loading
