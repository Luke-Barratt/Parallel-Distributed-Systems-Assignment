
//
// WRIT 1 Assignment - Image processing using parallel programing techniques (TTB)
// 16/11/2020
// Luke Barratt ST20121473
// v1.0

#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <cmath>
#include <complex>
#include <tbb/tbb.h>
#include <FreeImage\FreeImagePlus.h>

using namespace std;
using namespace std::chrono;
using namespace tbb;

// Declare structures for BYTE type pixel
class Pixel {
public:
	BYTE r, g, b;
};

// Declare structure for float type pixel
class fPixel {
public:
	float a, b, c;
};

// Main function to convert our rgb image to xyY colour space in parallel
void parallelImageConversion(void) {

	cout << "(Parallel Version)\n";
	cout << "Loading Image...\n";
	// Setup input image
	fipImage inputImage;
	inputImage.load("Images\\Barcelona_highres.jpg");

	// Convert to rgb values (each rgb pixel is 3 bytes, each element is 8bits, 8+8+8=24)
	inputImage.convertTo24Bits();
	Pixel* inputBuffer = (Pixel*)inputImage.accessPixels();

	// Get the dimensions of the input image
	unsigned int width = inputImage.getWidth();
	unsigned int height = inputImage.getHeight();

	// Setup intermediate image
	fipImage intermediateImage;
	// Output image is going to store values as floats (each xyY pixel is 3 bytes, each element is 32bits, 32+32+32=96)
	intermediateImage = fipImage(FIT_RGBF, width, height, 96);
	fPixel* intermediateBuffer = (fPixel*)intermediateImage.accessPixels();
	
	// Setup output image
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, width, height, 24);
	Pixel* outputBuffer = (Pixel*)outputImage.accessPixels();

	//--------------Processing--------------//
	cout << "Processing (converting RGB image to xyY colour space)...\n";

	tick_count t0 = tick_count::now();

	parallel_for(

		blocked_range2d<int, int>(0, height, 0, width),

		[&](const blocked_range2d<int, int>& range) {

			for (int y = range.rows().begin(); y < range.rows().end(); y++) {

				for (int x = range.cols().begin(); x < range.cols().end(); x++) {

					// Index of current pixel at (x, y)
					UINT i = y * width + x;

					// Kernal 1 - convert rgb values to xyY
					float r = (float)inputBuffer[i].r;
					float g = (float)inputBuffer[i].g;
					float b = (float)inputBuffer[i].b;

					// Converting rgb pixel values to floating point number for intermediate values XYZ
					float X = 0.4124 * r + 0.3576 * g + 0.1805 * b;
					float Y = 0.2126 * r + 0.7152 * g + 0.0722 * b;
					float Z = 0.0193 * r + 0.1192 * g + 0.9505 * b;

					
					// Converting XYZ to xyY and store in intermediate buffer
					intermediateBuffer[i].a = X / (X + Y + Z);
					intermediateBuffer[i].b = Y / (X + Y + Z);
					intermediateBuffer[i].c = Y;

				}
			}
		});

	cout << "Processing (adjusting brightness of image)...\n";

	parallel_for(

		blocked_range2d<int, int>(0, height, 0, width),

		[&](const blocked_range2d<int, int>& range) {

			for (int y = range.rows().begin(); y < range.rows().end(); y++) {

				for (int x = range.cols().begin(); x < range.cols().end(); x++) {

					// Index of current pixel at (x, y)
					UINT i = y * width + x;

					// Kernal 2 - adjust the luminance of each pixel to 20% of original value
					float luminance = intermediateBuffer[i].c;

					float adjustedLuminance = luminance * 0.2;

					intermediateBuffer[i].c = adjustedLuminance;

				}
			}
		});

	cout << "Processing (converting image back to RGB colour space)...\n";

	parallel_for(

		blocked_range2d<int, int>(0, height, 0, width),

		[&](const blocked_range2d<int, int>& range) {

			for (int y = range.rows().begin(); y < range.rows().end(); y++) {

				for (int x = range.cols().begin(); x < range.cols().end(); x++) {

					// Index of current pixel at (x, y)
					UINT i = y * width + x;

					// Kernal 3 - converting xyY image with adjusted luminance back to RGB
					float a = intermediateBuffer[i].a;
					float b = intermediateBuffer[i].b;
					float c = intermediateBuffer[i].c;

					// Converting xyY values back to XYZ values
					float X = a * (c / b);
					float Y = c;
					float Z = (1 - a - b) * (c / b);

					// Converting XYZ values back to RGB values
					float R = 3.2405 * X + -1.5371 * Y + -0.4985 * Z;
					float G = -0.9693 * X + 1.8760 * Y + 0.0416 * Z;
					float B = 0.0556 * X + -0.2040 * Y + 1.0572 * Z;

					// Write out new RGB values to the outputBuffer. Cast to BYTE's.
					outputBuffer[i].r = (BYTE(R));
					outputBuffer[i].g = (BYTE(G));
					outputBuffer[i].b = (BYTE(B));

				}
			}
		});

	tick_count t1 = tick_count::now();

	std::cout << "Saving new image...\n";
	outputImage.save("final-image-parallel.png");
	std::cout << "Finished\n\n";

	cout << "Time taken to process image = " << (t1 - t0).seconds() << " seconds\n\n";
}

