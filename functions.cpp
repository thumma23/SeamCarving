#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include "functions.h"

#define INFO(X) cout << "[INFO] ("<<__FUNCTION__<<":"<<__LINE__<<") " << #X << " = " << X << endl;

using std::cout;
using namespace std;
using std::endl;
using std::string;

Pixel** createImage(int width, int height) {
  cout << "Start createImage... " << endl;
  
  // Create a one dimensional array on the heap of pointers to Pixels 
  //    that has width elements (i.e. the number of columns)
  Pixel** image = new Pixel*[width];
  
  bool fail = false;
  
  for (int i=0; i < width; ++i) { // loop through each column
    // assign that column to a one dimensional array on the heap of Pixels
    //  that has height elements (i.e. the number of rows)
    image[i] = new Pixel[height];
    
    if (image[i] == nullptr) { // failed to allocate
      fail = true;
    }
  }
  
  if (fail) { // if any allocation fails, clean up and avoid memory leak
    // deallocate any arrays created in for loop
    for (int i=0; i < width; ++i) {
      delete [] image[i]; // deleting nullptr is not a problem
    }
    delete [] image; // dlete array of pointers
    return nullptr;
  }
  
  // initialize cells
  //cout << "Initializing cells..." << endl;
  for (int row=0; row<height; ++row) {
    for (int col=0; col<width; ++col) {
      //cout << "(" << col << ", " << row << ")" << endl;
      image[col][row] = { 0, 0, 0 };
    }
  }
  cout << "End createImage... " << endl;
  return image;
}

void deleteImage(Pixel** image, int width) {
  cout << "Start deleteImage..." << endl;
  // avoid memory leak by deleting the array
  for (int i=0; i<width; ++i) {
    delete [] image[i]; // delete each individual array placed on the heap
  }
  delete [] image;
  image = nullptr;
}

int* createSeam(int length) {
  int* array = new int[length];

  for(int i = 0; i < length; i++){
    array[i] = 0;
  }
  return array;
}

void deleteSeam(int* seam) {
    delete[] seam;
    seam = nullptr;
}

bool loadImage(string filename, Pixel** image, int width, int height) {
    ifstream ifs(filename);

    if(!ifs.is_open()){
      cout << "Error: failed to open input file - " << filename << endl;
      return false;
    }

    char type[3];

    ifs >> type;
    if((toupper(type[0]) != 'P') || (type[1] != '3')){
      cout << "Error: type is " << type << " instead of P3" << endl;
      return false;
    }

    int w;
    int h;
    ifs >> w >> h;

    if(ifs.fail()){
      cout << "Error: read non-integer value" << endl;
      return false;
    }

    if(w != width){
      cout << "Error: input width (" << width << ") does not match value in file (" << w << ")" << endl;
      return false;
    }
    if(h != height){
      cout << "Error: input height (" << height << ") does not match value in file (" << h << ")" << endl;
      return false;
    }

    int colorMax;
    ifs >> colorMax;
    if(colorMax != 255){
      cout << "Error: file is not using RGB color values. " << endl;
      return false;
    }

    int red = 0;
    int green = 0;
    int blue = 0;

    for(int i = 0; i < height; i++){
      for(int j = 0; j < width; j++){
        ifs >> red;
        ifs >> green;
        ifs >> blue;

        if(ifs.fail() && !(ifs.eof())){
          cout << "Error: read non-integer value" << endl;
          return false;
        }

        if(red < 0 || red > 255){
          cout << "Error: invalid color value " << red << endl;
          return false;
        }
        if(green < 0 || green > 255){
          cout << "Error: invalid color value " << green << endl;
          return false;
        }
        if(blue < 0 || blue > 255){
          cout << "Error: invalid color value " << blue << endl;
          return false;
        }

        image[j][i].r = red;
        image[j][i].b = blue;
        image[j][i].g = green;

        if(ifs.fail() && ifs.eof()){
          cout << "Error: not enough color values" << endl;
          return false;
        }  
      }
    }

    int extraVal;

    if(ifs >> extraVal){
      if(!(ifs.fail())){
        cout << "Error: too many color values" << endl;
        return false;
      }
    }

  return true;
}

bool outputImage(string filename, Pixel** image, int width, int height) {
  ofstream ofs(filename);

  if(!ofs.is_open()){
    cout << "Error: failed to open output file - " << filename << endl;
    return false;
  }

  ofs << "P3" << endl;
  ofs << width << " " << height << endl;
  ofs << 255 << endl;

  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      ofs << image[col][row].r << " ";
      ofs << image[col][row].g << " ";
      ofs << image[col][row].b << " ";
    }
    ofs << endl;
  }

  return true;
}

int energy(Pixel** image, int x, int y, int width, int height) { 
  Pixel left, right, up, down;
  int xDiff = 0;
  int yDiff = 0;

  if(x == 0){
    left = image[width-1][y];
  } else {
    left = image[x - 1][y];
  }
  if(x == (width - 1)){
    right = image[0][y];
  } else {
    right = image[x + 1][y];
  }
  if(y == 0){
    up = image[x][height - 1];
  } else {
    up = image[x][y - 1];
  }
  if(y == (height - 1)){
    down = image[x][0];
  } else {
    down = image[x][y + 1];
  }

  xDiff = pow((right.r - left.r), 2) + pow((right.g - left.g), 2) + pow((right.b - left.b), 2);
  yDiff = pow((up.r - down.r), 2) + pow((up.g - down.g), 2) + pow((up.b - down.b), 2);
  return  xDiff + yDiff;
}

