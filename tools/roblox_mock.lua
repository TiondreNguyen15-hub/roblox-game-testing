ColorSequence={new=function(...)return {...}end}
table.clone=function(t)local r={};for k,v in pairs(t)do r[k]=v end;return r end
-- Small headless test double. This does not simulate Roblox rendering or physics.
local tick=100
os.clock=function() return tick end
os.time=function() return 1800000000+math.floor(tick) end
function advance(n) tick=tick+(n or 1) end
math.clamp=function(x,a,b) return math.min(b,math.max(a,x)) end
local function signal()
 return {Callbacks={},Connect=function(self,f) table.insert(self.Callbacks,f); return {Disconnect=function()end} end,Fire=function(self,...) for _,f in ipairs(self.Callbacks)do f(...)end end}
end
RaycastParams={new=function()return {}end}
local vec={}
vec.__index=function(t,k) if k=='Magnitude' then return math.sqrt(t.X*t.X+t.Y*t.Y+t.Z*t.Z) elseif k=='Unit' then return t/t.Magnitude end end
vec.__add=function(a,b)return Vector3.new(a.X+b.X,a.Y+b.Y,a.Z+b.Z)end
vec.__sub=function(a,b)return Vector3.new(a.X-b.X,a.Y-b.Y,a.Z-b.Z)end
vec.__mul=function(a,b)if type(a)=='number'then a,b=b,a end;return Vector3.new(a.X*b,a.Y*b,a.Z*b)end
vec.__div=function(a,b)return Vector3.new(a.X/b,a.Y/b,a.Z/b)end
Vector3={new=function(x,y,z)return setmetatable({X=x or 0,Y=y or 0,Z=z or 0},vec)end}
Vector2={new=function(x,y)return {X=x or 0,Y=y or 0}end}
local cf={};cf.__mul=function(a,b)return a end
CFrame={new=function(x,y,z)return setmetatable({Position=type(x)=='table' and x or Vector3.new(x,y,z)},cf)end,lookAt=function(a,b)return setmetatable({Position=a},cf)end,Angles=function()return CFrame.new()end}
Color3={fromRGB=function(r,g,b)return {R=r/255,G=g/255,B=b/255}end}
BrickColor={new=function(v)return v end}
UDim={new=function(a,b)return {Scale=a,Offset=b}end}
UDim2={new=function(...)return {...}end,fromScale=function(x,y)return {x,0,y,0}end,fromOffset=function(x,y)return {0,x,0,y}end}
TweenInfo={new=function(...)return {...}end};NumberSequence={new=function(v)return v end};NumberSequenceKeypoint={new=function(...)return {...}end}
Enum=setmetatable({},{__index=function(t,k)local n=setmetatable({},{__index=function(a,b)rawset(a,b,k..'.'..b);return a[b]end});rawset(t,k,n);return n end})
local methods={};local node={}
node.__index=function(o,k) return methods[k] or o._p[k] end
node.__newindex=function(o,k,v)
 if k=='Parent' then
  local old=o._p.Parent;if old then for i,c in ipairs(old._children)do if c==o then table.remove(old._children,i);break end end end
  if v then table.insert(v._children,o) end
 end
 o._p[k]=v
end
function methods:IsA(cls)return self.ClassName==cls or (cls=='GuiObject' and (self.ClassName:match('Frame')~=nil or self.ClassName=='TextLabel' or self.ClassName=='TextButton')) or (cls=='BasePart' and (self.ClassName=='Part' or self.ClassName=='SpawnLocation'))end
function methods:GetChildren()local t={};for i,v in ipairs(self._children)do t[i]=v end;return t end
function methods:GetDescendants()local r={};for _,v in ipairs(self._children)do table.insert(r,v);for _,d in ipairs(v:GetDescendants())do table.insert(r,d)end end;return r end
function methods:FindFirstChild(n,recursive)for _,v in ipairs(self._children)do if v.Name==n then return v end end;if recursive then for _,v in ipairs(self._children)do local r=v:FindFirstChild(n,true);if r then return r end end end end
function methods:WaitForChild(n) local r=self:FindFirstChild(n);assert(r,'Missing mock child '..n..' in '..self.Name);return r end
function methods:FindFirstChildOfClass(cls)for _,v in ipairs(self._children)do if v:IsA(cls)then return v end end end
function methods:FindFirstAncestorOfClass(cls)local p=self.Parent;while p do if p:IsA(cls)then return p end;p=p.Parent end end
function methods:SetAttribute(n,v)self._attr[n]=v end
function methods:GetAttribute(n)return self._attr[n]end
function methods:Destroy()for _,c in ipairs(self:GetChildren())do c:Destroy()end;self.Parent=nil end
function methods:GetPropertyChangedSignal(k)self._signals[k]=self._signals[k] or signal();return self._signals[k]end
function methods:PivotTo(c)self.Pivot=c;local r=self:FindFirstChild('HumanoidRootPart');if r then local delta=c.Position-r.Position;for _,v in ipairs(self:GetDescendants())do if v:IsA('BasePart')then v.Position=v.Position+delta end end end end
function methods:EquipTool(t)t.Parent=self.Parent end
function methods:TakeDamage(n)self.Health=math.max(0,self.Health-n);if self.Health==0 then self.Died:Fire()end end
function methods:FireClient(p,kind,payload)p._messages=p._messages or {};table.insert(p._messages,{kind,payload})end
function methods:LoadCharacter()self.CharacterAdded:Fire(self.Character)end
function methods:Kick(m)self.Kicked=m end
function methods:GetPlayers()return self:GetChildren()end
function methods:GetPlayerFromCharacter(c)for _,p in ipairs(self:GetPlayers())do if p.Character==c then return p end end end
function methods:BindToClose(f)self.Close=f end
function methods:Clone()
 local o=Instance.new(self.ClassName);for k,v in pairs(self._p)do if k~='Parent' and type(v)~='function'then o[k]=v end end
 for _,v in ipairs(self:GetChildren())do local c=v:Clone();c.Parent=o end;return o
