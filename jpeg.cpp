#include "jpeg.h"
#include "AC_Hufftree.cpp"
#define PI 3.14159265359
#define total_dc 512*512/64

string input_file;
int main()
{
    int *i_input_file, *DCT_matrix, *to_raw_matrix, *final_matrix, qf=50;
    ///Initialize
    //memset(_output, '\0', sizeof(_output));
    DCT_matrix = new int[512*512];
    to_raw_matrix = new int[512*512];
    final_matrix = new int[512*512];
    _initial_ac_table(0,0,0);/* Luminance AC */
    //Interface
    cout<<"*Input filename:\n";
    cin>>input_file;
    ///set QF
    cout<<"QF:";
    cin>>qf;
    if(qf != 50)
        set_QF(qf);
    cout<<"Encoding...\n";
    ///read file
    Read_image(i_input_file);
    ///-128
    for(int i=0; i<512*512; i++)
        *(i_input_file+i) = *(i_input_file+i) -128;
    //cout<<*(i_input_file+1);
    ///DCT
    for(int i=0; i<258553;)//258552
    {
        //cout<<"~~~~~~~~~~~\n";
        FDCT((i_input_file+i), (DCT_matrix+i), 8);
        if( i%512==504)
            i = i + (512*7 + 8);
        else
            i = i + 8;
        //cout<<*(DCT_matrix);
    }
    ///~~DC~~
    ///deDPCM
    DPCM((DCT_matrix));
    cout<<"...";
    ///~~AC~~
    ///Zigzag
    for(int i=0; i<258553;)//258552
    {
        Zig((DCT_matrix+i));
        if( i%512==504)
            i = i + (512*7 + 8);
        else
            i = i + 8;
    }
    string filename_no_raw;
    filename_no_raw.append(input_file, 0, input_file.size()-4);
    cout<<"\nEncoding Done!!\nOutput file: "<<filename_no_raw<<endl;
    ///Output
    if((DC_chain.size() + AC_chain.size()) != 0)//補0
    {
        int div8 = (DC_chain.size() + AC_chain.size()) / 8;
        int dd = (DC_chain.size() + AC_chain.size()) - div8 * 8;
        for(int i=0; i<dd; i++)
            AC_chain = AC_chain + "0";
    }
    fstream ofile;
    char *to_wirte_filename_no_raw = new char[filename_no_raw.size() + 1];
    strcpy(to_wirte_filename_no_raw, filename_no_raw.c_str());
    ofile.open(to_wirte_filename_no_raw, fstream::out | fstream::binary);
    //ofile.open("bb", fstream::out | fstream::binary);
    string SS = DC_chain + AC_chain;
    for (int i = 0; i < SS.size(); i=i+8)//SS_write
    {
        int temp;
        temp = (SS[i]-'0')*128 + (SS[i+1]-'0')*64 + (SS[i+2]-'0')*32 + (SS[i+3]-'0')*16 + (SS[i+4]-'0')*8 + (SS[i+5]-'0')*4 + (SS[i+6]-'0')*2 + (SS[i+7]-'0')*1;
        int8_t int8_output = temp;
        ofile << int8_output;
    }
    ofile.close();
    ///Decoder
    cout<<"Decoding...\n";
    ///DC part~
    string string_DC = DC_chain;
    int DC_matrix[total_dc], dpcm_count=0, d_trace=0;
    ///查表
    for(dpcm_count=0; dpcm_count<total_dc; dpcm_count++)
    {
        string temp;
        //front
        int DC_huff_bitsnum = DC_front(string_DC, d_trace);
        //back
        for(int i=0; i <DC_huff_bitsnum; i++)
        {
            temp = temp + string_DC[d_trace + i];
        }
        if(temp[0] == '0')
        {
            for(int i=0; i<temp.size(); i++)
            {
                if(temp[i] == '0')
                    temp[i] = '1';
                else
                    temp[i] = '0';
            }
            DC_matrix[dpcm_count] = (-1)*bin2dec(temp);
        }
        else
            DC_matrix[dpcm_count] = bin2dec(temp);
        d_trace += DC_huff_bitsnum;
        ///Deocode_DPCM
        if(dpcm_count>0)
            DC_matrix[dpcm_count] = DC_matrix[dpcm_count] + DC_matrix[dpcm_count-1];
    }
    ///AC part~
    maketree();
    ///Zigzag
    int ac_trace=0;
    dpcm_count=0;
    for(int i=0; i<258553;)//258552
    {
        decode_ac(AC_chain, ac_trace, to_raw_matrix + i, DC_matrix, dpcm_count);
        if( i%512==504)
            i = i + (512*7 + 8);
        else
            i = i + 8;
        //cout<<*(DCT_matrix);
        dpcm_count++;
    }
    ///IDCT
    for(int i=0; i<258553;)//258552
    {
        IDCT((to_raw_matrix+i), (final_matrix+i), 8);
        if( i%512 == 504)
            i = i + (512*7 + 8);
        else
            i = i + 8;
    }
    ///PSNR
    PSNR(i_input_file, final_matrix);
    ///+128
    for(int i=0; i<512*512; i++)
        *(final_matrix+i) = *(final_matrix+i) +128;
    //cout<<*(final_matrix+1);
    ///write back to raw
    fstream rawfile;
    ///string to char[]
    input_file = "_" + input_file;
    stringstream ss;
    ss << qf;
    string qfs;
    ss >> qfs;
    input_file = qfs + input_file;
    char *to_wirte_name = new char[input_file.size() + 1];
    strcpy(to_wirte_name, input_file.c_str());
    rawfile.open(to_wirte_name, fstream::out | fstream::binary);
    //rawfile.open("bb_90.raw", fstream::out | fstream::binary);
    for(int i = 0; i < 512*512; i++)
    {
        int8_t int8_output = final_matrix[i];
        rawfile << int8_output;
    }
    rawfile.close();
    cout<<"Decoding Done!!\n"<<input_file;
    system("PAUSE");
    return 0;
}
void set_QF(int qf)
{
    int factor;
    if(qf<50)
        factor = 5000/qf;
    else
        factor = 200 - 2*qf;
    for(int i=0; i<64; i++)
        Quantize[i] = Quantize[i]*factor/100;
}
void PSNR(int *origin, int *compression)
{
    long long int sigma = 0;
    double MSE = 0.0;
    double PSNR = 0.0;
    int frameSize = 512*512;
    int gray1=0, gray2=0;

    for(int i=0; i<512; i++)
    {
        for(int j=0; j<512; j++)
        {
            gray1 = origin[i*512 + j];
            gray2 = compression[i*512 + j];
            sigma+=(gray1-gray2)*(gray1-gray2);
        }
    }
    MSE = sigma / (double)frameSize;
    PSNR = 10 * log10(255*255/MSE);
    cout<<"PSNR:"<<PSNR<<endl;
}
void FDCT(int *in_data, int *(out_data), int n)//n=8
{
    //cout<<out_data;
    /* ------------------------------------------------------------
     作用：將強度資料經DCT轉成頻率資料
     輸入：in_data = 強度資料
     n = 矩陣大小
     輸出：out_data = 頻率資料
    ------------------------------------------------------------ */
    double data, sqrt_2, data2;
    int i, j, u, v;

    sqrt_2 = sqrt(2.0);
    for (u=0; u<n; u++)
    {
        for (v=0; v<n; v++)
        {
            data = 0;
            for (i=0; i<n; i++)
                for (j=0; j<n; j++)
                    data += in_data[i*512+j] * cos((2*i+1)*u*PI/(2*n)) * cos((2*j+1)*v*PI/(2*n));//data += in_data[i*n+j] * cos((2*i+1)*u*M_PI/(2*n)) * cos((2*j+1)*v*M_PI/(2*n));
            if (u == 0)
                data /= sqrt_2;
            if (v == 0)
                data /= sqrt_2;
            data = data*2/n;//=date/4
            //cout<<data<<" ";
            ///Quantization
            data2 = data / Quantize[u*8+v];
            //cout<<data2<<" ";
            /* 四捨五入 */
            if (data2 < 0)
                data2 -= 0.5;
            else
                data2 += 0.5;
            out_data[u*512+v] = data2;
            //cout<<in_data[u*512+v]<<" ";
            //cout<<out_data[u*512+v]<<" ";
        }
        //cout<<endl;
    }
}
void IDCT(int *in_data, int *(out_data), int n)//n=8
{
    //cout<<out_data;
    /* ------------------------------------------------------------
     作用：將DCT後的頻率資料轉回強度資料
     輸入：in_data = 頻率資料
     n = 矩陣大小
     輸出：out_data = 強度資料
     ------------------------------------------------------------ */
    double data, sqrt_2, sub_data;
    int i, j, u, v;

    sqrt_2 = sqrt(2.0);
    ///de_Quantization
    for (i=0; i<n; i++)
        for (j=0; j<n; j++)
            in_data[i*512+j] = in_data[i*512+j] * Quantize[i*8+j];
    ///IDCT
    for (i=0; i<n; i++)
    {
        for (j=0; j<n; j++)
        {
            data = 0;
            for (u=0; u<n; u++)
                for (v=0; v<n; v++)
                {
                    sub_data = in_data[u*512+v] * cos((2*i+1)*u*PI/(2*n)) * cos((2*j+1)*v*PI/(2*n));
                    if (u == 0)
                        sub_data /= sqrt_2;
                    if (v == 0)
                        sub_data /= sqrt_2;
                    data += sub_data;
                }
            data = data*2/n;
            //cout<<data<<" ";
            /* 四捨五入 */
            if (data < 0)
                data -= 0.5;
            else
                data += 0.5;
            out_data[i*512+j] = data;
            //cout<<out_data[i*512+j]<<" ";
        }
        //cout<<endl;
    }
}

