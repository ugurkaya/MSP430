// BitmapToSerial.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SerialPort.h"
#include <ctime>
#include <chrono> 
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>


using std::cout;
using std::endl;
using namespace std::chrono;

#define IMAGE_BUFFER_SIZE 1024
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

/*Portname must contain these backslashes, and remember to
replace the following com port*/
const char *port_name = "\\\\.\\COM7";

unsigned char * convertToOLEDFormat(std::vector<unsigned char>* buffer) {

	unsigned char tmp = 0;
	int byteInPage = 0, page = 0;

	unsigned char * res = new unsigned char[1024];

	for (int i = 0; i < 1024; i++) {

		tmp |= (((*buffer)[(0 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 0;
		tmp |= (((*buffer)[(1 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 1;
		tmp |= (((*buffer)[(2 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 2;
		tmp |= (((*buffer)[(3 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 3;
		tmp |= (((*buffer)[(4 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 4;
		tmp |= (((*buffer)[(5 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 5;
		tmp |= (((*buffer)[(6 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 6;
		tmp |= (((*buffer)[(7 + page) * 16 + byteInPage] & (0x80 >> (i % 8))) >> (7 - i % 8)) << 7;

		res[i] = tmp;
		tmp = 0;
		if (i > 0 && i % 8 == 7) {
			if (byteInPage < 15) {
				byteInPage++;
			}
			else {
				byteInPage = 0; page += 8;
			}
		}
	}
	return res;

}

unsigned char * loadImage(const char* filename) {
	std::fstream fbin;

	unsigned char* displayBuffer = new unsigned char[1024];

	fbin.open(filename, std::ios::binary | std::ios::in | std::ios::out);
	if (!fbin)
	{
		cout << "Could not open file " << "face.bmp";
		//return -1;
	}
	if (fbin.is_open()) {
		fbin.seekg(0x3E, std::ios::beg);
		fbin.read((char *)displayBuffer, 1024);
		fbin.close();
	}
	return displayBuffer;
}

std::vector<unsigned char> * flipVertical(unsigned char* imageData) {

	std::vector<unsigned char> * formattedImageData = new std::vector<unsigned char>(1024);

	for (int i = 63; i >= 0; i--) {

		std::copy_n(imageData + i * 16, 16, formattedImageData->begin() + 63 * 16 - i * 16);

	}

	return formattedImageData;
}

int main()
{
	unsigned char* imagePixelData;
	unsigned char* oledPixelData;
	std::vector<unsigned char>* flippedImagePixelData;
	const time_t ctt = time(0);
	SerialPort serial(port_name);
	if (serial.isConnected()) cout << "Connection Established" << endl;
	else cout << "ERROR, check port name";

	int bufsize = 1024;

	char ts='0';
	while ( ts != 'e') {
		cout << "Loading image" << endl;
		imagePixelData = loadImage("face.bmp");

		flippedImagePixelData = flipVertical(imagePixelData);

		oledPixelData = convertToOLEDFormat(flippedImagePixelData);
		cout << "Image loaded!" << endl;
		cout << "Sending!" <<  endl;
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		serial.writeSerialPort(oledPixelData, bufsize);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		cout << "Sent! Duration:" << duration_cast<microseconds>(t2 - t1).count()<< endl;
		std::cin >> ts;
	}

}