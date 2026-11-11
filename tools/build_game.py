"""Package map-only and integrated ScreenGui editions. Run from repository root."""
from pathlib import Path
import runpy,xml.etree.ElementTree as E,shutil
runpy.run_path('tools/build_arena.py',run_name='__main__')
out=Path('output')
shutil.copyfile(out/'Blockline.rbxlx',out/'Arena-Phase1.rbxlx')
root=E.parse(out/'Blockline.rbxlx').getroot()
serial=10000

def item(parent,cls,name):
 global serial
 serial+=1
 o=E.SubElement(parent,'Item',{'class':cls,'referent':f'RBX{serial}'})
 p=E.SubElement(o,'Properties'); E.SubElement(p,'string',name='Name').text=name
 return o,p
server=next(o for o in root.findall('Item') if o.get('class')=='ServerScriptService')
o,p=item(server,'Script','BlocklineServer')
E.SubElement(p,'ProtectedString',name='Source').text=Path('Server.luau').read_text(); E.SubElement(p,'bool',name='Disabled').text='false'
starter=next(o for o in root.findall('Item') if o.get('class')=='StarterPlayer')
sp,_=item(starter,'StarterPlayerScripts','StarterPlayerScripts')
o,p=item(sp,'LocalScript','BlocklineMenu')
E.SubElement(p,'ProtectedString',name='Source').text=Path('Client.luau').read_text(); E.SubElement(p,'bool',name='Disabled').text='false'
o,p=item(sp,'LocalScript','BlocklineMovement')
E.SubElement(p,'ProtectedString',name='Source').text=Path('Movement.luau').read_text(); E.SubElement(p,'bool',name='Disabled').text='false'
first,_=item(root,'ReplicatedFirst','ReplicatedFirst')
o,p=item(first,'ModuleScript','BlocklineLoading')
E.SubElement(p,'ProtectedString',name='Source').text=Path('LoadingScreen.luau').read_text()
o,p=item(first,'LocalScript','BlocklineLoadingBoot')
E.SubElement(p,'ProtectedString',name='Source').text=Path('LoadingBoot.luau').read_text(); E.SubElement(p,'bool',name='Disabled').text='false'
E.ElementTree(root).write(out/'Blockline.rbxlx',encoding='utf-8',xml_declaration=True)
for name in ['Server','Client']: shutil.copyfile(name+'.luau',out/(name+'.luau'))
print('Built integrated place with organized server + ScreenGui LocalScript and separate map-only edition.')

shutil.copyfile('Movement.luau',out/'Movement.luau')

shutil.copyfile('LoadingScreen.luau',out/'LoadingScreen.luau')
shutil.copyfile('LoadingBoot.luau',out/'LoadingBoot.luau')
