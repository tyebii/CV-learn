// netpbm_hough.c
// Test and demo program for the Hough transform using the netpbm.c library.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "netpbm.h"

#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define PI 3.141592653589793

typedef struct
{
    int ***map;
    int ySize, xSize, rSize;
} Matrix3D;

Matrix3D createMatrix3D(int yAmt, int xAmt, int rSize)
{
    int i, j, k;
    Matrix3D mx;

    mx.ySize = yAmt;
    mx.xSize = xAmt;
    mx.rSize = rSize;

    mx.map = (int ***)malloc(yAmt * sizeof(int **));

    for (i = 0; i < yAmt; i++)
    {
        mx.map[i] = (int **)malloc(xAmt * sizeof(int *));
        for (j = 0; j < xAmt; j++)
        {
            mx.map[i][j] = (int *)malloc(rSize * sizeof(int));

            for (k = 0; k < rSize; k++)
                mx.map[i][j][k] = 0;
        }
    }

    return mx;
}


void deleteMatrix3D(Matrix3D mx)
{
    int y, x;

    for (y = 0; y < mx.ySize; y++)
    {
        for (x = 0; x < mx.xSize; x++)
        {
            free(mx.map[y][x]);
        }
        free(mx.map[y]);
    }

    free(mx.map);
}

Matrix projectCircleCenters(Matrix3D mxParams)
{
    int y, x, r;

    Matrix out = createMatrix(mxParams.ySize, mxParams.xSize);

    for (y = 0; y < mxParams.ySize; y++)
    {
        for (x = 0; x < mxParams.xSize; x++)
        {
			// find most likely center across all radii
            double best = 0;

            for (r = 0; r < mxParams.rSize; r++)
            {
                if (mxParams.map[y][x][r] > best)
                    best = mxParams.map[y][x][r];
            }

            out.map[y][x] = best;
        }
    }

    return out;
}

// Build a Hough parameter map for matrix mxSpatial for detecting straight lines.
// Rows in this map represent the normal alpha and columns represent the distance d from the origin.
// Increasing the size of the map in each dimension improves the resolution of the corresponding parameter.
Matrix3D houghTransformCircles(Matrix mxSpatial, int radMin, int radMax, int radStep)
{
	int m, n, r, deg;
	double angle, alpha, maxD = sqrt((double) (SQR(mxSpatial.height) + SQR(mxSpatial.width)));
	
	int ySize = mxSpatial.height;
    int xSize = mxSpatial.width;
	// amount of radii to check
	int rSize = (radMax - radMin + radStep - 1) / radStep;
	//int rSize = (radMax - radMin) / radStep + 1;

	Matrix3D mxParam = createMatrix3D(ySize, xSize, rSize);
	Matrix sincos = createMatrix(360, 2);

	// Generate lookup table for sin and cos values to speed up subsequent computation.
	for (int i = 0; i < 360; i++) {
		//convert to radians
		angle = i * PI / 180.0;
		sincos.map[i][0] = sin(angle);
		sincos.map[i][1] = cos(angle);
	}
	
	// for each edge candidate pixel, add all associated circle centers for given radii to accumulator
    for (m = 0; m < mxSpatial.height; m++) {
		for (n = 0; n < mxSpatial.width; n++) {
			// check if a valid edge pixel
			if (mxSpatial.map[m][n] > 0) {
				int rIndex = 0;
				for (r = radMin; r < radMax; r+= radStep, rIndex++){
					for (deg = 0; deg < 360; deg++)
						{
							// find circle center
							int x = (int)lround(n - r * sincos.map[deg][0]);
							int y = (int)lround(m - r * sincos.map[deg][1]);

							// add if in image 
							if (x >= 0 && x < xSize &&
								y >= 0 && y < ySize)
							{
								mxParam.map[y][x][rIndex]++;
							}
						}
				}
			}
		}
	}

	deleteMatrix(sincos);
	return mxParam; 
}


// Test whether entry (m, n) in matrix mx is a local maximum, i.e., is not exceeded by any of its 
// maximally 8 neighbors. Return 1 if true, 0 otherwise.
int isLocalMaximum(Matrix mx, int m, int n)
{
	double strength = mx.map[m][n];
	int i, j; 
	int iMin = (m == 0)? 0:(m - 1);
	int iMax = (m == mx.height -1)? m:(m + 1);
	int jMin = (n == 0)? 0:(n - 1);
	int jMax = (n == mx.width -1)? n:(n + 1);

	for (i = iMin; i <= iMax; i++)
		for (j = jMin; j <= jMax; j++)
			if (mx.map[i][j] > strength)
				return 0;
	return 1;
}


// Insert a new entry, consisting of vPos, hPos, and strength, into the list of maxima mx.
void insertMaxEntry(Matrix mx, int vPos, int hPos, double strength)
{
	int m, n = mx.width - 1;

	while (n > 0 && mx.map[2][n - 1] < strength)
	{
		for (m = 0; m < 3; m++)
			mx.map[m][n] = mx.map[m][n - 1];
		n--;
	}
	mx.map[0][n] = (double) vPos;
	mx.map[1][n] = (double) hPos;
	mx.map[2][n] = strength;
}


// Delete entry number i from the list of maxima mx.
void deleteMaxEntry(Matrix mx, int i)
{
	int m, n;

	for (n = i; n < mx.width - 1; n++)
		for (m = 0; m < 3; m++)
			mx.map[m][n] = mx.map[m][n + 1];

	mx.map[2][mx.width - 1] = -1.0;
}


