// FlexMeters.cpp: implementation of the CFlexMeters class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FlexMeters.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_VALUE 60
#define MAX_VALUE_F 60.0f
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlexMeters_MeterInfo::CFlexMeters_MeterInfo()
{
	SetDefaultValues();
}

CFlexMeters_MeterInfo::~CFlexMeters_MeterInfo()
{
	if(colourArray)
	{
		free(colourArray);
	}

}

float CFlexMeters_MeterInfo::GetFloatMeterWidth()
{
	return (float) meter_width / 100.0f;
}

float CFlexMeters_MeterInfo::GetFloatExtraSpacing()
{
	return (float) extra_spacing / 100.0f;
}

void CFlexMeters_MeterInfo::InitColourArray()
{
	if(colourArray)
		free(colourArray);
	
	colourArray=(RGBQUAD *) malloc(length * sizeof(RGBQUAD));

	int a=0;

	if(colours_used<2)
	{
		int mycolour=0xB0B0B0;

		if(colours_used==1)
		{
			mycolour=colour[0].colour;
		}

		for(a=0;a<length;a++)
		{
			RGBQUAD * rgbcolour=&colourArray[a];
			int * intcolour=(int *) &colourArray[a];
			
			*intcolour=mycolour;

			if (a & 1)
			{
				rgbcolour->rgbRed = (unsigned char) (rgbcolour->rgbRed * 0.8);
				rgbcolour->rgbGreen = (unsigned char) (rgbcolour->rgbGreen * 0.8);
				rgbcolour->rgbBlue = (unsigned char) (rgbcolour->rgbBlue * 0.8);
			}

		}

		return;
	}


	float masterfactor;

	//csprintf("------------------\n");

	int current_index=0;
	float start_value=colour[current_index].at_value / MAX_VALUE_F;
	RGBQUAD * rgbcolour1=(RGBQUAD *) &colour[current_index].colour;

	current_index++;
	
	float end_value=colour[current_index].at_value / MAX_VALUE_F;
	RGBQUAD * rgbcolour2=(RGBQUAD *) &colour[current_index].colour;

	float current_range=end_value-start_value;

	

	int array_index;
	int array_index_delta;

	if(gradient_direction==eMeterDirection_Backwards)
	{
		array_index=0;
		array_index_delta=1;
	}
	else
	{
		array_index=length-1;
		array_index_delta=-1;
	}

	for(a=0;a<length;a++)
	{
		RGBQUAD * rgbcolour=&colourArray[array_index];
		array_index+=array_index_delta;
		masterfactor = ((float) a) / (float) (length-1);

		if(end_value<masterfactor)
		{
			current_index++;
			if(!(current_index<colours_used)) current_index=colours_used-1;
			
			rgbcolour1=rgbcolour2;
			start_value=end_value;

			end_value=colour[current_index].at_value / MAX_VALUE_F;
			rgbcolour2=(RGBQUAD *) &colour[current_index].colour;

			current_range=end_value-start_value;
		}

		float factor=(masterfactor - start_value) / current_range;
		if(factor<0) factor=0;
		if(factor>1) factor=1;
		


		//csprintf("%i\t%f\n",a,factor);

		rgbcolour->rgbRed = (unsigned char) ((rgbcolour2->rgbRed * factor) + (rgbcolour1->rgbRed * (1 - factor)));
		rgbcolour->rgbGreen = (unsigned char) ((rgbcolour2->rgbGreen * factor) + (rgbcolour1->rgbGreen * (1 - factor)));
		rgbcolour->rgbBlue = (unsigned char) ((rgbcolour2->rgbBlue * factor) + (rgbcolour1->rgbBlue * (1 - factor)));



		if (a & 1)
		{
			rgbcolour->rgbRed = (unsigned char) (rgbcolour->rgbRed * 0.8);
			rgbcolour->rgbGreen = (unsigned char) (rgbcolour->rgbGreen * 0.8);
			rgbcolour->rgbBlue = (unsigned char) (rgbcolour->rgbBlue * 0.8);
		}
	}
}


CFlexMeters::CFlexMeters()
{
	pMeterInfoArray=0;
    memDC = NULL;
    memBM = NULL;

}

CFlexMeters::~CFlexMeters()
{
	Uninitialize();
    if (memDC) {
        DeleteDC(memDC);
    }
    if (memBM) {
        DeleteObject(memBM);
    }
}

void CFlexMeters::Uninitialize()
{
	int a=0;

	if(pMeterInfoArray)
	{
		for(a=0;a<fmis.meter_count;a++)
		{
			delete pMeterInfoArray[a];
		}

		free(pMeterInfoArray);
	}

}

