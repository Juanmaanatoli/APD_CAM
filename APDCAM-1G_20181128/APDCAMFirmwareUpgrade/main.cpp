#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QThread>

#include <arpa/inet.h>
#include <iostream>

#include "APDLib.h"

unsigned char DoFirmwareUpgrade();
unsigned char ReadFile();

int Open();
int Close();

ADT_HANDLE g_handle = 0;

#define PC_BOARD     2

#define FIRMWARE_VERSION    0x02

#define FIRMWARE_COPY_START     0x8A
#define FIRMWARE_CHAR_VALID     0x8C
#define FIRMWARE_UPGR_ERROR_CODE     0x8E
#define START_FIRMWARE_UPGRADE     0x90
#define FIRMWARE_NEXT_CHAR     0x92

QString firmwareFileName="APD_Camera_Contorller.hex";
QByteArray data;

using namespace std;

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    qDebug() << "\nAPDCAM Firmware Updater V0.8" ;

    /*if(argc > 1)
    {
        qDebug() << "\nFirmware file name: " << argv[1] ;

        firmwareFileName=argv[1];*/

        qDebug() << "\nReading file..." ;
        unsigned char error = ReadFile();
        if(error == 0)
        {
           qDebug() << "Reading done" ;
           qDebug() << "File size" << data.size();

           qDebug() << "\nStarting firmware upgrade...";
           error = DoFirmwareUpgrade();
           if(error == 0)
           {
              qDebug() << "Firmware upgrade finished successfully" ;


           }//if
           else
           {
              qDebug() << "Error during firmware upgrade: " << error ;

           }//else


        }//if
        else
        {
           qDebug() << "Unable to read file, exiting" ;

        }//else

    /*}//if
    else
    {
        qDebug() << "\nNo file name parameter found, exiting..." ;

    }//else*/


    return 0;

}//main