// Find the <number> highest maxima in a Hough parameter map that are separated by a Euclidean distance 
// of at least <minSeparation> in the map. The result is a three-row matrix with each column representing
// the row, the column, and the strength of one maximum, in descending order of strength.
Matrix findHoughMaxima(Matrix3D mx, int number, double minSeparation)
{
	int x, y, j, k, l, r, index, do_not_insert;
	double minSepSquare = SQR(minSeparation);
	double strength;

	// y x radius strength
	Matrix maxima = createMatrix(4, number);


	for (r = 0; r < mx.rSize; r++)
    {
        for (y = 0; y < mx.ySize; y++)
        {
            for (x = 0; x < mx.xSize; x++)
            {
                strength = mx.map[y][x][r];

                if (strength <= 0)
                    continue;

                do_not_insert = 0;

                // check separation in (a,b) space
                for (j = 0; j < number; j++)
                {
                    if (maxima.map[3][j] == 0)
                        continue;

                    k = y - maxima.map[0][j];
                    l = x - maxima.map[1][j];

                    if (k * k + l * l < minSepSquare)
                    {
                        if (strength > maxima.map[3][j])
                        {
                            maxima.map[0][j] = y;
                            maxima.map[1][j] = x;
                            maxima.map[2][j] = r;
                            maxima.map[3][j] = strength;
                        }

                        do_not_insert = 1;
                        break;
                    }
                }

				// skip if no insertion
                if (do_not_insert)
                    continue;

				index = number - 1;

				// ignore if less than lowest max
				if (strength <= maxima.map[3][index])
					continue;

				// find insertion index
				while (index > 0 && strength > maxima.map[3][index - 1])
				{
					maxima.map[0][index] = maxima.map[0][index - 1];
					maxima.map[1][index] = maxima.map[1][index - 1];
					maxima.map[2][index] = maxima.map[2][index - 1];
					maxima.map[3][index] = maxima.map[3][index - 1];
					index--;
				}

				// insert new maximum
				maxima.map[0][index] = y;
				maxima.map[1][index] = x;
				maxima.map[2][index] = r;
				maxima.map[3][index] = strength;
            }
        }
    }
	return maxima;
}


// Read image "desk.ppm" and write Hough transform related output images. 
void main()
{
	int i, m, n, m1, n1, m2, n2;
	double gaussFilter[3][3] = {{1.0, 2.0, 1.0}, {2.0, 4.0, 2.0}, {1.0, 2.0, 1.0}};
	Matrix gauss = createMatrixFromArray(&gaussFilter[0][0], 3, 3); 
	Image inputImage = readImage("desk.pgm");
	Matrix inputMatrix = image2Matrix(inputImage);
	Image edgeImage, houghImage; 
	Matrix edgeMatrix = createMatrix(inputImage.height, inputImage.width);
	Matrix3D houghMatrix;
	Matrix maxMatrix;
	double maxLength, alpha, dist;

	// Add code for generating edge matrix here!!!
	edgeImage = readImage("desk_canny.pbm");
	//edgeImage = readImage("desk_sobel.pgm");
	edgeMatrix = image2Matrix(edgeImage);

	//writeImage(edgeImage, "desk_edges.pgm"); done in main
	
	houghMatrix = houghTransformCircles(edgeMatrix, 70, 90, 1);
	houghImage = matrix2Image(projectCircleCenters(houghMatrix), 1, 3.0);
	writeImage(houghImage, "desk_hough.pgm");

	int num = 1;
	maxMatrix = findHoughMaxima(houghMatrix, num, 50.0);
	for (i = 0; i < num; i++)
		ellipse(houghImage, maxMatrix.map[1][i], maxMatrix.map[0][i], 20, 20, 2, 10, 7, 255, 255, 255, 0); 
	writeImage(houghImage, "desk_hough_max.ppm");

	maxLength = sqrt((double) (SQR(inputImage.height) + SQR(inputImage.width)));

	/**	for (i = 0; i < 5; i++) {
			alpha = -0.5*PI + 1.5*PI*maxMatrix.map[0][i]/(double) houghMatrix.height;
			dist = maxLength*maxMatrix.map[1][i]/(double) houghMatrix.width;
			m1 = (int) (dist*sin(alpha) - maxLength*cos(alpha) + 0.5);
			n1 = (int) (dist*cos(alpha) + maxLength*sin(alpha) + 0.5);
			m2 = (int) (dist*sin(alpha) + maxLength*cos(alpha) + 0.5);
			n2 = (int) (dist*cos(alpha) - maxLength*sin(alpha) + 0.5);
			line(inputImage, m1, n1, m2, n2, 2, 18, 10, 30, 30, 30, 0); 
		} 
		writeImage(inputImage, "desk_hough_lines.ppm");
	**/
	for (i = 0; i < num; i++)
	{
		int cy = maxMatrix.map[0][i];
		int cx = maxMatrix.map[1][i];
		int rad  = 80 + maxMatrix.map[2][i] * 2;
		ellipse(inputImage, cy, cx, rad, rad, 2, 2, 4, 255, 0, 0, 255);
	}	writeImage(inputImage, "desk_hough_lines.ppm");

	deleteMatrix(edgeMatrix);
	deleteMatrix3D(houghMatrix);
	deleteImage(inputImage);
	deleteImage(edgeImage);
	deleteImage(houghImage);
}
