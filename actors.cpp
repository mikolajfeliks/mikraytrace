#include <Eigen/Geometry>
#include <cmath>
#include "actors.h"


namespace mrtp {

using Vector3d = Eigen::Vector3d;

const double kMyZero = 0.0001;


static bool file_exists(const std::string& filename) {
    std::fstream check(filename.c_str());
    return check.good();
}


static double solve_quadratic(double a, double b, double c) {
    double delta = b * b - 4 * a * c;
    if (delta < 0) {
        return -1;
    }

    if (delta < kMyZero && -delta > -kMyZero) {
        return -b / (2 * a);
    }

    double sqdelta = sqrt(delta);
    double t = 0.5 / a;
    double ta = (-b - sqdelta) * t;
    double tb = (-b + sqdelta) * t;

    return (ta < tb) ? ta : tb;
}


static Vector3d fill_vector(const Vector3d& vec) {
    double x = (vec[0] < 0) ? -vec[0] : vec[0];
    double y = (vec[1] < 0) ? -vec[1] : vec[1];
    double z = (vec[2] < 0) ? -vec[2] : vec[2];

    Vector3d unit{0, 0, 1};

    if (x < y) {
        if (x < z) {
            unit << 1, 0, 0;
        }
    } else { // if ( x >= y)
        if (y < z) {
            unit << 0, 1, 0;
        }
    }
    return unit;
}


StandardBasis::StandardBasis(const Vector3d& o,
                             const Vector3d& i,
                             const Vector3d& j,
                             const Vector3d& k) :
    o(o), vi(i), vj(j), vk(k) {

}


StandardBasis::StandardBasis() {
    o << 0, 0, 0;
    vi << 1, 0, 0;
    vj << 0, 1, 0;
    vk << 0, 0, 1;
}


ActorBase::ActorBase(const StandardBasis& local_basis) :
    local_basis_(local_basis) {

}


class TexturedPlane : public ActorBase {
public:
    TexturedPlane(const StandardBasis& local_basis, MyTexture* texture) :
        ActorBase(local_basis), texture_(texture) {
    }

    ~TexturedPlane() override = default;

    bool has_shadow() const override {
        return false;
    }

    double solve_light_ray(const Vector3d& O, const Vector3d& D,
                           double min_dist, double max_dist) const override {
        double t = D.dot(local_basis_.vk);
        if (t > kMyZero || t < -kMyZero) {
            Vector3d v = O - local_basis_.o;
            double d = -v.dot(local_basis_.vk) / t;
            if (d > min_dist && d < max_dist) {
                return d;
            }
        }
        return -1;
    }

    MyPixel pick_pixel(const Vector3d& hit,
                       const Vector3d& normal_at_hit) const override {
        Vector3d v = hit - local_basis_.o;
        double tx_i = v.dot(local_basis_.vi);
        double tx_j = v.dot(local_basis_.vj);

        return texture_->pick_pixel(tx_i, tx_j);
    }

    Vector3d calculate_normal_at_hit(const Vector3d& hit) const override {
        return local_basis_.vk;
    }

private:
    MyTexture* texture_;
};


class TexturedSphere : public ActorBase {
public:
    TexturedSphere(const StandardBasis& local_basis, double radius,
                   MyTexture* texture) :
        ActorBase(local_basis),
        radius_(radius),
        texture_(texture) {
    }

    ~TexturedSphere() override = default;

    bool has_shadow() const override {
        return true;
    }

    double solve_light_ray(const Vector3d& O, const Vector3d& D,
                           double min_dist, double max_dist) const override {
        Vector3d t = O - local_basis_.o;

        double a = D.dot(D);
        double b = 2 * D.dot(t);
        double c = t.dot(t) - radius_ * radius_;
        double dist = solve_quadratic(a, b, c);

        if (dist > min_dist && dist < max_dist) {
            return dist;
        }
        return -1;
    }

    MyPixel pick_pixel(const Vector3d& hit,
                       const Vector3d& normal_at_hit) const override {
        // Taken from https://www.cs.unc.edu/~rademach/xroads-RT/RTarticle.html
        double dot_vi = normal_at_hit.dot(local_basis_.vi);
        double dot_vj = normal_at_hit.dot(local_basis_.vj);
        double dot_vk = normal_at_hit.dot(local_basis_.vk);

        double phi = std::acos(-dot_vj);
        double fracy = phi / M_PI;
        double theta = std::acos(dot_vi / std::sin(phi)) / (2 * M_PI);
        double fracx = (dot_vk > 0) ? theta : (1 - theta);

        return texture_->pick_pixel(fracx, fracy);
    }

