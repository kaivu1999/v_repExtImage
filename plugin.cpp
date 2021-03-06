#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "v_repPlusPlus/Plugin.h"
#include "plugin.h"
#include "stubs.h"
#include "config.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

std::map<int, cv::VideoCapture> videoCapture;

//#define SIMD_OPENCV_ENABLE
//#include <Simd/SimdLib.hpp>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>

class Image
{
private:
    static int nextId;
    static std::map<int, Image*> idMap;

public:
    int id;
    cv::Mat mat;

public:
    Image(cv::Mat mat_)
        : id(nextId++), mat(mat_)
    {
        idMap[id] = this;
    }

    ~Image()
    {
        idMap.erase(id);
    }

    static Image * byId(int id)
    {
        std::map<int, Image*>::iterator it = idMap.find(id);
        if(it == idMap.end()) return 0L;
        else return it->second;
    }

    static void getAllIds(std::vector<int> &v)
    {
        for(std::map<int, Image*>::iterator it = idMap.begin(); it != idMap.end(); ++it)
            v.push_back(it->first);
    }
};

int Image::nextId = 1;
std::map<int, Image*> Image::idMap;

cv::Point asPoint(const std::vector<int> &v)
{
    return cv::Point(v[0], v[1]);
}

cv::Size asSize(const std::vector<int> &v)
{
    return cv::Size(v[0], v[1]);
}

cv::Rect asRect(const std::vector<int> &p, const std::vector<int> &sz)
{
    return cv::Rect(p[0], p[1], sz[0], sz[1]);
}

cv::Scalar asRGB(const std::vector<int> &v)
{
    return CV_RGB(v[0], v[1], v[2]);
}

void toVector(const cv::Point &p, std::vector<int> &v)
{
    v.resize(2);
    v[0] = p.x;
    v[1] = p.y;
}

int parseFormat(int f, int def)
{
    switch(f)
    {
    case sim_im_fmt_8UC1:
        return CV_8UC1;
    case sim_im_fmt_8UC3:
        return CV_8UC3;
    case sim_im_fmt_8UC4:
        return CV_8UC4;
    case sim_im_fmt_32FC1:
        return CV_32FC1;
    case sim_im_fmt_32FC3:
        return CV_32FC3;
    case sim_im_fmt_32FC4:
        return CV_32FC4;
    }
    return def;
}

void create(SScriptCallBack *p, const char *cmd, create_in *in, create_out *out)
{
    if(in->width <= 0) throw std::runtime_error("invalid width");
    if(in->height <= 0) throw std::runtime_error("invalid height");
    int format = parseFormat(in->format, CV_8UC3);
    out->handle = (new Image(
            in->initialValue
            ? cv::Mat::ones(in->height, in->width, format) * in->initialValue
            : cv::Mat::zeros(in->height, in->width, format)
    ))->id;
}

void createFromData(SScriptCallBack *p, const char *cmd, createFromData_in *in, createFromData_out *out)
{
    if(in->width <= 0) throw std::runtime_error("invalid width");
    if(in->height <= 0) throw std::runtime_error("invalid height");
    int format = parseFormat(in->format, CV_8UC3);
    cv::Mat tmp(in->height, in->width, format, (void*)in->data.c_str());
    Image *img = new Image(cv::Mat());
    cv::cvtColor(tmp, img->mat, CV_RGB2BGR);
    out->handle = img->id;
}

void destroy(SScriptCallBack *p, const char *cmd, destroy_in *in, destroy_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    delete img;
}

void read(SScriptCallBack *p, const char *cmd, read_in *in, read_out *out)
{
    cv::Mat mat = cv::imread(in->filename, CV_LOAD_IMAGE_COLOR);
    if(!mat.data) throw std::runtime_error("invalid image");
    out->handle = (new Image(mat))->id;
}

void write(SScriptCallBack *p, const char *cmd, write_in *in, write_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::imwrite(in->filename, img->mat);
}

void convert(SScriptCallBack *p, const char *cmd, convert_in *in, convert_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    int format = parseFormat(in->format, CV_8UC3);
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    img->mat.convertTo(dstImg->mat, format, in->scale);
    out->handle = dstImg->id;
}

