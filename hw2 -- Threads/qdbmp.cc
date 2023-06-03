#include <iostream>
#include <string>
#include "qdbmp.h"

// constants
namespace {
  const UINT BMP_DEPTH = 32; // specific to doge.bmp
}

// Implement BitMap class methods
BitMap::BitMap(UINT width, UINT height) {
  m_bmpPtr = BMP_Create(width, height, BMP_DEPTH);
}

BitMap::BitMap(std::string file) {
  m_bmpPtr = BMP_ReadFile(file.c_str());
}

BitMap::~BitMap() {
  BMP_Free(m_bmpPtr);
}

UINT BitMap::width() {
  return BMP_GetWidth(m_bmpPtr);
}

UINT BitMap::height() {
  return BMP_GetHeight(m_bmpPtr);
}

RGB BitMap::get_pixel(UINT x, UINT y) {
  UCHAR r, g, b;
  BMP_GetPixelRGB(m_bmpPtr, x, y, &r, &g, &b);
  return RGB(r, g, b);
}

void BitMap::set_pixel(UINT x, UINT y, RGB rgb) {
  BMP_SetPixelRGB(m_bmpPtr, x, y, rgb.r, rgb.g, rgb.b);
}

void BitMap::write_file(std::string file) {
  BMP_WriteFile(m_bmpPtr, file.c_str());
}

BMP_STATUS BitMap::check_error() {
  return BMP_GetError();
}