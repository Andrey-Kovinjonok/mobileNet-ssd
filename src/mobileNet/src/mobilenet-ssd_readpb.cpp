#include <iostream>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>
#include <tensorflow/cc/ops/image_ops.h>
#include <string>
#include <vector>
#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

//opencv
#include <opencv2/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv/highgui.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std ;
using namespace tensorflow;
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

using namespace cv;
using namespace std;
using namespace tensorflow;
using namespace tensorflow::ops;


const size_t inWidth = 300;
const size_t inHeight = 300;
const float WHRatio = inWidth / (float)inHeight;
const vector<string > classNames = { "background",
                             "person",
                             "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
                             "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis",
                             "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
                             "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table",
                             "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase",
                             "scissors", "teddy bear", "hair drier", "toothbrush"};

string MODEL_PATH  = "/home/haosen/gitPro/catkin_new/data/ssd_mobilenet_v1_ppn_shared_box_predictor_300x300_coco14_sync_2018_07_03/frozen_inference_graph.pb";
string Image_path ="/home/haosen/catkin_ws/src/faster_rcnn_tf/data/demo/000456.jpg";

//从文件名中读取数据
Status ReadTensorFromImageFile(string file_name, const int input_height,
                               const int input_width,
                               vector<Tensor>* out_tensors) {
    auto root = Scope::NewRootScope();
    using namespace ops;

    auto file_reader = ops::ReadFile(root.WithOpName("file_reader"),file_name);
    const int wanted_channels = 3;
    Output image_reader;
    std::size_t found = file_name.find(".png");
    //判断文件格式
    if (found!=std::string::npos) {
        image_reader = DecodePng(root.WithOpName("png_reader"), file_reader,DecodePng::Channels(wanted_channels));
    }
    else {
        image_reader = DecodeJpeg(root.WithOpName("jpeg_reader"), file_reader,DecodeJpeg::Channels(wanted_channels));
    }
    // 下面几步是读取图片并处理
    auto float_caster =Cast(root.WithOpName("float_caster"), image_reader, DT_FLOAT);
    auto dims_expander = ExpandDims(root, float_caster, 0);
    auto resized = ResizeBilinear(root, dims_expander,Const(root.WithOpName("resize"), {input_height, input_width}));
    // Div(root.WithOpName(output_name), Sub(root, resized, {input_mean}),{input_std});
    Transpose(root.WithOpName("transpose"),resized,{0,2,1,3});

    GraphDef graph;
    root.ToGraphDef(&graph);

    unique_ptr<Session> session(NewSession(SessionOptions()));
    session->Create(graph);
    session->Run({}, {"transpose"}, {}, out_tensors);//Run，获取图片数据保存到Tensor中

    return Status::OK();
}