void rgb2gray(SScriptCallBack *p, const char *cmd, rgb2gray_in *in, rgb2gray_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_RGB2GRAY);
    out->handle = dstImg->id;
}

void gray2rgb(SScriptCallBack *p, const char *cmd, gray2rgb_in *in, gray2rgb_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_GRAY2RGB);
    out->handle = dstImg->id;
}

void rgb2hsv(SScriptCallBack *p, const char *cmd, rgb2hsv_in *in, rgb2hsv_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_RGB2HSV);
    out->handle = dstImg->id;
}

void hsv2rgb(SScriptCallBack *p, const char *cmd, hsv2rgb_in *in, hsv2rgb_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_HSV2RGB);
    out->handle = dstImg->id;
}

void rgb2hls(SScriptCallBack *p, const char *cmd, rgb2hls_in *in, rgb2hls_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_RGB2HLS);
    out->handle = dstImg->id;
}

void hls2rgb(SScriptCallBack *p, const char *cmd, hls2rgb_in *in, hls2rgb_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::cvtColor(img->mat, dstImg->mat, CV_HLS2RGB);
    out->handle = dstImg->id;
}

void split(SScriptCallBack *p, const char *cmd, split_in *in, split_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    const int ch = img->mat.channels();
    if(ch == 1) throw std::runtime_error("not a multichannel image");
    cv::Mat *dst = new cv::Mat[ch];
    cv::split(img->mat, &dst[0]);
    for(size_t i = 0; i < ch; i++)
        out->handles.push_back((new Image(dst[i]))->id);
    delete[] dst;
}

void merge(SScriptCallBack *p, const char *cmd, merge_in *in, merge_out *out)
{
    std::vector<cv::Mat> srcv;
    for(size_t i = 0; i < in->handles.size(); i++)
    {
        Image *img = Image::byId(in->handles[i]);
        if(!img) throw std::runtime_error((boost::format("invalid channel %d handle") % i).str());
        srcv.push_back(img->mat);
    }
    if(srcv.size() < 2) throw std::runtime_error("invalid number of channels");
    Image *img = new Image(cv::Mat());
    cv::merge(&srcv[0], srcv.size(), img->mat);
    out->handle = img->id;
}

void mixChannels(SScriptCallBack *p, const char *cmd, mixChannels_in *in, mixChannels_out *out)
{
    std::vector<cv::Mat> srcv;
    for(size_t i = 0; i < in->inputHandles.size(); i++)
    {
        Image *img = Image::byId(in->inputHandles[i]);
        if(!img) throw std::runtime_error((boost::format("invalid input image %d handle") % i).str());
        srcv.push_back(img->mat);
    }
    std::vector<cv::Mat> dstv;
    for(size_t i = 0; i < in->outputHandles.size(); i++)
    {
        Image *img = Image::byId(in->outputHandles[i]);
        if(!img) throw std::runtime_error((boost::format("invalid output image %d handle") % i).str());
        dstv.push_back(img->mat);
    }
    cv::mixChannels(&srcv[0], srcv.size(), &dstv[0], dstv.size(), &in->fromTo[0], in->fromTo.size());
}

void get(SScriptCallBack *p, const char *cmd, get_in *in, get_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    switch(img->mat.depth())
    {
    case CV_8U:
        for(size_t i = 0; i < img->mat.channels(); i++)
            out->value.push_back(img->mat.at<cv::Vec3b>(in->coord[1], in->coord[0])[i]);
        break;
    case CV_32F:
        for(size_t i = 0; i < img->mat.channels(); i++)
            out->value.push_back(img->mat.at<cv::Vec3f>(in->coord[1], in->coord[0])[i]);
        break;
    default:
        throw std::runtime_error("unsupported channel type");
    }
}

void set(SScriptCallBack *p, const char *cmd, set_in *in, set_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    if(in->value.size() != img->mat.channels()) throw std::runtime_error("invalid pixel size");
    switch(img->mat.depth())
    {
    case CV_8U:
        for(size_t i = 0; i < img->mat.channels(); i++)
            img->mat.at<cv::Vec3b>(in->coord[1], in->coord[0])[i] = in->value[i];
        break;
    case CV_32F:
        for(size_t i = 0; i < img->mat.channels(); i++)
            img->mat.at<cv::Vec3f>(in->coord[1], in->coord[0])[i] = in->value[i];
        break;
    default:
        throw std::runtime_error("unsupported channel type");
    }
}

