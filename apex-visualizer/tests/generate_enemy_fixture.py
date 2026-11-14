"""Original single-class contract test graph; NOT a trained enemy detector."""
import onnx,numpy as np
from onnx import helper as h,TensorProto as t,numpy_helper as nh
from pathlib import Path
boxes=np.zeros((1,300,6),np.float32);boxes[0,0]=[104,104,312,312,.8,0]
mask=np.zeros_like(boxes);mask[0,0,4]=.1
nodes=[h.make_node('ReduceMean',['image'],['mean'],keepdims=0),h.make_node('Mul',['mean','mask'],['delta']),h.make_node('Add',['base','delta'],['detections'])]
g=h.make_graph(nodes,'SYNTHETIC_PIXEL_DEPENDENT_CONTRACT_TEST',[h.make_tensor_value_info('image',t.FLOAT,[1,3,416,416])],[h.make_tensor_value_info('detections',t.FLOAT,[1,300,6])],[nh.from_array(boxes,'base'),nh.from_array(mask,'mask')])
m=h.make_model(g,opset_imports=[h.make_opsetid('',20)],ir_version=9);h.set_model_props(m,{'names':"{0: 'synthetic test only'}"});onnx.checker.check_model(m);onnx.save(m,str(Path(__file__).parent/'fixtures/enemy-contract-test.onnx'))
