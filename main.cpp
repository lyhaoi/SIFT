#include <iostream>

#include <vector>
#include <string>

#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/program_options.hpp>

#include "sift.hpp"
#include "interestpoint.hpp"

namespace po = boost::program_options;

/*
* Main Function takes a greyvalue image as input
*/
int main(int argc, char** argv) {
    std::string img_file;
    f32_t sigma, k; 
    u16_t octaves, dogsPerEpoch; 

    po::options_description desc("Options");

    desc.add_options() 
      ("help", "Print help messages") 
      ("img,i", po::value<std::string>(&img_file), "The image on which sift will be executed")
      ("sigma,s", po::value<f32_t>(&sigma)->default_value(1.6), "The sigma value of the Gaussian calculations")
      ("k,k", po::value<f32_t>(&k)->default_value(std::sqrt(2)), "The constant which is calculated on sigma for the DoGs")
      ("octaves,o", po::value<u16_t>(&octaves)->default_value(4), "How many octaves should be calculated")
      ("dogsPerEpoch,d", po::value<u16_t>(&dogsPerEpoch)->default_value(3), "How many DoGs should be created per epoch")
      ;  
    po::positional_options_description p; 
    p.add("img", 1);
    po::variables_map vm; 
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm); 
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }

        vigra::ImageImportInfo info(img_file.c_str());
        vigra::MultiArray<2, f32_t> img(vigra::Shape2(info.shape()));
        vigra::importImage(info, img);

        sift::Sift sift(dogsPerEpoch, octaves, sigma, k);
        std::vector<sift::InterestPoint> interestPoints = sift.calculate(img);

        auto image = cv::imread(img_file.c_str(), CV_LOAD_IMAGE_COLOR);
        for (const sift::InterestPoint& p : interestPoints) {
            u16_t x = p.loc.x * std::pow(2, p.octave);
            u16_t y = p.loc.y * std::pow(2, p.octave);
            cv::RotatedRect r(cv::Point2f(x, y), 
                              cv::Size(p.scale * 10, p.scale * 10),
                              *(p.orientation.begin()));
            cv::Point2f points[4]; 
            r.points( points );
            cv::line(image, points[0], points[1], cv::Scalar(255, 0, 0));
            cv::line(image, points[0], points[3], cv::Scalar(255, 0, 0));
            cv::line(image, points[2], points[3], cv::Scalar(255, 0, 0));
            cv::line(image, points[1], points[2], cv::Scalar(255, 0, 0));
        }

        cv::imwrite(img_file + "_orientation.png", image);

    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    
    return 0;
}