    Vector3d calculate_normal_at_hit(const Vector3d& hit) const override {
        Vector3d t = hit - local_basis_.o;
        return t * (1 / t.norm());
    }

private:
    double radius_;
    MyTexture* texture_;
};


class TexturedCylinder : public ActorBase {
public:
    TexturedCylinder(const StandardBasis& local_basis, double radius,
                     double length, MyTexture* texture) :
        ActorBase(local_basis),
        radius_(radius),
        length_(length),
        texture_(texture) {
    }

    ~TexturedCylinder() override = default;

    bool has_shadow() const override {
        return true;
    }

    /*
    Capital letters are vectors.
      A       Origin    of cylinder
      B       Direction of cylinder
      O       Origin    of ray
      D       Direction of ray
      P       Hit point on cylinder's surface
      X       Point on cylinder's axis closest to the hit point
      t       Distance between ray's      origin and P
      alpha   Distance between cylinder's origin and X

     (P - X) . B = 0
     |P - X| = R  => (P - X) . (P - X) = R^2

     P = O + t * D
     X = A + alpha * B
     T = O - A
     ...
     2t * (T.D - alpha * D.B)  +  t^2 - 2 * alpha * T.B  +
         +  alpha^2  =  R^2 - T.T
     a = T.D
     b = D.B
     d = T.B
     f = R^2 - T.T

     t^2 * (1 - b^2)  +  2t * (a - b * d)  -
         -  d^2 - f = 0    => t = ...
     alpha = d + t * b
    */
    double solve_light_ray(const Vector3d& O, const Vector3d& D,
                           double min_dist, double max_dist) const override {
        Vector3d vec = O - local_basis_.o;

        double a = D.dot(vec);
        double b = D.dot(local_basis_.vk);
        double d = vec.dot(local_basis_.vk);
        double f = radius_ * radius_ - vec.dot(vec);

        // Solving quadratic equation for t
        double aa = 1 - (b * b);
        double bb = 2 * (a - b * d);
        double cc = -(d * d) - f;
        double t = solve_quadratic(aa, bb, cc);

        if (t < min_dist || t > max_dist) {
            return -1;
        }
        // Check if cylinder is finite
        if (length_ > 0) {
            double alpha = d + t * b;
            if (alpha < -length_ || alpha > length_) {
                return -1;
            }
        }
        return t;
    }

    MyPixel pick_pixel(const Vector3d& hit,
                       const Vector3d& normal_at_hit) const override {
        Vector3d t = hit - local_basis_.o;

        double alpha = t.dot(local_basis_.vk);
        double dot = normal_at_hit.dot(local_basis_.vi);
        double frac_x = acos(dot) / M_PI;
        double frac_y = alpha / (2 * M_PI * radius_);

        return texture_->pick_pixel(frac_x, frac_y);
    }

