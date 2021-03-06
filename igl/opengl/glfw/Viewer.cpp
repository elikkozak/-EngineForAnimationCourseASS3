// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "Viewer.h"

//#include <chrono>
#include <thread>

#include <Eigen/LU>


#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <cassert>
#include <math.h>

#include <igl/project.h>
//#include <igl/get_seconds.h>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/adjacency_list.h>
#include <igl/writeOBJ.h>
#include <igl/writeOFF.h>
#include <igl/massmatrix.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/quat_mult.h>
#include <igl/axis_angle_to_quat.h>
#include <igl/trackball.h>
#include <igl/two_axis_valuator_fixed_up.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/unproject.h>
#include <igl/serialize.h>

// Internal global variables used for glfw event handling
//static igl::opengl::glfw::Viewer * __viewer;
static double highdpi = 1;
static double scroll_x = 0;
static double scroll_y = 0;


namespace igl
{
namespace opengl
{
namespace glfw
{

  void Viewer::Init(const std::string config)
  {
	  

  }

  IGL_INLINE Viewer::Viewer():
    data_list(1),
    selected_data_index(0),
    next_data_id(1),
	isPicked(false),
	isActive(false)
  {
    data_list.front().id = 0;

  

    // Temporary variables initialization
   // down = false;
  //  hack_never_moved = true;
    scroll_position = 0.0f;

    // Per face
    data().set_face_based(false);

    
#ifndef IGL_VIEWER_VIEWER_QUIET
    const std::string usage(R"(igl::opengl::glfw::Viewer usage:
  [drag]  Rotate scene
  A,a     Toggle animation (tight draw loop)
  F,f     Toggle face based
  I,i     Toggle invert normals
  L,l     Toggle wireframe
  O,o     Toggle orthographic/perspective projection
  T,t     Toggle filled faces
  [,]     Toggle between cameras
  1,2     Toggle between models
  ;       Toggle vertex labels
  :       Toggle face labels)"
);
    std::cout<<usage<<std::endl;
#endif
  }

  IGL_INLINE Viewer::~Viewer()
  {
  }

  IGL_INLINE bool Viewer::load_mesh_from_file(
      const std::string & mesh_file_name_string)
  {

    // Create new data slot and set to selected
    if(!(data().F.rows() == 0  && data().V.rows() == 0))
    {
      append_mesh();
    }
    data().clear();

    size_t last_dot = mesh_file_name_string.rfind('.');
    if (last_dot == std::string::npos)
    {
      std::cerr<<"Error: No file extension found in "<<
        mesh_file_name_string<<std::endl;
      return false;
    }

    std::string extension = mesh_file_name_string.substr(last_dot+1);

    if (extension == "off" || extension =="OFF")
    {
      Eigen::MatrixXd V;
      Eigen::MatrixXi F;
      if (!igl::readOFF(mesh_file_name_string, V, F))
        return false;
      data().set_mesh(V,F);
    }
    else if (extension == "obj" || extension =="OBJ")
    {
      Eigen::MatrixXd corner_normals;
      Eigen::MatrixXi fNormIndices;

      Eigen::MatrixXd UV_V;
      Eigen::MatrixXi UV_F;
      Eigen::MatrixXd V;
      Eigen::MatrixXi F;

      if (!(
            igl::readOBJ(
              mesh_file_name_string,
              V, UV_V, corner_normals, F, UV_F, fNormIndices)))
      {
        return false;
      }

      data().set_mesh(V,F);
      if (UV_V.rows() > 0)
      {
          data().set_uv(UV_V, UV_F);
      }

    }
    else
    {
      // unrecognized file type
      printf("Error: %s is not a recognized file type.\n",extension.c_str());
      return false;
    }

    data().compute_normals();
    data().uniform_colors(Eigen::Vector3d(51.0/255.0,43.0/255.0,33.3/255.0),
                   Eigen::Vector3d(255.0/255.0,228.0/255.0,58.0/255.0),
                   Eigen::Vector3d(255.0/255.0,235.0/255.0,80.0/255.0));

    // Alec: why?
    if (data().V_uv.rows() == 0)
    {
      data().grid_texture();
    }




    


    //for (unsigned int i = 0; i<plugins.size(); ++i)
    //  if (plugins[i]->post_load())
    //    return true;

    return true;
  }

