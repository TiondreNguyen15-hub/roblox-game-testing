"""Static geometry checks; not a Roblox physics or Studio playtest."""
from pathlib import Path
import json,collections,math,xml.etree.ElementTree as E
p=Path('output'); root=E.parse(p/'Arena-Phase1.rbxlx').getroot(); data=json.loads((p/'arena-geometry.json').read_text()); obstacles=data['obstacles']; parts=data['parts']
items=root.findall('.//Item'); refs=[i.get('referent') for i in items]; assert len(refs)==len(set(refs))
assert not any(i.get('class') in ['Script','LocalScript','ModuleScript','Tool'] for i in items)
assert len(obstacles)==24
for o in obstacles:
 x,y,z=o['pos']; size=o['size']
 for signx,signz in [(-1,1),(1,-1),(-1,-1)]: assert any(a['pos']==[signx*x,y,signz*z] and a['size']==size for a in obstacles)
assert {o['size'][1] for o in obstacles}=={5,6,9,14,18,24}
assert len([i for i in items if i.get('class')=='SpawnLocation'])==4
assert len([i for i in items if i.get('class')=='Clouds'])==1
assert len([i for i in items if i.get('class')=='Sky'])==1
for o in items:
 if o.get('class') not in ['Part','SpawnLocation']: continue
 props=o.find('Properties'); assert props.find("bool[@name='Anchored']").text=='true'
 if props.find("string[@name='Name']").text.startswith('Red '):
  for prop in ['CanCollide','CanTouch','CanQuery']: assert props.find(f"bool[@name='{prop}']").text=='false'
  assert props.find("token[@name='Material']").text=='288'
models=[i for i in items if i.get('class')=='Model' and i.find("Properties/string[@name='Name']").text!='BlocklineArena']
assert len(models)==24
for m in models:
 bodies=[i for i in m.findall('Item') if i.get('class')=='Part']; assert len(bodies)==13
 assert sum(i.find("Properties/bool[@name='CanCollide']").text=='true' for i in bodies)==1
# Standing capsule's horizontal footprint: conservative 2-stud radius.
def free(x,z,radius=2):
 if abs(x)>160-radius or abs(z)>100-radius: return False
 for o in obstacles:
  a,_,b=o['pos']; sx,sy,sz=o['size']; dx=max(abs(x-a)-sx/2,0); dz=max(abs(z-b)-sz/2,0)
  if dx*dx+dz*dz<radius*radius: return False
 return True
def swept(a,b):
 distance=math.dist(a,b); steps=max(1,math.ceil(distance/.5))
 return all(free(a[0]+(b[0]-a[0])*i/steps,a[1]+(b[1]-a[1])*i/steps) for i in range(steps+1))
for z in [-43,0,43]: assert swept((-138,z),(138,z)),f'Lane {z} obstructed'
start=(-144,-54); visited={start}; q=collections.deque([start])
while q:
 x,z=q.popleft()
 for n in [(x-2,z),(x+2,z),(x,z-2),(x,z+2)]:
  if n not in visited and swept((x,z),n): visited.add(n);q.append(n)
for x in [-144,144]:
 for z in [-54,54]: assert (x,z) in visited
for x in range(-136,137,2):
 for z in [-42,0,42]: assert (x,z) in visited
# Each solid cover blocks a straight segment through its body.
for o in obstacles:
 x,y,z=o['pos']; sx,sy,sz=o['size']; assert not swept((x-sx,z),(x+sx,z))
# Floor reaches walls without a gap; boundaries extend from y=-4 to 28.
floor=next(a for a in parts if a['name'].startswith('Arena floor'))
assert floor['pos']==[0,-4,0] and floor['size']==[320,8,200]
assert len([a for a in parts if 'boundary' in a['name']])==4
print(f'PASS: mirrored layout, 24 solid covers, 288 non-colliding red edge strips, four spawns, sky, clouds, no gameplay scripts.')
print(f'PASS: capsule-clear movement graph ({len(visited)} nodes) connects every spawn and three shooting lanes.')
print('PASS: swept paths collide with all cover bodies; continuous floor and four anchored boundary walls.')
print('LIMIT: static geometry only. Roblox Studio player movement, jumping, camera and physics require a live playtest.')
