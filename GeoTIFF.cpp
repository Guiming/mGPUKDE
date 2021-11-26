// https://gerasimosmichalitsianos.wordpress.com/2018/11/30/431/
#include <iostream>
#include <string>
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <gdalwarper.h>
#include <stdlib.h>
#include <chrono>
using namespace std;
using chrono::high_resolution_clock;
using chrono::duration_cast;
using chrono::duration;
using chrono::milliseconds;
//typedef std::string String;

class GeoTIFFReader {

private: // NOTE: "private" keyword is redundant here.  
         // we place it here for emphasis. Because these
         // variables are declared outside of "public", 
         // they are private. 

    const char* filename;        // name of Geotiff
    GDALDataset* geotiffDataset; // Geotiff GDAL datset object. 
    double geotransform[6];      // 6-element geotranform array.
    int dimensions[3];           // X,Y, and Z dimensions. 
    int NROWS, NCOLS, NLEVELS;     // dimensions of data in Geotiff. 

    //double NODATAVALUE;
    //const char* projection;

public:

    // define constructor function to instantiate object
    // of this Geotiff class. 
    GeoTIFFReader(const char* tiffname) {
        filename = tiffname;
        GDALAllRegister();

        // set pointer to Geotiff dataset as class member.  
        geotiffDataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);

        // set the dimensions of the Geotiff 
        NROWS = GDALGetRasterYSize(geotiffDataset);
        NCOLS = GDALGetRasterXSize(geotiffDataset);
        NLEVELS = GDALGetRasterCount(geotiffDataset);

