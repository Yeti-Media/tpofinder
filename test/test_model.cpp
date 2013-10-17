#include "test.h"
#include "tpofinder/configure.h"
#include "tpofinder/model.h"
#include "tpofinder/util.h"
#include "tpofinder/visualize.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace tpofinder;

struct model_view : public ::testing::Test {

    virtual void SetUp() {
        Mat ref = imread(PROJECT_BINARY_DIR + "/data/blokus/ref.jpg");
        Mat roi = imread(PROJECT_BINARY_DIR + "/data/blokus/roi.png", 0);
        ASSERT_FALSE(ref.empty());
        ASSERT_FALSE(roi.empty());
        view = PlanarView::create(ref, roi);
    }

    PlanarView view;

};

struct model_adapter : public ::testing::Test {

    virtual void SetUp() {
        adapterModel = PlanarModel::load(PROJECT_BINARY_DIR + "/data/adapter");
    }

    PlanarModel adapterModel;

};

struct model_blokus : public ::testing::Test {

    virtual void SetUp() {
        blokusModel = PlanarModel::load(PROJECT_BINARY_DIR + "/data/blokus");
    }

    PlanarModel blokusModel;

};

struct model_stockholm : public ::testing::Test {

    virtual void SetUp() {
        stockholmModel = PlanarModel::load(PROJECT_BINARY_DIR + "/data/stockholm");
    }

    PlanarModel stockholmModel;

};

struct model_taco : public ::testing::Test {

    virtual void SetUp() {
        tacoModel = PlanarModel::load(PROJECT_BINARY_DIR + "/data/taco");
    }

    PlanarModel tacoModel;

};

struct model_tea : public ::testing::Test {

    virtual void SetUp() {
        teaModel = PlanarModel::load(PROJECT_BINARY_DIR + "/data/tea");
    }

    PlanarModel teaModel;

};

struct model_simple : public ::testing::Test {

    virtual void SetUp() {
        Mat ref = imread(PROJECT_BINARY_DIR + "/data/blokus/ref.jpg");
        Mat roi = imread(PROJECT_BINARY_DIR + "/data/blokus/roi.png", 0);
        ASSERT_FALSE(ref.empty());
        ASSERT_FALSE(roi.empty());
        simpleModel = PlanarModel::create("blokus", ref, roi);
    }

    PlanarModel simpleModel;

};

struct model_base : public ::testing::Test {

    virtual void SetUp() {
        modelbase.add(PROJECT_BINARY_DIR + "/data/adapter");
        modelbase.add(PROJECT_BINARY_DIR + "/data/blokus");
        modelbase.add(PROJECT_BINARY_DIR + "/data/stockholm");
        modelbase.add(PROJECT_BINARY_DIR + "/data/taco");
        modelbase.add(PROJECT_BINARY_DIR + "/data/tea");
    }

    Modelbase modelbase;

};

struct model_homography_app : public ::testing::Test {

};

void viewModel(const string& test, const PlanarModel& model) {
    for (size_t i = 0; i < model.views.size(); i++) {
        Mat out = blend(model.views[0].image,
                model.views[i].image,
                model.views[i].homography);
        imshow(str(boost::format("%s.%03d") % test % i), out);
    }
}

TEST_F(model_view, createViewSiftCompatible) {
    Feature feature("SIFT", "SIFT", "BruteForce");
    PlanarView::create(view.image, view.roi, EYE_HOMOGRAPHY, feature);
}

TEST_F(model_view, createViewSurfCompatible) {
    Feature feature("SURF", "SURF", "BruteForce");
    PlanarView::create(view.image, view.roi, EYE_HOMOGRAPHY, feature);
}

TEST_F(model_view, hasKeypoints) {
    EXPECT_GE(view.keypoints.size(), 300);
}

TEST_F(model_view, roiIsSingleChannel) {
    EXPECT_EQ(view.roi.channels(), 1);
}