  IGL_INLINE bool Viewer::save_mesh_to_file(
      const std::string & mesh_file_name_string)
  {
    // first try to load it with a plugin
    //for (unsigned int i = 0; i<plugins.size(); ++i)
    //  if (plugins[i]->save(mesh_file_name_string))
    //    return true;

    size_t last_dot = mesh_file_name_string.rfind('.');
    if (last_dot == std::string::npos)
    {
      // No file type determined
      std::cerr<<"Error: No file extension found in "<<
        mesh_file_name_string<<std::endl;
      return false;
    }
    std::string extension = mesh_file_name_string.substr(last_dot+1);
    if (extension == "off" || extension =="OFF")
    {
      return igl::writeOFF(
        mesh_file_name_string,data().V,data().F);
    }
    else if (extension == "obj" || extension =="OBJ")
    {
      Eigen::MatrixXd corner_normals;
      Eigen::MatrixXi fNormIndices;

      Eigen::MatrixXd UV_V;
      Eigen::MatrixXi UV_F;

      return igl::writeOBJ(mesh_file_name_string,
          data().V,
          data().F,
          corner_normals, fNormIndices, UV_V, UV_F);
    }
    else
    {
      // unrecognized file type
      printf("Error: %s is not a recognized file type.\n",extension.c_str());
      return false;
    }
    return true;
  }
 
  IGL_INLINE bool Viewer::load_scene()
  {
    std::string fname = igl::file_dialog_open();
    if(fname.length() == 0)
      return false;
    return load_scene(fname);
  }

  IGL_INLINE bool Viewer::load_scene(std::string fname)
  {
   // igl::deserialize(core(),"Core",fname.c_str());
    igl::deserialize(data(),"Data",fname.c_str());
    return true;
  }

  IGL_INLINE bool Viewer::save_scene()
  {
    std::string fname = igl::file_dialog_save();
    if (fname.length() == 0)
      return false;
    return save_scene(fname);
  }

  IGL_INLINE bool Viewer::save_scene(std::string fname)
  {
    //igl::serialize(core(),"Core",fname.c_str(),true);
    igl::serialize(data(),"Data",fname.c_str());

    return true;
  }

  IGL_INLINE void Viewer::open_dialog_load_mesh()
  {
    std::string fname = igl::file_dialog_open();

    if (fname.length() == 0)
      return;

    this->load_mesh_from_file(fname.c_str());
  }

  IGL_INLINE void Viewer::fix_mesh_load()
  {
	  for (int i = 1; i < data_list.size(); ++i)
	  {

          //DRAW AN AXIS
          data_list[i].tree.init(data().V, data().F);
          igl::AABB<Eigen::MatrixXd, 3> tree = data().tree;
          Eigen::AlignedBox<double, 3> box = tree.m_box;
          data_list[i].drawAxis(box, i);


          num_of_links++;
          if (i == 1) {
              parents.push_back( - 1);
          }
          else
              parents.push_back(parents.size() - 1);


          data_list[i].SetCenterOfRotation(Eigen::Vector3d(0, 0, -0.8));
          Eigen::Vector3d center = data_list[i].GetCenterOfRotation();
          data_list[i].MyTranslate(Eigen::Vector3d(0, 0, 1.6 * 1), true);

          tip_pos.push_back((CalcParentsTrans(i) * data_list[i].MakeTransd() * Eigen::Vector4d(center.x(), center.y(), center.z(), 1)).head(3));

          
	  }

  }
  IGL_INLINE void Viewer::print_phi_thetha(int selected_data_index)
  {
      
	  if(selected_data_index == -1 || selected_data_index ==0)
	  {
          Eigen::Matrix3d myRot = this->GetRotation();
          std::cout << "no link picked here is the scn rot mat:\n";
          std::cout << myRot << std::endl;
	  }
      else {
          Eigen::Matrix3d myRot = this->data_list[selected_data_index].GetRotation();
          Eigen::Vector3d ea = myRot.eulerAngles(2, 0, 2);

          Eigen::Matrix3d Phi = Eigen::AngleAxisd(ea[0], Eigen::Vector3d::UnitZ()).toRotationMatrix();
          Eigen::Matrix3d Theta = Eigen::AngleAxisd(ea[1], Eigen::Vector3d::UnitX()).toRotationMatrix();

          std::cout << "Phi:\n";
          std::cout << Phi << std::endl;

          std::cout << "Theta:\n";
          std::cout << Theta << std::endl;
      }
  }


