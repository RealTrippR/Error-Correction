#define bitset(byte,nbit)   ((byte) |=  (1<<(nbit)))
#define bitclear(byte,nbit) ((byte) &= ~(1<<(nbit)))
#define bitflip(byte,nbit)  ((byte) ^=  (1<<(nbit)))
#define bitcheck(byte,nbit) (byte & (1 << nbit))
//#define bitcheck(byte,nbit) ((byte) &   (1<<(nbit))) // returns bit value (i.e: 1,2,4,8,16,32,64)

// W*H
#define chunksizeW 3
#define chunksizeH 3

#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

int random(int lb, int ub) {
    // Use current time as seed for random generator
    srand(time(0));
    return (rand() % (ub - lb + 1)) + lb;
}

// takes a string of bits and converts it to bytes
string bits_to_bytes(string bits) {
    string bytes;
    char byte = 0;

    for (int i = 0; i < ceil(float(bits.size()) / 8); i++) {
        for (int j = 0; j < 8; j++) {
            if ('1' == bits[j + (i * 8)]) {
                bitset(byte, j);
            }
        }
        bytes.push_back(byte);
        byte = 0;
    }

    return bytes;
}

// takes bytes and puts its bits in a string
string bytes_to_bits(string bytes) {
    string bits;

    for (int i = 0; i < bytes.size(); i++) {
        for (int j = 0; j < 8; j++) {
            if (bitcheck(bytes[i], j)) {
                bits.push_back('1');
            } else {
                bits.push_back('0');
            }
        }
    }

    return bits;
}

void printCodes(string bits, bool oneParity) {

    cout << "\n";

    int gridsize = (chunksizeH+1) * (chunksizeW+1);

    int W = chunksizeW+1;
    int H = chunksizeH+1;

    for (int c = 0; c < bits.size() / gridsize; c++)
    {
        vector<char> values; // negative sign for parity bit

        // first row is the column parity bits
        values.push_back(bits[c*gridsize]);
        for (int i = 1; i < W; i++) {
            values.push_back(-bits[(i) + c * gridsize]);
        }

        // this reads parity bits and data
        for (int i = 1; i < H; i++)
        {
            for (int j = 0; j < W; j++) {
                if (0 == j)
                {
                    values.push_back(-bits[(i * W) + j + c * gridsize]);
                } else {
                    values.push_back(bits[(i * W) + j + c * gridsize]);
                }
            }
        }

        // prints values
        for (int i = 0; i < values.size(); i++) {
            int row = floor(float(i) / W);
            int column = i % H;

            if (values[i] < 0) {
                cout << " P";
            } else {
                cout << "  ";
            }

            cout << char(abs(values[i])) << " ";
            
            //cout << "(" << column << "," << row << ")";
            if (column < W-1) {
                cout << "|";
            }

            if (column == W-1) {
                cout << "\n---------------------\n";
            }
        }
        // prints values
        // for (auto z : values) {
        //     cout << int(z) << ",";
        // }
        cout << "\n";
    }
}

