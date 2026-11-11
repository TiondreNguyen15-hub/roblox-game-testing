-- Shoulder camera and local presentation. Server owns speeds, stance and damage.
local Players=game:GetService('Players')
local UIS=game:GetService('UserInputService')
local CAS=game:GetService('ContextActionService')
local Run=game:GetService('RunService')
local p=Players.LocalPlayer
local combat=game:GetService('ReplicatedStorage'):WaitForChild('Blockline'):WaitForChild('Combat')
local yaw,pitch=0,math.rad(-8)
local stick=Vector2.new(); local crouch=false; local cameraPosition
local char,hum,root,tool; local waist,neck; local baseGrip
local params=RaycastParams.new(); params.FilterType=Enum.RaycastFilterType.Exclude
local function setCharacter(c)
 char=c; hum=c:WaitForChild('Humanoid',10); root=c:WaitForChild('HumanoidRootPart',10)
 cameraPosition=nil; crouch=false; tool=nil; baseGrip=nil
 params.FilterDescendantsInstances={c}
 waist=c:FindFirstChild('Waist',true) or c:FindFirstChild('RootJoint',true)
 neck=c:FindFirstChild('Neck',true)
 if root then local _,angle=root.CFrame:ToOrientation(); yaw=angle end
end
p.CharacterAdded:Connect(setCharacter); if p.Character then task.spawn(setCharacter,p.Character) end
UIS.InputChanged:Connect(function(input,processed)
 if p:GetAttribute('BLMenuOpen')~=false then return end
 local sensitivity=p:GetAttribute('BLSensitivity') or 1
 if input.UserInputType==Enum.UserInputType.MouseMovement then
  yaw=yaw-input.Delta.X*.003*sensitivity; pitch=math.clamp(pitch-input.Delta.Y*.003*sensitivity,math.rad(-65),math.rad(65))
 elseif input.KeyCode==Enum.KeyCode.Thumbstick2 then stick=input.Position
 elseif input.UserInputType==Enum.UserInputType.Touch and not processed and input.Position.X>workspace.CurrentCamera.ViewportSize.X*.5 then
  yaw=yaw-input.Delta.X*.004*sensitivity; pitch=math.clamp(pitch-input.Delta.Y*.004*sensitivity,math.rad(-65),math.rad(65))
 end
end)
local function stance(_,state)
 if p:GetAttribute('BLMenuOpen')~=false then return Enum.ContextActionResult.Pass end
 if state==Enum.UserInputState.Begin then crouch=not crouch; combat:FireServer('Crouch',crouch); p:SetAttribute('BLCrouch',crouch) end
 return Enum.ContextActionResult.Sink
end
local controls
local function bindStance()
 if not controls then
  local scripts=p:FindFirstChild('PlayerScripts'); local module=scripts and scripts:FindFirstChild('PlayerModule')
  if module then local ok,result=pcall(function()return require(module):GetControls()end); if ok then controls=result end end
 end
 if controls then if p:GetAttribute('BLMenuOpen')==false then controls:Enable() else controls:Disable() end end
 CAS:UnbindAction('BLCrouch')
 if p:GetAttribute('BLMenuOpen')==false then
  CAS:BindAction('BLCrouch',stance,true,Enum.KeyCode.C,Enum.KeyCode.LeftControl,Enum.KeyCode.ButtonR3)
  CAS:SetTitle('BLCrouch','CROUCH'); CAS:SetPosition('BLCrouch',UDim2.new(1,-320,1,-130))
 elseif crouch then crouch=false; combat:FireServer('Crouch',false); p:SetAttribute('BLCrouch',false) end
end
p:GetAttributeChangedSignal('BLMenuOpen'):Connect(bindStance); bindStance()
UIS.WindowFocusReleased:Connect(function() stick=Vector2.new() end)
Run:BindToRenderStep('BlocklineShoulderCamera',Enum.RenderPriority.Camera.Value+1,function(dt)
 if p:GetAttribute('BLMenuOpen')~=false or not char or not root or not hum or hum.Health<=0 then return end
 local camera=workspace.CurrentCamera; if not camera then return end
 camera.CameraType=Enum.CameraType.Scriptable; p.CameraMode=Enum.CameraMode.Classic
 local axis=Vector2.new(stick.X,stick.Y); local dead=.15
 if axis.Magnitude>dead then yaw=yaw-axis.X*dt*2.7; pitch=math.clamp(pitch+axis.Y*dt*2.2,math.rad(-65),math.rad(65)) end
 local aim=p:GetAttribute('BLAiming')==true
 local recoil=p:GetAttribute('BLRecoil') or 0
 if recoil>0 then pitch=math.clamp(pitch+math.rad(recoil),math.rad(-65),math.rad(65)); p:SetAttribute('BLRecoil',0) end
 local orientation=CFrame.Angles(0,yaw,0)*CFrame.Angles(pitch,0,0)
 local focus=root.Position+Vector3.new(0,crouch and 1.05 or 1.85,0)
 local desired=focus+orientation:VectorToWorldSpace(Vector3.new(aim and 1.8 or 2.5,.35,aim and 5 or 9))
 local hit=workspace:Raycast(focus,desired-focus,params)
 if hit then local distance=(focus-hit.Position).Magnitude; desired=distance>.01 and hit.Position+(focus-hit.Position).Unit*.35 or focus end
 local alpha=1-math.exp(-dt*18)
 -- Obstruction snaps inward; easing only outward prevents interpolating through walls.
 if not cameraPosition or hit then cameraPosition=desired else cameraPosition=cameraPosition:Lerp(desired,alpha) end
 camera.CFrame=CFrame.lookAt(cameraPosition,focus+orientation.LookVector*500)
 local targetFOV=(p:GetAttribute('BLFOV') or 80)-(aim and 20 or 0)
 camera.FieldOfView=camera.FieldOfView+(targetFOV-camera.FieldOfView)*alpha
 hum.AutoRotate=false
 if hum.MoveDirection.Magnitude>.05 or aim then
  root.CFrame=root.CFrame:Lerp(CFrame.new(root.Position)*CFrame.Angles(0,yaw,0),1-math.exp(-dt*16))
 end
 -- Roblox's default Animate handles locomotion/jump; additive stance/aim poses support R6/R15.
 local reduced=p:GetAttribute('BLReducedMotion')==true
 if waist then waist.Transform=CFrame.Angles(crouch and math.rad(18) or 0,0,0) end
 if neck then neck.Transform=CFrame.Angles(aim and pitch*.35 or 0,0,0) end
 local equipped=char:FindFirstChildOfClass('Tool')
 if equipped~=tool then tool=equipped; baseGrip=tool and tool.Grip end
 if tool and baseGrip then
  local reload=p:GetAttribute('BLReloading')==true
  local kick=math.max(0,(p:GetAttribute('BLShotAt') or 0)+.13-os.clock())/.13
  local angle=reload and math.rad(-35+math.sin(os.clock()*10)*8) or (reduced and 0 or math.rad(kick*8))
  tool.Grip=baseGrip*CFrame.Angles(angle,0,reload and math.rad(20) or 0)
 end
end)
