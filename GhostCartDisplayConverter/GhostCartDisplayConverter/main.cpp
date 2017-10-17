#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <map>

//  Specify the origin point (for our images we use (14, 13)
const int OriginX = 14;
const int OriginY = 13;

#define COMMON_COLOR_COUNT    63
const byte COMMON_COLORS[COMMON_COLOR_COUNT][3] =
{
	{   0,   0,   0 },  //  Black
	{ 255,   0,   0 },  //  Red
	{   0, 255,   0 },  //  Green
	{   0,   0, 255 },  //  Blue
	{ 200, 200,   0 },  //  Yellow
	{   0, 255, 255 },  //  Cyan (Mario Morph BG)
	{ 255,   0, 255 },  //  Magenta
	{ 255, 255, 255 },  //  White
	{ 100, 100, 100 },  //  Pale White (MegaMan BG)
	{  67,  79, 254 },  //  MegaMan Blue 1
	{  36, 242, 205 },  //  MegaMan Blue 2
	{ 251, 236, 154 },  //  MegaMan Skin
	{ 240, 208, 176 },  //  SMB3 Mario Skin
	{ 248, 056,   0 },  //  SMB3 Mario Raccoon 1
	{ 240, 144,  88 },  //  SMW2 Skin 1
	{ 232,  96,  80 },  //  SMW2 Skin 2
	{ 248, 248, 248 },  //  SMW2 Buttons
	{ 168,  80,   0 },  //  SMW2 Raccoon 1
	{ 120,  64,  40 },  //  SMW2 Raccoon 2
	{ 224, 128,  48 },  //  SMW2 Raccoon 3
	{ 240, 200,  48 },  //  SMW2 Raccoon 4
	{ 248, 184, 128 },  //  SMW2 Raccoon 5
	{  40,  72, 128 },  //  SMW2 Overalls 1 (dark blue)
	{  56, 112, 168 },  //  SMW2 Overalls 2 (light blue)
	{  72, 152, 208 },  //  SMW2 Overalls 3 (lightest blue)
	{ 160,   0,   0 },  //  SMW2 ShirtHat 1 (dark red)
	{ 200,   0,  24 },  //  SMW2 ShirtHat 2 (light red)
	{ 248,  32,  56 },  //  SMW2 ShirtHat 3 (lightest red)
	{ 231,  95,  19 },  //  SMB3 Tanuki 1
	{ 240, 208, 176 },  //  SMB3 Tanuki 2
	{ 144,  56,  24 },  //  SMW2 Tanuki 1 (dark brown)
	{ 184,  96,  24 },  //  SMW2 Tanuki 2 (light brown)
	{ 216, 128,  24 },  //  SMW2 Tanuki 3 (yellow brown)
	{ 184,  96,  24 },  //  SMW2 Tanuki 4 (lightest brown)
	{ 248,  56,   0 },  //  SMB3 FireFlower 1
	{ 255, 163,  71 },  //  SMB3 FireFlower 2
	{ 200,  16,   0 },  //  SMW2 FireFlower 1 (dark red)
	{ 248,  64,   0 },  //  SMW2 FireFlower 2 (orange)
	{ 248, 104,  32 },  //  SMW2 FireFlower 3 (light orange)
	{ 248, 208,  88 },  //  SMW2 FireHat 1 (yellow)
	{ 248, 240, 176 },  //  SMW2 FireHat 2 (light yellow)
	{  88,  72,  72 },  //  SMW2 hardhat 1 (dark gray)
	{ 136, 120, 120 },  //  SMW2 hardhat 2 (gray)
	{ 192, 176, 176 },  //  SMW2 hardhat 3 (light gray)
	{ 248, 208, 152 },  //  SMW2 YellowHat 2 (light yellow)
	{ 248, 176,  48 },  //  SMW2 YellowHat 1 (yellow)
	{ 176,  88,   0 },  //  SMW2 BrownSuit 1 (brown-yellow)
	{ 208, 128,  24 },  //  SMW2 BrownSuit 2 (yellow-brown)
	{ 255, 165,   0 },  //  Orange
	{ 128,   0, 128 },  //  Purple
	{ 253,  15,  15 },  //  Pac-Man Blinky Red
	{ 253, 150, 180 },  //  Pac-Man Pinky Pink
	{  93, 243, 212 },  //  Pac-Man Inky Blue
	{ 255, 171,  77 },  //  Pac-Man Clyde Orange
	{ 127, 127, 127 },  //  Tetris Background
	{ 248, 136,   0 },  //  MechaKoopa Orange 1
	{ 184,  40,   0 },  //  MechaKoopa Orange 2
	{   0, 120,   0 },  //  MechaKoopa Green 1
	{   0, 184,   0 },  //  MechaKoopa Green 2
	{   0, 248,   0 },  //  MechaKoopa Green 3
	{  48, 112, 128 },  //  MechaKoopa Cyan
	{ 110,   0, 220 },  //  Royal Purple
	{  32,  32, 192 },  //  Dead PacMan Ghost Skin Blue
};

