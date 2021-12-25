#pragma once
#include <Eigen/core>
#include <Eigen/Geometry>
#include <Eigen/dense>


class Movable
{
public:
	Movable();
	Movable(const Movable& mov);
	Eigen::Matrix4f MakeTransScale();
	Eigen::Matrix4d MakeTransd();
	Eigen::Matrix4d MakeTransScaled();
	void MyTranslate(Eigen::Vector3d amt, bool preRotation);
	void TranslateInSystem(Eigen::Matrix3d rot, Eigen::Vector3d amt);
	void MyRotate(Eigen::Vector3d rotAxis, double angle);
	void MyRotate(const Eigen::Matrix3d &rot);
	void RotateInSystem(Eigen::Vector3d rotAxis, double angle);
	void MyScale(Eigen::Vector3d amt);
	void TranslateInSystem(Eigen::Matrix4d Mat, Eigen::Vector3d amt, bool preRotation);
	void RotateInSystem(Eigen::Matrix4d Mat, Eigen::Vector3d rotAxis, double angle);
	void RotateInEuler(Eigen::Vector3d rotAxis, double angle);
	void Movable::SetCenterOfRotation(Eigen::Vector3d amt);

	Eigen::Vector3d GetCenterOfRotation();


	Eigen::Matrix3d GetRotation() const{ return Tout.rotation().matrix(); }

	virtual ~Movable() {}
private:
	Eigen::Affine3d Tout,Tin;
};

