"""Build a map-only Roblox XML place with no gameplay scripts. Python stdlib only."""
from pathlib import Path
import xml.etree.ElementTree as E
import math, json, argparse
parser=argparse.ArgumentParser(); parser.add_argument('--output',default='output'); args=parser.parse_args()
OUT=Path(args.output); OUT.mkdir(parents=True,exist_ok=True)
root=E.Element('roblox',version='4'); E.SubElement(root,'External').text='null'; E.SubElement(root,'External').text='nil'
serial=0; geometry=[]; obstacles=[]
def prop(p,t,n,v): E.SubElement(p,t,name=n).text=str(v)
def obj(parent,cls,name):
 global serial
 serial+=1; o=E.SubElement(parent,'Item',{'class':cls,'referent':f'RBX{serial}'}); p=E.SubElement(o,'Properties'); prop(p,'string','Name',name); return o,p
def vec(p,n,v):
 e=E.SubElement(p,'Vector3',name=n)
 for k,x in zip('XYZ',v): E.SubElement(e,k).text=str(x)
def rgb(p,n,c):
 e=E.SubElement(p,'Color3',name=n)
 for k,x in zip('RGB',c): E.SubElement(e,k).text=str(x/255)
def frame(p,pos):
 e=E.SubElement(p,'CoordinateFrame',name='CFrame')
 for k,x in zip('XYZ',pos): E.SubElement(e,k).text=str(x)
 for r in range(3):
  for c in range(3): E.SubElement(e,f'R{r}{c}').text=str(int(r==c))
 return e
def part(parent,name,size,pos,color,collide=True,material=272,cls='Part'):
 o,p=obj(parent,cls,name); vec(p,'size',size); frame(p,pos)
 prop(p,'Color3uint8','Color3uint8',(255<<24)+(color[0]<<16)+(color[1]<<8)+color[2])
 for n,v in [('Anchored',True),('CanCollide',collide),('CanTouch',collide),('CanQuery',collide)]: prop(p,'bool',n,str(v).lower())
 prop(p,'token','Material',material); prop(p,'token','TopSurface',0); prop(p,'token','BottomSurface',0)
 geometry.append(dict(name=name,size=size,pos=pos,collide=collide))
 return o,p
w,wp=obj(root,'Workspace','Workspace'); prop(wp,'float','FallenPartsDestroyHeight',-80)
arena,_=obj(w,'Model','BlocklineArena')
ground,_=obj(arena,'Folder','01 Floor and boundaries'); covers,_=obj(arena,'Folder','02 Black cover with red edges'); spawns,_=obj(arena,'Folder','03 Symmetrical spawns')
part(ground,'Arena floor - 320 x 200',(320,8,200),(0,-4,0),(48,50,54),material=816)
for z in [-102,102]: part(ground,'End boundary',(328,32,4),(0,12,z),(28,29,33),material=816)
for x in [-162,162]: part(ground,'Side boundary',(4,32,200),(x,12,0),(28,29,33),material=816)
def cover(name,x,z,sx,sy,sz):
 model,_=obj(covers,'Model',name); y=sy/2
 body,_=part(model,'Solid black collision block',(sx,sy,sz),(x,y,z),(9,10,13))
 obstacles.append(dict(name=name,size=(sx,sy,sz),pos=(x,y,z)))
 # Twelve real neon edge strips. No overlay that reveals occluded cover.
 t=.22
 for a in [-1,1]:
  for b in [-1,1]:
   part(model,'Red horizontal X edge',(sx+t,t,t),(x,y+a*sy/2,z+b*sz/2),(255,18,36),False,288)
   part(model,'Red vertical edge',(t,sy+t,t),(x+a*sx/2,y,z+b*sz/2),(255,18,36),False,288)
   part(model,'Red horizontal Z edge',(t,t,sz+t),(x+a*sx/2,y+b*sy/2,z),(255,18,36),False,288)
for sx in [-1,1]:
 for sz in [-1,1]:
  for label,x,z,dim in [
   ('Base shield',114,56,(16,18,20)),
   ('Flank tower',72,76,(28,14,12)),
   ('Mid wall',70,23,(18,9,10)),
   ('Central monolith',24,23,(28,24,12)),
   ('Low flank block',30,68,(10,5,10)),
   ('Small outer block',132,20,(8,6,8)),
  ]: cover(f'{label} {sx:+d} {sz:+d}',sx*x,sz*z,*dim)
# Neutral spawn pads enable default Roblox movement testing without team scripts.
for x in [-143,143]:
 for z in [-54,54]:
  s,p=part(spawns,f'Spawn {x} {z}',(12,.4,12),(x,.2,z),(71,74,80),cls='SpawnLocation')
  prop(p,'bool','Neutral','true'); prop(p,'float','Duration',0); prop(p,'bool','AllowTeamChangeOnTouch','false')
  prop(p,'bool','Enabled','true')