std::map<int, void*> ColorsUsed;

struct ImageData
{
public:
	HANDLE hfile;
	DWORD written;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	RGBTRIPLE *image;
};

struct Color
{
	int R;
	int G;
	int B;

	Color(int r, int g, int b) : R(r), G(g), B(b) {}
	void Set(int r, int g, int b) { R = r; G = g; B = b; }
	bool Equals(const Color& other) const { return ((R == other.R) && (G == other.G) && (B == other.B)); }
	std::string String() const { return "Color(" + std::to_string(R) + ", " + std::to_string(G) + ", " + std::to_string(B) + ")";  }
};

void ReOrderColors(RGBTRIPLE& color)
{
	color.rgbtRed ^= color.rgbtBlue;
	color.rgbtBlue ^= color.rgbtRed;
	color.rgbtRed ^= color.rgbtBlue;
}


void SwapRGBTRIPLE(RGBTRIPLE& tripleA, RGBTRIPLE& tripleB)
{
	tripleA.rgbtRed ^= tripleB.rgbtRed;
	tripleB.rgbtRed ^= tripleA.rgbtRed;
	tripleA.rgbtRed ^= tripleB.rgbtRed;
	tripleA.rgbtGreen ^= tripleB.rgbtGreen;
	tripleB.rgbtGreen ^= tripleA.rgbtGreen;
	tripleA.rgbtGreen ^= tripleB.rgbtGreen;
	tripleA.rgbtBlue ^= tripleB.rgbtBlue;
	tripleB.rgbtBlue ^= tripleA.rgbtBlue;
	tripleA.rgbtBlue ^= tripleB.rgbtBlue;
}

void readBMPFile(ImageData& imageData, char* filename)
{
	//ImageData newImageData("BMPTest.bmp");

	// Open the file
	imageData.hfile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	// Read the header
	ReadFile(imageData.hfile, &imageData.bfh, sizeof(imageData.bfh), &imageData.written, NULL);
	ReadFile(imageData.hfile, &imageData.bih, sizeof(imageData.bih), &imageData.written, NULL);
	// Read image
	DWORD imagesize = imageData.bih.biWidth * imageData.bih.biHeight; // Helps you allocate memory for the image
	imageData.image = new RGBTRIPLE[imagesize]; // Create a new image (I'm creating an array during runtime)
	ReadFile(imageData.hfile, imageData.image, imagesize * sizeof(RGBTRIPLE), &imageData.written, NULL); // Reads it off the disk
																		   // Close source file
	CloseHandle(imageData.hfile);
	// Now for some information
	std::cout << "The image width is " << imageData.bih.biWidth << "\n"; // Will output the width of the bitmap
	std::cout << "The image height is " << imageData.bih.biHeight << "\n"; // Will output the height of the bitmap

																 //  Re-order each pixel into the correct order (BGR to RGB)
	for (DWORD i = 0; i < imagesize; ++i) std::swap<BYTE>(imageData.image[i].rgbtRed, imageData.image[i].rgbtBlue);

	for (int i = 0; i < imageData.bih.biWidth; ++i)
		for (int j = 0; j < imageData.bih.biHeight; ++j)
			ReOrderColors(imageData.image[j * imageData.bih.biWidth + i]);

	//  Re-order the pixel list into the correct order
	for (int i = 0; i < imageData.bih.biHeight / 2; ++i)
	{
		for (int j = 0; j < imageData.bih.biWidth; ++j)
		{
			SwapRGBTRIPLE(imageData.image[i * imageData.bih.biWidth + j], imageData.image[(imageData.bih.biHeight - 1 - i) * imageData.bih.biWidth + j]);
		}
	}
}

