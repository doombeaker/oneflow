from __future__ import absolute_import

import oneflow.python.framework.compile_context as compile_context
import oneflow.python.framework.remote_blob as remote_blob_util
import oneflow.python.framework.id_util as id_util
import oneflow.core.operator.op_conf_pb2 as op_conf_util
import oneflow.core.register.logical_blob_id_pb2 as logical_blob_id_util

def InputOpByBlobDesc(blob_desc):
    op_conf = op_conf_util.OperatorConf()
    op_conf.name = id_util.UniqueStr('Input_')
    op_conf.input_conf.out = "out"
    op_conf.input_conf.blob_conf.CopyFrom(blob_desc.ToInterfaceBlobConf())
    compile_context.CurJobAddOp(op_conf)
    lbi = logical_blob_id_util.LogicalBlobId()
    lbi.op_name = op_conf.name
    lbi.blob_name = "out"
    return remote_blob_util.RemoteBlob(lbi)

def OutputOpByRemoteBlob(remote_blob):
    op_conf = op_conf_util.OperatorConf()
    op_conf.name = id_util.UniqueStr('Output_')
    setattr(op_conf.output_conf, 'in', remote_blob.lbn)
    op_conf.output_conf.out = "out"
    compile_context.CurJobAddOp(op_conf)
    lbi = logical_blob_id_util.LogicalBlobId()
    lbi.op_name = op_conf.name
    lbi.blob_name = "out"
    return remote_blob_util.RemoteBlob(lbi)
