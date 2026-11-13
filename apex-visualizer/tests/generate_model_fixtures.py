"""Original tiny FP32 dynamic ONNX graph for selected-model regression tests."""
from pathlib import Path
import json

def varint(v):
    out=bytearray()
    while v>127:out.append((v&127)|128);v>>=7
    out.append(v);return bytes(out)
def integer(f,v):return varint(f<<3)+varint(v)
def data(f,b):
    if isinstance(b,str):b=b.encode()
    return varint((f<<3)|2)+varint(len(b))+b

def info(name):
    dims=data(1,data(2,'batch'))+data(1,integer(1,8))
    return data(1,name)+data(2,data(1,integer(1,1)+data(2,dims)))
node=data(1,'features')+data(2,'result')+data(3,'test_relu')+data(4,'Relu')
graph=data(1,node)+data(2,'OriginalDynamicShapeRegression')+data(11,info('features'))+data(12,info('result'))
folder=Path(__file__).parent/'fixtures'
(folder/'dynamic-relu.onnx').write_bytes(integer(1,8)+data(2,'Authorized test')+data(7,graph)+data(8,integer(2,13)))
(folder/'dynamic-shapes.json').write_text(json.dumps({'features':[16,8]}))
