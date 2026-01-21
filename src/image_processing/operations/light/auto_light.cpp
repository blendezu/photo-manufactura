#include "auto_light.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/imgproc.hpp>

AutoLightSettings AutoLight::analyze(const cv::Mat& srcImg) {
    AutoLightSettings settings;

    if (srcImg.empty()) {
        return settings;
    }

    // 1. Prepare data for Analysis
    cv::Mat hslImg;
    cv::Mat processingSrc;

    // Convert depth to 8-bit if needed (16-bit -> 8-bit)
    if (srcImg.depth() == CV_16U) {
        srcImg.convertTo(processingSrc, CV_8U, 1.0 / 256.0);
    } else {
        processingSrc = srcImg;
    }

    // Convert to HSL if Color Image
    int channelIdx = 0;
    if (processingSrc.channels() == 3) {
        cv::cvtColor(processingSrc, hslImg, cv::COLOR_BGR2HLS);
        channelIdx = 1;
    } else {
        hslImg = processingSrc;
        channelIdx = 0;
    }

    // --- 2. Calculate Histogram & Percentiles (1%, 50%, 99%) ---
    // 2.1 Create the Histogram
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;

    // 2.2 Calculate Histogram specifically for the L-channel
    int channels[] = {channelIdx};
    cv::calcHist(&hslImg, 1, channels, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

    // 2.3 Search for Percentiles (1%, 50%, 99%)
    float totalPixels = hslImg.total();
    float sum = 0;
    int p01 = 0;  // 1st Percentile (Black point)
    int p50 = 0;  // Median (Mid point)
    int p99 = 0;  // 99th Percentile (White point)

    bool foundP01 = false;
    bool foundP50 = false;
    bool foundP99 = false;

    for (int i = 0; i < 256; i++) {
        sum += hist.at<float>(i);
        float percentage = sum / totalPixels;

        if (!foundP01 && percentage >= 0.01f) {
            p01 = i;
            foundP01 = true;
        }
        if (!foundP50 && percentage >= 0.50f) {
            p50 = i;
            foundP50 = true;
        }
        if (!foundP99 && percentage >= 0.99f) {
            p99 = i;
            foundP99 = true;
            break;
        }
    }

    std::cout << "P1: " << p01 << "; P99: " << p99 << std::endl;

    // --- Decision: Good Dynamic Range vs. Safe To Adjust ---
    // "Good DR": Black point is low enough (< safeBlack) AND White point is high enough (>
    // safeWhite). In this case, using Brightness to avoid changing Dynamic Range

    bool safeBlack = p01 < 40;
    bool safeWhite = p99 > 200;

    if (safeBlack && safeWhite) {
        // --- PATH A: Good DR -> Adjust Brightness (Midtone Curve) ---

        float currentMid = p50 / 255.0f;
        float targetMid = 0.5f;

        // Required shift
        float shift = targetMid - currentMid;

        // Brightness slider only 70.0 to avoid over correcting, mapped to approx -1.0 to 1.0 shift
        float brightnessVal = shift * 70.0f;

        // Clamp to safe range (-50 to +50 is usually enough for auto)
        settings.brightness = std::clamp(brightnessVal, -50.0f, 50.0f);

        // Don't touch exposure/shadows/highlights
        settings.exposure = 0.0f;
        settings.contrast = 0.0f;
        settings.shadow = 0.0f;
        settings.highlight = 0.0f;

    } else {
        // --- PATH B: Bad/Partial DR -> Adjust Exposure (Legacy Logic) ---

        // 3. Auto Exposure: Move Median (p50) to Middle Gray (around 127)
        if (p50 > 0) {
            float targetMean = 127.0f;
            float exposureDiff = std::log2(targetMean / (float)p50);

            // Clamp and attenuate the exposureDiff with 0.8 to avoid too strong reaction
            settings.exposure = std::clamp(exposureDiff * 0.8f, -2.0f, 2.0f);
        }

        // 4. Auto Contrast: Maximize Dynamic Range
        // Range is p99 - p01. Ideal is close to 255.
        // If range is small (e.g. < 150), boost contrast.
        // int dynamicRange = p99 - p01;
        // if (dynamicRange < 200) {
        //     // Simple heuristic: Map 0..200 range to 0..30 contrast boost
        //     float boost = (200.0f - dynamicRange) / 200.0f;  // 0.0 to 1.0
        //     settings.contrast = boost * 30.0f;               // Max 30 contrast
        // }

        // 5. Smart Recovery (Highlights & Shadows)
        // Now we simulate what the exposure boost did to the edges.

        // Estimate new Black/White points after exposure shift
        // (Approximation: Value * 2^EV)
        float evScale = std::pow(2.0f, settings.exposure);
        float newP01 = p01 * evScale;
        float newP99 = p99 * evScale;

        // A. Shadows Recovery
        if (newP01 < 10.0f) {
            float recovery = (10.0f - newP01) * 2.0f;
            settings.shadow = std::clamp(recovery, 0.0f, 40.0f);
        }

        // B. Highlights Recovery
        // If we brightened the image (ev > 0), whites might clip to 255.
        if (newP99 > 245.0f) {
            // Reduce highlights to recover detail
            float recovery = (newP99 - 245.0f) * 2.0f;
            settings.highlight = std::clamp(-recovery, -50.0f, 0.0f);
        }
    }

    return settings;
}
