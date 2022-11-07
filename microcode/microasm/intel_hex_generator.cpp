/* Intel HEX File Generator
 * Author: Joey Herguth
 * Last change: 7/18/2019
 * Description: Generates an Intel HEX file from my custom ROM description format: Put the binary, octal, decimal, or hexidecimal
   values into a file in order of occurance in the ROM. The base address is always zero. Comments are allowed and must use either
   "//" or ';'.
 */

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

char toHexChar(const int);
string toHex(const int);
int hexToDec(const char);

int main(int argc, char **argv) {
   
   ifstream sourceFile;
   string sourceFileName;
   ofstream hexFile;
   string hexFileName;
   string response;
   int base;
   string fileLine;
   string temp;
   int character;
   long int address = 0;
   int check;
   
   //Get the filename of the ROM description file.
   // do {
   //    cout << "Enter the name of the ROM description file." << endl;
   //    getline(cin, sourceFileName);

   if (argc > 2) {
      sourceFileName = argv[2];
      sourceFile.open(sourceFileName);
   } else {
      cout << "ERROR: Input file is required." << endl;
      exit(1);
   }

   //Open the file.
   if (!sourceFile) {
      // cout << "File could not be opened. Please try again." << endl;
      perror("ERROR");
      abort();
   }
   // } while(!sourceFile);
   
   //Ask if the user wants to use the source file name for the HEX file (with a .hex extention).
   // do {
   //    do {
   //       cout << "Do you want to use the same name as the source file? (y/n)" << endl;
   //       getline(cin, response);
   //    } while (!((response[0] == 'y') || (response[0] == 'n')));
      
      //Convert the source file name to a HEX file name.
      // if (response[0] == 'y') {
      if (argc > 3) {
         hexFileName = argv[3];
         hexFileName = sourceFileName.substr(0, sourceFileName.find_last_of('.')) + ".hex";
         cout << hexFileName << endl;
      //Get the HEX file name from the user.
      } else {
         hexFileName = sourceFileName.substr(0, sourceFileName.find_last_of('.')) + ".hex";
         cout << hexFileName << endl;
         // cout << "Enter the name of the HEX file." << endl;
         // getline(cin, hexFileName);
      }
      
      //Open the file.
      hexFile.open(hexFileName.c_str());
      
      if (!hexFile) {
         // cout << "File could not be opened. Please try again." << endl;
         perror("ERROR");
         abort();
      }
   // } while (!hexFile);
   
   //Get the number sytem base of the data.
   // do {
   //    cout << "Is the source file formatted in hex (h), decimal (d), octal (o), or binary (b)?" << endl;
   //    cin >> response;
   // } while ((response[0] != 'h') && (response[0] != 'd') && (response[0] != 'o') && (response[0] != 'b'));
   if (argc > 1) {
      response = argv[1];
   } else {
      cout << "ERROR: Data format is required (h, d, o, b)." << endl;
      exit(1);
   }
   
   switch (response[0]) {
      case 'h': base = 16; break;
      case 'd': base = 10; break;
      case 'o': base = 8; break;
      case 'b': base = 2; break;
      default:
         cerr << "ERROR: Unrecognized data format. Valid formats: h, d, o, b" << endl;
         exit(1);
   }
   
   //Repeat for each line.
   while (!sourceFile.eof()) {
      getline(sourceFile, fileLine);
      
      //Condense line just to readable characters. Possibly oversimplified.
      temp = "";
      for (int i = 0; i < fileLine.length(); i++) {
         if ('/' <= fileLine[i]) {
            if (base == 16) {
               if ((fileLine[i] == '/') || (fileLine[i] == ';') || (fileLine[i] <= '9') || (('a' <= fileLine[i]) && (fileLine[i] <= 'f')) || (('A' <= fileLine[i]) && (fileLine[i] <= 'F'))) {
                  temp += fileLine[i];
               }
            } else {
               if ((fileLine[i] == '/') || (fileLine[i] == ';') || (fileLine[i] <= (base + '0'))) {
                  temp += fileLine[i];
               }
            }
         }
      }
      
      //Skip the convert to HEX for this line if the line is empty or a comment.
      if ((temp == "") || (temp.find(';') != string::npos) || (temp.find('/') != string::npos))
         continue;
      
      fileLine = temp;
      
      //Convert data to hexadecimal.
//       temp = "";
      if (base != 16) {       //Data may already be in the proper format. Why waste time?
         temp = "";
         for (int i = 0; i < fileLine.length();) {
            switch (base) {
               case 2:
                  character = (fileLine[i] == '1') ? 8 : 0;
                  i += (i < fileLine.length()-1) ? 1 : 0;
                  character += (fileLine[i] == '1') ? 4 : 0;
                  i += (i < fileLine.length()-1) ? 1 : 0;
                  character += (fileLine[i] == '1') ? 2 : 0;
                  i += (i < fileLine.length()-1) ? 1 : 0;
                  character += (fileLine[i] == '1') ? 1 : 0;
                  i += (i < fileLine.length()) ? 1 : 0;
               break;
               case 8:
                  cout << "Code this yourself." << endl;
                  return 0;
               break;
               case 10:
                  cout << "Code this yourself." << endl;
                  return 0;
               break;
               default:
                  cerr << "ERROR: 2" << endl;
                  return 2;
            }
            temp += toHexChar(character);
         }
      }
      
      fileLine = temp;
      
      //Convert line to Intel HEX format.
      if ((fileLine.length() % 2) != 0) {
         fileLine = fileLine.substr(0, fileLine.length() - 1);
      }
      
      //temp = number of bytes of data + base address + "00" + hex data
      temp = toHex(fileLine.length()/2) + toHex((address>>8) & 0x00FF) + toHex(address & 0x00FF) + "00" + fileLine;
      
      //Increase the ROM address.
      address += fileLine.length() / 2;
      
      //Write the hex code to the HEX file.
      hexFile << ":" << temp;
      
      //Generate checksum.
      check = 0;
      for (int i = 0; i < temp.length(); i += 2) {
         check += hexToDec(temp[i]) * 16 + hexToDec(temp[i+1]);
      }
      
      //Write checksum to end of the line of the HEX file.
      hexFile << toHex((~check + 1) & 0xFF) << endl;
   }
   
   //End of file.
   hexFile << ":00000001FF";
   
   //Close files.
   sourceFile.close();
   hexFile.close();
   
   cout << "Finished." << endl;
   
   return 0;
}

char toHexChar(const int num) {
   if (num > 0) {
      if (num < 10) {
         return static_cast<char>(num + '0');
      } else if (num < 16) {
         return static_cast<char>(num + 'A' - 10);
      } else {
         return '0';
      }
   } else {
      return '0';
   }
}

string toHex(const int num) {
   return string(1, toHexChar((num>>4) & 0x0F)) + string(1, toHexChar(num & 0x0F));
}

int hexToDec(const char num) {
   if (('0' <= num) && (num <= '9')) {
      return static_cast<int>(num) - '0';
   } else if (('A' <= num) && (num <= 'F')) {
      return static_cast<int>(num) + 10 - 'A';
   } else if (('a' <= num) && (num <= 'f')) {
      return static_cast<int>(num) + 10 - 'a';
   } else {
      return 0;
   }
}