int parseInterp(int i, int def)
{
    switch(i)
    {
    case sim_im_interp_nearest:
        return cv::INTER_NEAREST;
    case sim_im_interp_linear:
        return cv::INTER_LINEAR;
    case sim_im_interp_area:
        return cv::INTER_AREA;
    case sim_im_interp_cubic:
        return cv::INTER_CUBIC;
    case sim_im_interp_lanczos4:
        return cv::INTER_LANCZOS4;
    }
    return def;
}

void resize(SScriptCallBack *p, const char *cmd, resize_in *in, resize_out *out)
{
    if(in->width <= 0) throw std::runtime_error("invalid width");
    if(in->height <= 0) throw std::runtime_error("invalid height");
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    int interp = parseInterp(in->interpolation, sim_im_interp_linear);
    cv::resize(img->mat, dstImg->mat, cv::Size(in->width, in->height), 0, 0, interp);
    out->handle = dstImg->id;
}

void size(SScriptCallBack *p, const char *cmd, size_in *in, size_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    out->size.resize(2);
    out->size[0] = img->mat.cols;
    out->size[1] = img->mat.rows;
}

void copy(SScriptCallBack *p, const char *cmd, copy_in *in, copy_out *out)
{
    if(in->size[0] <= 0) throw std::runtime_error("invalid width");
    if(in->size[1] <= 0) throw std::runtime_error("invalid height");
    Image *srcImg = Image::byId(in->srcHandle);
    if(!srcImg) throw std::runtime_error("invalid source image handle");
    Image *dstImg = Image::byId(in->dstHandle);
    if(!dstImg) throw std::runtime_error("invalid destination image handle");
    cv::Mat src = srcImg->mat(asRect(in->srcOffset, in->size));
    cv::Mat dst = dstImg->mat(asRect(in->dstOffset, in->size));
    src.copyTo(dst);
}

void clipLine(SScriptCallBack *p, const char *cmd, clipLine_in *in, clipLine_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::Point p1(asPoint(in->p1)), p2(asPoint(in->p2));
    out->valid = cv::clipLine(cv::Rect(0, 0, img->mat.cols, img->mat.rows), p1, p2);
    toVector(p1, out->p1);
    toVector(p2, out->p2);
}

void line(SScriptCallBack *p, const char *cmd, line_in *in, line_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::line(img->mat, asPoint(in->p1), asPoint(in->p2), asRGB(in->color), in->thickness, in->type, in->shift);
}

void arrowedLine(SScriptCallBack *p, const char *cmd, arrowedLine_in *in, arrowedLine_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
#ifdef HAVE_CV_ARROWEDLINE
    cv::arrowedLine(img->mat, asPoint(in->p1), asPoint(in->p2), asRGB(in->color), in->thickness, in->type, in->shift, in->tipLength);
#else
    throw std::runtime_error("cv::arrowedLine not available in current version of OpenCV");
#endif
}

void polylines(SScriptCallBack *p, const char *cmd, polylines_in *in, polylines_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    int sum = 0;
    for(size_t i = 0; i < in->numPoints.size(); i++) sum += 2 * in->numPoints[i];
    if(sum != in->points.size()) throw std::runtime_error("invalid number of points or invalid elements in numPoints");
    std::vector<cv::Point> points;
    for(size_t i = 0; i < in->points.size(); i += 2)
        points.push_back(cv::Point(in->points[i], in->points[i+1]));
    const cv::Point **pts = new const cv::Point*[in->numPoints.size()];
    const cv::Point *pt = &points[0];
    for(size_t i = 0; i < in->numPoints.size(); i++)
    {
        pts[i] = pt;
        pt += in->numPoints[i];
    }
    cv::polylines(img->mat, pts, &in->numPoints[0], in->numPoints.size(), in->isClosed, asRGB(in->color), in->thickness, in->type, in->shift);
    delete[] pts;
}

