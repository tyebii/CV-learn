//
//  main.c
//
//  Last modified by Tyler B on 3/12/26
//  proj2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"
#include <stdio.h>
#include "unionfind.h"

// predeclaring function to flag as void return type
Image imageBlackWhite(Image img);
Image function_noiseImage(Image img, float p);
Image expandImage(Image img);
Image shrinkImage(Image img);
int label_components(Image img);
Matrix smoothing_filter(Matrix m1, Matrix m2);
Matrix median_filter(Matrix m1, Matrix m2);


int main(int argc, const char * argv[]) {
    //-------------------------------------------------------------------------------
       //create blackWhiteImage:
       Image inputImage = readImage("/Users/Testarossa/Downloads/project1/project1/netpbm/car.ppm");
       Image blackWhiteImage = imageBlackWhite(inputImage);

       //-------------------------------------------------------------------------------
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
    Image rotatedImage = createImage(inputImage.height, inputImage.width);
    Image invertedImage = createImage(inputImage.height, inputImage.width);
       for (int x = 0; x < inputImage.width; x++)
           for (int y = 0; y < inputImage.height; y++)
           {
               rotatedImage.map[y][x] = inputImage.map[inputImage.height - y - 1][inputImage.width - x - 1];

               invertedImage.map[y][x].r = 255 - inputImage.map[y][x].r;
               invertedImage.map[y][x].g = 255 - inputImage.map[y][x].g;
               invertedImage.map[y][x].b = 255 - inputImage.map[y][x].b;
               // Let's just ignore 'i' here; it's irrelevant if we want to save image as PPM.
           }
       writeImage(rotatedImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/rotated.ppm");
       writeImage(invertedImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/inverted.ppm");
       writeImage(inputImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/gray.pgm");
       writeImage(blackWhiteImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/black-white.pbm");

    //-------------------------------------------------------------------------------

       // create noiseImage:
       float noiseProbability = 0.05f;
       Image noiseImage = function_noiseImage(blackWhiteImage, noiseProbability);
       writeImage(noiseImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/noise.pbm");


       // create expandImage:
       Image expandedImage = expandImage(noiseImage);
       writeImage(expandedImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/expanded.pbm");

       // create shrinkImage:
       Image shrunkImage = shrinkImage(noiseImage);
       writeImage(shrunkImage, "/Users/Testarossa/Downloads/project1/project1/netpbm/shrunken.pbm");

       // shrink and expand
       Image noiseRemoved = expandImage(shrinkImage(shrinkImage(expandedImage)));
       writeImage(noiseRemoved, "/Users/Testarossa/Downloads/project1/project1/netpbm/denoised.pbm");

       //-------------------------------------------------------------------------------
       // Function that does threshold, noise and numbers of spanding and shrinking
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
       //function_readImage();

       //-------------------------------------------------------------------------------

       // label components
       label_components(blackWhiteImage);

       // Matrix for smoothing
       Matrix smoothMatrix = image2Matrix(inputImage);

       // smooth image with averaging filter
       double filter3x3[3][3] = {(double)1/9, (double)1/9, (double)1/9, (double)1/9, (double)1/9, (double)1/9, (double)1/9, (double)1/9, (double)1/9};
       Matrix averagingFilter = createMatrixFromArray(&filter3x3[0][0], 3, 3);
       Image smoothAvg = matrix2Image(smoothing_filter(smoothMatrix, averagingFilter), 0, 0.0);
       writeImage(smoothAvg, "/Users/Testarossa/Downloads/project1/project1/netpbm/smoothed_avg.pgm");

       // smooth image with median filter
       Image smoothMedian = matrix2Image(median_filter(smoothMatrix, averagingFilter), 0, 0);
       writeImage(smoothMedian, "/Users/Testarossa/Downloads/project1/project1/netpbm/smoothed_median.pgm");


       //-------------------------------------------------------------------------------

        /* Delete back and white, noise, Expand and Shrink */
       deleteImage(inputImage);
       deleteImage(rotatedImage);
       deleteImage(invertedImage);

       printf("Program ends ... ");


       return 0;
}

//-------------------------------function_imageBlackWhite-------------------------------------------------
/* function that receives an Image structure and an intensity threshold
 to convert each pixel in the image to either black (intensity = 0)
 or white (intensity = 255). The function should return an Image structure
 containing the result. */

 Image imageBlackWhite(Image img) {
    Image blackwhite = createImage(img.height, img.width);
    for (int x = 0; x < img.width; x++)
           for (int y = 0; y < img.height; y++)
           {
               if (img.map[y][x].i > 127) {
                blackwhite.map[y][x].i = 255;
               } else {
                blackwhite.map[y][x].i  = 0;
               }
           }
    return blackwhite;
}

//--------------------------------Expand function-----------------------------------------------------------
/* Expand operation */

Image expandImage(Image img) {
    Image expanded = createImage(img.height, img.width);
    for (int x = 0; x < img.width; x++)
    {
        for (int y = 0; y < img.height; y++)
        {
            expanded.map[y][x].i = img.map[y][x].i;

            //whitespace
            if (img.map[y][x].i == 255) {
                
                //left
                if (x != 0 && img.map[y][x - 1].i == 0) {
                    expanded.map[y][x].i = 0;
                }
                //right
                if (x != img.width-1 && img.map[y][x + 1].i == 0) {
                    expanded.map[y][x].i = 0;
                }
                //down
                if (y != 0 && img.map[y - 1][x].i == 0) {
                    expanded.map[y][x].i = 0;
                }
                //up
                if (y != img.height-1 && img.map[y + 1][x].i == 0) {
                    expanded.map[y][x].i = 0;
                }
            }
        }
    }
    return expanded;
}

//--------------------------------Shrink function-----------------------------------------------------------
/* Shrink operation */

Image shrinkImage(Image img) {
    Image shrunk = createImage(img.height, img.width);
    for (int x = 0; x < img.width; x++)
    {
        for (int y = 0; y < img.height; y++)
        {
            shrunk.map[y][x].i = img.map[y][x].i;

            //blackspace
            if (img.map[y][x].i == 0) {

                //left
                if (x != 0 && img.map[y][x - 1].i == 255) {
                    shrunk.map[y][x].i = 255;
                }
                //right
                if (x != img.width - 1 && img.map[y][x + 1].i == 255) {
                    shrunk.map[y][x].i = 255;
                }
                //down
                if (y != 0 && img.map[y - 1][x].i == 255) {
                    shrunk.map[y][x].i = 255;
                }
                //up
                if (y != img.height - 1 && img.map[y + 1][x].i == 255) {
                    shrunk.map[y][x].i = 255;
                }
            }
        }
    }
    return shrunk;
}

//--------------------------------noise function-----------------------------------------------------------
/* function that adds binary noise to an image. This function receives an
 image and a floating point number p that indicates the probability
 (in percent) that each pixel in the image will be flipped, i.e.,
 turned from black to white or vice versa.
 */

Image function_noiseImage(Image img, float p) {
    Image noiseImage = createImage(img.height, img.width);

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            noiseImage.map[y][x] = img.map[y][x];

            float r = (float)rand() / (float)(RAND_MAX);
            if (r <= p) {
                //swap pixel
                if (img.map[y][x].i > 0) {
                    noiseImage.map[y][x].i = 0;
                }
                else {
                    noiseImage.map[y][x].i = 255;
                }
            }
            
        }
    }
    return noiseImage;
}

//--------------------------------labeling + color grouping function-----------------------------------------------------------
/* function that assigns labels to all pixel groups. This function receives an
 image and identifies all connected components, then gives each component a random color.
 */

int label_components(Image img) {
    Matrix labels = createMatrix(img.height, img.width);
    UnionFind *equiv = uf_create(32); //arbitrary;
    
    // assigning a new label requires uf_make_label(uf);
    // top to bottom, left to right
    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            if (img.map[y][x].i == 0) {
                int left  = (x > 0) ? labels.map[y][x - 1] : 0;
                int up    = (y > 0) ? labels.map[y - 1][x] : 0;

                if (left == 0 && up == 0) { // no neighbor
                    labels.map[y][x] = uf_make_label(equiv);
                } else if (left != 0 && up == 0) { // only up
                    labels.map[y][x] = left;
                } else if (up != 0 && left == 0) { // only left
                    labels.map[y][x] = up;
                } else { // both neighbors
                    labels.map[y][x] = (left < up) ? left : up;
                    if (left != up) {
                        uf_unite(equiv, left, up);
                    }
                }
            }
        }
    }

    // for all valid pixels, merge components if equivalent
    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            if (labels.map[y][x] != 0) {
                labels.map[y][x] = uf_find(equiv, (int)labels.map[y][x]);
            }
        }
    }


    // calculate and store the sizes of each component for thresholding
    int *component_size = calloc(equiv->size, sizeof(int)); // size enough for all labels
    for (int y = 0; y < labels.height; y++) {
        for (int x = 0; x < labels.width; x++) {
            int label = labels.map[y][x];
            if (label != 0) {
                int rep = uf_find(equiv, label);
                component_size[rep]++;
            }
        }
    }
    
    // minimum size for a component to be colored
    int size_threshold = 30;

    // Generate random colors
    unsigned char **colors = malloc(equiv->size * sizeof(unsigned char *));
    colors[0] = malloc(3 * sizeof(unsigned char));
    colors[0][0] = 0;
    colors[0][1] = 0;
    colors[0][2] = 0;

    for (int i = 1; i < equiv->size; i++) {
        colors[i] = malloc(3 * sizeof(unsigned char));
        if (component_size[i] >= size_threshold) {
            colors[i][0] = rand() % 256;
            colors[i][1] = rand() % 256;
            colors[i][2] = rand() % 256;
        } else {
            colors[i][0] = 0;
            colors[i][1] = 0;
            colors[i][2] = 0;
        }
    }

    // Create the output image
    Image output = createImage(img.height, img.width);
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int label = labels.map[y][x];
            if (label != 0) {
                int rep = uf_find(equiv, label);
                if (component_size[rep] >= size_threshold) {
                    output.map[y][x].r = colors[rep][0];
                    output.map[y][x].g = colors[rep][1];
                    output.map[y][x].b = colors[rep][2];
                } else {
                    // leave as original black/white
                    output.map[y][x].r = img.map[y][x].i;
                    output.map[y][x].g = img.map[y][x].i;
                    output.map[y][x].b = img.map[y][x].i;
                }
            } else {
                // background
                output.map[y][x].r = img.map[y][x].i;
                output.map[y][x].g = img.map[y][x].i;
                output.map[y][x].b = img.map[y][x].i;
            }
        }
    }

    int num_letters = 0;
    for (int i = 1; i < equiv->size; i++) {
        if (component_size[i] >= size_threshold)
            num_letters++;
    }
    printf("Number of objects = %d\n", num_letters);

    writeImage(output, "image_colored.ppm");

    return 1;
}

