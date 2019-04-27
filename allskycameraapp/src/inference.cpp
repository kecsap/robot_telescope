/**
 *  This file is part of allskycameraapp
 *
 *  Copyright (C) 2017 Csaba Kertész (csaba.kertesz@gmail.com)
 *
 *  AiBO+ is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  AiBO+ is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include "inference.h"

#include <MCBinaryData.hpp>
#include <MCDefs.hpp>
#include <MEImage.hpp>

#include <QFile>

#include <opencv2/core.hpp>

bool CppInference::Load(const QString& model_str)
{
  printf("%s\n", qPrintable(model_str));

  // Read in the protobuf graph we exported
  tensorflow::Status Status;

  Status = tensorflow::ReadBinaryProto(tensorflow::Env::Default(), (model_str+".pb").toStdString(), &GraphDef);
  if (!Status.ok())
  {
    printf("Error reading graph definition from %s: %s\n", (model_str+".pb").toStdString().c_str(),
           Status.ToString().c_str());
    return false;
  }
  // Ignore info logs
  char EnvStr[] = "TF_CPP_MIN_LOG_LEVEL=1";

  putenv(EnvStr);
  // Dynamic GPU memory allocation
  tensorflow::SessionOptions SessionOptions;

  SessionOptions.config.mutable_gpu_options()->set_allow_growth(true);
  Session = tensorflow::NewSession(SessionOptions);
  if (Session == nullptr)
  {
    printf("Could not create Tensorflow session.\n");
    return false;
  }

  // Add the graph to the session
  Status = Session->Create(GraphDef);
  if (!Status.ok())
  {
    printf("Error creating graph: %s\n", Status.ToString().c_str());
    return false;
  }

/*
  // This code demonstrates how to load a saved checkpoint
  const std::string PathToGraph = model_str.toStdString()+".meta";
  const std::string CheckpointPath(model_str.toStdString());

  printf("%s\n", qPrintable(model_str));
  Session = tensorflow::NewSession(tensorflow::SessionOptions());
  if (Session == nullptr)
  {
    printf("Could not create Tensorflow session.\n");
    return false;
  }

  // Read in the protobuf graph we exported
  tensorflow::Status Status;

  Status = ReadBinaryProto(tensorflow::Env::Default(), PathToGraph, &GraphDef);
  if (!Status.ok())
  {
    printf("Error reading graph definition from %s: %s\n", PathToGraph.c_str(), Status.ToString().c_str());
    return false;
  }

  // Add the graph to the session
  Status = Session->Create(GraphDef.graph_def());
  if (!Status.ok())
  {
    printf("Error creating graph: %s\n", Status.ToString().c_str());
    return false;
  }

  // Read weights from the saved checkpoint
  tensorflow::Tensor CheckpointPathTensor(tensorflow::DT_STRING, tensorflow::TensorShape());

  CheckpointPathTensor.scalar<std::string>()() = CheckpointPath;
  Status = Session->Run(
        {{ GraphDef.saver_def().filename_tensor_name(), CheckpointPathTensor },},
        {},
        { GraphDef.saver_def().restore_op_name() },
        nullptr);

  if (!Status.ok())
  {
    printf("Error loading checkpoint from %s: %s\n", CheckpointPath.c_str(), Status.ToString().c_str());
    return false;
  }
*/
}


int CppInference::Predict(MEImage& image)
{
  if (Session == nullptr || image.GetWidth() != 160 || image.GetHeight() != 96 || image.GetLayerCount() != 1)
    return -1;

  tensorflow::Tensor X(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1, 96, 160, 1 }));
  std::vector<std::pair<std::string, tensorflow::Tensor>> Input = { { "conv1_input", X } };
  std::vector<tensorflow::Tensor> Outputs;
  float* XData = X.flat<float>().data();

  for (int i = 0; i < image.GetImageDataSize(); ++i)
  {
    XData[i] = (float)reinterpret_cast<unsigned char*>(image.GetIplImage()->imageData)[i];
  }
  tensorflow::Status Status = Session->Run(Input, { "output/Softmax" }, {}, &Outputs);

  if (!Status.ok())
  {
    printf("Error in prediction: %s\n", Status.ToString().c_str());
    return -1;
  }
  if (Outputs.size() != 1)
  {
    printf("Missing prediction! (%d)\n", (int)Outputs.size());
    return -1;
  }

  auto Item = Outputs[0].shaped<float, 2>({ 1, 2 }); // { 1, 2 } -> One sample+2 label classes

  // printf("Debug inference output: %1.4f %1.4f\n", (float)Item(0, 0), (float)Item(0, 1));
  if ((float)Item(0, 0) < (float)Item(0, 1))
    return 1;

  return 0;
}