FlexMeters_InitStruct * CFlexMeters::Initialize_Step1()
{
	fmis_temp.max_x=512;
	fmis_temp.max_y=512;

	fmis_temp.hWndFrame=0;

	fmis_temp.border.left=3;
	fmis_temp.border.right=3;
	fmis_temp.border.bottom=3;
	fmis_temp.border.top=3;

	fmis_temp.meter_count=0;

	fmis_temp.horizontal_meters=0;

	return &fmis_temp;
}

int CFlexMeters::Initialize_Step2(FlexMeters_InitStruct * pInitStruct)
{
	memcpy(&fmis,pInitStruct,sizeof(FlexMeters_InitStruct));

	if(!fmis.meter_count) return -1;

	pMeterInfoArray=(CFlexMeters_MeterInfo **) malloc(fmis.meter_count * sizeof(int *));

	int a=0;

	for(a=0;a<fmis.meter_count;a++)
	{
		CFlexMeters_MeterInfo * pMeterInfo;

		pMeterInfo=new CFlexMeters_MeterInfo;

		pMeterInfoArray[a]=pMeterInfo;
	}

	return 0;
}

CFlexMeters_MeterInfo * CFlexMeters::GetMeterInfoObject(int index)
{
	return pMeterInfoArray[index];
}


void CFlexMeters::Initialize_Step3()
{

    BITMAPINFO bitmapinfo;

    bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapinfo.bmiHeader.biWidth = fmis.max_x;
    bitmapinfo.bmiHeader.biHeight = -fmis.max_y;
    bitmapinfo.bmiHeader.biPlanes = 1;
    bitmapinfo.bmiHeader.biBitCount = 32;
    bitmapinfo.bmiHeader.biCompression = BI_RGB;
    bitmapinfo.bmiHeader.biSizeImage = fmis.max_x * fmis.max_y * 4;
    bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biClrUsed = 0;
    bitmapinfo.bmiHeader.biClrImportant = 0;

    bitmapsize = bitmapinfo.bmiHeader.biSizeImage;

    // create our doublebuffer
    memDC = CreateCompatibleDC(NULL);
    memBM = CreateDIBSection(memDC,
                             &bitmapinfo,
                             DIB_RGB_COLORS, (void **) & buffer, NULL, 0);
    oldBM = (HBITMAP) ::SelectObject(memDC, memBM);

    intbuffer = (int *) buffer;




    RECT rect;

    GetClientRect(fmis.hWndFrame, &rect);

    meterx = (rect.left);
    meterx2 = (rect.right);
    metery = (rect.top);
    metery2 = (rect.bottom);





	//this is where the fun begins. calculate which pixel offsets to actually use

	int a=0;

	float xwidth_meters=0;
	float xwidth_extra=0;


	for(a=0;a<fmis.meter_count;a++)
	{
		if(a==fmis.meter_count-1)
		{
			xwidth_meters+=GetMeterInfoObject(a)->GetFloatMeterWidth();
		}
		else
		{
			xwidth_extra+=GetMeterInfoObject(a)->GetFloatExtraSpacing();
			xwidth_meters+=1;
		}
	}

	//csprintf("xwidth_meters: %f\n",xwidth_meters);
	//csprintf("xwidth_extra: %f\n",xwidth_extra);

	int available_pixels;
	int length_pixels;
	int length_offset;
	int width_offset;

	if(fmis.horizontal_meters)
	{
		available_pixels = metery2 - metery - fmis.border.top - fmis.border.bottom;
		width_offset=fmis.border.top;
	}
	else
	{
		available_pixels = meterx2 - meterx - fmis.border.left - fmis.border.right;
		width_offset=fmis.border.left;
	}

	//csprintf("available pixels: %i\n",available_pixels);

	float total_factor=available_pixels / (xwidth_meters + xwidth_extra);
	
	float meter_factor=total_factor;
	//float meter_factor=total_factor * (xwidth_meters / (xwidth_meters + xwidth_extra));
	

	//float width_per_meter=0;

	//csprintf("width per meter: %f\n",width_per_meter);

	float x=0;
	
	for(a=0;a<fmis.meter_count;a++)
	{
		if(a==fmis.meter_count-1)
		{
			x+=floor(meter_factor*GetMeterInfoObject(a)->GetFloatMeterWidth());
		}
		else
		{
			x+=floor(meter_factor);
		}
		
		x+=floor(GetMeterInfoObject(a)->GetFloatExtraSpacing()*meter_factor);
	}

	int difference=(available_pixels - (int) x);

	int xstart=(difference/2) + width_offset;

	if(fmis.horizontal_meters)
	{
		length_pixels = meterx2 - meterx - fmis.border.left - fmis.border.right;
		length_offset=fmis.border.left;
	}
	else
	{
		length_pixels = metery2 - metery - fmis.border.top - fmis.border.bottom - (difference);
		length_offset=fmis.border.top + (difference/2);
	}


	for(a=0;a<fmis.meter_count;a++)
	{
		GetMeterInfoObject(a)->position=xstart;
		GetMeterInfoObject(a)->length=length_pixels;
		GetMeterInfoObject(a)->length_offset=length_offset;

		GetMeterInfoObject(a)->width = (int) ((meter_factor*GetMeterInfoObject(a)->GetFloatMeterWidth())-0.5);

		xstart+=(int) floor(meter_factor);
		xstart+=(int) floor(GetMeterInfoObject(a)->GetFloatExtraSpacing()*meter_factor);

		GetMeterInfoObject(a)->InitColourArray();
	}
	

	//csprintf("xstart=%i\n",xstart);

	//	csprintf("%i   x=%f\n",a,x);


}


