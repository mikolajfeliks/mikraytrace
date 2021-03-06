#include <cstring>

#include "png.hpp"
#include "texture.h"


namespace mrtp {

using Vector3d = Eigen::Vector3d;


TexturePixel::TexturePixel() :
    red(0), green(0), blue(0) {
}


TexturePixel::TexturePixel(unsigned char r,
                           unsigned char g,
                           unsigned char b) :
    red(r), green(g), blue(b) {
}


TexturePixel::TexturePixel(const Vector3d& color_vec) {
    Vector3d v = 255 * color_vec;
    red = static_cast<unsigned char>(v[0]);
    green = static_cast<unsigned char>(v[1]);
    blue = static_cast<unsigned char>(v[2]);
}


Vector3d TexturePixel::to_vec() const {
    return Vector3d{
        static_cast<double>(red),
        static_cast<double>(green),
        static_cast<double>(blue)
    } / 255;
}


TextureSharedState::TextureSharedState(const std::string& texture_filename) {
    png::image<png::rgb_pixel> image(texture_filename.c_str());

    texture_width_ = image.get_width();
    texture_heigth_ = image.get_height();

    texture_data_.reserve(texture_width_ * texture_heigth_);

    for (unsigned int i = 0; i < texture_heigth_; i++) {
        png::rgb_pixel* in = &image[i][0];
        for (unsigned j = 0; j < texture_width_; j++, in++) {
            TexturePixel pixel{in->red, in->green, in->blue};
            texture_data_.push_back(pixel);
        }
    }
}


/*
fracx, fracy are within a range of <0..1> and
define fractions of the x- and y-dimension
of a texture.
A reasonable scale for a 256x256 texture is 0.15.
*/
TexturePixel TextureSharedState::pick_pixel(double frac_x, double frac_y, double scale_coeff) const {
    unsigned int u = (static_cast<unsigned int>(
                          frac_x * static_cast<double>(texture_width_) * scale_coeff)) % texture_width_;
    unsigned int v = (static_cast<unsigned int>(
                          frac_y * static_cast<double>(texture_heigth_) * scale_coeff)) % texture_heigth_;

    return texture_data_[u + v * texture_width_];
}


bool TextureSharedState::is_same_texture(const std::string& texture_filename) const {
    return texture_filename_ == texture_filename;
}


MyTexture::MyTexture(TextureSharedState* shared_state,
                     double reflection_coeff,
                     double scale_coeff) :
    reflection_coeff_(reflection_coeff),
    scale_coeff_(scale_coeff),
    shared_state_(shared_state) {

}


MyPixel MyTexture::pick_pixel(double frac_x, double frac_y) const {
    TexturePixel pixel = shared_state_->pick_pixel(frac_x, frac_y, scale_coeff_);

    return MyPixel{pixel, reflection_coeff_};
}


MyTexture* TextureFactory::create_texture(const std::string& texture_filename,
                                          double reflection_coeff,
                                          double scale_coeff) {
    for (auto& shared_state : shared_states_) {
        if (shared_state.is_same_texture(texture_filename)) {
            MyTexture new_texture(&shared_state, reflection_coeff, scale_coeff);
            textures_.push_back(new_texture);
            return &textures_.back();
        }
    }

    TextureSharedState new_shared_state(texture_filename);
    shared_states_.push_back(new_shared_state);

    MyTexture new_texture(&shared_states_.back(), reflection_coeff, scale_coeff);
    textures_.push_back(new_texture);
    return &textures_.back();
}


}  // namespace mrtp