CInference::~CInference()
{
  if (Graph == nullptr)
    return;

  TF_DeleteGraph(Graph);
  TF_DeleteSession(Session, Status);
  TF_DeleteSessionOptions(SessionOpts);
  TF_DeleteStatus(Status);
  TF_DeleteImportGraphDefOptions(GraphDefOpts);
}


bool CInference::Load(const QString& model_str)
{
  if (Graph != nullptr)
    return false;

  printf("%s\n", qPrintable(model_str));
  // The protobuf is loaded with C API here
  QFile CurrentFile((model_str+".pb"));

  if (!CurrentFile.open(QIODevice::ReadOnly))
  {
    return false;
  }

  QByteArray FileBuffer = CurrentFile.readAll();

  Status = TF_NewStatus();
  Graph = TF_NewGraph();
  GraphDef.reset(TF_NewBuffer());
  GraphDef->data = FileBuffer.data();
  GraphDef->length = FileBuffer.size();
  // Deallocator is skipped in this example
  GraphDef->data_deallocator = nullptr;
  GraphDefOpts = TF_NewImportGraphDefOptions();
  TF_GraphImportGraphDef(Graph, GraphDef.get(), GraphDefOpts, Status);
  if (TF_GetCode(Status) != TF_OK)
  {
    printf("Tensorflow status %d - %s\n", TF_GetCode(Status), TF_Message(Status));
    return false;
  }
  SessionOpts = TF_NewSessionOptions();
  Session = TF_NewSession(Graph, SessionOpts, Status);
  if (TF_GetCode(Status) != TF_OK)
  {
    printf("Tensorflow status %d - %s\n", TF_GetCode(Status), TF_Message(Status));
    return false;
  }
}


int CInference::Predict(MEImage& image)
{
  if (Session == nullptr || image.GetWidth() != 160 || image.GetHeight() != 96 || image.GetLayerCount() != 1)
    return -1;

  // Make prediction with C API
  float* InputData = (float*)malloc(sizeof(float)*96*160);

  for (int i = 0; i < image.GetImageDataSize(); ++i)
  {
    InputData[i] = (float)reinterpret_cast<unsigned char*>(image.GetIplImage()->imageData)[i];
  }

  // Input node
  int64_t InputDim[] = { 1, 96, 160, 1 };
  TF_Tensor* ImageTensor = TF_NewTensor(TF_FLOAT, InputDim, 4, InputData, 96*160*sizeof(float), nullptr, nullptr);
  TF_Output Input = { TF_GraphOperationByName(Graph, "conv1_input") };
  TF_Tensor* InputValues[] = { ImageTensor };

  // Output node
  TF_Output Output = { TF_GraphOperationByName(Graph, "output/Softmax") };
  TF_Tensor* OutputValues = nullptr;

  // Run prediction
  TF_SessionRun(Session, nullptr,
                &Input, &InputValues[0], 1,
                &Output, &OutputValues, 1,
                nullptr, 0, nullptr, Status);

  delete InputData;
  InputData = nullptr;
  if (TF_GetCode(Status) != TF_OK)
  {
    printf("Tensorflow status %d - %s\n", TF_GetCode(Status), TF_Message(Status));
    return false;
  }
  int Result = (float)*(float*)TF_TensorData(OutputValues);

  // Clean up
  return Result;
}
