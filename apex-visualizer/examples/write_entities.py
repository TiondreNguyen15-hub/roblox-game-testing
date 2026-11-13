"""Sample data producer: only generates its own simulated player records."""
import argparse,json,math,os,tempfile,time
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,default=Path('apex_entities.json'));args=parser.parse_args()
args.output.parent.mkdir(parents=True,exist_ok=True)
print('Writing simulated snapshots to',args.output.resolve(),'(Ctrl+C to stop)')
start=time.monotonic()
try:
 while True:
  t=time.monotonic()-start
  snapshot={'players':[{'name':'Player 1','health':100,'shield':100,'distance':45,'x':500+math.sin(t)*100,'y':300,'visible':True},{'name':'Player 2','health':75,'shield':50,'distance':90,'x':800+math.sin(t*.7)*100,'y':400,'visible':True}]}
  fd,name=tempfile.mkstemp(dir=args.output.parent,suffix='.tmp')
  try:
   with os.fdopen(fd,'w',encoding='utf-8')as f:json.dump(snapshot,f);f.flush();os.fsync(f.fileno())
   for attempt in range(10):
    try:os.replace(name,args.output);break
    except PermissionError:
     if attempt==9:raise
     time.sleep(.02)
  finally:
   if os.path.exists(name):os.unlink(name)
  time.sleep(.1)
except KeyboardInterrupt:print('\nStopped.')