// Detects errors and corrects them
string correctError(string data, bool oneParity)
{
    string databits = bytes_to_bits(data);
    
    string bits = "";

    string text_data_bits;

    int gridsize = (chunksizeH+1) * (chunksizeW+1);

    int W = chunksizeW+1;
    int H = chunksizeH+1;

    for (int c = 0; c < databits.size() / gridsize; c++)
    {

        bool column_errors[chunksizeW] = {false,false,false};
        bool row_errors[chunksizeH] = {false,false,false};

        vector<char> values; // negative sign for parity bit

        // first row is the column parity bits
        values.push_back(databits[c*gridsize]-48);
        for (int i = 1; i < W; i++) {
            values.push_back(-(databits[(i) + c * gridsize]-48));
        }

        // this reads row parity bits and data
        for (int i = 1; i < H; i++)
        {
            for (int j = 0; j < W; j++) {
                if (0 == j)
                {
                    values.push_back((databits[(i * W) + j + c * gridsize]-48));
                } else {
                    values.push_back(databits[(i * W) + j + c * gridsize]-48);
                }
            }
        }

        // Error Detection (Rows)
        for (int i = 1; i < H; i++) {
            bool parity = abs(values[(W*i)]);
            int onebits = 0;
            for (int j = 1; j < W; j++) {
                if (values[(i*W)+j] == 1) {
                    onebits++;
                }
                cout << char(values[(i*W)+j]+48) << ",";
            }
            cout << "(" << parity << "," << onebits << ")";
            if (onebits % 2 != parity) {
                row_errors[i-1] = true;
                cout << "(ROW ERROR)";
            }
            cout << "\n";
        }

        // Error Detection (Columns)
        for (int i = 1; i < W; i++) {
            bool parity = abs(values[i]);
            int onebits = 0;
            for (int j = 1; j < H; j++) {
                if (values[(W*j)+i] == 1) {
                    onebits++;
                }
            }
            cout << "(PARITY:" << parity << "," << onebits << ")";
            if (onebits % 2 != parity) {
                column_errors[i-1] = true;
                cout << "(COLUMN ERROR)";
            }
        }
        cout << "\nROW ERRORS: ";
        for (auto z : row_errors)
        {
            if (z == true) {
                cout << "1,";
            } else {
                cout << "0,";
            }
        }

        cout << "\nCOLUMN ERRORS: ";

        for (auto z : column_errors)
        {
            if (z == true) {
                cout << "1,";
            } else {
                cout << "0,";
            }
        }
        // Corrects corrupted bits
        for (int i = 0; i < sizeof(row_errors)/sizeof(row_errors[0]); i++)
        {
            int x = 0;
            int y = 0;
            if (row_errors[i] == true)
            {
                x = i;
            }
            for (int j = 0; j < sizeof(column_errors)/sizeof(column_errors[0]); j++)
            {
                if (column_errors[j] == true)
                {
                    y = j;
                    values[(y+1)*W + (x+1)] = (!values[(y+1)*W + (x+1)]);
                    // cout << "(X: " << x << ", Y:" << y << ")";
                }
            }
        }

        // checks if overall parity of packet is accurate
        int oneBits = 0;
        for (int i = 1; i < H; i++) {
            for (int j = 1; j < W; j++) {
                if (values[(i*W)+j] == 1) {
                    oneBits++;
                }
            }
        }
        
        int parity = databits[c * gridsize]-48;
        cout << "(ONEBITS:" << oneBits << "| PARITY:" << parity << ")";
        if ((oneBits % 2) != parity) {
            cout << "RESEND PACKET";
            // request packet resend
        }

        for (int z = 0; z < values.size(); z++) {
            values[z] = abs(values[z]) + 48;
        }

        bits.append(&values[0]);
    }

    cout << "\nBITS: " << bits << "\n";

    printCodes(bits, true);

    return bits;
}

// Takes arg bits, num of bits to flip
void makeError(string &baseText)
{
    //int charIndex = random(0, baseText.size() - 1);
    int charIndex = 2;
    int bitIndex = 5;
    //int bitIndex = random(0, 7);
    cout << "IND" << charIndex;
    bitflip(baseText[charIndex],bitIndex);
}

string readDeltaCodes(string databits, bool oneParity) {
    int gridsize = (chunksizeH+1) * (chunksizeW+1);
    int W = chunksizeW+1;
    int H = chunksizeH+1;
    string bits;
    for (int c = 0; c < databits.size() / gridsize; c++)
    {
        for (int i = 1; i < H; i++)
        {
            for (int j = 1; j < W; j++) 
            {
                bits.push_back(databits[i*W+j + c * gridsize]);
            }
        }
    }
    cout << bits;
    return bits_to_bytes(bits);
}

