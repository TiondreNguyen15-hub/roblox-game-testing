local Players=game:GetService('Players')
local UIS=game:GetService('UserInputService')
local CAS=game:GetService('ContextActionService')
local Run=game:GetService('RunService')
local p=Players.LocalPlayer
local RS=game:GetService('ReplicatedStorage')
local remote=RS:WaitForChild('BlocklineAction')
local status=RS:WaitForChild('BlocklineStatus')
p.CameraMode=Enum.CameraMode.LockFirstPerson
local gui=Instance.new('ScreenGui',p:WaitForChild('PlayerGui')); gui.Name='BlocklineHUD'; gui.ResetOnSpawn=false
local function label(name,text,pos,size,fontSize)
 local l=Instance.new('TextLabel',gui); l.Name=name; l.Text=text; l.Position=pos; l.Size=size
 l.BackgroundColor3=Color3.fromRGB(16,22,32); l.BackgroundTransparency=0.18; l.BorderSizePixel=0
 l.TextColor3=Color3.fromRGB(227,237,246); l.Font=Enum.Font.GothamBold; l.TextSize=fontSize or 18
 local corner=Instance.new('UICorner',l); corner.CornerRadius=UDim.new(0,8)
 return l
end
label('Brand','BLOCKLINE  /  CITY ARENA',UDim2.new(0,16,0,16),UDim2.new(0,280,0,36),16)
local match=label('Match','CONNECTING',UDim2.new(0.5,-190,0,62),UDim2.new(0,380,0,40),18)
local ammo=label('Ammo','AR-24  /  30',UDim2.new(1,-238,1,-95),UDim2.new(0,220,0,52),24)
label('Controls','FIRE: CLICK / RT   •   AIM: RMB / LT   •   RELOAD: R / X   •   SPRINT: SHIFT / L3',UDim2.new(0.5,-300,1,-37),UDim2.new(0,600,0,26),11)
local health=label('Health','100 HP',UDim2.new(0,16,1,-95),UDim2.new(0,150,0,52),24)
local cross=label('Crosshair','+',UDim2.new(0.5,-15,0.5,-15),UDim2.new(0,30,0,30),28); cross.BackgroundTransparency=1
local hit=label('Hit','',UDim2.new(0.5,-65,0.5,32),UDim2.new(0,130,0,28),14); hit.BackgroundTransparency=1
local firing=false; local aiming=false; local ammoCount=30; local reloading=false; local hitVersion=0
local function reset() firing=false; aiming=false; UIS.MouseIconEnabled=false end
p.CharacterAdded:Connect(reset); reset()
status:GetPropertyChangedSignal('Value'):Connect(function() match.Text=status.Value end); match.Text=status.Value
remote.OnClientEvent:Connect(function(action,a,b)
 if action=='Ammo' then ammoCount=a; reloading=b; ammo.Text=b and 'RELOADING…' or ('AR-24  /  '..a..'  ∞') end
 if action=='Hit' then
  hitVersion=hitVersion+1; local version=hitVersion; hit.Text=a and 'ELIMINATED' or 'HIT'; hit.TextColor3=a and Color3.fromRGB(255,140,90) or Color3.fromRGB(90,220,255)
  task.delay(0.4,function() if version==hitVersion then hit.Text='' end end)
 end
end)
local function action(name,state)
 local down=state==Enum.UserInputState.Begin
 if name=='Fire' then firing=down
 elseif name=='Aim' then aiming=down
 elseif name=='Reload' and down then remote:FireServer('Reload')
 elseif name=='Sprint' then remote:FireServer('Sprint',down) end
 return Enum.ContextActionResult.Sink
end
CAS:BindAction('Fire',action,true,Enum.UserInputType.MouseButton1,Enum.KeyCode.ButtonR2)
CAS:BindAction('Aim',action,true,Enum.UserInputType.MouseButton2,Enum.KeyCode.ButtonL2)
CAS:BindAction('Reload',action,true,Enum.KeyCode.R,Enum.KeyCode.ButtonX)
CAS:BindAction('Sprint',action,true,Enum.KeyCode.LeftShift,Enum.KeyCode.ButtonL3)
CAS:SetTitle('Fire','FIRE'); CAS:SetTitle('Aim','AIM'); CAS:SetTitle('Reload','RELOAD'); CAS:SetTitle('Sprint','RUN')
CAS:SetPosition('Fire',UDim2.new(1,-165,1,-155)); CAS:SetPosition('Aim',UDim2.new(1,-245,1,-210))
CAS:SetPosition('Reload',UDim2.new(1,-245,1,-130)); CAS:SetPosition('Sprint',UDim2.new(0,100,1,-165))
UIS.WindowFocusReleased:Connect(function() firing=false; aiming=false; remote:FireServer('Sprint',false) end)
local lastShot=0
Run.RenderStepped:Connect(function(dt)
 local cam=workspace.CurrentCamera
 if not cam then return end
 cam.FieldOfView=cam.FieldOfView+((aiming and 52 or 78)-cam.FieldOfView)*math.min(1,dt*14)
 local char=p.Character; local hum=char and char:FindFirstChildOfClass('Humanoid')
 if hum then health.Text=math.ceil(hum.Health)..' HP' end
 if firing and hum and hum.Health>0 and char:FindFirstChild('AR-24') and ammoCount>0 and not reloading and os.clock()-lastShot>=0.11 then
  lastShot=os.clock()
  local ray=cam:ViewportPointToRay(cam.ViewportSize.X/2,cam.ViewportSize.Y/2)
  remote:FireServer('Shoot',ray.Direction.Unit)
  cam.CFrame=cam.CFrame*CFrame.Angles(math.rad(0.5),0,0)
 end
end)
remote:FireServer('Sync')