    Vector3d calculate_normal_at_hit(const Vector3d& hit) const override {
        // N = Hit - [B . (Hit - A)] * B
        Vector3d v = hit - local_basis_.o;
        double alpha = local_basis_.vk.dot(v);
        Vector3d w = local_basis_.o + alpha * local_basis_.vk;
        Vector3d normal = hit - w;

        return normal * (1 / normal.norm());
    }

private:
    double radius_;
    double length_;
    MyTexture* texture_;
};


TomlActorFactory::TomlActorFactory(TextureFactory* texture_factory) :
    texture_factory_(texture_factory) {

}


std::shared_ptr<ActorBase> TomlActorFactory::create_plane(std::shared_ptr<cpptoml::table> plane_items) {
    auto plane_center = plane_items->get_array_of<double>("center");
    if (!plane_center) {
        //TODO
    }
    Vector3d plane_center_vec(plane_center->data());

    auto plane_normal = plane_items->get_array_of<double>("normal");
    if (!plane_normal) {
        //TODO
    }
    Vector3d plane_normal_vec(plane_normal->data());

    auto plane_texture = plane_items->get_as<std::string>("texture");
    if (!plane_texture) {
        //TODO
    }
    std::string plane_texture_str(plane_texture->data());
    if (!file_exists(plane_texture_str)) {
        //TODO
    }

    double scale_coef = plane_items->get_as<double>("scale").value_or(0.15);
    double reflect_coef = plane_items->get_as<double>("reflect").value_or(0);

    MyTexture* plane_texture_ptr = texture_factory_->create_texture(
                plane_texture_str, reflect_coef, scale_coef);

    Vector3d fill_vec = fill_vector(plane_normal_vec);

    Vector3d plane_vec_i = fill_vec.cross(plane_normal_vec);
    Vector3d plane_vec_j = plane_normal_vec.cross(plane_vec_i);

    plane_vec_i *= (1 / plane_vec_i.norm());
    plane_vec_j *= (1 / plane_vec_j.norm());
    plane_normal_vec *= (1 / plane_normal_vec.norm());

    StandardBasis plane_basis(
        plane_center_vec,
        plane_vec_i,
        plane_vec_j,
        plane_normal_vec
    );

    auto plane_ptr = std::shared_ptr<ActorBase>(new TexturedPlane(
        plane_basis,
        plane_texture_ptr
    ));

    return plane_ptr;
}


std::shared_ptr<ActorBase> TomlActorFactory::create_sphere(std::shared_ptr<cpptoml::table> sphere_items) {
    auto sphere_center = sphere_items->get_array_of<double>("center");
    if (!sphere_center) {
        //TODO
    }
    Vector3d sphere_center_vec(sphere_center->data());

    Vector3d sphere_axis_vec(0, 0, 1);
    auto sphere_axis = sphere_items->get_array_of<double>("axis");
    if (sphere_axis) {
        Vector3d tmp_vec(sphere_axis->data());
        sphere_axis_vec = tmp_vec;
    }

    auto sphere_texture = sphere_items->get_as<std::string>("texture");
    if (!sphere_texture) {
        //TODO
    }
    std::string sphere_texture_str(sphere_texture->data());
    if (!file_exists(sphere_texture_str)) {
        //TODO
    }

    double sphere_radius = sphere_items->get_as<double>("radius").value_or(1);
    double reflect_coef = sphere_items->get_as<double>("reflect").value_or(0);

    MyTexture* sphere_texture_ptr = texture_factory_->create_texture(
                sphere_texture_str, reflect_coef, 1);

    Vector3d fill_vec = fill_vector(sphere_axis_vec);

    Vector3d sphere_vec_i = fill_vec.cross(sphere_axis_vec);
    Vector3d sphere_vec_j = sphere_axis_vec.cross(sphere_vec_i);

    sphere_vec_i *= (1 / sphere_vec_i.norm());
    sphere_vec_j *= (1 / sphere_vec_j.norm());
    sphere_axis_vec *= (1 / sphere_axis_vec.norm());

    StandardBasis sphere_basis(
        sphere_center_vec,
        sphere_vec_i,
        sphere_vec_j,
        sphere_axis_vec
    );

    auto sphere_ptr = std::shared_ptr<ActorBase>(new TexturedSphere(
        sphere_basis,
        sphere_radius,
        sphere_texture_ptr
    ));

    return sphere_ptr;
}


std::shared_ptr<ActorBase> TomlActorFactory::create_cylinder(std::shared_ptr<cpptoml::table> cylinder_items) {
    auto cylinder_center = cylinder_items->get_array_of<double>("center");
    if (!cylinder_center) {
        //TODO
    }
    Vector3d cylinder_center_vec(cylinder_center->data());

    auto cylinder_direction = cylinder_items->get_array_of<double>("direction");
    if (!cylinder_direction) {
        //TODO
    }
    Vector3d cylinder_direction_vec(cylinder_direction->data());

    auto cylinder_texture = cylinder_items->get_as<std::string>("texture");
    if (!cylinder_texture) {
        //TODO
    }
    std::string cylinder_texture_str(cylinder_texture->data());
    if (!file_exists(cylinder_texture_str)) {
        //TODO
    }

    double cylinder_span = cylinder_items->get_as<double>("span").value_or(-1);
    double cylinder_radius = cylinder_items->get_as<double>("radius").value_or(1);
    double cylinder_reflect = cylinder_items->get_as<double>("reflect").value_or(0);

    MyTexture* cylinder_texture_ptr = texture_factory_->create_texture(
                cylinder_texture_str, cylinder_reflect, 1);

    Vector3d fill_vec = fill_vector(cylinder_direction_vec);

    Vector3d cylinder_vec_i = fill_vec.cross(cylinder_direction_vec);
    Vector3d cylinder_vec_j = cylinder_direction_vec.cross(cylinder_vec_i);

    cylinder_vec_i *= (1 / cylinder_vec_i.norm());
    cylinder_vec_j *= (1 / cylinder_vec_j.norm());
    cylinder_direction_vec *= (1 / cylinder_direction_vec.norm());

    StandardBasis cylinder_basis(
        cylinder_center_vec,
        cylinder_vec_i,
        cylinder_vec_j,
        cylinder_direction_vec
    );

    auto cylinder_ptr = std::shared_ptr<ActorBase>(new TexturedCylinder(
        cylinder_basis,
        cylinder_radius,
        cylinder_span,
        cylinder_texture_ptr
    ));

    return cylinder_ptr;
}


}  // namespace mrtp

