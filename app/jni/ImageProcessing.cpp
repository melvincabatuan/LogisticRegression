#include "io_github_melvincabatuan_logisticregression_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>
#include <stdlib.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>

 

using namespace std;
using namespace cv;
using namespace cv::ml;

/*
 * Class:     io_github_melvincabatuan_logisticregression_MainActivity
 * Method:    predict
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_io_github_melvincabatuan_logisticregression_MainActivity_predict
  (JNIEnv * pEnv, jobject pClass, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat srcGray(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);

/***************************************************************************************************/
    /// Native Image Processing HERE... 
  
    /// Resize srcGray to 28 x 28 image
     Mat gray28;
     resize(srcGray, gray28, Size(28,28));  // INTER_LINEAR default

    /// Vectorize: 1 x 784
     Mat vecGray = gray28.reshape(0,1); // (int channels, int rows)

    /// Convert to CV_32F
     vecGray.convertTo(vecGray, CV_32F);

     Mat result = Mat::ones(1, 1, CV_32S) * (-1); // Stores prediction result (2 to make it not zero or one) 
    /// Load classifier
     try
     {
           char path[100];  
           sprintf( path, "%s/%s", getenv("ASSETDIR"), "classifier.xml");

           /// Check path
           //__android_log_print(ANDROID_LOG_VERBOSE, "LogisticRegression", "The value of path is %s", path);

           Ptr<LogisticRegression> logReg = StatModel::load<LogisticRegression>(path);

           /// Predict whether 0 or 1
           logReg->predict(vecGray, result);
           __android_log_print(ANDROID_LOG_VERBOSE, "LogisticRegression", "The prediction is %f", result.at<float>(0,0));
     }
     catch (exception& e)
     {

     }

    /// If result is 0 or 1, encode it 
        if (result.at<float>(0,0) == 0){
            putText(srcGray, "0", Point( bitmapInfo.width/2, bitmapInfo.height/2), FONT_HERSHEY_COMPLEX, 1, Scalar(0,0,255),1,1); 
        }
        else if (result.at<float>(0,0) == 1)
	{
            putText(srcGray, "1", Point( bitmapInfo.width/2, bitmapInfo.height/2), FONT_HERSHEY_COMPLEX, 1, Scalar(255,0,0),1,1); 
        }
 
    /// Return orig srcGray
        cvtColor(srcGray, mbgra, CV_GRAY2BGRA);

/***************************************************************************************************/    
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();
}