TEST_F(model_view, roiIsInteger) {
    EXPECT_EQ(view.roi.depth(), CV_8U);
}

TEST_F(model_view, roiIsBinary) {
    for (int i = 0; i < view.roi.rows; i++) {
        for (int j = 0; j < view.roi.cols; j++) {
            ASSERT_TRUE((view.roi.at<uint8_t > (i, j) == 0)
                    || (view.roi.at<uint8_t > (i, j) == 255));
        }
    }
}

TEST_F(model_view, keypointsConfinedToRoi) {

    BOOST_FOREACH(const KeyPoint& k, view.keypoints) {
        EXPECT_EQ(view.roi.at<uint8_t > (k.pt), 255);
    }
}

TEST_F(model_blokus, keypointsConfinedToRoi) {
    Mat se = Mat::ones(3, 3, CV_8UC1);

    BOOST_FOREACH(PlanarView& v, blokusModel.views) {
        // The image needs to be slightly dilated such that keypoints on the
        // boundary of the mask are not leading to test failure.
        Mat dilatedRoi;
        dilate(v.roi, dilatedRoi, se);

        BOOST_FOREACH(const KeyPoint& k, v.keypoints) {
            EXPECT_EQ(dilatedRoi.at<uint8_t > (k.pt), 255);
        }
    }
}

TEST_F(model_blokus, allKeypointsConfinedToReferenceRoi) {
    Mat se = Mat::ones(10, 10, CV_8UC1);
    Mat dilatedRoi;
    dilate(blokusModel.views[0].roi, dilatedRoi, se);

    BOOST_FOREACH(const KeyPoint& k, blokusModel.allKeypoints) {
        EXPECT_EQ(dilatedRoi.at<uint8_t > (k.pt), 255);
    }
}

TEST_F(model_adapter, modelViews) {
    viewModel("model_adapter.modelViews", adapterModel);
}

TEST_F(model_blokus, modelViews) {
    viewModel("model_blokus.modelViews", blokusModel);
}

TEST_F(model_stockholm, modelViews) {
    viewModel("model_stockholm.modelViews", stockholmModel);
}

TEST_F(model_taco, modelViews) {
    viewModel("model_taco.modelViews", tacoModel);
}

TEST_F(model_tea, modelViews) {
    viewModel("model_tea.modelViews", teaModel);
}

TEST_F(model_simple, redColor) {
    EXPECT_EQ(simpleModel.color, Scalar(0, 0, 255, 255));
}

TEST_F(model_blokus, greenColor) {
    EXPECT_EQ(blokusModel.color, Scalar(0, 255, 0, 255));
}

TEST_F(model_simple, allViewsLoaded) {
    EXPECT_EQ(simpleModel.views.size(), 1);
}

TEST_F(model_blokus, allViewsLoaded) {
    EXPECT_EQ(blokusModel.views.size(), 4);
}

TEST_F(model_simple, consistentFeatureNumber) {
    EXPECT_EQ(simpleModel.allKeypoints.size(), simpleModel.views[0].keypoints.size());
    EXPECT_EQ(simpleModel.allKeypoints.size(), simpleModel.allDescriptors.rows);
}

TEST_F(model_blokus, consistentFeatureNumber) {
    size_t s = 0;
    size_t t = 0;

    BOOST_FOREACH(PlanarView& v, blokusModel.views) {
        s += v.keypoints.size();
        t += v.descriptors.rows;
    }
    EXPECT_EQ(s, t);
    EXPECT_EQ(blokusModel.allKeypoints.size(), s);
    EXPECT_EQ(blokusModel.allDescriptors.rows, t);
}

TEST_F(model_blokus, homographyLoaded) {
    Mat h = readHomography(PROJECT_BINARY_DIR + "/data/blokus/001.yml");
    EXPECT_DOUBLE_EQ(norm(blokusModel.views[1].homography - h), 0);
}

