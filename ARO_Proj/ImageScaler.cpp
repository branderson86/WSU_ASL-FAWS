////////////////////
// CameraDisplay.cpp - implementation of CameraDisplay class
// Last edited: 08/02/2021 by Andrew O'Kins (formatting comments)
////////////////////

#include "stdafx.h"
#include "ImageScaler.h"

#include <algorithm> // max() and min()

// Constructor
// Input: output_image_width - x diminsion size of output image
//		 output_image_height - y diminsion size of output image
//		  output_image_depth - z diminsion size of output image
ImageScaler::ImageScaler(int output_image_width, int output_image_height, int output_image_depth) :
output_image_width_(output_image_width), output_image_height_(output_image_height), output_image_depth_(output_image_depth) {
	bin_size_x_ = bin_size_y_ = -1;
	max_bins_x_ = max_bins_y_ = -1;
	used_bins_x_ = used_bins_y_ = -1;
	remainder_x_ = remainder_y_ = -1;
	left_remainder_x_ = top_remainder_y_ = -1;
	requirement_set_bin_size_ = requirement_set_used_bins_ = false;
}

// Set bin size
// Input: bin_size_x - x dimension size of bins
//		  bin_size_y - y dimension size of bins
void ImageScaler::SetBinSize(int bin_size_x, int bin_size_y) {
	bin_size_x_ = bin_size_x;
	bin_size_y_ = bin_size_y;
	max_bins_x_ = output_image_width_ / bin_size_x;
	max_bins_y_ = output_image_height_ / bin_size_y;
	remainder_x_ = output_image_width_ % bin_size_x;
	remainder_y_ = output_image_height_ % bin_size_y;
	requirement_set_bin_size_ = true;
}

// Get the maximum number of bins based on image size and bin size
// Input: max_bins_x - output to be set with max bin numbers in the x dimension
//		  max_bins_y - output to be set with max bin numbers in the y dimension
void ImageScaler::GetMaxBins(int &max_bins_x, int &max_bins_y) {
	max_bins_x = max_bins_x_;
	max_bins_y = max_bins_y_;
}

// Sets the number of bins that there will be in the x and y dimensions
// Input: used_bins_x - the number of bins in the x dimension
//		  used_bins_y - the number of bins in the y dimension
void ImageScaler::SetUsedBins(int used_bins_x, int used_bins_y) {
	used_bins_x_ = max(0, min(used_bins_x, max_bins_x_));
	used_bins_y_ = max(0, min(used_bins_y, max_bins_y_));
	remainder_x_ = (output_image_width_ % bin_size_x_) + ((max_bins_x_ - used_bins_x_) * bin_size_x_);
	remainder_y_ = (output_image_height_ % bin_size_y_) + ((max_bins_y_ - used_bins_y_) * bin_size_y_);
	left_remainder_x_ = remainder_x_ / 2;
	top_remainder_y_ = (remainder_y_ / 2) *output_image_width_;
	requirement_set_used_bins_ = true;
}

// Gets the total number of bins
int ImageScaler::GetTotalBinNum() {
	return used_bins_x_ * used_bins_y_;
}

// Puts a value of zero into every bin in an image
// Input: output_image - the image to be zeroed
// Output: output_image is filled with 0's
void ImageScaler::ZeroOutputImage(unsigned char* output_image) {
	for (int i = 0; i < output_image_depth_*output_image_height_*output_image_width_; i++)	{
		output_image[i] = 0;
	}
}

// Takes an array holding values for each bin and fills an image with those values
// Implicit sizes are according to the construction of the scaler
// Input: input_image - the array holding all the bin values
//		 output_image - the array to store the output image (already allocated)
// Output: output_image stores the results
void ImageScaler::TranslateImage(int* input_image, unsigned char* output_image) {
	if (requirement_set_bin_size_ && requirement_set_used_bins_)
	{	// prevent action if all steps to set up image scaling have not been completed
		int start_point = top_remainder_y_ + left_remainder_x_;
		for (int i = 0; i < used_bins_y_; i++)
		{	// for each row
			int line_start_point = start_point + (i*(bin_size_y_*output_image_width_));
			for (int j = 0; j < used_bins_x_; j++)
			{	// for each bin in the row
				int bin_start_point = line_start_point + (j * bin_size_x_);
				int pix_value;

				pix_value = (input_image[(i * used_bins_x_) + j]);

				for (int k = 0; k < bin_size_y_; k++)
				{	// for each line in each bin
					int write_start_point = bin_start_point + (k * output_image_width_);
					for (int l = 0; l < bin_size_x_; l++)
					{	// for each space in each line
						int write_point = write_start_point + l;
						output_image[write_point*output_image_depth_] = (unsigned char)pix_value;
						if (output_image_depth_ > 1)
						{
							output_image[(write_point*output_image_depth_) + 1] = (pix_value >> 8);
						}
					}
				}
			}
		}
	}
}