int loadVerticalSeam(Pixel** image, int start_col, int width, int height, int* seam) {
  int energy_middle;
  int energy_left;
  int energy_right;
  int energy_of_seam = energy(image, start_col, 0, width, height);
  seam[0] = start_col;
  for(int i = 1; i < height; i++){
    if(start_col == 0){
      energy_middle = energy(image, start_col, i, width, height);
      energy_left = energy(image, start_col + 1, i, width, height);
      energy_right = 1147483647;
    }
    else if(start_col == width - 1){
      energy_middle = energy(image, start_col, i, width, height);
      energy_left = 1147483647;
      energy_right = energy(image, start_col - 1, i, width, height);
    }else{
      energy_middle = energy(image, start_col, i, width, height);
      energy_left = energy(image, start_col + 1, i, width, height);
      energy_right = energy(image, start_col - 1, i, width, height);
    }
    
    if(energy_middle <= energy_left && energy_middle <= energy_right){
      energy_of_seam += energy_middle;
      seam[i] = start_col;
      start_col = seam[i];
    }
    else if(energy_left <= energy_middle && energy_left <= energy_right){
      energy_of_seam += energy_left;
      seam[i] = start_col + 1;
      start_col = seam[i];
    }else{
      energy_of_seam += energy_right;
      seam[i] = start_col - 1;
      start_col = seam[i];
    }

  }
  return energy_of_seam;
}

int loadHorizontalSeam(Pixel** image, int start_row, int width, int height, int* seam) {
  int energy_middle;
  int energy_left;
  int energy_right;
  int energy_of_seam = energy(image, 0, start_row, width, height);
  seam[0] = start_row;
  for(int i = 1; i < width; i++){
    if(start_row == 0){
      energy_middle = energy(image, i, start_row, width, height);
      energy_left = 1147483647;
      energy_right = energy(image, i, start_row + 1, width, height);
    }
    else if(start_row == height - 1){
      energy_middle = energy(image, i, start_row, width, height);
      energy_left = energy(image, i, start_row - 1, width, height);
      energy_right = 1147483647;
    }else{
      energy_middle = energy(image, i, start_row, width, height);
      energy_left = energy(image, i, start_row - 1, width, height);
      energy_right = energy(image, i, start_row + 1, width, height);
    }
    
    if(energy_middle <= energy_left && energy_middle <= energy_right){
      energy_of_seam += energy_middle;
      seam[i] = start_row;
      start_row = seam[i];
    }
    else if(energy_left <= energy_middle && energy_left <= energy_right){
      energy_of_seam += energy_left;
      seam[i] = start_row - 1;
      start_row = seam[i];
    }else{
      energy_of_seam += energy_right;
      seam[i] = start_row + 1;
      start_row = seam[i];
    }

  }
  return energy_of_seam;
}

int* findMinVerticalSeam(Pixel** image, int width, int height) {
  int* seam = createSeam(height);
  int min_seam_energy = loadVerticalSeam(image, 0, width, height, seam);
  for(int i = 1; i < width; i++){
    int* comparison_seam = createSeam(height);
    int comparison_energy = loadVerticalSeam(image, i, width, height, comparison_seam);
    if(comparison_energy < min_seam_energy){
      min_seam_energy = comparison_energy;
      deleteSeam(seam);
      seam = comparison_seam;
    }else{
      deleteSeam(comparison_seam);
    }
  }
  return seam;
}

int* findMinHorizontalSeam(Pixel** image, int width, int height) {
  int* seam = createSeam(width);
  int min_seam_energy = loadHorizontalSeam(image, 0, width, height, seam);
  for(int i = 1; i < height; i++){
    int* comparison_seam = createSeam(width);
    int comparison_energy = loadHorizontalSeam(image, i, width, height, comparison_seam);
    if(comparison_energy < min_seam_energy){
      min_seam_energy = comparison_energy;
      deleteSeam(seam);
      seam = comparison_seam;
    }else{
      deleteSeam(comparison_seam);
    }
  }
  return seam;
}

void removeVerticalSeam(Pixel** image, int width, int height, int* verticalSeam) {
  verticalSeam = findMinVerticalSeam(image, width, height);
  for(int i = 0; i < height; i++){
    int column_to_remove = verticalSeam[i];
    for(int j = column_to_remove; j < width - 1; j++){
      image[j][i] = image[j + 1][i];
    }
  }
  deleteSeam(verticalSeam);
}

void removeHorizontalSeam(Pixel** image, int width, int height, int* horizontalSeam) {
  horizontalSeam = findMinHorizontalSeam(image, width, height);
  for(int i = 0; i < width; i++){
    int row_to_remove = horizontalSeam[i];
      for(int j = row_to_remove; j < height - 1; j++){
        image[i][j] = image[i][j + 1];
      }
    }
  deleteSeam(horizontalSeam);
}