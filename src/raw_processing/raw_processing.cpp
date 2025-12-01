#include "raw_processing.h"

// Zusätzliche Includes für die Implementation
#include <libraw.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

cv::Mat RawProcessing::loadRawImg(const std::string& raw_path) {
    LibRaw processor;
    int ret = 0;

    // Datei öffnen
    if ((ret = processor.open_file(raw_path.c_str())) != LIBRAW_SUCCESS) {
        throw std::runtime_error("LibRaw: open_file fehlgeschlagen: " +
                                 std::string(libraw_strerror(ret)));
    }

    // Entpacken
    if ((ret = processor.unpack()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("LibRaw: unpack fehlgeschlagen: " +
                                 std::string(libraw_strerror(ret)));
    }

    // Parameter setzen
    processor.imgdata.params.output_bps = 16;    // 16-bit
    processor.imgdata.params.use_camera_wb = 1;  // Kamera-Weißabgleich
    processor.imgdata.params.use_auto_wb = 0;    // Kein Auto-WB
    processor.imgdata.params.output_color = 1;   // sRGB

    // Verarbeiten
    if ((ret = processor.dcraw_process()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("LibRaw: dcraw_process fehlgeschlagen: " +
                                 std::string(libraw_strerror(ret)));
    }

    // Bild aus Speicher holen
    libraw_processed_image_t* image = processor.dcraw_make_mem_image(&ret);
    if (!image || ret != LIBRAW_SUCCESS) {
        throw std::runtime_error("LibRaw: dcraw_make_mem_image fehlgeschlagen: " +
                                 std::string(libraw_strerror(ret)));
    }

    if (image->bits != 16) {
        processor.dcraw_clear_mem(image);
        processor.recycle();
        throw std::runtime_error("LibRaw: Bild ist nicht 16-bit!");
    }

    // OpenCV-Matrix erstellen
    cv::Mat rgb16(image->height, image->width, CV_16UC3, image->data);
    cv::Mat rgb16_copy = rgb16.clone();

    // Speicher freigeben
    processor.dcraw_clear_mem(image);
    processor.recycle();

    // RGB -> BGR konvertieren
    cv::Mat bgr16;
    cv::cvtColor(rgb16_copy, bgr16, cv::COLOR_RGB2BGR);

    return bgr16;
}