//--------------------------------averaging filter-----------------------------------------------------------
/* function that applies an averaging filter to the entire image to smooth it. The function assumes that
 the filter matrix is already initialized with the correct values (1 / # of elements in the filter)
 */

Matrix smoothing_filter(Matrix m1, Matrix m2) {
    Matrix output = createMatrix(m1.height, m1.width);

    if (m2.height > m1.height || m2.width > m1.width){
        printf("Error: Filter size is greater than image size.");
        return output;
    }

    // compute center of filter
    int offset_x = (m2.width-1) / 2;
    int offset_y = (m2.height-1) / 2;
    
    for (int y = 0; y < m1.height - m2.height + 1; y++) {
        for (int x = 0; x < m1.width - m2.width + 1; x++) {
            int avg = 0;
            for(int i = 0; i < m2.height; i++){
                for (int j = 0; j < m2.width; j++){
                    avg += m1.map[y+i][x+j] * m2.map[i][j];
                }
            }

            //output.map[y + offset_y][x + offset_x] = avg / (m2.height * m2.width);
            output.map[y + offset_y][x + offset_x] = avg;
        }  
    }
    
    return output;
}

//--------------------------------median filter-----------------------------------------------------------
/* function that applies a median filter to the entire image to smooth it.
 */

 // Comparison function for quicksort