void CFlexMeters::drawmeter(int value, int * buf, int ymulti, int xp, int yp, int x_size, int y_size, RGBQUAD * colourArray, eMeterDirection direction)
{
    int x = 0;
    int y = 0;
    int localcolour = 0;

    if (!x_size) return ;

	//value = 700;

    RGBQUAD * rgbcolour = (RGBQUAD *) & localcolour;


    int m_count = 0;
    int thresh = 0;

    if (direction == eMeterDirection_Normal)
    {
        thresh = yp + (((y_size) * (MAX_VALUE-value)) / MAX_VALUE);
    }
    else
    {
        thresh = yp + (((y_size) * (value)) / MAX_VALUE);
    }

    for (y = yp;y < yp + y_size;y++)
    {
        int ymul = y * ymulti;
        if ((y < thresh && direction == eMeterDirection_Backwards) || (y >= thresh && direction == eMeterDirection_Normal))
        {
			localcolour = *((int *) &colourArray[y-yp]);
        }
        else
        {
            localcolour = 0;
        }
        for (x = xp;x < xp + x_size;x++)
        {
            buf[(ymul) + x] = localcolour;
        }
        m_count++;
    }


}


void CFlexMeters::drawmeter_horizontal(int value, int * buf, int ymulti, int xp, int yp, int x_size, int y_size, RGBQUAD * colourArray, eMeterDirection direction)
{
    int x = 0;
    int y = 0;
	int yofs = 0;
    int localcolour = 0;

    if (!y_size) return ;

	//value = 700;

    RGBQUAD * rgbcolour = (RGBQUAD *) & localcolour;


    int thresh = 0;

    if (direction == eMeterDirection_Backwards)
    {
        thresh = xp + (((x_size) * (MAX_VALUE-value)) / MAX_VALUE);
    }
    else
    {
        thresh = xp + (((x_size) * (value)) / MAX_VALUE);
    }

    for (x = xp;x < xp + x_size;x++)
    {
        if ((x < thresh && direction == eMeterDirection_Normal) || (x >= thresh && direction == eMeterDirection_Backwards))
        {
			localcolour = *((int *) &colourArray[(x_size-1)-(x-xp)]);
        }
        else
        {
            localcolour = 0;
        }

		yofs = yp * ymulti;
        for (y = 0;y < y_size;y++)
        {
            buf[x+yofs] = localcolour;
			yofs+=ymulti;
        }
    }


}



void CFlexMeters::RenderMeters(HDC hDC)
{

	int a=0;


	if(fmis.horizontal_meters)
	{
		for(a=0;a<fmis.meter_count;a++)
		{
			CFlexMeters_MeterInfo * pMeter=GetMeterInfoObject(a);
			drawmeter_horizontal( pMeter->value,intbuffer,fmis.max_x,pMeter->length_offset,pMeter->position,pMeter->length,pMeter->width,pMeter->colourArray,pMeter->direction);
		}
	}
	else
	{
		for(a=0;a<fmis.meter_count;a++)
		{
			CFlexMeters_MeterInfo * pMeter=GetMeterInfoObject(a);
			drawmeter(pMeter->value,intbuffer,fmis.max_x,pMeter->position,pMeter->length_offset,pMeter->width,pMeter->length,pMeter->colourArray,pMeter->direction);
		}
	}

	CFlexMeters * other=this;
	BitBlt(hDC, other->meterx, other->metery, other->meterx2 - other->meterx, other->metery2 - other->metery, memDC, 0, 0, SRCCOPY);
}