void rectangle(SScriptCallBack *p, const char *cmd, rectangle_in *in, rectangle_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::rectangle(img->mat, asPoint(in->p1), asPoint(in->p2), asRGB(in->color), in->thickness, in->type, in->shift);
}

void circle(SScriptCallBack *p, const char *cmd, circle_in *in, circle_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::circle(img->mat, asPoint(in->center), in->radius, asRGB(in->color), in->thickness, in->type, in->shift);
}

void ellipse(SScriptCallBack *p, const char *cmd, ellipse_in *in, ellipse_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    cv::ellipse(img->mat, asPoint(in->center), asSize(in->radius), in->angle, in->startAngle, in->endAngle, asRGB(in->color), in->thickness, in->type, in->shift);
}

void fillPoly(SScriptCallBack *p, const char *cmd, fillPoly_in *in, fillPoly_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    int sum = 0;
    for(size_t i = 0; i < in->numPoints.size(); i++) sum += 2 * in->numPoints[i];
    if(sum != in->points.size()) throw std::runtime_error("invalid number of points or invalid elements in numPoints");
    std::vector<cv::Point> points;
    for(size_t i = 0; i < in->points.size(); i += 2)
        points.push_back(cv::Point(in->points[i], in->points[i+1]));
    const cv::Point **pts = new const cv::Point*[in->numPoints.size()];
    const cv::Point *pt = &points[0];
    for(size_t i = 0; i < in->numPoints.size(); i++)
    {
        pts[i] = pt;
        pt += in->numPoints[i];
    }
    cv::fillPoly(img->mat, pts, &in->numPoints[0], in->numPoints.size(), asRGB(in->color), in->type, in->shift, asPoint(in->offset));
    delete[] pts;
}

void fillConvexPoly(SScriptCallBack *p, const char *cmd, fillConvexPoly_in *in, fillConvexPoly_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    std::vector<cv::Point> points;
    for(size_t i = 0; i < in->points.size(); i += 2)
        points.push_back(cv::Point(in->points[i], in->points[i+1]));
    cv::fillConvexPoly(img->mat, &points[0], points.size(), asRGB(in->color), in->type, in->shift);
}

int parseFontFace(int f, int def)
{
    switch(f)
    {
    case sim_im_fontface_simplex:
        return cv::FONT_HERSHEY_SIMPLEX;
    case sim_im_fontface_plain:
        return cv::FONT_HERSHEY_PLAIN;
    case sim_im_fontface_duplex:
        return cv::FONT_HERSHEY_DUPLEX;
    case sim_im_fontface_complex:
        return cv::FONT_HERSHEY_COMPLEX;
    case sim_im_fontface_triplex:
        return cv::FONT_HERSHEY_TRIPLEX;
    case sim_im_fontface_complex_small:
        return cv::FONT_HERSHEY_COMPLEX_SMALL;
    case sim_im_fontface_script_simplex:
        return cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    case sim_im_fontface_script_complex:
        return cv::FONT_HERSHEY_SCRIPT_COMPLEX;
    default:
        return def;
    }
}

void text(SScriptCallBack *p, const char *cmd, text_in *in, text_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    int ff = parseFontFace(in->fontFace, cv::FONT_HERSHEY_SIMPLEX) | (in->italic ? cv::FONT_ITALIC : 0);
    cv::putText(img->mat, in->str, asPoint(in->pos), ff, in->fontScale, asRGB(in->color), in->thickness, in->type, in->bottomLeftOrigin);
}

void textSize(SScriptCallBack *p, const char *cmd, textSize_in *in, textSize_out *out)
{
    int ff = parseFontFace(in->fontFace, cv::FONT_HERSHEY_SIMPLEX) | (in->italic ? cv::FONT_ITALIC : 0);
    cv::Size sz = cv::getTextSize(in->str, ff, in->fontScale, in->thickness, &out->baseline);
    out->width = sz.width;
    out->height = sz.height;
}

void abs(SScriptCallBack *p, const char *cmd, abs_in *in, abs_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    dstImg->mat = cv::abs(img->mat);
    out->handle = dstImg->id;
}

