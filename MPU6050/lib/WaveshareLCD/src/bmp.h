#pragma once
#include <Arduino.h>
#include <SPIFFS.h>

typedef struct __attribute__((packed)) {
  uint16_t signature;   //Should be 'BM'
  uint32_t fileSz;      //in bytes
  uint32_t rsrvd;
  uint32_t dataOffset;  //Offset to where raster starts
  uint32_t InfoHdrSz;   //Should be 40
  uint32_t Width;
  uint32_t Height;      //Image Size
  uint16_t planes;      //Number of planes, should be 1
  uint16_t bitCount;    //Bits per Pixel: 1:Monochrome; 4:4bit 16 colors; 8:8bit 256 colors; 16:2byte RGB 64K colors; 24:3byte RGB 16M colors
  uint32_t Compression;
  uint32_t ImageSz;
  uint32_t XpixPerM, YpixPerM;
  uint32_t ColorsUsed;  //How many colors in pallette
} BMP_HDR;

/// @author Amit Resh June 2025
/// @brief This Class stores a BMP file in a Linked-List
class BmpMF {
  enum _mode {Abs, Rel};
  typedef struct _record
  {
      unsigned char* record;
      int16_t len;
      struct _record *next;
  } Rec;

  Rec *head, *RdRec;
  int16_t RdOffset;
public:
  BmpMF(void) : head(nullptr), RdRec(nullptr), RdOffset(0) {}
  bool BmpMFwrite(unsigned char *data, size_t len);
  void BmpMFfree(void);
  size_t BmpMFread(unsigned char *data, size_t len);
  void BmpMFrewind(void);
  void BmpMFseek(size_t pos, enum _mode mode=Abs);
};

extern BMP_HDR BMPhdr;

boolean ReadBmpHeader(File f);
boolean ReadBmpHeader(BmpMF& bmpMF);
void PrintBmpHeader(void);
void dump_bmp(char *fileName);
