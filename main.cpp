#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <sstream>
#include <cstring>

using namespace std;

// Huffman Tree Node
struct HuffmanNode {
    char ch;
    unsigned freq;
    HuffmanNode *left, *right;
    HuffmanNode(char c, unsigned f) : ch(c), freq(f), left(NULL), right(NULL) {}
};

// Comparator for priority queue
struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// Generate Huffman Tree from frequencies
HuffmanNode* buildHuffmanTree(const unordered_map<char, unsigned>& freq) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> minHeap;

    for (auto pair : freq) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    while (minHeap.size() != 1) {
        HuffmanNode *left = minHeap.top(); minHeap.pop();
        HuffmanNode *right = minHeap.top(); minHeap.pop();

        HuffmanNode *top = new HuffmanNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        minHeap.push(top);
    }

    return minHeap.top();
}

// Generate Huffman Codes
void generateCodes(HuffmanNode* root, const string& str, unordered_map<char, string>& huffCodes) {
    if (root == NULL) return;

    if (!root->left && !root->right) {
        huffCodes[root->ch] = str;
    }

    generateCodes(root->left, str + "0", huffCodes);
    generateCodes(root->right, str + "1", huffCodes);
}

// Encode file content
string encodeFile(const string& input, unordered_map<char, string>& huffCodes) {
    stringstream encodedStream;
    for (char ch : input) {
        encodedStream << huffCodes[ch];
    }
    return encodedStream.str();
}

// Decode Huffman encoded string
string decodeFile(const string& encoded, HuffmanNode* root) {
    stringstream decodedStream;
    HuffmanNode* current = root;
    for (char bit : encoded) {
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (!current->left && !current->right) {
            decodedStream << current->ch;
            current = root;
        }
    }
    return decodedStream.str();
}

// Compress file
void compressFile(const string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile) {
        cerr << "Unable to open file " << inputFileName << endl;
        return;
    }

    unordered_map<char, unsigned> freq;
    char ch;
    stringstream contentStream;

    while (inputFile.get(ch)) {
        freq[ch]++;
        contentStream << ch;
    }
    inputFile.close();

    HuffmanNode* root = buildHuffmanTree(freq);
    unordered_map<char, string> huffCodes;
    generateCodes(root, "", huffCodes);

    string encodedStr = encodeFile(contentStream.str(), huffCodes);
    ofstream outFile(outputFileName, ios::binary);
    if (!outFile) {
        cerr << "Unable to open file " << outputFileName << endl;
        return;
    }

    bitset<8> bits;
    for (size_t i = 0; i < encodedStr.size(); i++) {
        bits[i % 8] = encodedStr[i] - '0';
        if (i % 8 == 7) {
            outFile.put(bits.to_ulong());
        }
    }
    if (encodedStr.size() % 8 != 0) {
        outFile.put(bits.to_ulong());
    }
    outFile.close();
}

// Decompress file
void decompressFile(const string& inputFileName, const string& outputFileName, HuffmanNode* root) {
    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile) {
        cerr << "Unable to open file " << inputFileName << endl;
        return;
    }

    stringstream encodedStream;
    char byte;
    while (inputFile.get(byte)) {
        bitset<8> bits(byte);
        for (int i = 7; i >= 0; i--) {
            encodedStream << bits[i];
        }
    }
    inputFile.close();

    string decodedStr = decodeFile(encodedStream.str(), root);
    ofstream outFile(outputFileName, ios::binary);
    if (!outFile) {
        cerr << "Unable to open file " << outputFileName << endl;
        return;
    }

    outFile << decodedStr;
    outFile.close();
}

// Helper function to get file size
long getFileSize(const string& fileName) {
    ifstream file(fileName, ios::binary | ios::ate);
    return file.tellg();
}

int main() {
    const string inputFileName = "input.txt";
    const string compressedFileName = "compressed.bin";
    const string decompressedFileName = "decompressed.txt";

    long originalSize = getFileSize(inputFileName);
    cout << "Original file size: " << originalSize << " bytes" << endl;

    // Compress the file
    compressFile(inputFileName, compressedFileName);

    long compressedSize = getFileSize(compressedFileName);
    cout << "Compressed file size: " << compressedSize << " bytes" << endl;

    // Decompress the file
    ifstream inputFile(inputFileName, ios::binary);
    unordered_map<char, unsigned> freq;
    char ch;
    while (inputFile.get(ch)) {
        freq[ch]++;
    }
    inputFile.close();

    HuffmanNode* root = buildHuffmanTree(freq);
    decompressFile(compressedFileName, decompressedFileName, root);

    // Verify decompression
    long decompressedSize = getFileSize(decompressedFileName);
    cout << "Decompressed file size: " << decompressedSize << " bytes" << endl;

    return 0;
}
