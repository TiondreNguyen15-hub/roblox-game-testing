"""Generate the deterministic, untrained ONNX validation model without Python packages.
The protobuf wire encoding is standard ONNX ModelProto, IR 8, opset 13.
Reference weights and inputs are specified in compute_common.cpp.
"""
from pathlib import Path
import struct, hashlib

def varint(v):
    out = bytearray()
    while v > 127:
        out.append((v & 127) | 128); v >>= 7
    out.append(v)
    return bytes(out)

def integer(field, v): return varint(field << 3) + varint(v)
def data(field, b):
    if isinstance(b, str): b = b.encode()
    return varint((field << 3) | 2) + varint(len(b)) + b

def info(name, shape):
    dims = b''.join(data(1, integer(1, d)) for d in shape)
    tensor_type = integer(1, 1) + data(2, dims)
    return data(1, name) + data(2, data(1, tensor_type))
weights = b''.join(struct.pack('<f', ((i % 13) - 6) * .03125) for i in range(256 * 256))
tensor = integer(1, 256) + integer(1, 256) + integer(2, 1) + data(8, 'weights') + data(9, weights)
matmul = data(1, 'input') + data(1, 'weights') + data(2, 'product') + data(3, 'validation_matmul') + data(4, 'MatMul')
relu = data(1, 'product') + data(2, 'output') + data(3, 'validation_relu') + data(4, 'Relu')
graph = data(1, matmul) + data(1, relu) + data(2, 'ApexSyntheticComputeValidation') + data(5, tensor) + data(11, info('input', [128, 256])) + data(12, info('output', [128, 256]))
model = integer(1, 8) + data(2, 'Apex standalone compute test') + data(7, graph) + data(8, integer(2, 13))
path = Path(__file__).resolve().parent.parent / 'models' / 'compute_validation.onnx'
path.parent.mkdir(exist_ok=True)
path.write_bytes(model)
print(path.name, len(model), hashlib.sha256(model).hexdigest())