void absdiff(SScriptCallBack *p, const char *cmd, absdiff_in *in, absdiff_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::absdiff(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void absdiffK(SScriptCallBack *p, const char *cmd, absdiffK_in *in, absdiffK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::absdiff(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void add(SScriptCallBack *p, const char *cmd, add_in *in, add_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::add(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void addK(SScriptCallBack *p, const char *cmd, addK_in *in, addK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::add(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void subtract(SScriptCallBack *p, const char *cmd, subtract_in *in, subtract_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::subtract(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void subtractK(SScriptCallBack *p, const char *cmd, subtractK_in *in, subtractK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::subtract(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void multiply(SScriptCallBack *p, const char *cmd, multiply_in *in, multiply_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::multiply(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void divide(SScriptCallBack *p, const char *cmd, divide_in *in, divide_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::divide(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void divideK(SScriptCallBack *p, const char *cmd, divideK_in *in, divideK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::divide(in->k, img->mat, dstImg->mat);
    out->handle = dstImg->id;
}

int parseCmpOp(int o, int def)
{
    switch(o)
    {
    case sim_im_cmpop_eq:
        return cv::CMP_EQ;
    case sim_im_cmpop_gt:
        return cv::CMP_GT;
    case sim_im_cmpop_ge:
        return cv::CMP_GE;
    case sim_im_cmpop_lt:
        return cv::CMP_LT;
    case sim_im_cmpop_le:
        return cv::CMP_LE;
    case sim_im_cmpop_ne:
        return cv::CMP_NE;
    default:
        return def;
    }
}

void compare(SScriptCallBack *p, const char *cmd, compare_in *in, compare_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    int op = parseCmpOp(in->op, cv::CMP_EQ);
    cv::compare(img1->mat, img2->mat, dstImg->mat, op);
    out->handle = dstImg->id;
}

void compareK(SScriptCallBack *p, const char *cmd, compareK_in *in, compareK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    int op = parseCmpOp(in->op, cv::CMP_EQ);
    cv::compare(img->mat, in->k, dstImg->mat, op);
    out->handle = dstImg->id;
}

int parseReduceOp(int o, int def)
{
    switch(o)
    {
    case sim_im_reduceop_sum:
        return CV_REDUCE_SUM;
    case sim_im_reduceop_avg:
        return CV_REDUCE_AVG;
    case sim_im_reduceop_max:
        return CV_REDUCE_MAX;
    case sim_im_reduceop_min:
        return CV_REDUCE_MIN;
    default:
        return def;
    }
}

void reduce(SScriptCallBack *p, const char *cmd, reduce_in *in, reduce_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    int op = parseReduceOp(in->op, CV_REDUCE_SUM);
    cv::reduce(img->mat, dstImg->mat, in->dim, op);
    out->handle = dstImg->id;
}

void repeat(SScriptCallBack *p, const char *cmd, repeat_in *in, repeat_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::repeat(img->mat, in->ny, in->nx, dstImg->mat);
    out->handle = dstImg->id;
}

int parseFlipOp(int o, int def)
{
    switch(o)
    {
    case sim_im_flipop_x:
        return 1;
    case sim_im_flipop_y:
        return 0;
    case sim_im_flipop_both:
        return -1;
    default:
        return def;
    }
}

void flip(SScriptCallBack *p, const char *cmd, flip_in *in, flip_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    int op = parseFlipOp(in->op, 0);
    cv::flip(img->mat, dstImg->mat, op);
    out->handle = dstImg->id;
}

void log(SScriptCallBack *p, const char *cmd, log_in *in, log_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::log(img->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void exp(SScriptCallBack *p, const char *cmd, exp_in *in, exp_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::exp(img->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void pow(SScriptCallBack *p, const char *cmd, pow_in *in, pow_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::pow(img->mat, in->power, dstImg->mat);
    out->handle = dstImg->id;
}

void sqrt(SScriptCallBack *p, const char *cmd, sqrt_in *in, sqrt_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::sqrt(img->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void addWeighted(SScriptCallBack *p, const char *cmd, addWeighted_in *in, addWeighted_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::addWeighted(img1->mat, in->alpha, img2->mat, in->beta, in->gamma, dstImg->mat);
    out->handle = dstImg->id;
}

void scaleAdd(SScriptCallBack *p, const char *cmd, scaleAdd_in *in, scaleAdd_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::scaleAdd(img1->mat, in->alpha, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void magnitude(SScriptCallBack *p, const char *cmd, magnitude_in *in, magnitude_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = new Image(cv::Mat());
    cv::magnitude(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void phase(SScriptCallBack *p, const char *cmd, phase_in *in, phase_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = new Image(cv::Mat());
    cv::phase(img1->mat, img2->mat, dstImg->mat, in->angleInDegrees);
    out->handle = dstImg->id;
}

void polar2cart(SScriptCallBack *p, const char *cmd, polar2cart_in *in, polar2cart_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg1 = new Image(cv::Mat());
    Image *dstImg2 = new Image(cv::Mat());
    cv::cartToPolar(img1->mat, img2->mat, dstImg1->mat, dstImg2->mat, in->angleInDegrees);
    out->handle1 = dstImg1->id;
    out->handle2 = dstImg2->id;
}

void cart2polar(SScriptCallBack *p, const char *cmd, cart2polar_in *in, cart2polar_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg1 = new Image(cv::Mat());
    Image *dstImg2 = new Image(cv::Mat());
    cv::cartToPolar(img1->mat, img2->mat, dstImg1->mat, dstImg2->mat, in->angleInDegrees);
    out->handle1 = dstImg1->id;
    out->handle2 = dstImg2->id;
}

void bitwiseAnd(SScriptCallBack *p, const char *cmd, bitwiseAnd_in *in, bitwiseAnd_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::bitwise_and(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseAndK(SScriptCallBack *p, const char *cmd, bitwiseAndK_in *in, bitwiseAndK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::bitwise_and(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseOr(SScriptCallBack *p, const char *cmd, bitwiseOr_in *in, bitwiseOr_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::bitwise_or(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseOrK(SScriptCallBack *p, const char *cmd, bitwiseOrK_in *in, bitwiseOrK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::bitwise_or(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseXor(SScriptCallBack *p, const char *cmd, bitwiseXor_in *in, bitwiseXor_out *out)
{
    Image *img1 = Image::byId(in->handle1);
    if(!img1) throw std::runtime_error("invalid image 1 handle");
    Image *img2 = Image::byId(in->handle2);
    if(!img2) throw std::runtime_error("invalid image 2 handle");
    Image *dstImg = in->inPlace ? img1 : new Image(cv::Mat());
    cv::bitwise_xor(img1->mat, img2->mat, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseXorK(SScriptCallBack *p, const char *cmd, bitwiseXorK_in *in, bitwiseXorK_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::bitwise_xor(img->mat, in->k, dstImg->mat);
    out->handle = dstImg->id;
}

void bitwiseNot(SScriptCallBack *p, const char *cmd, bitwiseNot_in *in, bitwiseNot_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    cv::bitwise_not(img->mat, dstImg->mat);
    out->handle = dstImg->id;
}

int parseDistanceType(int d, int def)
{
    switch(d)
    {
    case sim_im_dist_L1:
        return CV_DIST_L1;
    case sim_im_dist_L2:
        return CV_DIST_L2;
    case sim_im_dist_C:
        return CV_DIST_C;
    }
    return def;
}

int parseMaskSize(int m, int def)
{
    switch(m)
    {
    case sim_im_masksize_3x3:
        return 3;
    case sim_im_masksize_5x5:
        return 5;
    case sim_im_masksize_precise:
        return CV_DIST_MASK_PRECISE;
    }
    return def;
}

void distanceTransform(SScriptCallBack *p, const char *cmd, distanceTransform_in *in, distanceTransform_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");
    Image *dstImg = in->inPlace ? img : new Image(cv::Mat());
    int dt = parseDistanceType(in->distanceType, CV_DIST_L2);
    int ms = parseMaskSize(in->maskSize, CV_DIST_MASK_PRECISE);
    cv::distanceTransform(img->mat, dstImg->mat, dt, ms);
    out->handle = dstImg->id;
}

void handles(SScriptCallBack *p, const char *cmd, handles_in *in, handles_out *out)
{
    Image::getAllIds(out->handles);
}

void writeToVisionSensor(SScriptCallBack *p, const char *cmd, writeToVisionSensor_in *in, writeToVisionSensor_out *out)
{
    Image *img = Image::byId(in->handle);
    if(!img) throw std::runtime_error("invalid image handle");

    simInt resolution[2];
    if(-1 == simGetVisionSensorResolution(in->sensorHandle, &resolution[0]))
        throw std::runtime_error("failed to get sensor resolution");

    if(img->mat.cols != resolution[0] || img->mat.rows != resolution[1])
        throw std::runtime_error((boost::format("sensor resolution (%dx%d) does not match image size (%dx%d)") % resolution[0] % resolution[1] % img->mat.cols % img->mat.rows).str());

    cv::Mat tmp;
    cv::cvtColor(img->mat, tmp, CV_BGR2RGB);
    if(-1 == simSetVisionSensorCharImage(in->sensorHandle, (const simUChar*)tmp.data))
        throw std::runtime_error("failed to write to vision sensor");
}

void readFromVisionSensor(SScriptCallBack *p, const char *cmd, readFromVisionSensor_in *in, readFromVisionSensor_out *out)
{
    simInt resolution[2];
    if(-1 == simGetVisionSensorResolution(in->sensorHandle, &resolution[0]))
        throw std::runtime_error("failed to get sensor resolution");

    Image *img = in->handle != -1 ? Image::byId(in->handle) : new Image(cv::Mat::zeros(resolution[1], resolution[0], CV_8UC3));
    if(!img) throw std::runtime_error("invalid image handle");

    if(img->mat.cols != resolution[0] || img->mat.rows != resolution[1])
    {
        if(in->handle == -1) delete img;
        throw std::runtime_error((boost::format("sensor resolution (%dx%d) does not match image size (%dx%d)") % resolution[0] % resolution[1] % img->mat.cols % img->mat.rows).str());
    }

    simUChar* data = simGetVisionSensorCharImage(in->sensorHandle, &resolution[0], &resolution[1]);
    if(data)
    {
        cv::Mat(resolution[1], resolution[0], CV_8UC3, data).copyTo(img->mat);
        simReleaseBuffer(reinterpret_cast<simChar*>(data));
        out->handle = img->id;
    }
    else
    {
        if(in->handle == -1) delete img;
        out->handle = -1;
        throw std::runtime_error("failed to read from vision sensor");
    }
}

void openVideoCapture(SScriptCallBack *p, const char *cmd, openVideoCapture_in *in, openVideoCapture_out *out)
{
    if(!videoCapture[in->deviceIndex].isOpened())
        videoCapture[in->deviceIndex].open(in->deviceIndex);

    if(!videoCapture[in->deviceIndex].isOpened())
        throw std::runtime_error("failed to open device");
}

void closeVideoCapture(SScriptCallBack *p, const char *cmd, closeVideoCapture_in *in, closeVideoCapture_out *out)
{
    std::map<int, cv::VideoCapture>::iterator it = videoCapture.find(in->deviceIndex);
    if(it == videoCapture.end())
        throw std::runtime_error("invalid device. did you call simIM.openVideoCapture() first?");

    if(!videoCapture[in->deviceIndex].isOpened())
        throw std::runtime_error("device is not opened");

    videoCapture[in->deviceIndex].release();
}

void readFromVideoCapture(SScriptCallBack *p, const char *cmd, readFromVideoCapture_in *in, readFromVideoCapture_out *out)
{
    std::map<int, cv::VideoCapture>::iterator it = videoCapture.find(in->deviceIndex);
    if(it == videoCapture.end() || !videoCapture[in->deviceIndex].isOpened())
        throw std::runtime_error("invalid device. did you call simIM.openVideoCapture() first?");

    Image *img = in->handle != -1 ? Image::byId(in->handle) : new Image(cv::Mat());
    if(!img) throw std::runtime_error("invalid image handle");

    if(videoCapture[in->deviceIndex].read(img->mat))
    {
        out->handle = img->id;
    }
    else
    {
        if(in->handle == -1) delete img;
        out->handle = -1;
        throw std::runtime_error("failed to read video frame");
    }
}

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        simSetModuleInfo(PLUGIN_NAME, 0, "OpenCV-based Image Processing Plugin", 0);
        simSetModuleInfo(PLUGIN_NAME, 1, BUILD_DATE, 0);
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
