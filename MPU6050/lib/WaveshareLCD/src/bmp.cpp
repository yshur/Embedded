#include <SPIFFS.h>

#include "bmp.h"

#define BUFFPIXEL_X3(__val)    ( (__val) * 3)                 // BUFFPIXELx3
#define RGB24TORGB565(R,G,B) (( (R) >> 3 ) << 11 ) | (( (G) >> 2 ) << 5) | ( (B) >> 3)
#define RGB555TORGB565(L,H)  (((H)<<9) | (((L)&0xC0)<<1) | ((L)&0x1F))

BMP_HDR BMPhdr;                         //BMP file header: See LCD_BMP.h

//  === BMP Memory File Class Methods ==============================================

/// @author Amit Resh June 2025
/// @brief Add record to BmpMemoryFile linked-list
/// @param head Head of linkedList
/// @param data pointer to data
/// @param len length of data
/// @return success flag
bool BmpMF::BmpMFwrite(unsigned char *data, size_t len)
{
    Rec *tail = head;

    size_t contigeousBlk = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    if ( contigeousBlk < 2 * 1436 || contigeousBlk < len+sizeof(Rec)) {           //Verify there is enough contigeous memory to continue
      Serial.println("\nWarning: low memory, refusing upload");
      // Serial.printf("Memory Allocation free:%d (Contigeous:%d)\n", ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
      BmpMFfree();
      return false;
    }

    Rec *new_item = (Rec *)malloc(sizeof(Rec));
    new_item->record = (unsigned char*)malloc(len);
    memmove(new_item->record, data, len);
    new_item->len = len;
    new_item->next = NULL;        //setup new_item as the last item

    if (head == NULL) {           //Was list empty to begin with?
      RdRec = head = new_item;    //Yes. new_item is the head
      return true;                     // and we're done
    }

    while (tail->next != NULL)  //Find last Item
        tail = tail->next;

    tail->next = new_item;      //Link it to new_item, which becomes the last
    return true;
}

/// @author Amit Resh June 2025
/// @brief Free BMP Memory File Memory
/// @param head 
void BmpMF::BmpMFfree(void)
{
	Rec* to_free = head;
	while (to_free != NULL)
	{
		head = head->next;
    free(to_free->record);
		free(to_free);
		to_free = head;
	}
}

/// @author Amit Resh June 2025
/// @brief Read the next <len> bytes from the BMP Memory File and store in <data>
/// @param data Pointer to store retrieved data
/// @param len  length of required data
/// @return actual length retrieved
size_t BmpMF::BmpMFread(unsigned char *data, size_t len)
{
  if (RdRec==nullptr)
    return 0;                                                     //Return <<Error>> if no more data

  int16_t currRecLen = RdRec->len - RdOffset;
    //If current record has enough data to retrieve
  if (currRecLen > len) {
    memmove(data, RdRec->record+RdOffset, len);
    RdOffset += len;
    return len;
  }
    //If not enough in current record:
  memmove(data, RdRec->record+RdOffset, currRecLen);              //Move what is available
  RdRec = RdRec->next;                                            //Step over to next record
  RdOffset = 0;
  return currRecLen+BmpMFread(data+currRecLen, len-currRecLen);   //Recursively get the rest
}

/// @author Amit Resh June 2025
/// @brief Rewind the memory file to read it again
void BmpMF::BmpMFrewind(void)
{
  RdRec=head;
  RdOffset=0;
}

/// @author Amit Resh June 2025
/// @brief Seek to position in BMP Memory File
/// @param pos Position to goto
/// @param mode Seek from beginning (Abs) or current position (Rel)
void BmpMF::BmpMFseek(size_t pos, enum _mode mode)
{
  if (mode==Abs)
    BmpMFrewind();
  int16_t currRecLen = RdRec->len - RdOffset;
    //If current record has enough data to retrieve
  if (currRecLen > pos) {
    RdOffset += pos;
    return;
  }
    //If not enough in current record:
  RdRec = RdRec->next;                //Step over to next record
  RdOffset = 0;
  BmpMFseek(pos-currRecLen, Rel);     //Recursively get the rest
}

//==============================================================================

