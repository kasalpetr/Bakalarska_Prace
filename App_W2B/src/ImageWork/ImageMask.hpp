#include <opencv2/opencv.hpp>
#include <../Detector/ShapeAndEdges.hpp>

class ImageMask
{
public:
    static cv::Mat createMask(const cv::Mat& img);
    static cv::Mat maskEdges(const cv::Mat& img, const std::vector<Edge>& edges);
    static cv::Mat createTransparentResidual(const cv::Mat& img, const cv::Mat& maskedImg);
private:
    static cv::Mat maskShape(const cv::Mat& img, const std::string& filePath);
    static cv::Mat maskText(const cv::Mat& img, const std::string& filePath);
    static cv::Mat extractForegroundMask(const cv::Mat& img);
    static cv::Scalar classifyResidualColor(const cv::Scalar& avgBgr);
};