        //NODATAVALUE = GetNoDataValue();
        //GetGeoTransform();
        //projection = GetProjection();

    }

    // define destructor function to close dataset, 
    // for when object goes out of scope or is removed
    // from memory. 
    ~GeoTIFFReader() {
        // close the Geotiff dataset, free memory for array.  
        GDALClose(geotiffDataset);
        GDALDestroyDriverManager();
    }

    const char* GetFileName() {
        /*
         * function GetFileName()
         * This function returns the filename of the Geotiff.
         */
        return filename;
    }

    const char* GetProjection() {
        /* function const char* GetProjection():
         *  This function returns a character array (string)
         *  for the projection of the geotiff file. Note that
         *  the "->" notation is used. This is because the
         *  "geotiffDataset" class variable is a pointer
         *  to an object or structure, and not the object
         *  itself, so the "." dot notation is not used.
         */
        return geotiffDataset->GetProjectionRef();
    }

    double* GetGeoTransform() {
        /*
         * function double *GetGeoTransform()
         *  This function returns a pointer to a double that
         *  is the first element of a 6 element array that holds
         *  the geotransform of the geotiff.
         */
        geotiffDataset->GetGeoTransform(geotransform);
        return geotransform;
    }

    double GetNoDataValue() {
        /*
         * function GetNoDataValue():
         *  This function returns the NoDataValue for the Geotiff dataset.
         *  Returns the NoData as a double.
         */
        return (double)geotiffDataset->GetRasterBand(1)->GetNoDataValue();
    }

    int* GetDimensions() {
        /*
         * int *GetDimensions():
         *
         *  This function returns a pointer to an array of 3 integers
         *  holding the dimensions of the Geotiff. The array holds the
         *  dimensions in the following order:
         *   (1) number of columns (x size)
         *   (2) number of rows (y size)
         *   (3) number of bands (number of bands, z dimension)
         */
        dimensions[0] = NROWS;
        dimensions[1] = NCOLS;
        dimensions[2] = NLEVELS;
        return dimensions;
    }

    float** GetRasterBand(int z) {

        /*
         * function float** GetRasterBand(int z):
         * This function reads a band from a geotiff at a
         * specified vertical level (z value, 1 ...
         * n bands). To this end, the Geotiff's GDAL
         * data type is passed to a switch statement,
         * and the template function GetArray2D (see below)
         * is called with the appropriate C++ data type.
         * The GetArray2D function uses the passed-in C++
         * data type to properly read the band data from
         * the Geotiff, cast the data to float**, and return
         * it to this function. This function returns that
         * float** pointer.
         */

        float** bandLayer = new float* [NROWS];
        switch (GDALGetRasterDataType(geotiffDataset->GetRasterBand(z))) {
        case 0:
            return NULL; // GDT_Unknown, or unknown data type.
        case 1:
            // GDAL GDT_Byte (-128 to 127) - unsigned  char
            return GetArray2D<unsigned char>(z, bandLayer);
        case 2:
            // GDAL GDT_UInt16 - short
            return GetArray2D<unsigned short>(z, bandLayer);
        case 3:
            // GDT_Int16
            return GetArray2D<short>(z, bandLayer);
        case 4:
            // GDT_UInt32
            return GetArray2D<unsigned int>(z, bandLayer);
        case 5:
            // GDT_Int32
            return GetArray2D<int>(z, bandLayer);
        case 6:
            // GDT_Float32
            return GetArray2D<float>(z, bandLayer);
        case 7:
            // GDT_Float64
            return GetArray2D<double>(z, bandLayer);
        default:
            break;
        }
        return NULL;
    }

    template<typename T>
    float** GetArray2D(int layerIndex, float** bandLayer) {

        /*
         * function float** GetArray2D(int layerIndex):
         * This function returns a pointer (to a pointer)
         * for a float array that holds the band (array)
         * data from the geotiff, for a specified layer
         * index layerIndex (1,2,3... for GDAL, for Geotiffs
         * with more than one band or data layer, 3D that is).
         *
         * Note this is a template function that is meant
         * to take in a valid C++ data type (i.e. char,
         * short, int, float), for the Geotiff in question
         * such that the Geotiff band data may be properly
         * read-in as numbers. Then, this function casts
         * the data to a float data type automatically.
         */

         // get the raster data type (ENUM integer 1-12, 
         // see GDAL C/C++ documentation for more details)        
        GDALDataType bandType = GDALGetRasterDataType(
            geotiffDataset->GetRasterBand(layerIndex));

        // get number of bytes per pixel in Geotiff
        int nbytes = GDALGetDataTypeSizeBytes(bandType);

        // allocate pointer to memory block for one row (scanline) 
        // in 2D Geotiff array.  
        T* rowBuff = (T*)CPLMalloc(nbytes * NCOLS);

        for (int row = 0; row < NROWS; row++) {     // iterate through rows

          // read the scanline into the dynamically allocated row-buffer       
            CPLErr e = geotiffDataset->GetRasterBand(layerIndex)->RasterIO(
                GF_Read, 0, row, NCOLS, 1, rowBuff, NCOLS, 1, bandType, 0, 0);
            if (!(e == 0)) {
                cout << "Warning: Unable to read scanline in Geotiff!" << endl;
                exit(1);
            }

            bandLayer[row] = new float[NCOLS];
            for (int col = 0; col < NCOLS; col++) { // iterate through columns
                bandLayer[row][col] = (float)rowBuff[col];
            }
        }
        CPLFree(rowBuff);
        return bandLayer;
    }
};

class GeoTIFFWriter
{
private:
    GDALDriver* driverGeotiff;

public:

    GeoTIFFWriter() {
        GDALAllRegister();
        driverGeotiff = GetGDALDriverManager()->GetDriverByName("GTiff");
    }

    ~GeoTIFFWriter() {
        GDALDestroyDriverManager();
    }

