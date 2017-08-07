(CESHI)GPS A,ATGM332D,OK,0|7|0|0,12,0,0,30,ditu.google.cn/?q=30.328053N,120.055273E
(CESHI)GPS $GNGGA,053131.000,3019.6774,N,12003.3095,E,1,12,1.0,27.5,M,0.0,M,,*43
$GNGLL,3019.6774,N,12003.3095,E,053131.000,A,A*44
$GPGSA,A,3,06,30,28,03,11,19,22,01,17,,,,1.8,1.0,1.5*3D
$BDGSA,A,3,13,08,12,,,,,,,,,,1.8,1.0,1.5*26
$GPGSV,3,1,09,01,29,043,35,03,53,084,26,06,33,251,23,11,19,062,24*7F
$GPGSV,3,2,09,17,50,332,38,19,33,309,17,22,34,057,24,28,82,192,26*72
$GPGSV,3,3,09,30,16,207,16*46
$BDGSV,1,1,03,08,59,338,33,12,33,303,27,13,49,296,30*50
$GNRMC,053131.000,A,3019.6774,N,12003.3095,E,0.33,178.03,270717,,,A*7A
$GNVTG,178.03,T,,M,0.33,N,0.61,K,A*29
$GNZDA,053131.000,27,07,2017,00,00*4B
$GPTXT,01,01,01,ANTENNA OK*35

直接取GPS/BD的原始NEMA数据进行解析
1、提取定位精度信息，来源GSA。提取2D/3D定位状态，位置精度因子，水平精度因子，垂直精度因子。
工装根据设置的参数，限制定位精度信息必须优于设定值。
2、可见星的载噪比，来源GSV。提取载噪比信息。
工装根据设置的参数，比方说：要求载噪比>45的卫星数>1，载噪比>40的卫星数>3，载噪比>35的卫星数>10
3、位置信息，来源RMC。提取经纬度信息。
工装根据设置的参数，限制待检设备的经纬度在要求的区域半径范围内，同时限制一段时间内的静态漂流距离。
4、区分GPS和BD两种定位模式
5、提取定位后的时间，用于工装内部校时。来源RMC。

(CESHI)GPS $GNGGA,,,,,,0,00,25.5,,,,,,*64
$GNGLL,,,,,,V,M*79
$GPGSA,A,1,,,,,,,,,,,,,25.5,25.5,25.5*02
$BDGSA,A,1,,,,,,,,,,,,,25.5,25.5,25.5*13
$GPGSV,1,1,01,17,,,37*7A
$BDGSV,1,1,00*68
$GNRMC,,V,,,,,,,,,,M*4E
$GNVTG,,,,,,,,,M*2D
$GNZDA,,,,,,*56
$GPTXT,01,01,01,ANTENNA OK*35


(CESHI)GPS A,ATGM332D,OK,0|5|0|0,12,0,0,31,ditu.google.cn/?q=30.328053N,120.055273E


#define NumMsg 9

typedef struct{
	u8		Gps_Numsv;							//卫星总数
	u8 		Gps_Cnr[NumMsg - 1];		//GPS载噪比数据
	char  Gps_Lat;								//GPS定位纬度标识
	char  Gps_Longit;							//GPS定位经度标识
	float Gps_Lat_Loc;						//GPS定位纬度
	float Gps_Longit_Loc;					//GPS定位经度
	
	float Gps_Pdop;								//位置精度因子
	float Gps_Hdop;								//水平精度因子
	float Gps_Vdop;								//垂直精度因子
	
	
  int tm_sec;   /* seconds after the minute, 0 to 60 (0 - 60 allows for the occasional leap second) */
  int tm_min;   /* minutes after the hour, 0 to 59 */
  int tm_hour;  /* hours since midnight, 0 to 23 */
  int tm_mday;  /* day of the month, 1 to 31 */
  int tm_mon;   /* months since January, 0 to 11 */
  int tm_year;  /* years since 1900 */
	
	u8		Gps;			
	
	
}	__TesterV2_DealFnct_GPS_NEMA_St__;

//固定长度读取数据