TEST_F(model_blokus, visualizeReferenceView) {
    imshow("model_blokus.visualizeReferenceView", blokusModel.views[0].image);
}

TEST_F(model_blokus, visualizeReferenceViewRoi) {
    imshow("model_blokus.visualizeReferenceViewRoi", blokusModel.views[0].roi);
}

TEST_F(model_blokus, visualizeReferenceViewKeypoints) {
    Mat out;
    PlanarView v = blokusModel.views[0];
    drawKeypoints(v.roi, v.keypoints, out);
    imshow("model_blokus.visualizeReferenceViewKeypoints", out);
}

TEST_F(model_blokus, visualizeSecondView) {
    imshow("model_blokus.visualizeSecondView", blokusModel.views[1].image);
}

TEST_F(model_blokus, visualizeSecondViewRoi) {
    imshow("model_blokus.visualizeSecondViewRoi", blokusModel.views[1].roi);
}

TEST_F(model_blokus, visualizeSecondViewKeypoints) {
    Mat out;
    PlanarView& v = blokusModel.views[1];
    drawKeypoints(v.roi, v.keypoints, out);
    imshow("model_blokus.visualizeSecondViewKeypoints", out);
}

TEST_F(model_blokus, visualizeThirdViewKeypoints) {
    Mat out;
    PlanarView& v = blokusModel.views[2];
    drawKeypoints(v.roi, v.keypoints, out);
    imshow("model_blokus.visualizeThirdViewKeypoints", out);
}

TEST_F(model_blokus, visualizeFourthViewKeypoints) {
    Mat out;
    PlanarView& v = blokusModel.views[3];
    drawKeypoints(v.roi, v.keypoints, out);
    imshow("model_blokus.visualizeFourthViewKeypoints", out);
}


TEST_F(model_blokus, visualizeAllKeypoints) {
    Mat out;
    PlanarView v = blokusModel.views[0];
    drawKeypoints(v.roi, blokusModel.allKeypoints, out);
    imshow("model_blokus.visualizeAllKeypoints", out);
}

TEST_F(model_base, findByName) {
    EXPECT_EQ(0, modelbase.findByName("adapter"));
    EXPECT_EQ(1, modelbase.findByName("blokus"));
    EXPECT_EQ(2, modelbase.findByName("stockholm"));
    EXPECT_EQ(3, modelbase.findByName("taco"));
    EXPECT_EQ(4, modelbase.findByName("tea"));
    EXPECT_EQ(-1, modelbase.findByName("doesnotexist"));
}

TEST_F(model_homography_app, alternativeHomographyMatrix) {
    // The model_homography executable actually produces a homography that
    // looks fairly different from data/taco/001.yml, even though the
    // variability in the manual selection of keypoints is not very high.
    Mat h_org = readHomography(PROJECT_BINARY_DIR + "/data/taco/001.yml");
    Mat h_alt = readHomography(PROJECT_BINARY_DIR + "/data/test/alternative-taco-001.yml");

    Mat_<Point2f> src;
    src.push_back(Point2f(266, 307));
    src.push_back(Point2f(265, 301));
    src.push_back(Point2f(373, 302));
    src.push_back(Point2f(374, 208));

    Mat_<Point2f> dst_h_org, dst_h_alt;
    
    perspectiveTransform(src, dst_h_org, h_org);
    perspectiveTransform(src, dst_h_alt, h_alt);

    for (size_t i = 0; i < 4; i++) {
        Point2f p_org = dst_h_org.at<Point2f>(i);
        Point2f p_alt = dst_h_alt.at<Point2f>(i);

        EXPECT_NEAR(p_org.x, p_alt.x, 1);
        EXPECT_NEAR(p_org.y, p_alt.y, 1);
        // Comment out for displaying the pairs
        // std::cout << (boost::format("%3.0f %3.0f and %3.0f %3.0f") % p_org.x % p_org.y % p_alt.x % p_alt.y) << std::endl;
    }
}