unsigned char DoFirmwareUpgrade()
{
    unsigned char error=0;

    /*unsigned char buf[10] = {0};
    unsigned char buf_read[10] = {0};*/

    unsigned char *buf = new unsigned char[10];
    unsigned char *buf_read = new unsigned char[10];

    APDCAM_Init();

    int err = Open();
    if(err == 0)
    {
        qDebug() << "APDCAM opened" ;

        ADT_RESULT res = APDCAM_ReadPDI(g_handle, PC_BOARD, FIRMWARE_VERSION, buf_read, 2);

        unsigned short value=0;

        for(int i=0;i<2;i++)
        {
            value|=(unsigned short)((buf_read[i]) << 8*i);
        }

        qDebug() << "APDCAM Current firmware version: " << value;


        buf[0]=1;

        res = APDCAM_WritePDI(g_handle, PC_BOARD, FIRMWARE_COPY_START, buf, 1);

        if (res != ADT_OK)
        {
            qDebug() << "Error writing firmware upgrade start register" ;
            error = 1;
            return error;
        }
        else
        {
            res = APDCAM_ReadPDI(g_handle, PC_BOARD, FIRMWARE_COPY_START, buf_read, 1);
            if (res != ADT_OK)
            {
                qDebug() << "Error reading firmware upgrade register" ;
                error = 1;
                return error;
            }
            else
            {
                if(buf_read[0] == 1)
                {
                   qDebug() << "Firmware upgrade enabled\n" ;

                   //buf[0] = 1;

                   float progress = 0.0;
                   int barWidth = 50;

                   std::cout << "[";

                   unsigned char* data2=reinterpret_cast<unsigned char*>(data.data());

                   QThread thread;
                   thread.msleep(200);

                   for(int i=0;i<data.size();i++)
                   {

                       int pos = barWidth * progress;
                       for (int j = 0; j < barWidth; ++j)
                       {
                           if (j < pos) std::cout << "=";
                           else if (j == pos) std::cout << ">";
                           else std::cout << " ";
                       }

                       progress=(float)(i+1)/(float)data.size();

                       std::cout << "] " << int(progress * 100.0) <<"% Bytes written: " << i+1 <<"\r";
                       std::cout.flush();

                       res = APDCAM_WritePDI(g_handle, PC_BOARD, FIRMWARE_NEXT_CHAR, &data2[i], 1);

                       if (res != ADT_OK)
                       {
                           qDebug() << "Error writing new firmware data" ;
                           error = 1;
                           return error;
                       }

                       buf[0] = 1;
                       buf_read[0] = 1;

                       res = APDCAM_WritePDI(g_handle, PC_BOARD, FIRMWARE_CHAR_VALID, buf, 1);

                       if (res != ADT_OK)
                       {
                           qDebug() << "Error writing new firmware data" ;
                           error = 1;
                           return error;
                       }

                       do
                       {
                           res = APDCAM_ReadPDI(g_handle, PC_BOARD, FIRMWARE_CHAR_VALID, buf_read, 1);
                           if (res != ADT_OK)
                           {
                               qDebug() << "Error reading firmware start register" ;
                               error = 1;
                               return error;
                           }

                       }while (buf_read[0]!=0);

                   }//for

                   std::cout << std::endl;

                   qDebug() << "Uploading firmware data finished" ;

                   thread.msleep(3000);

                   buf[0] = 0xC4;  //Start firmware upgrade
                   //buf[0] = 0x28;   //chk check

                   qDebug() << buf[0] ;

                   qDebug() << "Starting upgrade" ;

                   res = APDCAM_WritePDI(g_handle, PC_BOARD, START_FIRMWARE_UPGRADE, buf, 1);

                   if (res != ADT_OK)
                   {
                       qDebug() << "Error writing new firmware data" ;
                       error = 1;
                       return error;
                   }

                   res = APDCAM_ReadPDI(g_handle, PC_BOARD, START_FIRMWARE_UPGRADE, buf_read, 1);
                   if (res != ADT_OK)
                   {
                       qDebug() << "Error reading firmware start register" ;
                       error = 1;
                       return error;
                   }

                   qDebug() << buf_read[0] ;


                   for(int i=0;i<1000;i++)
                   {

                       thread.msleep(250);

                       buf_read[0] = 0;

                       res = APDCAM_ReadPDI(g_handle, PC_BOARD, FIRMWARE_UPGR_ERROR_CODE, buf_read, 1);
                       if (res != ADT_OK)
                       {
                           qDebug() << "Error reading firmware upgrade register" ;
                           error = 1;
                           //return error;
                       }
                       else if(buf_read[0] == 0x11)
                       {
                           qDebug() << "Firmware upgrade OK" << i;
                           error = 0;

                       }
                       else if(buf_read[0] == 0x12)
                       {
                           qDebug() << "Firmware upgrade was unsuccessful" << i;
                           error = 1;
                           //return error;

                       }
                       else
                       {
                          qDebug() << "Unknown behaviour " << i << buf_read[0];
                          error = 1;

                       }

                   }//for


                }//if
                else
                {
                   qDebug() << "Error enabling firmware upgrade" ;
                   error = 1;
                   return error;
                }

            }//else


        }//else

        res = APDCAM_ReadPDI(g_handle, PC_BOARD, FIRMWARE_VERSION, buf_read, 2);

        value=0;

        for(int i=0;i<2;i++)
        {
            value|=(unsigned short)((buf_read[i]) << 8*i);
        }

        qDebug() << "APDCAM Current firmware version: " << value;

    }//if
    else
    {
        qDebug() << "Error opening APDCAM" ;
        error = 1;
    }



    delete [] buf,
    delete [] buf_read;

    err = Close();

    APDCAM_Done();

    return error;

}//DoFirmwareUpgrade


unsigned char ReadFile()
{
    unsigned char error=0;

    QFile file(firmwareFileName);



    if (!file.open(QIODevice::ReadOnly))
    {

        error=1;  //can't open input file
        return error;

    }//if

    data=file.readAll();


    file.close();

    return error;

}//ReadFile


int Open()
{
    unsigned long table[32];
    table[0] = 0;
    int nelems = 0;

    APDCAM_Find(ntohl(inet_addr("10.123.13.101")), ntohl(inet_addr("10.123.13.101")), table, 1, &nelems, (char *)"*", 5000);


    if (nelems == 0) return -1;

    printf("%d board found,\n", nelems);
    g_handle = APDCAM_Open(table[0],1);

    if (g_handle == 0) return -1;


    return 0;

}//Open


int Close()
{
    if (g_handle != 0)
    {
        APDCAM_Close(g_handle);
        g_handle = 0;
    }

    return 0;

}//Close
