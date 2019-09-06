from __future__ import absolute_import

import oneflow as flow
import oneflow.core.operator.op_conf_pb2 as op_conf_util
import oneflow.core.register.logical_blob_id_pb2 as logical_blob_id_util
import oneflow.python.framework.id_util as id_util
import oneflow.python.framework.compile_context as compile_context
import oneflow.python.framework.remote_blob as remote_blob_util
from oneflow.python.oneflow_export import oneflow_export


@oneflow_export("layers.dense")
def dense(
    inputs,
    units,
    activation=None,
    use_bias=True,
    kernel_initializer=None,
    bias_initializer=None,
    trainable=True,
    name=None,
):
    in_shape = inputs.static_shape
    in_num_axes = len(in_shape)
    assert in_num_axes >= 2

    name_prefix = name if name is not None else id_util.UniqueStr("Dense_")
    inputs = flow.reshape(inputs, (-1, in_shape[-1])) if in_num_axes > 2 else inputs
    weight = flow.get_variable(
        name="{}-weight".format(name_prefix),
        shape=(units, inputs.static_shape[1]),
        dtype=inputs.dtype,
        initializer=(
            kernel_initializer
            if kernel_initializer is not None
            else flow.constant_initializer(0)
        ),
        trainable=trainable,
        model_name="weight",
        split_axis=None,
    )
    out = flow.matmul(
        a=inputs, b=weight, transpose_b=True, name="{}_matmul".format(name_prefix)
    )
    if use_bias:
        bias = flow.get_variable(
            name="{}-bias".format(name_prefix),
            shape=(units,),
            dtype=inputs.dtype,
            initializer=(
                bias_initializer
                if bias_initializer is not None
                else flow.constant_initializer(0)
            ),
            trainable=trainable,
            model_name="bias",
            split_axis=None,
        )
        out = flow.nn.bias_add(out, bias, name="{}_bias_add".format(name_prefix))
    out = activation(out) if activation is not None else out
    out = flow.reshape(out, in_shape[:-1] + (units,)) if in_num_axes > 2 else out

    return out


@oneflow_export("layers.layer_norm")
def layer_norm(
    inputs,
    center=True,
    scale=True,
    trainable=True,
    begin_norm_axis=1,
    begin_params_axis=-1,
    name=None,
):
    op_conf = op_conf_util.OperatorConf()
    setattr(
        op_conf, "name", name if name is not None else id_util.UniqueStr("LayerNorm_")
    )
    setattr(op_conf, "trainable", trainable)
    setattr(op_conf.layer_norm_conf, "in", inputs.logical_blob_name)
    setattr(op_conf.layer_norm_conf, "out", "out")
    setattr(op_conf.layer_norm_conf, "center", center)
    setattr(op_conf.layer_norm_conf, "scale", scale)
    setattr(op_conf.layer_norm_conf, "begin_norm_axis", begin_norm_axis)
    setattr(op_conf.layer_norm_conf, "begin_params_axis", begin_params_axis)
    compile_context.CurJobAddOp(op_conf)
    out_lbi = logical_blob_id_util.LogicalBlobId()
    setattr(out_lbi, "op_name", op_conf.name)
    setattr(out_lbi, "blob_name", "out")
    return remote_blob_util.RemoteBlob(out_lbi)