terrain,_=obj(w,'Terrain','Terrain'); clouds,clp=obj(terrain,'Clouds','Arena clouds')
prop(clp,'float','Cover',.35); prop(clp,'float','Density',.6); rgb(clp,'Color',(245,248,255)); prop(clp,'bool','Enabled','true')
light,lp=obj(root,'Lighting','Lighting'); prop(lp,'float','ClockTime',13.5); prop(lp,'float','Brightness',2.5)
rgb(lp,'Ambient',(110,115,130)); rgb(lp,'OutdoorAmbient',(145,150,165))
sky,skp=obj(light,'Sky','Blue sky'); prop(skp,'bool','CelestialBodiesShown','true')
# Built-in Roblox sky textures; no external or uploaded assets needed.
for name,face in [('SkyboxBk','bk'),('SkyboxDn','dn'),('SkyboxFt','ft'),('SkyboxLf','lf'),('SkyboxRt','rt'),('SkyboxUp','up')]:
 e=E.SubElement(skp,'Content',name=name); E.SubElement(e,'url').text=f'rbxasset://textures/sky/sky512_{face}.tex'
cam,cp=obj(w,'Camera','Camera'); pos=(245,245,285); n=frame(cp,pos); prop(cp,'token','CameraType',5); prop(wp,'Ref','CurrentCamera',cam.get('referent'))
length=math.sqrt(sum(v*v for v in pos)); back=[v/length for v in pos]; right=[back[2],0,-back[0]]; rl=math.sqrt(sum(v*v for v in right)); right=[v/rl for v in right]
up=[back[1]*right[2]-back[2]*right[1],back[2]*right[0]-back[0]*right[2],back[0]*right[1]-back[1]*right[0]]
for r in range(3):
 for c,axis in enumerate([right,up,back]): n.find(f'R{r}{c}').text=str(axis[r])
# Default third-person avatar movement only. No guns, HUD, teams, or round scripts.
st,p=obj(root,'StarterPlayer','StarterPlayer'); prop(p,'token','CameraMode',0); prop(p,'float','CharacterWalkSpeed',16); prop(p,'float','CharacterJumpPower',50)
for name in ['ServerScriptService','ReplicatedStorage','StarterPack']: obj(root,name,name)
E.ElementTree(root).write(OUT/'Blockline.rbxlx',encoding='utf-8',xml_declaration=True)
r=E.Element('roblox',version='4'); r.append(arena); E.ElementTree(r).write(OUT/'BlocklineArena.rbxmx',encoding='utf-8',xml_declaration=True)
(OUT/'arena-geometry.json').write_text(json.dumps(dict(parts=geometry,obstacles=obstacles),indent=2))
# Honest plan-view preview, not a Studio screenshot.
svg=['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1000 710">','<rect width="1000" height="710" fill="#101115"/>','<text x="40" y="46" fill="white" font-size="25" font-family="sans-serif">BLOCKLINE / PHASE 1 ARENA</text>','<text x="40" y="75" fill="#a4a7b0" font-size="14" font-family="sans-serif">320 × 200 studs · plan view · 24 outlined cover structures · 4 mirrored spawns</text>','<g transform="translate(500 385) scale(2.8)">','<rect x="-162" y="-102" width="324" height="204" fill="#303236" stroke="#737780" stroke-width="3"/>']
for z in [-43,0,43]: svg.append(f'<path d="M -135 {z} H 135" stroke="#6b6f79" stroke-width=".6" stroke-dasharray="3 3"/>')
for o in obstacles:
 x,_,z=o['pos']; sx,sy,sz=o['size']; svg.append(f'<rect x="{x-sx/2}" y="{z-sz/2}" width="{sx}" height="{sz}" fill="#090a0d" stroke="#ff1224" stroke-width=".85"/><text x="{x}" y="{z+1.5}" text-anchor="middle" fill="#d1d3d7" font-size="3.5" font-family="sans-serif">{sy}h</text>')
for x in [-143,143]:
 for z in [-54,54]: svg.append(f'<rect x="{x-6}" y="{z-6}" width="12" height="12" fill="#747c88"/><text x="{x}" y="{z+1.4}" text-anchor="middle" fill="white" font-size="3.8" font-family="sans-serif">SP</text>')
svg+=['</g>','<text x="45" y="695" fill="#a4a7b0" font-size="13" font-family="sans-serif">Dashed guides show open shooting lanes. Labels show cover height in studs. All red edge strips are non-collidable.</text>','</svg>']
(OUT/'Arena-Overview.svg').write_text(''.join(svg))
print(f'Built map-only place: {len(obstacles)} covers, {len(geometry)} parts, 4 spawns, sky and clouds; no scripts.')