string writeDELTACodes(string data)
{
    // converts data to bits
    string databits = bytes_to_bits(data);

    // writes chunks
    string bits;

    for (int c = 0; c < databits.size() % (chunksizeH * chunksizeW); c++)
    {
        // allocates space for column parity bits
        for (int i = 0; i < chunksizeW + 1; i++)
        {
            bits.push_back('0');
        }
        //cout << "\nc:" << c;
        for (int i = 0; i < chunksizeH; i++)
        {
            // reserve a 0 - space for parity bit
            // default (0) is even
            string linebits = "0";
            //string linebits;
            int oneBits = 0;
            for (int j = 0; j < chunksizeW; j++)
            {
                // prevents seg fault
                if (i+j > databits.size() - 1) {
                    break;
                } else {
                    linebits.push_back(databits[(i*chunksizeW)+j+(c*chunksizeW*chunksizeH)]);

                    if ('1' == databits[(i*chunksizeW)+j+(c*chunksizeW*chunksizeH)]) {
                        oneBits++;
                    }
                }
            }

            //cout << oneBits << "i";

            if ((oneBits % 2 != 0)) {
                linebits[0] = '1';
            }

            bits.append(linebits);
        }

        //int columnParityBits[chunksizeW];
        // scans through string bits and calculates the parity bit for columns
        for (int i = 0; i < chunksizeW; i++)
        {
            int oneBits = 0;
            for (int j = 0; j < chunksizeH; j++)
            {
                cout << databits[i+j*chunksizeW+(c*chunksizeW*chunksizeH)] << ",";

                if (i+j*chunksizeW+(c*chunksizeW*chunksizeH) > databits.size() - 1)
                {
                    break;
                }
                else if ('1' == databits[i+j*chunksizeW+(c*chunksizeW*chunksizeH)])
                {
                    // cout << "T";
                    oneBits++;
                }
            }
            int gridsize = (chunksizeW+1)*(chunksizeH+1);
            if ((oneBits % 2) != 0) {
                bits[1+i+(c*gridsize)] = '1';
            }
        }
        // sets bits[0] to parity of overall parity of databits
        int oneBits = 0;
        for (int z = c*chunksizeW*chunksizeH; z < (c+1)*chunksizeH*chunksizeW; z++) {
            if (databits[z] == '1') {
                oneBits++;
            }
        }
        cout << "\nONEBITS: " << oneBits << "\n";

        if (oneBits % 2 == 0) {
            bits[c*((chunksizeW+1)*(chunksizeH+1))] = '0';
        } else {
            bits[c*((chunksizeW+1)*(chunksizeH+1))] = '1';
        }
    }

    cout << "DATABITS: " << databits << "\n";
    cout << "BITS: " << bits << "\n";

    string bytes = bits_to_bytes(bits);

    printCodes(bits, false);

    return bytes;
}

string readFile(string filename)
{
    string buffer;
    string line;
    ifstream inFile (filename);
    if (!inFile) {
        perror("Could not open file!");
    }
    if (inFile.is_open())
    {
        while (getline(inFile,line))
        {
            buffer.append(line);
        }
        inFile.close();
    }
    return buffer;
}

void writeToFile(string filename, string text)
{
    ofstream outFile;
    outFile.open("RobustFiles/" + filename);
    outFile << text;
    outFile.close();
}

int main()
{
    string InFolderName = "TextFiles";
    string filename = "test.txt";

    cout << "Enter a filename to write DELTA codes for\n";
    //cin >> filename;

    string text = readFile(InFolderName + "/" + filename);
    cout << "\nTEXT BITS:" << bytes_to_bits(text) << "\n";
    
    text = writeDELTACodes(text);

    makeError(text);

    //cout << "\nCODES\n";
    //printCodes(bytes_to_bits(text), true);

    // if it cannot find where the error is, it should request a resending of the packet
    string bufferBits = correctError(text, true);

    text = readDeltaCodes(bufferBits, true);

    cout << "\nTEXT:" << text;
    writeToFile(filename, text);
}
