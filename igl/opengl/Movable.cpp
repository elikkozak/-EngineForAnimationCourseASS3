#include "Movable.h"
#include <iostream>
#include <math.h>
#include <Eigen/Geometry>
Movable::Movable()
{
	Tout = Eigen::Affine3d::Identity();
	Tin = Eigen::Affine3d::Identity();
}

Movable::Movable(const Movable& mov)
{
	Tout = mov.Tout;
	Tin = mov.Tin;
}

Eigen::Matrix4f Movable::MakeTransScale()
{
	return (Tout.matrix()*Tin.matrix()).cast<float>();
}

Eigen::Matrix4d Movable::MakeTransScaled()
{
	return (Tout.matrix() * Tin.matrix());
}

Eigen::Matrix4d Movable::MakeTransd()
{
	Eigen::Matrix4d mat = Eigen::Matrix4d::Identity();
	mat.col(3) << Tin.translation(), 1;

	return (Tout.matrix() * mat);
}

void Movable::MyTranslate(Eigen::Vector3d amt, bool preRotation)
{
	
	if(preRotation)
		Tout.pretranslate(amt);
	else
		Tout.translate(amt);
}

void Movable::TranslateInSystem(Eigen::Matrix3d rot, Eigen::Vector3d amt)
{
	Tout.pretranslate(rot.transpose() * amt);
}

//angle in radians
void Movable::MyRotate(Eigen::Vector3d rotAxis, double angle)
{
	Tout.rotate(Eigen::AngleAxisd(angle, rotAxis.normalized()));
}

void Movable::MyRotate(const Eigen::Matrix3d& rot)
{
	Tout.rotate(rot);
}

void Movable::RotateInSystem(Eigen::Vector3d rotAxis, double angle)
{
	Tout.rotate(Eigen::AngleAxisd(angle, Tout.rotation().transpose() * (rotAxis.normalized())));
}

void Movable::MyScale(Eigen::Vector3d amt)
{
	Tin.scale(amt);
}



























void Movable::TranslateInSystem(Eigen::Matrix4d Mat, Eigen::Vector3d amt, bool preRotation)
{
	Eigen::Vector3d v = Mat.transpose().block<3, 3>(0, 0) * amt; //transpose instead of inverse
	MyTranslate(v, preRotation);
}
//
void Movable::RotateInSystem(Eigen::Matrix4d Mat, Eigen::Vector3d rotAxis, double angle)
{
	Eigen::Vector3d v = Mat.transpose().block<3, 3>(0, 0) * rotAxis; //transpose instead of inverse
	MyRotate(v.normalized(), angle);
}

void Movable::RotateInEuler(Eigen::Vector3d rotAxis, double angle)
{
	Eigen::Matrix3d myRot = this->GetRotation();
	Eigen::Vector3d ea = myRot.eulerAngles(2, 0, 2);

	auto Z0 = Eigen::AngleAxisd(ea[0], Eigen::Vector3d::UnitZ());
	auto X0 = Eigen::AngleAxisd(ea[1], Eigen::Vector3d::UnitX());
	auto Z1 = Eigen::AngleAxisd(ea[2], Eigen::Vector3d::UnitZ());

	auto new_R = Eigen::AngleAxisd(angle, (rotAxis.normalized())).toRotationMatrix();
	Eigen::Vector3d ea2 = new_R.eulerAngles(2, 0, 2);
	auto new_Z0 = Eigen::AngleAxisd(ea2[0], Eigen::Vector3d::UnitZ());
	auto new_X0 = 3.14159 - abs(ea[2]) < 0.1 ? Eigen::AngleAxisd(-ea2[1], Eigen::Vector3d::UnitX()) : Eigen::AngleAxisd(ea2[1], Eigen::Vector3d::UnitX());
	auto new_Z1 = Eigen::AngleAxisd(ea2[2], Eigen::Vector3d::UnitZ());


	auto res = new_Z1 * Z0 * new_X0 * X0 * Z1;
	Tout.rotate(myRot.transpose());
	Tout.rotate(res);


	// double x = atan2(myRot(2, 1), myRot(2, 2));
	// double y = atan2(-myRot(2, 0), sqrt(pow(myRot(2, 1),2) + pow(myRot(2, 2), 2)));
	// double z = atan2(myRot(1, 0), myRot(0, 0));

	// std::cout << x << y << z << std::endl;
	
}
//
//
void Movable::SetCenterOfRotation(Eigen::Vector3d amt)
{
	Tout.pretranslate(Tout.rotation().matrix().block<3, 3>(0, 0) * amt);
	Tin.translate(-amt);
}

Eigen::Vector3d Movable::GetCenterOfRotation()
{
	return -Tin.translation();
}
