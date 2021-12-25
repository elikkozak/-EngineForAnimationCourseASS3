#include "tutorial/sandBox/sandBox.h"
#include "igl/edge_flaps.h"
#include "igl/collapse_edge.h"
#include "Eigen/dense"
#include <functional>



SandBox::SandBox()
{

}

void SandBox::Init(const std::string &config)
{
	std::string item_name;
	std::ifstream nameFileout;
	doubleVariable = 0;
	nameFileout.open(config);
	if (!nameFileout.is_open())
	{
		std::cout << "Can't open file "<<config << std::endl;
	}
	else
	{
		
		while (nameFileout >> item_name)
		{
			std::cout << "openning " << item_name << std::endl;
			load_mesh_from_file(item_name);
			
			
			data().add_points(Eigen::RowVector3d(0, 0, 0), Eigen::RowVector3d(0, 0, 1));
			data().show_overlay_depth = false;
			data().point_size = 10;
			data().line_width = 2;
			data().set_visible(false, 1);

			

		}
		nameFileout.close();
	}
	parents.push_back(-1);
	MyTranslate(Eigen::Vector3d(-2, 0, -12), true);
	data_list[0].MyTranslate(Eigen::Vector3d(5, 0, 0),true);
	tip_pos.push_back((data_list[0].MakeTransd() * Eigen::Vector4d(0, 0, 0, 1)).head(3));
	//data_list[1].MyTranslate(Eigen::Vector3d(0, 0, 1.6),true);
	
	data().set_colors(Eigen::RowVector3d(0.9, 0.1, 0.1));
	fix_mesh_load();
	
}

SandBox::~SandBox()
{

}

void SandBox::Animate()
{
	if (isActive)
	{
		
		
		
	}
	if(move)
	{
		//CCD();
		Fabrik();
	}
}


