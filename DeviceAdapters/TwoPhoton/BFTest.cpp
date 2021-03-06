#include "BFCamera.h"
#include "MMDeviceConstants.h"

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <direct.h>
#include <conio.h>
#include <stdlib.h>

#define PIE 3.141582



int TestCamera()
{
   BFCamera cam(true);
   int ret = cam.Initialize();
   if (ret != DEVICE_OK)
      return ret;

   unsigned bufSize = cam.GetBufferSize();
   unsigned numBuffers = cam.GetNumberOfBuffers();
   unsigned rc=0;
   char errbuf[200];
   const unsigned char* buf = cam.GetImage(rc, errbuf, 200, 0);
   if (buf == 0)
      return 2;

   ret = cam.Shutdown();
   if (ret != DEVICE_OK)
      return ret;

   return 0;
}

int CosineWarp()
{
   int real_pixel[601]; /*array dimension must = image width*/ 
   int new_pixel[601];  /*LUT for correct image*/

   double cosine;
	double angle_factor;
	double angle_position;
	double radian_position;
	double linear_position;
	double correction_factor;
	double lowest_factor;
	double correct_pixel;
   int old_pixel;
	int pixel;
	int center_pixel;
   char ch;

   /* image width in pixels after pixel reversal routine
      half value in H_Max_Capture_size in MU Tech Driver file
      Could be an option in the Stream Filter
      May also need to be adjusted by the offset in the Pixel reversal process
   */

   int image_width = 480;

   /*calculate the angle per clock interval for H scan line
   360.0  = 360 degrees for forward and vbackward scan,
   1400 = number of clock intervals for each line
   (from the Mu-Tech driver file: [PPL Control],  Pixel_Per_Line = 1400)
   This variable could be automatically read or be part of the
   stream filter options*/

   angle_factor=360.0/1400;

   /*
   Dec 2006
   PixelsPerLine = 127.0 * FREQ – FREQ can be varied by the user 
   127 = usecond for the line scan and FREQ = the acquisition H clock frequency. 
   Thus for the Raven and CRS mirror this = 127 x 12 = 1524 ppl

   angle_factor= 360/PixelsPerline – this is what is used now 

   1. First convert the image pixel location to a degree location
   This is done from the right side of the image as this is the
   only location for which the degree position (90) is known.
   2. Calculate radian position (linear equivalent of angle) from the right edge
   of the image.
   3. Determine the cosine value of the radian position.
   4. Determine the linear position in radians with respect to the image center
   5. Determine the correction factor, linear position/cosine position
   relatative to image center
   6. Find pixel = to image center based on mirror rotation,
   where correction factor = 1.0000000000
   7. Repeat steps 1 - 5 in 2nd loop
   8. Calculate corrected pixel location from center: Center position +
   pixel distance from center divide by correction factor */

   /*6. First loop to find center pixel*/
   
   lowest_factor=2.0; /*set comparison factor at greater value*/
   for(pixel=0;pixel<=image_width;pixel++)
	{
	   old_pixel = image_width - pixel;
	   angle_position = 90.0 - (pixel * angle_factor);         /*1*/
	   radian_position =(90.0*PIE/180.0) - (angle_position * PIE/180.0);  /*2*/
	   cosine=cos(radian_position);                           /*3*/
	   linear_position = (PIE/2.0) - radian_position;         /*4*/
	   correction_factor = linear_position/cosine;                /*5*/
	   /*printf("\n %d  %d %f",pixel,old_pixel,correction_factor);*/    /*line to check values*/
	   if(correction_factor<lowest_factor)                                /*6*/
		{
		   lowest_factor=correction_factor;
		   center_pixel=image_width - pixel;
		}
	}
   /*
   This code in blue is in the original dll and was used to find where the correction factor went to 1.0 
   but I believe it can be replaced with a much simpler code

   The pixel of the image where the distortion factor = 1 is at angle time 0 (or phase 0) and is 
	= (FREQx127.0)/4  (pixels per line/4)
   However this must be expressed relative to position where the distortion is the greatest at time, T or Phase Pie/2)
	Correct Image Center position = PPL/2 – FREQx127/4
   */

   printf("\nCenter pixel = %d, correlation factor = %f",center_pixel,lowest_factor);

   /*7. Loop to calculated corrected pixel position*/
   for(pixel=0;pixel<=image_width;pixel++)
	{
	   old_pixel = image_width - pixel;
	   angle_position = 90.0 - (pixel * angle_factor);         /*1*/
	   radian_position =(90.0*PIE/180.0) - (angle_position * PIE/180.0);  /*2*/
	   cosine=cos(radian_position);                           /*3*/
	   linear_position = (PIE/2.0) - radian_position;         /*4*/
	   correction_factor = linear_position/cosine;           /*5*/
	   real_pixel[old_pixel] = center_pixel + (int)((double)(old_pixel - center_pixel)/correction_factor); /*8*/
	}
   
   FILE* fp;
   fopen_s(&fp, "lut.txt", "w");

   /*
   Again this code can be replaced by simpler code 
   The correction factor = ø /sin ø where  ø = pixel number (from the correct center pixel) * Δø 
   Δø = 2Π/freq*127. The corretion factor is calculated for each pixel and applied to the pixel
   */

   /*9 Loop to shift pixels to new image LUT*/
   for(pixel=0;pixel<=image_width;pixel++)
	{
	   new_pixel[pixel]=real_pixel[pixel]-real_pixel[0];
	   printf("\nold pixel %d, new pixel %d", pixel, new_pixel[pixel]);
      fprintf(fp, "%d,%d\n", pixel, new_pixel[pixel]);
	}

   return 0;
}


int main(const int &)
{
   return TestCamera();
}