end
Instance={new=function(cls,parent)
 local o=setmetatable({_p={ClassName=cls,Name=cls,Health=100,MaxHealth=100,HipHeight=2,Archivable=true},_children={},_attr={},_signals={}},node)
 for _,n in ipairs({'Died','CharacterAdded','OnServerEvent','OnClientEvent','MouseEnter','MouseLeave','SelectionGained','SelectionLost','Activated','PlayerAdded','PlayerRemoving','WindowFocusReleased','GamepadConnected','Heartbeat','InputChanged','RenderStepped'})do o[n]=signal()end
 o.Parent=parent;return o
end}
function typeof(v)if getmetatable(v)==vec then return 'Vector3' end;return type(v)end
local scheduled={};task={spawn=function(f,...)table.insert(scheduled,{f,{...}})end,delay=function(t,f,...)table.insert(scheduled,{f,{...}})end,wait=function(n)advance(n or .1)end}
function flushOne()local t=table.remove(scheduled,1);if t then t[1](table.unpack(t[2]))end end
local function deep(t)if type(t)~='table'then return t end;local r={};for k,v in pairs(t)do r[k]=deep(v)end;return r end
FakeStore={Data={},Fail=false}
function FakeStore:UpdateAsync(k,f)if self.Fail then error('Simulated DataStore failure')end;local n=f(deep(self.Data[k]));if n then self.Data[k]=deep(n)end;return n and deep(n)end
local services={}
game=Instance.new('DataModel');game.JobId='mock-session'
function game:GetService(n)if not services[n]then services[n]=Instance.new(n);services[n].Name=n end;return services[n]end
workspace=Instance.new('Workspace');workspace.Name='Workspace'
workspace.CurrentCamera=Instance.new('Camera',workspace);workspace.CurrentCamera.ViewportSize=Vector2.new(1440,900);workspace.CurrentCamera.FieldOfView=80
function workspace:Raycast()return self.NextHit end
local d=game:GetService('DataStoreService');function d:GetDataStore()return FakeStore end
local h=game:GetService('HttpService');function h:GenerateGUID()return 'mock-guid'end;function h:JSONEncode(v)return deep(v)end;function h:JSONDecode(v)return deep(v)end
local tw=game:GetService('TweenService');function tw:Create(o,info,props)return {Play=function()for k,v in pairs(props)do o[k]=v end end}end
local cas=game:GetService('ContextActionService');cas.Actions={};function cas:BindAction(n,f,...)self.Actions[n]=f end;function cas:UnbindAction(n)self.Actions[n]=nil end;function cas:SetTitle()end;function cas:SetPosition()end
local ds=game:GetService('Debris');function ds:AddItem()end
local content=game:GetService('ContentProvider');function content:PreloadAsync(assets,f)for _,v in ipairs(assets)do f(v,Enum.AssetFetchStatus.Success)end end
local arena=Instance.new('Model',workspace);arena.Name='BlocklineArena'
function newPlayer(id,name)
 local p=Instance.new('Player',game:GetService('Players'));p.Name=name;p.UserId=id
 local backpack=Instance.new('Backpack',p);backpack.Name='Backpack'
 local pg=Instance.new('PlayerGui',p);pg.Name='PlayerGui'
 local char=Instance.new('Model',workspace);char.Name=name;p.Character=char
 local hum=Instance.new('Humanoid',char);hum.Name='Humanoid'
 for _,n in ipairs({'HumanoidRootPart','Head'})do local part=Instance.new('Part',char);part.Name=n;part.Position=Vector3.new()end
 return p
end
warn=function(...)end
return {deep=deep}
