#include<iostream>
#include<cstdlib>
#include<bitset>
//#include "jpeg.h"
using namespace std;

void addANode_0(Node *&ptr); //要用參考才能改變ptr引數的指向位址
void addANode_1(Node *&ptr);
string AC_Huff(int,int);

void maketree()
{
    for(int i=0; i<16; i++)
    {
        for(int j=0; j<11; j++)
        {
            string h = AC_Huff(i, j);
            Node *hufftree = _HuffTree;

            for(int k=0; k<h.size(); k++)
            {
                switch ( h[k] )
                {
                case '0':
                    if(hufftree->next_0 == NULL)
                        addANode_0(hufftree);
                    else
                        hufftree = hufftree->next_0;
                    break;
                case '1':
                    if(hufftree->next_1 == NULL)
                        addANode_1(hufftree);
                    else
                        hufftree = hufftree->next_1;
                    break;
                default:
                    break;
                }
                ///is Leaf
                if(k == (h.size()-1))
                {
                    if(hufftree->LEAF == false)
                    {
                        hufftree->data1 = i;
                        hufftree->data2 = j;
                        hufftree->LEAF = true;
                    }
                }
                ///EOB&ZRL
                if(i==0 && j==0)
                    hufftree->EOB = true;
                if(i==15 && j==0)
                    hufftree->ZRL = true;
            }
        }
    }
};
void decode_ac(string string_AC,int &ac_trace, int *to_raw_matrix, int *DC_matrix, int &dpcm_count)
{
    //maketree();

    //printfNode(_HuffTree);
    //string string_AC = "10011100010111001010001000011111001001011100101001101101000010110010111011001110100111001111111110011110011010";///global
    //int ac_trace=0;
    ///decode_Zig(ac_trace)
    ///Zigzag
    int temp_mat[64], z=1;
    while(z<64)
    {
        Node *hh = _HuffTree;
        int temp_ac=0;
        while(1)
        {
            if(string_AC[ac_trace] == '0')
                {hh = hh->next_0;}
            else
                hh = hh->next_1;
            //cout<<hh->data1<<hh->data2<<endl;
            ac_trace++;
            if(hh->LEAF == true)
                break;//cout<<hh->data1<<hh->data2;
        }
        string temp;
        //front

        //back
        for(int i=0; i < hh->data2; ac_trace++, i++)
        {
            temp = temp + string_AC[ac_trace];
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
            temp_ac = (-1) * bin2dec(temp);
        }
        else
            temp_ac = bin2dec(temp);
        ///Zigzag starting
        if(hh->data1 == 0 && hh->data2 ==0)///EOB
        {
            ///0填充到最後
            for( ; z < 64; z++)
                temp_mat[ Zigzag[z] ] = 0;
        }
        else if(hh->data1 == 15 && hh->data2 == 0)///ZRL
        {
            ///0填充 16個0
            for(int zero=0; zero < 16; zero++, z++)
                temp_mat[ Zigzag[z] ] = 0;
        }
        else
        {
            ///0填充
            for(int zero=0; zero < hh->data1; zero++, z++)
                temp_mat[ Zigzag[z] ] = 0;
            ///填入非0值
            temp_mat[ Zigzag[z] ] = temp_ac;
            z++;
        }

    };
    ///轉存
    int n=8;
    for (int i=0; i<n; i++)
    {
        for (int j=0; j<n; j++)
        {
            if(i!=0 || j!=0)
                to_raw_matrix[i*512+j] = temp_mat[i*8+j];
            else//add DC
                to_raw_matrix[i*512+j] = DC_matrix[dpcm_count];
            //cout<<to_raw_matrix[i*512+j]<<" ";
        }
        //cout<<endl;
    }
};
Node *createNode()
{
    Node *head = new Node;
    if (!head)
    {
        cout << "配置記憶體失敗" << endl;
        exit(1);
    }

    head->next_0 = NULL;
    head->next_1 = NULL;
    return head;
};
void addANode_0(Node *&ptr)
{
    Node *newnode = new Node;
    if (!newnode)
    {
        cout << "配置記憶體失敗" << endl;
        exit(1);
    }

    newnode->next_0 = NULL;
    newnode->next_1 = NULL;
    ptr->next_0 = newnode;
    ptr = ptr->next_0;
};
void addANode_1(Node *&ptr)
{
    Node *newnode = new Node;
    if (!newnode)
    {
        cout << "配置記憶體失敗" << endl;
        exit(1);
    }

    newnode->next_0 = NULL;
    newnode->next_1 = NULL;
    ptr->next_1 = newnode;
    ptr = ptr->next_1;
};
