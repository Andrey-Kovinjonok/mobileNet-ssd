#ifndef MOBILENET_H
#define MOBILENET_H

#include <iostream>
#include <vector>
#include <string>
#include "mobilenetconfig.h"
#include <tensorflow/cc/client/client_session.h>
#include <tensorflow/cc/ops/standard_ops.h>
#include <tensorflow/core/platform/net.h>
#include <tensorflow/cc/training/queue_runner.h>

namespace ssd {

using namespace tensorflow;
using namespace tensorflow::ops;


class mobileNet
{
public:
  mobileNet(const Scope &scope);
  ~mobileNet();
public:
  /*
   *  Preprocesses a numpy array encoding a batch of images.
   * Arguments:
   *      x: a 4D numpy array consists of RGB values within [0, 255].
   * Returns:
   *      Preprocessed array.
   * */
  mobileNet* preprocessInput();
  /*
   * Add an initial convolution layer(with batch normalization and relu6)
   * Arguments:
   *    inputs:Input tensor of shape(rows,cols,3)or(3,rows,cols)
   *    It should have exactly 3 inputs channels and width and height should be no smaller than 32
   *    filters:Integer,the dimensionality of the output space
   *    alpha: controls the width of the network.
   *    kernel: An integer or tuple/list of 2 integers.
   *    strides: An integer or tuple/list of 2 integers.
   *
   * Input shape:
   *    4D tensor with shape:
   *     (samples,channels,rows,cols)or(samples,rows,cols,channels)
   * Output shape:
   *    tensor of block.
   * */
  mobileNet* convBlock(Tensor inputs, int filters, float alpha, std::vector<int> kernel = {3,3}, std::vector<int> strides ={1,1});
  /*
   * Adds a depthwise convolution block
   *  Arguments:
   *      inputs: Input tensor of shape(rows,cols,channels)or(....)
   *      pointwise_conv_filters: Integer output size
   *      alpha:  controls the width of the network
   *      depth_multiplier: The number of depthwise convolution output channels for eath input channel
   *      strides:  An integer or tuple/list of 2 integers.
   *  Input shape:
   *    4D tensor with shape:
   *     (samples,channels,rows,cols)or(samples,rows,cols,channels)
   *  Output shape:
   *    tensor of block.
   * */
  mobileNet* depthwiseConvBlock(Tensor inputs,int pointwise_conv_filters,float alpha,int depth_multiplier=1,std::vector<int> strides = {1,1},int block_id =1);

  mobileNet* relu6(Tensor x, std::string name);

  mobileNet* BatchNorm(Input inputs);

  void network(float alpha =1.0,int depth_multiplier=1,
               float dropout =0.001,bool include_top =true,std::string weights= "imagenet",
               int classes = 1000,std::string pooling="none");

public:
  tensorflow::Scope scope;
  tensorflow::ClientSession session;
  std::vector<Tensor> BN_output;
  Output tensor;
  Tensor relu_output;
  Tensor convBlock_output;
  //weight biases
  Output weight;
  Output biases;
  bool initialized,initialized_depth;

  mobileNetConfig params;
};
}//namespace ssd

#endif