  IGL_INLINE void Viewer::fix_myTip()
  {
      for (int i = 0; i < data_list.size(); i++)
      {
          Eigen::Vector3d center = data_list[i].GetCenterOfRotation();
          tip_pos[i] = ((CalcParentsTrans(i) * data_list[i].MakeTransd() * Eigen::Vector4d(center.x(), center.y(), center.z(), 1)).head(3));
      }
      
  }
  IGL_INLINE void Viewer::CCD()
  {
      fix_myTip();
    
      Eigen::Vector3d E = tip_pos[num_of_links] + (CalcParentsTrans(num_of_links).block<3,3>(0,0) * data_list[num_of_links].GetRotation() * Eigen::Vector3d(0, 0, 1.6));
      Eigen::Vector3d D = tip_pos[0];
  	  if((E-D).norm()<0.1){
		  std::cout << "reach\n";
	      move = false;

	      return;
      }
      if((tip_pos[1]-D).norm() > 1.6*num_of_links){
          std::cout << "cannot reach\n";
          move = false;

          return;
      }
      for (int i = num_of_links; i >= 1 ; --i)
      {
          Eigen::Vector3d R = tip_pos[i];

          Eigen::Vector3d a = E - R;
          Eigen::Vector3d b = D - R;
          double cos_theta = a.normalized().dot(b.normalized());
          cos_theta = std::min(std::max(cos_theta, -1.0),1.0);
          double theta =  acos(cos_theta)/10.0;
          Eigen::Vector3d cross = (a.cross(b).normalized());
          Eigen::Matrix3d rot = Eigen::AngleAxisd(theta,cross).toRotationMatrix();
          E = rot * (E-R) + R;
          Eigen::Matrix3d mat_rot = CalcParentsTrans(i ).block<3, 3>(0, 0) * data_list[i].GetRotation();
      	  data_list[i].MyRotate(mat_rot.transpose() * cross, theta);
          for (int j = i+1; j <= num_of_links; ++j)
          {
              tip_pos[j] = rot * (tip_pos[j] - R) + R;
          }
			
      }
      fix_myTip();

  }

  IGL_INLINE void Viewer::Fabrik()
  {
      fix_myTip();
    

      Eigen::Vector3d T = tip_pos[0];
      double r_i = 0.0;
      double lambda_i = 0;
      Eigen::Vector3d center = data_list[num_of_links].GetCenterOfRotation();
      Eigen::Vector3d E = ((CalcParentsTrans(num_of_links) * data_list[num_of_links].MakeTransd() * Eigen::Vector4d(center.x(), center.y(), center.z() + 1.6, 1)).head(3));
      new_joint = tip_pos;
      new_joint.push_back(E);

      if ((tip_pos[1] - T).norm() > 1.6 * num_of_links)
      {
	      for (int i = 1; i <= num_of_links; ++i)
	      {
              r_i = (T - new_joint[i]).norm();
              lambda_i = 1.6 / r_i;
              new_joint[i + 1] = (1 - lambda_i) * tip_pos[i] + lambda_i * T;
	      }

      }

      else
      {

          Eigen::Vector3d B = tip_pos[1];
          double dif_A = (E - T).norm();

          if (dif_A > 0.1){
	          new_joint = tip_pos;
	          new_joint.push_back(E);
          }
          else
          {
              std::cout << "touch\n";
              move = false;
              return;
          }
          
          new_joint.back() = T;
          for (int i = new_joint.size()-2; i >= 1; --i)
          {
              r_i = (new_joint[i + 1] - new_joint[i]).norm();
              lambda_i = 1.6 / r_i;
              new_joint[i ] = (1 - lambda_i) * new_joint[i+1] + lambda_i * new_joint[i ];
          }
          new_joint[1] = B;

          for (int i = 1; i < new_joint.size() - 1 ; ++i)
          {
              r_i = (new_joint[i + 1] - new_joint[i]).norm();
              lambda_i = 1.6 / r_i;
              new_joint[i + 1] = (1 - lambda_i) * new_joint[i] + lambda_i * new_joint[i + 1];
          }
          dif_A = (new_joint.back() - T).norm();
          
      }
    
      for (int i = 1; i <= num_of_links ; ++i)
      {
        
          E = ((CalcParentsTrans(num_of_links) * data_list[num_of_links].MakeTransd() * Eigen::Vector4d(center.x(), center.y(), center.z() + 1.6, 1)).head(3));
          Eigen::Vector3d end_joint = i==num_of_links ? ((CalcParentsTrans(num_of_links) * data_list[num_of_links].MakeTransd() * Eigen::Vector4d(center.x(), center.y(), center.z() + 1.6, 1)).head(3)) : tip_pos[i + 1];
          Eigen::Vector3d R = tip_pos[i];
          Eigen::Vector3d D = new_joint[i+1];
          Eigen::Vector3d a = (end_joint - R).normalized();
          Eigen::Vector3d b = (D - R).normalized();
          double cos_theta = a.dot(b);
          cos_theta = std::min(std::max(cos_theta, -1.0), 1.0);
          double theta = acos(cos_theta)/10.0 ;
          Eigen::Vector3d cross = (a.cross(b));
          Eigen::Matrix3d mat_rot = CalcParentsTrans(i).block<3, 3>(0, 0) * data_list[i].GetRotation();

          Eigen::Matrix3d rot = Eigen::AngleAxisd(theta, cross).toRotationMatrix();

         
      	  data_list[i].MyRotate(mat_rot.transpose()*cross, theta);
          fix_myTip();

      }
  }