int Read_pb()
{
    GraphDef graph_def;
    Status status = ReadBinaryProto(Env::Default(),MODEL_PATH,&graph_def);
    if(!status.ok())
    {
        cout<<status.ToString()<<endl;
        return 1;
    }

    //新建session
    Session *session;
    status = NewSession(SessionOptions(),&session);
    status = session->Create(graph_def);
    if(!status.ok())
    {
        cout<<status.ToString()<<endl;
        return 1;
    }

    cout<<"tensorflow model load succeed"<<endl;
    //读取图像到inputs中
    int input_height = 300;
    int input_width = 300;
    vector<Tensor> inputs;
    if (!ReadTensorFromImageFile(Image_path, input_height, input_width,&inputs).ok()) {
        cout<<"Read image file failed"<<endl;
        return -1;
    }

    // 导入模型参数
    Tensor checkpointPathTensor(DT_STRING, TensorShape());
    checkpointPathTensor.scalar<std::string>()() = std::string("/home/haosen/gitPro/catkin_new/data/ssd_mobilenet_v1_ppn_shared_box_predictor_300x300_coco14_sync_2018_07_03/");

    for(int i=0;i<graph_def.node_size();i++)
    {
        std::cout<<graph_def.node().Get(i).name()<<std::endl;
    }
//    status = session->Run(
//    {{ graph_def.saver_def().filename_tensor_name(), checkpointPathTensor },},
//    {},
//    {graph_def.saver_def().restore_op_name()},
//    nullptr);
    if (!status.ok()) {
    throw runtime_error("Error loading checkpoint: " + status.ToString());
    }

    vector<Tensor> outputs;
    string input = "ToFloat";
    string test1 = "detection_boxes:0";
    string test2 = "num_detections:0";
    string output = "detection_classes";//graph中的输入节点和输出节点，需要预先知道
    string output1 = "detection_scores";

    pair<string,Tensor>img(input,inputs[0]);
    status = session->Run({img},{output,output1,test1,test2}, {}, &outputs);//Run,得到运行结果，存到outputs中
    if (!status.ok()) {
        cout<<"Running model failed"<<endl;
        cout<<status.ToString()<<endl;
        return -1;
    }
    //std::cout<<img.first<<img.second.tensor<float,4>()<<std::endl;

    //得到模型运行结果
    Tensor t = outputs[0];
    auto tmap = t.tensor<float, 2>();
    Tensor t1 = outputs[1];
    auto tmap1 = t1.tensor<float, 2>();
    auto tmap3 = outputs[3].tensor<float, 1>();
    int output_dim = t.shape().dim_size(1);
    std::cout<<"detection_classes:"<<outputs[0].DebugString()
            <<endl<<output_dim<<endl
            <<endl<<tmap<<endl
           <<endl<<outputs[1].DebugString()<<endl
          <<endl<<classNames[(tmap3(0)-1)]<<endl
            <<"detection_scores:"<<tmap1<<std::endl
              <<"detection_boxes:"<<endl<<outputs[2].DebugString()<<endl
           <<"detection_boxes:"<<endl<<outputs[2].tensor<float,3>()<<endl;

    Tensor boxes = Tensor(DT_FLOAT,{100,4});
    auto boxestemp = boxes.tensor<float,2>();
    for(int i=0;i<outputs[2].dim_size(1);i++)
    {
        for(int j=0;j<outputs[2].dim_size(2);j++)
        {
            boxestemp(i,j)=
            outputs[2].tensor<float,3>()(0,i,j);
        }
    }
    std::cout<<boxes.tensor<float,2>()<<std::endl;

    //show
    cv::Mat frame = cv::imread(Image_path);
    cv::Size frame_size = frame.size();

    cv::Size cropSize;
    if (frame_size.width / (float)frame_size.height > WHRatio)
    {
        cropSize = cv::Size(static_cast<int>(frame_size.height * WHRatio),
            frame_size.height);
    }
    else
    {
        cropSize =cv::Size(frame_size.width,
            static_cast<int>(frame_size.width / WHRatio));
    }

    cv::Rect crop(cv::Point((frame_size.width - cropSize.width) / 2,
        (frame_size.height - cropSize.height) / 2),
        cropSize);


    frame = frame(crop);
    float confidenceThreshold = 0.30;
    for (int i = 0; i < boxes.dim_size(1); i++)
    {
        float confidence = boxestemp(i, 2);
        std::cout<<"confidence:"<<confidence<<endl;

        if (confidence > confidenceThreshold)
        {
            size_t objectClass = (size_t)(boxestemp(i, 1));

            int xLeftBottom = static_cast<int>(boxestemp(i, 3) * frame.cols);
            int yLeftBottom = static_cast<int>(boxestemp(i, 4) * frame.rows);
            int xRightTop = static_cast<int>(boxestemp(i, 5) * frame.cols);
            int yRightTop = static_cast<int>(boxestemp(i, 6) * frame.rows);
            std::cout<<"x,y,x,y :"<<xLeftBottom<<endl
                    <<yLeftBottom<<endl
                   <<xRightTop<<endl
                  <<yRightTop<<endl;

            ostringstream ss;
            ss << confidence;
            cv::String conf(ss.str());

            Rect object((int)xLeftBottom, (int)yLeftBottom,
                (int)(xRightTop - xLeftBottom),
                (int)(yRightTop - yLeftBottom));

            rectangle(frame, object, Scalar(0, 255, 0),2);
            rectangle(frame,cv::Point(xLeftBottom,yLeftBottom),Point(xRightTop - xLeftBottom,yRightTop - yLeftBottom),Scalar(255,0,0),1,1,0);
            String label = String(classNames[objectClass]) + ": " + conf;
            std::cout<<"className:"<<label<<endl;
            int baseLine = 0;
            cv::Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            rectangle(frame, cv::Rect(cv::Point(xLeftBottom, yLeftBottom - labelSize.height),
                cv::Size(labelSize.width, labelSize.height + baseLine)),
                Scalar(0, 255, 0), CV_FILLED);
            putText(frame, label, Point(xLeftBottom, yLeftBottom),
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
        }
    }

    cv::imshow("image", frame);
    cv::waitKey(0);



}

int main()
{
    Read_pb();
    return 0;
}