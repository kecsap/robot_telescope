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

#pragma once

#include <tensorflow/c/c_api.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>
#include <tensorflow/core/public/session.h>

#include <QString>

#include <memory>

class MEImage;

class CppInference
{
public:
  CppInference() = default;

  bool Load(const QString& model_str);
  int Predict(MEImage& image);

  tensorflow::Session* Session { nullptr };
  tensorflow::GraphDef GraphDef;
  // Metagraph for checkpoint loading
//  tensorflow::MetaGraphDef GraphDef;
};

class CInference
{
public:
  CInference() = default;
  ~CInference();

  bool Load(const QString& model_str);
  int Predict(MEImage& image);

  TF_Status* Status { nullptr };
  TF_Graph* Graph { nullptr };
  TF_ImportGraphDefOptions* GraphDefOpts { nullptr };
  TF_Session* Session { nullptr };
  TF_SessionOptions* SessionOpts { nullptr };
  std::shared_ptr<TF_Buffer> GraphDef;
};
