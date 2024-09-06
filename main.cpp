#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
using namespace std;

uint16_t read2Bytes(ifstream &file){
  uint16_t bytes;
  file.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));
  return bytes;
}

uint32_t read4Bytes(ifstream &file){
  uint32_t bytes;
  file.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));
  return bytes;
}


vector<int> readPixel(ifstream &file){
  vector<int> pixel;
  for (int i = 0; i < 3; i++) {
    uint8_t byte;
    file.read(reinterpret_cast<char*>(&byte), sizeof(byte));
    pixel.push_back(byte);
  }
  return pixel;
}

int normalizeRBGValue(int value){
  if(value <= 30) return 0;
  else if (value <= 70) return 1;
  else if (value <= 120) return 2;
  else if (value <= 170) return 3;
  else if (value <= 220) return 4;
  else if (value <= 256) return 5;
  return 0;
}


string rs = ".+>{T@";
string bs = ",-<)OS";
string gs = "`^*/S#";

char RBGtoASCII(int r, int b, int g){
  if(r >= b && r >= g) return rs[normalizeRBGValue(r)];
  else if(b >= g && b >= r) return bs[normalizeRBGValue(b)];
  else if (g >= r && g >= b) return gs[normalizeRBGValue(g)];
  else return '#';
}

int main(int argc, char *argv[]){
  if(argc < 2) {
    cout << "Usage: ./main <image_file.bmp>\n";
    return 0;
  }

  string imageFileName = argv[1];
  ifstream image(imageFileName, ios::binary);
  if(!image) {
    cout << "The image file didn't open\n"; 
    return 0;
  }
  
  string outputFileName = imageFileName.substr(0, imageFileName.size()-4) + ".txt";
  fstream output;
  output.open("./" + outputFileName, ios::out | ios::trunc);
  
  if (!output) {
    cout << "Error writing to output file\n";
    return 0;
  }

  uint16_t trashData2Bytes;
  uint32_t trashDAta4Bytes;
  
  // file header
  char signature[2];
  image.read(signature, 2); 
  output << "file type: " << signature << "\n";
  if(signature[0] != 'B' || signature[1] != 'M'){
    cout << "ERROR: Only BMP files are supported\n";
    return 0;
  }

  uint32_t filesize = read4Bytes(image);
  output << "file size: " << filesize << " bytes\n";

  uint32_t reserved = read4Bytes(image);
  output << "reserved: " << reserved << "\n";

  uint32_t dataOffset = read4Bytes(image);
  output << "Data Offset: " << dataOffset << "\n";

  // DIB header
  uint32_t headerSize = read4Bytes(image);
  output << "Headere size: " << headerSize << "\n";

  uint32_t imageWidthInPixels = read4Bytes(image);
  output << "Image width: " << imageWidthInPixels << " px\n";

  uint32_t imageHeightInPixels = read4Bytes(image);
  output << "Image height: " << imageHeightInPixels << " px\n";
  
  trashData2Bytes = read2Bytes(image); // Color Planes
  
  uint32_t bitsPerPixel = read2Bytes(image);
  output << "Bits per pixel: " << bitsPerPixel << " bits\n";

  trashDAta4Bytes = read4Bytes(image); // Compression type
  trashDAta4Bytes = read4Bytes(image); // horizontal resolution
  trashDAta4Bytes = read4Bytes(image); // vertical resolution
  trashDAta4Bytes = read4Bytes(image); // colors in color pallete
  trashDAta4Bytes = read4Bytes(image); // important colors

  if (bitsPerPixel != 24) {
      cout << "ERROR: Only 24-bit BMP files are supported" << endl;
      return 1;
  }

  // pixels 
  
  vector<vector<char>> asciiImage(imageHeightInPixels, vector<char>(imageWidthInPixels));

  int rowPadding = (4 - (imageWidthInPixels * 3) % 4) % 4;
  
  for (int i = imageHeightInPixels-1; i >= 0; i--) {
    for (int j = 0; j < imageWidthInPixels; j++) {
        vector<int> pixelRBG = readPixel(image);
        int blue = pixelRBG[0], green = pixelRBG[1], red = pixelRBG[2];
        asciiImage[i][j] = RBGtoASCII(red, blue, green);
    }
    // Skip row padding bytes
    image.seekg(rowPadding, ios::cur);
  }


  for (int i = 0; i < imageHeightInPixels; i++) {
    for (int j = 0; j < imageWidthInPixels; j++) {
      output << asciiImage[i][j] << asciiImage[i][j];
       
      output.flush(); 
    }
    output << endl;
  }
  

  cout << "The image was converted succesfully !\n" << endl;
  output.close();
  image.close();
  return 0;
}