u16 fussy_String2Dec_Len(char *pStr,u8 Len)
{
	u8  i     = 0;
	u32 uTemp = 0;
	while (Len--) {
		if ((pStr[i] >= '0')  &&
	     	(pStr[i] <= '9')) {
			uTemp = uTemp * 10 + (pStr[i] - '0');
		} else {
			break;
		}
		i ++;
	}
	return uTemp;
}

#define 

u8 TesterV2_DealFnct_GPS_NEMA_GetVal(__TesterV2Info__ *pTesterV2Info, __TesterV2_DealFnct_GPS_NEMA_St__ *pTNEMAVal)
{
	u16 temp_t;
	char *str;
    int i, try = 5;
    u8 temp_i,temp_j;
	if (pTGPIVal == NULL) {
		return false;
	}
	*pTGPIVal = 0;
	sprintf((char *)pTesterV2Info->sendbuf, "(CESHI)GPS MONITOR");
    for (i = 0; i < try; i++) {
	    pTesterV2Info->pUARTDrv->setTimeOut(TESTERV2_UART_TIMEOUT);
		pTesterV2Info->pUARTDrv->clrbuf();
        pTesterV2Info->pUARTDrv->sendStr(pTesterV2Info->sendbuf);
		pTesterV2Info->pUARTDrv->readStr(pTesterV2Info->readbuf, TESTERV2_READBUF_MAXLEN);
        pTesterV2Info->pUARTDrv->setTimeOut(10);
        pTesterV2Info->pUARTDrv->waitidle();
		do {
			str = strstr((char *)pTesterV2Info->readbuf, "(CESHI)GPS ");
			if (str == NULL) {
				break;	
			}
			/* 解析GSA数据 */
			str = strstr((char *)pTesterV2Info->readbuf, "$GPGSA");                       
			str += sizeof("$GPGSA") - 1;
			temp_i = 0;
			do{
				if (str == NULL) {
					break;	
				}
				if( *str == ',' )
					{
						temp_i ++;
						}				
				str++;
			}while(temp_i < 15);
			pTNEMAVal->Gps_Pdop = fussy_String2Dec_Tmp(str);	
			str = strchr(str,'，');
			if (str == NULL) {
				break;	
			}
			str++；
			pTNEMAVal->Gps_Hdop = ussy_String2Dec_Tmp(str);
			str = strchr(str,',');
			if (str == NULL) {
				break;	
			}
			str++;						
			pTNEMAVal->Gps_Vdop = ussy_String2Dec_Tmp(str);	
			/* 解析GSV数据 */
			str = strstr((char *)pTesterV2Info->readbuf, "$GPGSV");														
			if (str == NULL) {
				break;	
			}			
			str += sizeof("$GPGSV") - 1;	
			do{
				if (str == NULL) {
					break;	
				}
				if( *str == ',' )
					{
						temp_i ++;
						}				
				str++;
			}while(temp_i < 3);			
			pTNEMAVal->Gps_Numsv = fussy_String2Dec(str);
			temp_i = 0;
			temp_j = 0;	
			while( temp_j < pTNEMAVal->Gps_Numsv)
			{
				if((temp_j % 4 == 0 ) && (temp_j != 0) )
					{
						str = strstr((char *)pTesterV2Info->readbuf, "$GPGSV");
						if (str == NULL) {
								break;	
						}
						str += sizeof("$GPGSV") - 1;
						temp_i = 0;
						do{
							if (str == NULL) {
								break;	
							}
							if( *str == ',' )
							{
								temp_i ++;
							}				
							str++;
						}while(temp_i < 3);							
					}
					temp_i = 0;
				do{
					if (str == NULL) {
						break;	
					}
					if( *str == ',' )
					{
						temp_i ++;
						}				
					str++;
				}while(temp_i < 4);
				if( *str == ',' )
					{
						break;
						}
				pTNEMAVal->Gps_Cnr[temp_j] = fussy_String2Dec(str);
				temp_j ++;
				temp_i == 0;								
			}
			/* 解析RMC数据 */
			str = strstr((char *)pTesterV2Info->readbuf, "$GNRMC");
			if (str == NULL) {
				break;	
			}			
			str += sizeof("$GNRMC") - 1;	
			if( *str == ',')
			{
				break;
			}
			str++;	
			pTNEMAVal-> tm_hour = fussy_String2Dec_Len(str,2);
			str+=2;
			pTNEMAVal-> tm_min = fussy_String2Dec_Len(str,2);
			str+=2;
			pTNEMAVal-> tm_sec = fussy_String2Dec_Len(str,2);	
			temp_i = 0;
			do{
				if (str == NULL) {
					break;	
				}
				if( *str == ',' )
				{
					temp_i ++;
					}				
				str++;
			}while(temp_i < 2);				
			pTNEMAVal-> Gps_Lat_Loc = ussy_String2Dec_Tmp(str);
			str = strchr(str,',');
			if (str == NULL) {
				break;	
			}	
			str++;
			if( *str == ',')
			{
					break;
			} else
				{
						pTNEMAVal-> Gps_Lat = ussy_String2Dec_Tmp(str);
					}
			if( *str == 'N' )
				{
					pTNEMAVal-> Gps_Lat = 'N';
				}	else
				{
					pTNEMAVal-> Gps_Lat = 'S';
				}
			str+=2;
			if( str == NULL)
			{
				break;
			}
			pTNEMAVal-> Gps_Longit = ussy_String2Dec_Tmp(str);			
			return true;
		} while (0);
	}
    return false;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DATASTORE_PID_STRLEN      15
#define u32 unsigned int
#define u8  unsigned char
typedef    signed  int      s32;

#define FALSE   0
#define TRUE    1

typedef struct
{
    u32  LOADWORD;
    u32  Useage;
    u32  CheckCount;
    u32  PassCount;
	char PID[DATASTORE_PID_STRLEN+1];
	u8   RES[24];
	u32  ExportSeq;
    u32  CHKSUM;
} __PIDStore__;

__PIDStore__ PIDStore;




float fussy_String2Dec_Tmp(char *pStr)
{
	u8  isNegative = FALSE;
	u8  i      = 0;
	float sTemp  = 0;
	float sTemp1 = 0;
	if (pStr[0] == '-') {
		isNegative = TRUE;
		pStr ++;
	}
	while (1) {
		if ((pStr[i] >= '0')  &&
	     	(pStr[i] <= '9')) {
			sTemp = sTemp * 10.0 + (pStr[i] - '0');
		} else {
			break;
		}
		i ++;
	}
	//sTemp /= 100;
	if (pStr[i] != '.') {
		return sTemp;
	}
	i ++;
	pStr = &pStr[i];
	i    = 0;
	while (1) {
		if ((pStr[i] >= '0')  &&
	     	(pStr[i] <= '9')) {
			sTemp1 = sTemp1 * 10.0 + (pStr[i] - '0');

		} else {
			break;
		}
		i ++;
	}
	while ( i-- )
        {
             sTemp1 = sTemp1 * 0.1;

        }
	/*while (i < 1) {
		sTemp1 *= 10;
		i ++;
	}*/
	sTemp += sTemp1;
	if (isNegative == TRUE) {
		sTemp = -sTemp;
	}
	return sTemp;
}


int main()
{
        int  ustrlen;
        char *srt="2.564,,,CGH,,,,,,,,,,,,,12003.3095,2.5,0.9";
        char *hjl;
        char  b[16]="";
        memset(&PIDStore,0,sizeof(__PIDStore__)-1);
        static char a[DATASTORE_PID_STRLEN + 1] = "";
        strcpy(a,PIDStore.PID);
        ustrlen = strlen(a);
        printf("Hello world!\n");
        printf("Test::::%d\n",ustrlen);
        printf("Test::::%d\n",strlen(b));
        printf("%d\n",PIDStore.PID[0]);
        printf("%d %d %d %d\n",a[0],a[1],a[2],a[3]);
        hjl = strstr(srt,"CGH");
        hjl = strchr(hjl,'.');
        if (hjl == NULL) {
                return 0;
        }

       do {
                if( hjl == NULL )
                {
                        return 0;
                }
                hjl --;
        }while( *hjl != ',');
        hjl++;
        printf("%f\n",fussy_String2Dec_Tmp(hjl));
        hjl = strchr(hjl,',');
        hjl++;
        printf("%f\n",fussy_String2Dec_Tmp(hjl));
        hjl = strchr(hjl,',');
        hjl++;
        printf("%f\n",fussy_String2Dec_Tmp(hjl));
        return 0;
}