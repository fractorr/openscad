#include "Camera.h"
#include "rendersettings.h"
#include "printutils.h"

Camera::Camera() :
	projection(ProjectionType::PERSPECTIVE), fov(22.5), viewall(false)
{
	PRINTD("Camera()");

	// gimbal cam values
	resetView();

	pixel_width = RenderSettings::inst()->img_width;
	pixel_height = RenderSettings::inst()->img_height;
	autocenter = false;
}

void Camera::setup(std::vector<double> params)
{
	if (params.size() == 7) {
		setVpt(params[0], params[1], params[2]);
		setVpr(params[3], params[4], params[5]);
		viewer_distance = params[6];
	} else if (params.size() == 6) {
		Eigen::Vector3d eye(params[0], params[1], params[2]);
		Eigen::Vector3d center(params[3], params[4], params[5]);
		object_trans = -center;
		auto dir = center - eye;
		viewer_distance = dir.norm();
		object_rot.z() = (!dir[1] && !dir[0]) ? dir[2] < 0 ? 0
		                                                   : 180
		                                      : -atan2(dir[1], dir[0]) * 180 / M_PI + 90;
		object_rot.y() = 0;
		Eigen::Vector3d projection(dir[0], dir[1], 0);
		object_rot.x() = -atan2(dir[2], projection.norm()) * 180 / M_PI;
	} else {
		assert("Gimbal cam needs 7 numbers, Vector camera needs 6");
	}
}
/*!
	Moves camera so that the given bbox is fully visible.
*/
void Camera::viewAll(const BoundingBox &bbox)
{
	if (this->autocenter) {
		// autocenter = point camera at the center of the bounding box.
		this->object_trans = -bbox.center();
	}

	double bboxRadius = bbox.diagonal().norm() / 2;
	double radius = (bbox.center() + object_trans).norm() + bboxRadius;
	this->viewer_distance = radius / sin(this->fov / 2 * M_PI / 180);
	PRINTDB("modified obj trans x y z %f %f %f",object_trans.x() % object_trans.y() % object_trans.z());
	PRINTDB("modified obj rot   x y z %f %f %f",object_rot.x() % object_rot.y() % object_rot.z());
}

void Camera::zoom(int delta)
{
	this->viewer_distance *= pow(0.9, delta / 120.0);
}

void Camera::setProjection(ProjectionType type)
{
	this->projection = type;
}

void Camera::resetView()
{
	object_rot << 20, 0, 45;
	object_trans << -0.0, -0.0, -0.0;
	viewer_distance = 140;
}

double Camera::zoomValue() const
{
	return viewer_distance;
}

std::string Camera::statusText() const
{
	auto vpt = getVpt();
	auto vpr = getVpr();
	boost::format fmt(_("Viewport: translate = [ %.2f %.2f %.2f ], rotate = [ %.2f %.2f %.2f ], distance = %.2f"));
	fmt % vpt.x() % vpt.y() % vpt.z()
		% vpr.x() % vpr.y() % vpr.z()
		% viewer_distance;
	return fmt.str();
}