void Zig(int *DCT)//zgzg[4096][64]
{
    int n=8, count_zero=0, z=0, getbitsnum=0;
    int temp_mat[64];
    ///轉存好操作
    for (int i=0; i<n; i++)
        for (int j=0; j<n; j++)
            temp_mat[i*8+j] = DCT[i*512+j];
    ///Zigzag starting
    for(int i=1; i<n*n; i++)//AC start to 1
    {
        if(i == (n*n-1))///last
            AC_chain = AC_chain + "1010";//EOB
        else if(temp_mat[Zigzag[i]] == 0)
            count_zero++;
        else
        {
            ///!!ZRL!! overflow >=16 0
            if(count_zero >= 16)
            {
                AC_chain = AC_chain + AC_Huff(15, 0);
                count_zero -= 16;
            }
            string ac_back = AC_back(temp_mat[Zigzag[i]], getbitsnum);
            string ac_front = AC_Huff(count_zero,getbitsnum);
            AC_chain = AC_chain + ac_front + ac_back;
            count_zero = 0;
            z++;
        }
    }
}
void DPCM(int *DCT_matrix)
{
//    strcat(dpcm_matrix[0], DC_Huff(*i_input_file).c_str());
//    for(int i=1; i<512*512; )
//    {
//        strcat(*(dpcm_matrix + i), DC_Huff(*(i_input_file+i) - *(i_input_file+i-1)).c_str());
//        if(i == 504)
//            i = i + 512*7 + 8;
//        else
//            i = i + 8;
//    }
    int c=0;
    DC_chain = DC_chain + DC_Huff(*DCT_matrix);
    //cout<<*DCT_matrix<<" ";
    for(int i=8; i<512*512; )
    {
//        if(i%512 == 0)
//            cout<<*(DCT_matrix+i) - *(DCT_matrix+i-512*7-8)<<" ";
//        else
//            cout<<*(DCT_matrix+i) - *(DCT_matrix+i-8)<<" ";
        //cout<<DC_c
        if(i%512 == 0)
            DC_chain = DC_chain + DC_Huff(*(DCT_matrix+i) - *(DCT_matrix+i-512*7-8));
        else
            DC_chain = DC_chain + DC_Huff(*(DCT_matrix+i) - *(DCT_matrix+i-8));
        ///~~
        if(i%512 == 504)
            {i = i + 512*7 + 8;}
        else
            i = i + 8;
        c++;
    }
    //cout<<endl<<c;
}
string DC_Huff(int dc)
{
    char Hcode[12][10] =
    {
        "00", "010", "011", "100", "101", "110", "1110", "11110",
        "111110", "1111110", "11111110", "111111110"
    };
    int index;
    string DCH;
    if(dc == 0)
        DCH = "00";
    else if(dc > 0)
    {
        index = floor(log2(dc));
        DCH = *(Hcode+index+1) + dec2bin(dc);
    }
    else
    {
        index = floor(log2(abs(dc)));
        string bdc = dec2bin(abs(dc));
        for(int i=0; i<bdc.size(); i++)
        {
            if(bdc[i] == '0')
                bdc[i] = '1';
            else
                bdc[i] = '0';
        }
        DCH = *(Hcode+index+1) + bdc;
    }
    return DCH;
}
string AC_Huff(int count_zero,int getbitsnum)
{
    ///Convert to string of binary
    string s = bitset< 16 >( AC[0].CodeWord[count_zero][getbitsnum] ).to_string();
    string ss;
    bool flag = false;
    if(count_zero == 0 && getbitsnum == 2)
        ss = "01";
    else
    {
        for(int k=0; k<16; k++)
        {
            if(flag == false && k==15 && s[k] != '1')
                ss="00";
            else if(flag == false)
            {
                if(s[k] == '1')
                {
                    ss=ss+s[k];
                    flag = true;
                }
            }
            else
                ss=ss+s[k];
        }
    }
    return ss;
}
string AC_back(int acf, int &getbitsnum)
{
    int index;
    string ACF;
    if(acf == 0)
        ACF = "0";
    else if(acf > 0)
    {
        index = floor(log2(acf));
        ACF = dec2bin(acf);
    }
    else
    {
        index = floor(log2(abs(acf)));
        string bdc = dec2bin(abs(acf));
        for(int i=0; i<bdc.size(); i++)
        {
            if(bdc[i] == '0')
                bdc[i] = '1';
            else
                bdc[i] = '0';
        }
        ACF = bdc;
    }
    getbitsnum = index + 1;
    return ACF;
}
void _initial_ac_table(Byte identifier, Byte (*ac_length)[17], Byte (*ac_value)[162])
{
    if( ac_length==0 ) ac_length=AC_length; //使用預設的 Huffman 表
    if( ac_value ==0 ) ac_value =AC_value ;

    Word code=0;
    int point=0, total=0;

    for(int i=1; i<=16; i++)
    {
        total=ac_length[identifier][i];

        for(int offset=0;  offset<total; offset++)
        {
            int category = ac_value[identifier][point+offset];

            AC[identifier].CodeLength[ (category&0xF0)>>4 ][ (category&0x0F) ] = i   ;
            AC[identifier].CodeWord  [ (category&0xF0)>>4 ][ (category&0x0F) ] = code;

            code ++;
        }
        point+=total;
        code = code << 1;
    }
    //cout<<(AC[identifier].CodeWord  [ i ][ j ])<<endl;//cout<<std::bitset<16>(AC[identifier].CodeWord  [ i ][ j ])<<endl;
}
void Read_image(int *(&i_input_file))
{
    ///input picture
    streampos size;
    char * memblock;
    fstream ofile;
    char *_wirte_filename = new char[input_file.size() + 1];
    strcpy(_wirte_filename, input_file.c_str());
    ifstream file (_wirte_filename, ios::in|ios::binary|ios::ate);
    //ifstream file ("baboon.raw", ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
    }
    i_input_file = new int [size];
    int *bit_input_file = new int [size * 8];
    ///get bits of input c
    for (int i = 0; i < size * 8; i++)
    {
        unsigned char c = memblock[i / 8];
        int bitPos = i % 8;
        *(bit_input_file + i) = (int)((c >> (7 - bitPos)) & 1);
        //cout << *(bit_input_file + i);
    }
    ///change to int
    for (int i=0, j=0; i < size * 8; i=i+8, j++)
    {
        *(i_input_file+j) = (*(bit_input_file+i))*128 + (*(bit_input_file+i+1))*64 + (*(bit_input_file+i+2))*32 + (*(bit_input_file+i+3))*16 + (*(bit_input_file+i+4))*8 + (*(bit_input_file+i+5))*4 + (*(bit_input_file+i+6))*2 + *(bit_input_file+i+7);
        //cout<<*(i_input_file+j);
    }
}
int DC_front(string string_DC, int &i)
{
    if(string_DC[i] == '0' && string_DC[i+1] == '0')///SSSS=0
    {
        i = i + 2;
        return 0;
    }
    else if(string_DC[i] == '0' && string_DC[i+1] == '1' && string_DC[i+2] == '0')///SSSS=1 010
    {
        i = i + 3;
        return 1;
    }
    else if(string_DC[i] == '0' && string_DC[i+1] == '1' && string_DC[i+2] == '1')///SSSS=2 011
    {
        i = i + 3;
        return 2;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '0' && string_DC[i+2] == '0')///SSSS=3 100
    {
        i = i + 3;
        return 3;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '0' && string_DC[i+2] == '1')///SSSS=4 101
    {
        i = i + 3;
        return 4;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '0')///SSSS=5 110
    {
        i = i + 3;
        return 5;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '0')///SSSS=6
    {
        i = i + 4;
        return 6;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '1' && string_DC[i+4] == '0')///SSSS=7
    {
        i = i + 5;
        return 7;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '1' && string_DC[i+4] == '1' && string_DC[i+5] == '0')///SSSS=8
    {
        i = i + 6;
        return 8;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '1' && string_DC[i+4] == '1' && string_DC[i+5] == '1' && string_DC[i+6] == '0')///SSSS=9
    {
        i = i + 7;
        return 9;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '1' && string_DC[i+4] == '1' && string_DC[i+5] == '1' && string_DC[i+6] == '1' && string_DC[i+7] == '0')///SSSS=10
    {
        i = i + 8;
        return 10;
    }
    else if(string_DC[i] == '1' && string_DC[i+1] == '1' && string_DC[i+2] == '1' && string_DC[i+3] == '1' && string_DC[i+4] == '1' && string_DC[i+5] == '1' && string_DC[i+6] == '1' && string_DC[i+7] == '1' && string_DC[i+8] == '0')///SSSS=11
    {
        i = i + 9;
        return 11;
    }
}
