package com.securics.wildlifecapture;

public class SizeCalculationData 
{
	double imgVertPx;
	double imgHorizPx;
	
	/**
	 * set the values of imgVertPx and img HorizPx to 0
	 */
	public SizeCalculationData()
	{
		imgVertPx=0;
		imgHorizPx=0;
	}
	
	/**
	 * set the values of imgVertPx to newImgVertPx and imgHorizPx to newImgHorizPx
	 * @param newImgVertPx
	 * @param newImgHorizPx
	 */
	public SizeCalculationData(double newImgVertPx, double newImgHorizPx)
	{
		imgVertPx=newImgVertPx;
		imgHorizPx=newImgHorizPx;
	}
}