    void WriteGeoTIFFByLine(const char* tiffname, double* geotransform, const char* projection, int nrows, int ncols, double nodata, float** banddata) {

        GDALDataset* geotiffDataset;
        
        int NROWS;
        int NCOLS;

        NROWS = nrows;
        NCOLS = ncols;  
        
        //printf("%d %d %s %s\n", ncols, nrows, tiffname, projection);

        geotiffDataset = driverGeotiff->Create(tiffname, ncols, nrows, 1, GDT_Float32, NULL);
        /*
        char** option = new char* [1]; // COMPRESSION NOT WORKING AS EXPECTED
        option[0] = { "COMPRESS=DEFLATE" };
        geotiffDataset = driverGeotiff->Create(tiffname, ncols, nrows, 1, GDT_Float32, option);
        */

        //printf("***\n");

        if(geotransform != NULL) geotiffDataset->SetGeoTransform(geotransform);
        if (projection != NULL) geotiffDataset->SetProjection(projection);
        geotiffDataset->GetRasterBand(1)->SetNoDataValue(nodata);

        //printf("***\n");

        /* allocate memory block (essentially sequential array of
         * memory cells) to hold a row's worth of array data (from
         * within the 2D array). To this end, we create a
         * pointer variable that holds the memory address,
         * or "points to", a float. This float is actually
         * the first element of an array that is pointed
         * to by *rowBuff.
         */

        float* rowBuff = (float*)CPLMalloc(sizeof(float) * NCOLS);

        /* iterate through all pixels in 2D dBZ
         * reflectivity array and assign values into rowBuff
         * array. Then write each row into Geotiff object.
         */

        for (int row = 0; row < NROWS; row++) {
            for (int col = 0; col < NCOLS; col++) {
                rowBuff[col] = (float)banddata[row][col];
            }
            geotiffDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, row, NCOLS, 1, rowBuff, NCOLS, 1, GDT_Float32, 0, 0);
        }
        
        //compute and set statistics
        double bMin, bMax, bMean, bStd;
        geotiffDataset->GetRasterBand(1)->ComputeStatistics(true, &bMin, &bMax, &bMean, &bStd, NULL, NULL);
        geotiffDataset->GetRasterBand(1)->SetStatistics(bMin, bMax, bMean, bStd);

        // close the Geotiff dataset, free memory for array.  
        GDALClose(geotiffDataset);      

        CPLFree(rowBuff);

    }


    void WriteGeoTIFF(const char* tiffname, double* geotransform, const char* projection, int nrows, int ncols, double nodata, float* banddata) {
        
        auto t1 = high_resolution_clock::now();

        GDALDataset* geotiffDataset;

        int NROWS;
        int NCOLS;

        NROWS = nrows;
        NCOLS = ncols;

        geotiffDataset = driverGeotiff->Create(tiffname, ncols, nrows, 1, GDT_Float32, NULL);

        if (geotransform != NULL) geotiffDataset->SetGeoTransform(geotransform);
        if (projection != NULL) geotiffDataset->SetProjection(projection);
        geotiffDataset->GetRasterBand(1)->SetNoDataValue(nodata);
        /* allocate memory block (essentially sequential array of
         * memory cells) to hold a row's worth of array data (from
         * within the 2D array). To this end, we create a
         * pointer variable that holds the memory address,
         * or "points to", a float. This float is actually
         * the first element of an array that is pointed
         * to by *rowBuff.
         */

        geotiffDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, NCOLS, NROWS, banddata, NCOLS, NROWS, GDT_Float32, 0, 0);

        //compute and set statistics
        double bMin, bMax, bMean, bStd;
        geotiffDataset->GetRasterBand(1)->ComputeStatistics(true, &bMin, &bMax, &bMean, &bStd, NULL, NULL);
        geotiffDataset->GetRasterBand(1)->SetStatistics(bMin, bMax, bMean, bStd);

        // close the Geotiff dataset, free memory for array.  
        GDALClose(geotiffDataset);

        auto t2 = high_resolution_clock::now();
        /* Getting number of milliseconds as a double. */
        duration<double, std::milli> ms_double = t2 - t1;
        printf("...writing %s took %f ms\n", tiffname, ms_double.count());

    }

};