/// @author Amit Resh May 2025
/// @brief Printout BMP Header to Serial
void PrintBmpHeader(void)
{
  Serial.printf("\nBMPhdr.signature:%x\n",BMPhdr.signature);
  Serial.printf("BMPhdr.fileSz:%d\n",BMPhdr.fileSz);
  Serial.printf("BMPhdr.dataOffset:%d\n",BMPhdr.dataOffset);
  Serial.printf("BMPhdr.InfoBMPhdrSz:%d\n",BMPhdr.InfoHdrSz);
  Serial.printf("BMPhdr.Width:%d\n",BMPhdr.Width);
  Serial.printf("BMPhdr.Height:%d\n",BMPhdr.Height);
  Serial.printf("BMPhdr.planes:%d\n",BMPhdr.planes);
  Serial.printf("BMPhdr.bitCount:%d\n",BMPhdr.bitCount);
  Serial.printf("BMPhdr.ImageSz:%d\n",BMPhdr.ImageSz);
  Serial.printf("BMPhdr.ColorsUsed:%d\n",BMPhdr.ColorsUsed);
}

/// @author Amit Resh May 2025
/// @brief Read a BMP header from (already) open file
/// @param f - Open File
/// @return Boolean success
boolean ReadBmpHeader(File f)
{ 
  f.readBytes((char*)&BMPhdr, sizeof(BMPhdr));

  if (BMPhdr.signature != 0x4D42)
    return false;                       // magic bytes 'BM' missing

  if (BMPhdr.planes != 1)
    return false;

  return true;
}

/// @author Amit Resh June 2025
/// @brief Read a BMP header from a BMP Memory File
/// @param bmpMF a BMP Memory File object
/// @return Boolean success
boolean ReadBmpHeader(BmpMF& bmpMF)
{
  bmpMF.BmpMFrewind();
  bmpMF.BmpMFread((unsigned char*)&BMPhdr, sizeof(BMPhdr));

  if (BMPhdr.signature != 0x4D42)
    return false;                       // magic bytes 'BM' missing

  if (BMPhdr.planes != 1)
    return false;

  return true;
}


/// @author Amit Resh May 2025
/// @brief Dump Contents of BMP file (HEX)
/// @param fileName Name of file in SPIFFS (must start with '/')
void dump_bmp(char *fileName)
{
    File bmpFile;
    bool ARGB_Format=true;

    bmpFile = SPIFFS.open(fileName);
    if (!bmpFile || !bmpFile.available()) {
        Serial.printf("File %s not Found\n", fileName);
        return ;
    }

    if (!ReadBmpHeader(bmpFile)) {
        Serial.printf(" %s - Bad BMP file\n", fileName);
        return;
    }

    PrintBmpHeader();           //Display BMP Header Fields

    uint16_t bytePerPix = BMPhdr.bitCount/8;
    uint16_t rasterWidth = BMPhdr.Width*bytePerPix;
    if (rasterWidth%4)
    rasterWidth += 4-(rasterWidth%4);
    uint8_t rasterLine[rasterWidth];                       //Buffer for one full raster line

    uint32_t LUT[BMPhdr.ColorsUsed];

    if (BMPhdr.ColorsUsed) {
        bmpFile.seek(14+40);                                 //Skip Headers to Color Pallette
        bmpFile.readBytes((char*)LUT, BMPhdr.ColorsUsed*4);
        for (int i=0 ; i<BMPhdr.ColorsUsed; ++i)
            ARGB_Format &= (LUT[i]>>24)==0xFF;
        Serial.printf("Using a color palette Format: %s\n", ARGB_Format ? "ARGB":"BGR0");
    }

    bmpFile.seek( BMPhdr.dataOffset);

  for ( uint16_t line = 0; line < BMPhdr.Height; ++line) {  //Loop for all Raster lines
    bmpFile.read(rasterLine, rasterWidth);

    //  ======  Copy BMP line to LCD   ===================================================
    uint16_t k;
    for (uint16_t pix=0 ; pix<BMPhdr.Width ; ++pix) {
      k=pix*bytePerPix;
      switch (bytePerPix) {
        case 1:
        //   if (BMPhdr.ColorsUsed) {                                      //This BMP using a color pallette?
        //     uint8_t lutIdx = rasterLine[k];
        //     if (ARGB_Format)                                            //Using ARGB:
        //       SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>8)&0xFF, LUT[lutIdx]&0xFF));
        //     else                                                        //Using BGR0:
        //       SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>8)&0xFF, (LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>24)&0xFF));
        //   }                                                             //Using Monochrome (256 shades)
        //   else SPI4W_Write_Word(RGB24TORGB565(rasterLine[k], rasterLine[k], rasterLine[k]));
          break;
        case 2:
        //   SPI4W_Write_Word(RGB555TORGB565(rasterLine[k], rasterLine[k+1]));
          break;
        case 3:
          Serial.printf("(%02x,%02x,%02x) ", rasterLine[k + 2], rasterLine[k + 1], rasterLine[k]);
          break;
      }
    }
    Serial.println();
  }
  bmpFile.close();
}