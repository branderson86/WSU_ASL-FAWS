////////////////////
// ImageScaler.h - Header file of ImageScaler which is used to scale an image from one size to another
////////////////////

#ifndef IMAGE_SCALER_H_
#define IMAGE_SCALER_H_

class ImageScaler {
private:
	int output_image_width_, output_image_height_, output_image_depth_;
	int bin_size_x_, bin_size_y_;
	int max_bins_x_, max_bins_y_;
	int used_bins_x_, used_bins_y_;
	int remainder_x_, remainder_y_;
	int left_remainder_x_, top_remainder_y_;
	bool requirement_set_bin_size_, requirement_set_used_bins_;
public:
	ImageScaler(int output_image_width, int output_image_height, int output_image_depth);

	void SetBinSize(int bin_size_x, int bin_size_y);
	void GetMaxBins(int &max_bins_x, int &max_bins_y);
	void SetUsedBins(int used_bins_x, int used_bins_y);
	int GetTotalBinNum();
	void TranslateImage(int* input_image, unsigned char* output_image);
	void ZeroOutputImage(unsigned char* output_image);
};

#endif
