#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// JPEG编码函数
void EncodeJPEG(const char *inputFilePath, bool isOut) {
    // 读取图像
    Mat image = imread(inputFilePath, IMREAD_UNCHANGED);

    // 进行JPEG编码
    vector<uint8_t> encoded;
    imencode(".jpg", image, encoded);

    if (isOut) {
        // 输出文件名
        const char *outputFile = "encodedImage.jpg";
        int bufLen = encoded.size();
        if (bufLen) {
            // 获取编码后的数据并写入文件
            uint8_t *pYuvBuf = encoded.data();
            FILE *fclr = fopen(outputFile, "wb");
            fwrite(pYuvBuf, 1, bufLen, fclr);
            fclose(fclr);
        }
    }
}

// JPEG解码函数
void DecodeJPEG(const char *inputFilePath, bool isOut) {
    // 读取二进制文件
    ifstream in(inputFilePath, ios::binary);
    string s((istreambuf_iterator<char>(in)), (istreambuf_iterator<char>()));
    in.close();

    // 将字符串数据转换为向量
    vector<char> pic(s.c_str(), s.c_str() + s.length());
    Mat image;
    
    // 进行JPEG解码
    imdecode(pic, IMREAD_UNCHANGED, &image);

    if (isOut) {
        // 输出文件名
        const char *outputFile = "decodedImage.bmp";
        
        // 将解码后的图像写入文件
        imwrite(outputFile, image);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "使用方式：./test_ocv_jpumulti <inputfile> <test type: 1-编码, 2-解码> <isOut: 0-不生成输出文件, 1-生成输出文件>" << endl;
        return 1;
    }

    const char *inputFile = argv[1];
    int testType = stoi(argv[2]);
    bool isOut = stoi(argv[3]);

    switch (testType) {
        case 1:
            EncodeJPEG(inputFile, isOut);
            cout << "JPEG编码完成。" << endl;
            break;
        case 2:
            DecodeJPEG(inputFile, isOut);
            cout << "JPEG解码完成。" << endl;
            break;
        default:
            cerr << "错误：请提供正确的测试类型，1-编码，2-解码。" << endl;
            return 1;
    }

    return 0;
}
