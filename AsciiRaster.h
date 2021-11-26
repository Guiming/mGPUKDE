// Copyright 2016 Guiming Zhang (gzhang45@wisc.edu)
// Distributed under GNU General Public License (GPL) license

#ifndef _ASCIIRASTER_H_
#define _ASCIIRASTER_H_


// AsciiRaster Structure declaration
typedef struct {
	size_t nCols;
	size_t nRows;
	float xLLCorner;
	float yLLCorner;
	float cellSize;
	float noDataValue;
	float* elements;
	
	//Guiming 2021-08-18
	size_t start; //Used to identify starting index (in elements) when cells are being worked with accross multiple GPUs, inclusive
	size_t end; //Ending index, exclusive

	double* geotransform;
	const char* projection;

} AsciiRaster;


#endif // _ASCIIRASTER_H_