int compare(const void* a, const void* b) {
   return (*(int*)a - *(int*)b);
}

Matrix median_filter(Matrix m1, Matrix m2) {
    Matrix output = createMatrix(m1.height, m1.width);

    if (m2.height > m1.height || m2.width > m1.width){
        printf("Error: Filter size is greater than image size.");
        return output;
    }

    // compute center of filter
    int offset_x = (m2.width-1) / 2;
    int offset_y = (m2.height-1) / 2;

    int filtersize = m2.height*m2.width;

    int *values = malloc(filtersize * sizeof(int));
    
    for (int y = 0; y < m1.height - m2.height + 1; y++) {
        for (int x = 0; x < m1.width - m2.width + 1; x++) {
            //int *values = malloc(filtersize * sizeof(int));
            if (!values) {
                printf("Memory allocation failed\n");
                return output;
            }

            for(int i = 0; i < m2.height; i++){
                for (int j = 0; j < m2.width; j++){

                    values[i * m2.width + j] = m1.map[y+i][x+j];

                    //avg += m1.map[y+i][x+j] * m2.map[i][j];
                }
            }
            //find median with quicksort
            qsort(values, filtersize, sizeof(int), compare);

            //even or odd median
            if (filtersize%2 == 0) {
                output.map[y + offset_y][x + offset_x] = (values[filtersize/2] + values[filtersize/2 - 1]) / 2;
            } else {
                output.map[y + offset_y][x + offset_x] = values[(filtersize-1)/2];
            }

            //free(values);
        }  
    }
    free(values);
    return output;
}