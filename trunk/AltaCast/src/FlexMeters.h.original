// EdCastMeters.h: interface for the CFlexMeters class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDCASTMETERS_H__15890841_8319_4F80_9B30_D6444B917B6B__INCLUDED_)
#define AFX_EDCASTMETERS_H__15890841_8319_4F80_9B30_D6444B917B6B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum eMeterDirection
{
	eMeterDirection_Normal,
	eMeterDirection_Backwards,
};

struct FlexMeters_ColourInfo
{
	int colour;		//rgb colour
	int at_value;	//0 to 1000
};


class CFlexMeters_MeterInfo
{
public:
	CFlexMeters_MeterInfo();
	~CFlexMeters_MeterInfo();
	friend class CFlexMeters;

	int meter_width;	//percentage. 100% = full width with, no spacing between meters. 
	int extra_spacing;	//percentage. 100 = one full width of extra spacing.

	int value;			//0 to 1000 - actual meter value

	eMeterDirection direction;
	
	eMeterDirection gradient_direction;

	FlexMeters_ColourInfo colour[8];

	int colours_used;

private:
	void SetDefaultValues()
	{
		value = 100;

		meter_width=90;
		extra_spacing=0;
		direction=eMeterDirection_Normal;
		gradient_direction=eMeterDirection_Normal;

		colours_used=2;
		colour[0].colour=0xFF0000;
		colour[0].at_value=0;
		colour[1].colour=0xFF726F;
		colour[1].at_value=1000;

		colourArray=0;
	};

	void InitColourArray();

	RGBQUAD * colourArray;

	int position;
	int width;
	int length;
	int length_offset;

	float temp;

	float GetFloatMeterWidth();
	float GetFloatExtraSpacing();
};



struct FlexMeters_InitStruct
{
public:

	//double-buffer size. must be at least as big as the meter you're drawing.

	int max_x;
	int max_y;

	//this is the window to grab the coordinates from -
	//so that you don't have to mess with pixels.

	HWND hWndFrame;

	//how much black space around the meter? 0 = none, fill the mapped window completely
	
	RECT border;


	//meter definitions

	int meter_count; 	//number of meters. 2 for stereo

	int horizontal_meters;	//vertical if 0, horizontal if 1

};





class CFlexMeters  
{
public:
	CFlexMeters();
	virtual ~CFlexMeters();

	FlexMeters_InitStruct * Initialize_Step1();	//hands you a struct on a silver platter
	int					Initialize_Step2(FlexMeters_InitStruct * pInitStruct);	//reads in your values, new's meterinfo objects
	CFlexMeters_MeterInfo * GetMeterInfoObject(int index);	//returns pointer to you so you can set things up.
	void					Initialize_Step3();	//final init. after this, you can call RenderMeters.

	void RenderMeters(HDC hDC);

	void Uninitialize();

private:

	void drawmeter(int value, int * buf, int yaxis_shift, int xp, int yp, int xsize, int ysize, RGBQUAD * colourArray, eMeterDirection direction);
	
	void drawmeter_horizontal(int value, int * buf, int yaxis_shift, int xp, int yp, int xsize, int ysize, RGBQUAD * colourArray, eMeterDirection direction);

	// Double buffering data
	HDC memDC;		// memory device context
	HBITMAP	memBM,  // memory bitmap (for memDC)
			oldBM;  // old bitmap (from memDC)

	int meterx;
	int metery;
	int meterx2;
	int metery2;

	int bitmapsize;
	RGBQUAD * buffer;
	int * intbuffer;

	FlexMeters_InitStruct fmis;

	FlexMeters_InitStruct fmis_temp;

	CFlexMeters_MeterInfo ** pMeterInfoArray;

};

#endif // !defined(AFX_EDCASTMETERS_H__15890841_8319_4F80_9B30_D6444B917B6B__INCLUDED_)
