#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <mobileNet/mobilenetconfig.h>
#include <unsupported/Eigen/CXX11/Tensor>

namespace ssd {

typedef Eigen::Tensor<float,1,Eigen::RowMajor> Tensor1f;
typedef Eigen::Tensor<float,2,Eigen::RowMajor> Tensor2f;
typedef Eigen::Tensor<float,3,Eigen::RowMajor> Tensor3f;
typedef Eigen::Tensor<float,4,Eigen::RowMajor> Tensor4f;

//struct about anchors params
struct anchor_struct
{
    Tensor1f anchor_height,anchor_width;
    Tensor3f anchor_x,anchor_y;
};

using namespace std;

class ssd
{
public:
    ssd();
    ~ssd();
    void network();


public:
    /*Computer anchor boxes for all feature layers*/
    vector<anchor_struct> ssd_anchors_all_layers(int image_shape, int feat_shape, vector<vector<float> > anchor_sizes, vector<vector<float> > anchor_ratios, vector<int> anchor_step, float offset);
    /*Computer SSD default anchor boxes for one feature layers*/
    anchor_struct ssd_anchor_one_layers(int image_shape, int feat_shape,
                                                 vector<float> sizes, vector<float> ratios,
                                                 int step, float offset);

public:
    ssdParams params;
    std::vector<std::vector<float> > layers_anchors;

    struct return_result
    {
        Tensor1f oneF;
        Tensor2f twoF;
        Tensor3f threeF;
    }inline_test;
};

}//namespace ssd