  IGL_INLINE void Viewer::open_dialog_save_mesh()
  {
    std::string fname = igl::file_dialog_save();

    if(fname.length() == 0)
      return;

    this->save_mesh_to_file(fname.c_str());
  }

  IGL_INLINE ViewerData& Viewer::data(int mesh_id /*= -1*/)
  {
    assert(!data_list.empty() && "data_list should never be empty");
    int index;
    if (mesh_id == -1)
      index = selected_data_index;
    else
      index = mesh_index(mesh_id);

    assert((index >= 0 && index < data_list.size()) &&
      "selected_data_index or mesh_id should be in bounds");
    return data_list[index];
  }

  IGL_INLINE const ViewerData& Viewer::data(int mesh_id /*= -1*/) const
  {
    assert(!data_list.empty() && "data_list should never be empty");
    int index;
    if (mesh_id == -1)
      index = selected_data_index;
    else
      index = mesh_index(mesh_id);

    assert((index >= 0 && index < data_list.size()) &&
      "selected_data_index or mesh_id should be in bounds");
    return data_list[index];
  }

  IGL_INLINE int Viewer::append_mesh(bool visible /*= true*/)
  {
    assert(data_list.size() >= 1);

    data_list.emplace_back();
    selected_data_index = data_list.size()-1;
    data_list.back().id = next_data_id++;
    //if (visible)
    //    for (int i = 0; i < core_list.size(); i++)
    //        data_list.back().set_visible(true, core_list[i].id);
    //else
    //    data_list.back().is_visible = 0;
    return data_list.back().id;
  }

  IGL_INLINE bool Viewer::erase_mesh(const size_t index)
  {
    assert((index >= 0 && index < data_list.size()) && "index should be in bounds");
    assert(data_list.size() >= 1);
    if(data_list.size() == 1)
    {
      // Cannot remove last mesh
      return false;
    }
    data_list[index].meshgl.free();
    data_list.erase(data_list.begin() + index);
    if(selected_data_index >= index && selected_data_index > 0)
    {
      selected_data_index--;
    }

    return true;
  }

  IGL_INLINE size_t Viewer::mesh_index(const int id) const {
    for (size_t i = 0; i < data_list.size(); ++i)
    {
      if (data_list[i].id == id)
        return i;
    }
    return 0;
  }

  Eigen::Matrix4d Viewer::CalcParentsTrans(int indx) 
  {
	  Eigen::Matrix4d prevTrans = Eigen::Matrix4d::Identity();

	  for (int i = indx; parents[i] >= 0; i = parents[i])
	  {
		  //std::cout << "parent matrix:\n" << scn->data_list[scn->parents[i]].MakeTrans() << std::endl;
		  prevTrans = data_list[parents[i]].MakeTransd() * prevTrans;
	  }

	  return prevTrans;
  }

} // end namespace
} // end namespace
}