bool GetColorAtPosition(const ImageData& imageData, const int x, const int y, Color& returnColor)
{
	if (x < 0 || x >= imageData.bih.biWidth) return false;
	if (y < 0 || y >= imageData.bih.biHeight) return false;

	returnColor.Set(imageData.image[y * imageData.bih.biWidth + x].rgbtRed, imageData.image[y * imageData.bih.biWidth + x].rgbtGreen, imageData.image[y * imageData.bih.biWidth + x].rgbtBlue);
	return true;
}

std::string ConvertRGBToColorString(const Color& color)
{
	for (int i = 0; i < COMMON_COLOR_COUNT; ++i)
	{
		if (COMMON_COLORS[i][0] == color.R && COMMON_COLORS[i][1] == color.G && COMMON_COLORS[i][2] == color.B)
		{
			ColorsUsed[i] = nullptr;
			return "Color" + std::to_string(i);
		}
	}

	return color.String();
}

int OutputColor(std::ofstream& outputFile, const ImageData& imageData, int x, int y, const Color& pixelColor, const bool newRow)
{
	if (outputFile.bad() || !outputFile.good()) return 0;
	if (!outputFile.is_open()) return 0;

	int xOffset = x - OriginX;
	int yOffset = y - OriginY;

	Color iterColor(-1, -1, -1);
	int count = 1;

	for (int i = 1; i < imageData.bih.biWidth; ++i)
	{
		if (x + i >= imageData.bih.biWidth) break;

		GetColorAtPosition(imageData, x + i, y, iterColor);
		if (!iterColor.Equals(pixelColor)) break;

		++count;
	}

	std::string textString = "";

	if (count == 1)
	{
		textString = "  SetLED(x ";
		textString += (xOffset < 0) ? "- " : "+ ";
		textString += std::to_string(std::abs(xOffset));
		textString += (yOffset < 0) ? ", y - " : ", y + ";
		textString += std::to_string(std::abs(yOffset));
		textString += ", ";
		textString += ConvertRGBToColorString(pixelColor);
		textString += ");";

		if (newRow)
		{
			textString += " // START ROW ";
			textString += (yOffset < 0) ? "-" : "+";
			textString += std::to_string(std::abs(yOffset));
		}

		textString += "\n";
	}
	else
	{
		textString = "  SetLights(x ";
		textString += (xOffset < 0) ? "- " : "+ ";
		textString += std::to_string(std::abs(xOffset));
		textString += (yOffset < 0) ? ", y - " : ", y + ";
		textString += std::to_string(std::abs(yOffset));
		textString += ", ";
		textString += std::to_string(count);
		textString += ", ";
		textString += ConvertRGBToColorString(pixelColor);
		textString += ");";

		if (newRow)
		{
			textString += " // START ROW ";
			textString += (yOffset < 0) ? "-" : "+";
			textString += std::to_string(std::abs(yOffset));
		}

		textString += "\n";
	}

	outputFile << textString;
	return count;
}

void main(void)
{
	ImageData imageData;
	readBMPFile(imageData, "ConvertImage.bmp");

	//  Specify the color to be ignored (background color)
	Color backgroundColor(100, 100, 100);

	//  Create a color for grabbing pixels and the last color as well
	Color pixelColor(-1, -1, -1);
	Color lastColor(-1, -1, -1);

	//  Set the boolean for a new row
	bool newRow = false;

	//  Open a text file for the output
	std::ofstream outputFile("outputCode.txt", std::ios_base::trunc);
	if (outputFile.bad() || !outputFile.good()) return;

	//  For each row in the image...
	for (int i = 0; i < imageData.bih.biHeight; ++i)
	{
		newRow = true;

		//  Loop through each pixel and find collections, writing each out to the outputFile
		for (int j = 0; j < imageData.bih.biWidth; ++j)
		{
			GetColorAtPosition(imageData, j, i, pixelColor);
			if (pixelColor.Equals(lastColor)) continue;
			if (pixelColor.Equals(backgroundColor)) continue;

			int count = OutputColor(outputFile, imageData, j, i, pixelColor, newRow);
			j += count - 1;
			newRow = false;
		}
	}

	outputFile << std::endl << std::endl;
	outputFile << std::to_string(ColorsUsed.size());

	//  Close the output file now that we've written to it
	outputFile.close();
}