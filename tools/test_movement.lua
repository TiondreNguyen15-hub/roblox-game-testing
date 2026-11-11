dofile('tools/roblox_mock.lua')
-- Mathematical camera test double, not Roblox physics or rendered animation.
local vmeta=getmetatable(Vector3.new());local oldIndex=vmeta.__index
vmeta.__index=function(v,k)
 if k=='Lerp'then return function(a,b,t)return a+(b-a)*t end end
 if k=='Dot'then return function(a,b)return a.X*b.X+a.Y*b.Y+a.Z*b.Z end end
 return oldIndex(v,k)
end
Vector2.new=function(x,y)return setmetatable({X=x or 0,Y=y or 0},{__index=function(v,k)if k=='Magnitude'then return math.sqrt(v.X*v.X+v.Y*v.Y)end end})end
local cf={}
local function frame(pos,m)return setmetatable({Position=pos,M=m or {1,0,0,0,1,0,0,0,1}},cf)end
local function rotate(m,v)return Vector3.new(m[1]*v.X+m[2]*v.Y+m[3]*v.Z,m[4]*v.X+m[5]*v.Y+m[6]*v.Z,m[7]*v.X+m[8]*v.Y+m[9]*v.Z)end
cf.__index=function(c,k)
 if k=='LookVector'then return Vector3.new(-c.M[3],-c.M[6],-c.M[9])end
 if k=='RightVector'then return Vector3.new(c.M[1],c.M[4],c.M[7])end
 if k=='VectorToWorldSpace'then return function(self,v)return rotate(self.M,v)end end
 if k=='ToOrientation'then return function(self)return 0,math.atan(self.M[3],self.M[1]),0 end end
 if k=='Lerp'then return function(a,b,t)return frame(a.Position:Lerp(b.Position,t),b.M)end end
end
cf.__mul=function(a,b)
 local m={};for r=0,2 do for c=0,2 do local n=0;for k=0,2 do n=n+a.M[r*3+k+1]*b.M[k*3+c+1]end;m[r*3+c+1]=n end end
 return frame(a.Position+rotate(a.M,b.Position),m)
end
CFrame.new=function(x,y,z)return frame(type(x)=='table'and x or Vector3.new(x,y,z))end
CFrame.Angles=function(x,y,z)
 local cx,sx,cy,sy=math.cos(x),math.sin(x),math.cos(y),math.sin(y)
 return frame(Vector3.new(),{cy,sy*sx,sy*cx,0,cx,-sx,-sy,cy*sx,cy*cx})
end
CFrame.lookAt=function(pos,target)
 local look=(target-pos).Unit;local yaw=math.atan(-look.X,-look.Z);local pitch=math.asin(look.Y)
 local c=CFrame.Angles(0,yaw,0)*CFrame.Angles(pitch,0,0);c.Position=pos;return c
end
Enum.RenderPriority={Camera={Value=200}}
local baseIndex=getmetatable(workspace).__index
getmetatable(workspace).__index=function(o,k)if o._p[k]~=nil then return o._p[k] end;return baseIndex(o,k)end
local p=newPlayer(77,'Mover');game:GetService('Players').LocalPlayer=p
p.GetAttributeChangedSignal=function(self,k)return self:GetPropertyChangedSignal('attr:'..k)end
local oldSet=p.SetAttribute;p.SetAttribute=function(self,k,v)local old=self:GetAttribute(k);oldSet(self,k,v);if old~=v then self:GetPropertyChangedSignal('attr:'..k):Fire()end end
local root=p.Character:FindFirstChild('HumanoidRootPart');root.CFrame=CFrame.new();root.Position=Vector3.new()
local hum=p.Character:FindFirstChildOfClass('Humanoid');hum.MoveDirection=Vector3.new(1,0,0)
local rs=game:GetService('ReplicatedStorage');local folder=Instance.new('Folder',rs);folder.Name='Blockline';local combat=Instance.new('RemoteEvent',folder);combat.Name='Combat';combat.Calls={}
function combat:FireServer(...)table.insert(self.Calls,{...})end
local run=game:GetService('RunService');function run:BindToRenderStep(n,priority,f)self.CameraStep=f end
p:SetAttribute('BLMenuOpen',false);p:SetAttribute('BLFOV',80)
assert(loadfile('Movement.luau'))();flushOne()
local cam=workspace.CurrentCamera;workspace.NextHit=nil
for i=1,60 do run.CameraStep(1/60)end
local focus=root.Position+Vector3.new(0,1.85,0)
assert((cam.CFrame.Position-focus).Magnitude>8,'Camera is not behind the character')
local before=cam.CFrame.LookVector
game:GetService('UserInputService').InputChanged:Fire({UserInputType=Enum.UserInputType.MouseMovement,Delta=Vector3.new(30,10,0)},false)
run.CameraStep(.1);assert((cam.CFrame.LookVector-before).Magnitude>.02,'Mouse aim did not change')
p:SetAttribute('BLFOV',100);run.CameraStep(1);assert(cam.FieldOfView>99)
p:SetAttribute('BLAiming',true);run.CameraStep(1);assert(cam.FieldOfView<81)
workspace.NextHit={Position=focus+Vector3.new(0,0,2)};run.CameraStep(.1);assert((cam.CFrame.Position-focus).Magnitude<2,'Obstruction did not pull camera inward')
local cas=game:GetService('ContextActionService');assert(cas.Actions.BLCrouch)
cas.Actions.BLCrouch('BLCrouch',Enum.UserInputState.Begin);assert(combat.Calls[#combat.Calls][1]=='Crouch'and combat.Calls[#combat.Calls][2]==true)
p:SetAttribute('BLMenuOpen',true);assert(not cas.Actions.BLCrouch and combat.Calls[#combat.Calls][2]==false)
local oldCamera=cam.CFrame;run.CameraStep(.1);assert(cam.CFrame==oldCamera,'Movement camera overrode menu')
print('PASS: shoulder-camera distance, mouse rotation, adjustable FOV/aim, camera obstruction, crouch input/touch binding, stance reset and menu-camera isolation (mathematical test doubles).')