void sequentialImageConversion(void) {

	cout << "(Sequential Version)\n";
	cout << "Loading Image...\n";
	// Setup input image
	fipImage inputImage;
	inputImage.load("Images\\Barcelona_highres.jpg");

	// Convert to rgb values (each rgb pixel is 3 bytes, each element is 8bits, 8+8+8=24)
	inputImage.convertTo24Bits();
	Pixel* inputBuffer = (Pixel*)inputImage.accessPixels();

	// Get the dimensions of the input image
	unsigned int width = inputImage.getWidth();
	unsigned int height = inputImage.getHeight();

	// Setup intermediate image
	fipImage intermediateImage;
	// Output image is going to store values as floats (each xyY pixel is 3 bytes, each element is 32bits, 32+32+32=96)
	intermediateImage = fipImage(FIT_RGBF, width, height, 96);
	fPixel* intermediateBuffer = (fPixel*)intermediateImage.accessPixels();

	// Setup output image
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, width, height, 24);
	Pixel* outputBuffer = (Pixel*)outputImage.accessPixels();

	//--------------Processing--------------//
	cout << "Processing (converting RGB image to xyY colour space)...\n";

	tick_count t0 = tick_count::now();

	for (int y = 0; y < height; y++) {
		
		for (int x = 0; x < width; x++) {
			
			// Index of current pixel at (x, y)
			UINT i = y * width + x;

			// Kernal 1 - convert rgb values to xyY
			float r = (float)inputBuffer[i].r;
			float g = (float)inputBuffer[i].g;
			float b = (float)inputBuffer[i].b;

			// Converting rgb pixel values to floating point number for intermediate values XYZ
			float X = 0.4124 * r + 0.3576 * g + 0.1805 * b;
			float Y = 0.2126 * r + 0.7152 * g + 0.0722 * b;
			float Z = 0.0193 * r + 0.1192 * g + 0.9505 * b;


			// Converting XYZ to xyY and store in intermediate buffer
			intermediateBuffer[i].a = X / (X + Y + Z);
			intermediateBuffer[i].b = Y / (X + Y + Z);
			intermediateBuffer[i].c = Y;
		}
	}

	cout << "Processing (adjusting brightness of image)...\n";

	for (int y = 0; y < height; y++) {

		for (int x = 0; x < width; x++) {

			// Index of current pixel at (x, y)
			UINT i = y * width + x;

			// Kernal 2 - adjust the luminance of each pixel to 20% of original value
			float luminance = intermediateBuffer[i].c;

			float adjustedLuminance = luminance * 0.2;

			intermediateBuffer[i].c = adjustedLuminance;

		}
	}

	cout << "Processing (converting image back to RGB colour space)...\n";

	for (int y = 0; y < height; y++) {

		for (int x = 0; x < width; x++) {

			// Index of current pixel at (x, y)
			UINT i = y * width + x;

			// Kernal 3 - converting xyY image with adjusted luminance back to RGB
			float a = intermediateBuffer[i].a;
			float b = intermediateBuffer[i].b;
			float c = intermediateBuffer[i].c;

			// Converting xyY values back to XYZ values
			float X = a * (c / b);
			float Y = c;
			float Z = (1 - a - b) * (c / b);

			// Converting XYZ values back to RGB values
			float R = 3.2405 * X + -1.5371 * Y + -0.4985 * Z;
			float G = -0.9693 * X + 1.8760 * Y + 0.0416 * Z;
			float B = 0.0556 * X + -0.2040 * Y + 1.0572 * Z;

			// Write out new RGB values to the outputBuffer. Cast to BYTE's.
			outputBuffer[i].r = (BYTE(R));
			outputBuffer[i].g = (BYTE(G));
			outputBuffer[i].b = (BYTE(B));
		}
	}
	tick_count t1 = tick_count::now();

	std::cout << "Saving new image...\n";
	outputImage.save("final-image-sequential.png");
	std::cout << "Finished\n\n";

	cout << "Time taken to process image = " << (t1 - t0).seconds() << " seconds\n\n";

}

void main(void) {

	// Calling our function
	parallelImageConversion();
	sequentialImageConversion();

}