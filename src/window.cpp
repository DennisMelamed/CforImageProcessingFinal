#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcbuffer.h>

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/overlay.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/vector.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <window.h>

static const wxChar *FILETYPES = _T("All files (*.*)|*.*");

IMPLEMENT_APP(BasicApplication)

bool BasicApplication::OnInit()
{
	wxInitAllImageHandlers();
	MyFrame *frame = new MyFrame(_("Basic Frame"), 50, 50, 800, 600);
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width, int height): wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)){

	fileMenu = new wxMenu;
	fileMenu->Append(LOAD_FILE_ID, _T("&Open file"));
	fileMenu->AppendSeparator();

	//###########################################################//
	//----------------------START MY MENU -----------------------//
	//###########################################################//

	fileMenu->Append(RESET_IMAGE_ID, _T("&Reset image"));
	fileMenu->Append(INVERT_IMAGE_ID, _T("&Invert image"));
	fileMenu->Append(SCALE_IMAGE_ID, _T("&Scale image"));
	fileMenu->Append(CONVOLUTE_IMAGE_ID, _T("&Convolute image"));
	fileMenu->Append(MY_IMAGE_ID, _T("&Shift Function")); //--->To be modified!
	
	fileMenu->Append(UNDO_ID, _T("&Undo"));

	//###########################################################//
	//----------------------END MY MENU -------------------------//
	//###########################################################//

	fileMenu->AppendSeparator();
	fileMenu->Append(SAVE_IMAGE_ID, _T("&Save image"));
	fileMenu->Append(EXIT_ID, _T("E&xit"));



	// MENU FOR LAB6
	lab6Menu = new wxMenu;
	lab6Menu->Append(NOISE_IMAGE_ID, _T("&Add  S/P Noise"));
	lab6Menu->Append(MIN_FILTER_IMAGE_ID, _T("&Min Filter"));
	lab6Menu->Append(MAX_FILTER_IMAGE_ID, _T("&Max Filter"));
	lab6Menu->Append(MID_FILTER_IMAGE_ID, _T("&Mid Filter"));

	//MENU FOR LAB 7
	lab7Menu = new wxMenu;
	lab7Menu->Append(NEG_LINEAR_TRANSFORM_IMAGE_ID, _T("&Negative Linear"));
	lab7Menu->Append(LOG_TRANSFORM_IMAGE_ID, _T("&Logarithmic"));
	lab7Menu->Append(POWER_TRANSFORM_IMAGE_ID, _T("&Power Transform"));
	lab7Menu->Append( RAND_LOOKUP_TABLE_TRANSFORM_ID, _T("&Random Lookup"));


	menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(lab6Menu, _T("&Order Statistic Filtering"));
	menuBar->Append(lab7Menu, _T("&Point Processing"));

	SetMenuBar(menuBar);
	CreateStatusBar(3);

	SetBackgroundStyle(wxBG_STYLE_PAINT);

	//this = new wxPanel( this );
	this->Bind( wxEVT_LEFT_DOWN, &MyFrame::OnLeftDown, this );
	this->Bind( wxEVT_LEFT_UP, &MyFrame::OnLeftUp, this );
	this->Bind( wxEVT_MOTION, &MyFrame::OnMotion, this );
	selecting = false;
	ROI[0] = 0;
	ROI[1] = 0;
	ROI[2] = 0;
	ROI[3] = 0;

}

MyFrame::~MyFrame(){

	/* release resources */
	if(loadedImage.Ok()) {
		loadedImage.Destroy();
	}

	if (origImage.Ok()) {
		origImage.Destroy();
	}

}

void MyFrame::OnOpenFile(wxCommandEvent & WXUNUSED(event)) {

	wxFileDialog openFileDialog(this, _T("Open file"), _T(""), _T(""), FILETYPES, wxFD_OPEN, wxDefaultPosition);

	if(openFileDialog.ShowModal() == wxID_OK){
		wxString filename = openFileDialog.GetFilename();
		wxString path = openFileDialog.GetPath();
		printf("Loading image form file...");

		if(loadedImage.Ok()) {
			loadedImage.Destroy();
		}

		if (origImage.Ok()) {
			origImage.Destroy();
		}
		UndoImages.clear();

		loadedImage = wxImage(path); //Image Loaded form file


		if(loadedImage.Ok()) {
			origImage = loadedImage.Copy();
			UndoImages.push_back(origImage);

			printf("Done! \n");
		}
		else {
			printf("error:...");
		}
	}
	Refresh();

}


//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE ABOVE--------------------//
//-----------------------------------------------------------//
//###########################################################//


//INVERT IMAGE
void MyFrame::OnInvertImage(wxCommandEvent & WXUNUSED(event)) {

	printf("Inverting...");
	wxImage tmpImage = loadedImage.Copy();

	for(int i = 0; i < loadedImage.GetWidth(); i++) {
		for(int j = 0;j < loadedImage.GetHeight(); j++) {
			{
				loadedImage.SetRGB(i, j, 255 - tmpImage.GetRed(i,j),
						255 - tmpImage.GetGreen(i,j),
						255 - tmpImage.GetBlue(i,j));
			}
		}
	}

	tmpImage.Destroy();
	printf(" Finished inverting.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
	
}

//IMAGE SCALEING
void MyFrame::OnScaleImage(wxCommandEvent & WXUNUSED(event)) {

	printf("Scaling...");
	wxTextEntryDialog *openTextEntryDialog = new wxTextEntryDialog ( this, _T("scale value"), _T(""), _T(""),wxTextEntryDialogStyle , wxDefaultPosition);
	if(openTextEntryDialog->ShowModal() == wxID_OK){
		wxString value = openTextEntryDialog->GetValue();
		double factor;
		if(!value.ToDouble(&factor)){ printf("error!");}
		//cout << factor<< endl;

		//free(loadedImage);
		wxImage tmpImage = loadedImage.Copy();

		double r,g,b;

		//std::vector<std::vector<std::vector<double> > > rgb;
		std::vector<std::vector<double> > rvec(loadedImage.GetWidth(),std::vector<double>(loadedImage.GetHeight(), 0));
		std::vector<std::vector<double> > gvec(loadedImage.GetWidth(),std::vector<double>(loadedImage.GetHeight(), 0));
		std::vector<std::vector<double> > bvec(loadedImage.GetWidth(),std::vector<double>(loadedImage.GetHeight(), 0));
		//rgb.push_back(rvec);
		//rgb.push_back(gvec);
		//rgb.push_back(bvec);

		double maxr = 255, maxg = 255,  maxb = 255;
		double minr = 0,  ming = 0,  minb = 0;
		for( int i=0;i<loadedImage.GetWidth();i++)
			for(int j=0;j<loadedImage.GetHeight();j++){
				r = tmpImage.GetRed(i,j);
				r = round(r*factor);
				if(maxr < r)
				{
					maxr = r;
				}
				if(minr > r)
				{
					minr = r;
				}

				g = tmpImage.GetGreen(i,j);
				g = round(g*factor);
				if(maxg < g)
				{
					maxg = g;
				}
				if(ming > g)
				{
					ming = g;
				}

				b = tmpImage.GetBlue(i,j);
				b = round(b*factor);
				if(maxb < b)
				{
					maxb = b;
				}
				if(minb > b)
				{
					minb = b;
				}



				//loadedImage->SetRGB(i,j,r,g,b);
				rvec[i][j] = r;
				gvec[i][j] = g;
				bvec[i][j] = b;
			}

		for(int i=0; i< loadedImage.GetWidth(); i++)
		{
			for(int j = 0; j<loadedImage.GetHeight(); j++)
			{
				r = rvec[i][j];
				g = gvec[i][j];
				b = bvec[i][j];
				if(maxr>255)
				{
					r = round(r*(255/maxr));
				}
				if(maxg > 255)
				{
					g = round(g*(255/maxg));
				}
				if(maxb > 255)
				{
					b = round(b*(255/maxb));
				}
				loadedImage.SetRGB(i,j,r,g,b);
			}
		}

		printf(" Finished scaling.\n");
		Refresh();
		UndoImages.push_back(loadedImage);
	}
}

void MyFrame::OnConvoluteImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Convolute function...");
	//free(loadedImage);
	wxImage tmpImage = loadedImage.Copy();

	//unsigned char r, g, b;

	double r,g,b;
	int decision = 0;
	int imgWidth = loadedImage.GetWidth();
	int imgHeight = loadedImage.GetHeight();

	wxArrayString choices;
	//wxArrayString &choice = *choices;
	choices.Add(_T("Averaging"));
	choices.Add(_T("Weighted Averaging"));
	choices.Add(_T("4 neighbor Laplace"));
	choices.Add(_T("8 neighbor Laplace"));
	choices.Add(_T("4 neighbor Laplace enhancement"));
	choices.Add(_T("8 neighbor Laplace enhancement"));
	choices.Add(_T("Roberts1"));
	choices.Add(_T("Roberts2"));
	choices.Add(_T("SobelX"));
	choices.Add(_T("SobelY"));
	choices.Add(_T("Bi-Directional Edge Detect"));

	//mask vector
	std::vector<std::vector<double> > mask(3, std::vector<double>(3, 0));

	wxSingleChoiceDialog openSingleChoiceDialog(
			this,
			_T("Select Mask for Convolution"),
			_T(""),
			choices,
			NULL,
			wxCHOICEDLG_STYLE,
			wxDefaultPosition);


	if (openSingleChoiceDialog.ShowModal() == wxID_OK)
	{
		decision = openSingleChoiceDialog.GetSelection();
		switch(decision)
		{
		case 0:
			//mask vectors

			mask[0][0] = .1111111111;
			mask[0][1] = .1111111111;
			mask[0][2] = .1111111111;
			mask[1][0] = .1111111111;
			mask[1][1] = .1111111111;
			mask[1][2] = .1111111111;
			mask[2][0] = .1111111111;
			mask[2][1] = .1111111111;
			mask[2][2] = .1111111111;
			break;
		case 1:
			mask[0][0] = .0625;
			mask[0][1] = .125;
			mask[0][2] = .0625;
			mask[1][0] = .125;
			mask[1][1] = .25;
			mask[1][2] = .125;
			mask[2][0] = .0625;
			mask[2][1] = .125;
			mask[2][2] = .0625;
			break;

		case 2:
			mask[0][0] = 0;
			mask[0][1] = -1;
			mask[0][2] = 0;
			mask[1][0] = -1;
			mask[1][1] = .4;
			mask[1][2] = -1;
			mask[2][0] = 0;
			mask[2][1] = -1;
			mask[2][2] = 0;
			break;
		case 3:
			mask[0][0] = -1;
			mask[0][1] = -1;
			mask[0][2] = -1;
			mask[1][0] = -1;
			mask[1][1] = 8;
			mask[1][2] = -1;
			mask[2][0] = -1;
			mask[2][1] = -1;
			mask[2][2] = -1;
			break;
		case 4:
			mask[0][0] = 0;
			mask[0][1] = -1;
			mask[0][2] = 0;
			mask[1][0] = -1;
			mask[1][1] = 5;
			mask[1][2] = -1;
			mask[2][0] = 0;
			mask[2][1] = -1;
			mask[2][2] = 0;
			break;
		case 5:
			mask[0][0] = -1;
			mask[0][1] = -1;
			mask[0][2] = -1;
			mask[1][0] = -1;
			mask[1][1] = 9;
			mask[1][2] = -1;
			mask[2][0] = -1;
			mask[2][1] = -1;
			mask[2][2] = -1;
			break;
		case 6:
			mask[0][0] = 0;
			mask[0][1] = 0;
			mask[0][2] = 0;
			mask[1][0] = 0;
			mask[1][1] = 0;
			mask[1][2] = 1;
			mask[2][0] = 0;
			mask[2][1] = -1;
			mask[2][2] = 0;
			break;
		case 7:
			mask[0][0] = 0;
			mask[0][1] = 0;
			mask[0][2] = 0;
			mask[1][0] = 0;
			mask[1][1] = -1;
			mask[1][2] = 0;
			mask[2][0] = 0;
			mask[2][1] = 0;
			mask[2][2] = 1;
			break;
		case 8:
			mask[0][0] = -1;
			mask[0][1] = 0;
			mask[0][2] = 1;
			mask[1][0] = -2;
			mask[1][1] = 0;
			mask[1][2] = 2;
			mask[2][0] = -1;
			mask[2][1] = 0;
			mask[2][2] = 1;
			break;
		case 9:
			mask[0][0] = -1;
			mask[0][1] = -2;
			mask[0][2] = -1;
			mask[1][0] = 0;
			mask[1][1] = 0;
			mask[1][2] = 0;
			mask[2][0] = 1;
			mask[2][1] = 2;
			mask[2][2] = 1;
			break;
		case 10:
			mask[0][0] = -1;
			mask[0][1] = 0;
			mask[0][2] = 1;
			mask[1][0] = -2;
			mask[1][1] = 0;
			mask[1][2] = 2;
			mask[2][0] = -1;
			mask[2][1] = 0;
			mask[2][2] = 1;
			break;

		}
	}




	std::vector < std::vector<double> > rvec(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double> > gvec(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double> > bvec(imgWidth, std::vector<double>(imgHeight, 0));
	
	//only for bi-directional edge detect
	std::vector < std::vector<double> > rvecy(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double> > gvecy(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double> > bvecy(imgWidth, std::vector<double>(imgHeight, 0));



	double maxr = 255, maxg = 255, maxb = 255;
	double minr = 0, ming = 0, minb = 0;
	for (int i = 1; i < imgWidth-1; i++)
		for (int j = 1; j < imgHeight-1; j++)
		{
			r = 0;
			b = 0;
			g = 0;
			//r = loadedImage->GetRed(i, j);

			for(int a = -1; a<2; a++)
			{
				for(int c = -1; c<2; c++)
				{

					r += ((loadedImage.GetRed(i-c, j-a))*mask[c+1][a+1]);
					//printf("%f \n", mask[c+1][a+1]);
					g += ((loadedImage.GetGreen(i-c, j-a))*mask[c+1][a+1]);
					b += ((loadedImage.GetBlue(i-c, j-a))*mask[c+1][a+1]);
				}
			}


			if (maxr < r)
			{
				maxr = r;
			}
			if (minr > r)
			{
				minr = r;
			}


			if (maxg < g)
			{
				maxg = g;
			}
			if (ming > g)
			{
				ming = g;
			}


			if (maxb < b)
			{
				maxb = b;
				printf("maxb: %f", maxb);
			}
			if (minb > b)
			{
				minb = b;

			}

			//loadedImage->SetRGB(i,j,r,g,b);
			rvec[i][j] = r;
			gvec[i][j] = g;
			bvec[i][j] = b;
		}
	if(decision == 10)
	{
		mask[0][0] = -1;
					mask[0][1] = -2;
					mask[0][2] = -1;
					mask[1][0] = 0;
					mask[1][1] = 0;
					mask[1][2] = 0;
					mask[2][0] = 1;
					mask[2][1] = 2;
					mask[2][2] = 1;
	
	for (int i = 1; i < imgWidth-1; i++)
			for (int j = 1; j < imgHeight-1; j++)
			{
				r = 0;
				b = 0;
				g = 0;
				//r = loadedImage->GetRed(i, j);

				for(int a = -1; a<2; a++)
				{
					for(int c = -1; c<2; c++)
					{

						r += ((loadedImage.GetRed(i-c, j-a))*mask[c+1][a+1]);
						//printf("%f \n", mask[c+1][a+1]);
						g += ((loadedImage.GetGreen(i-c, j-a))*mask[c+1][a+1]);
						b += ((loadedImage.GetBlue(i-c, j-a))*mask[c+1][a+1]);
					}
				}


				if (maxr < r + rvec[i][j])
				{
					maxr = r;
				}
				if (minr > r+ rvec[i][j])
				{
					minr = r;
				}


				if (maxg < g+ gvec[i][j])
				{
					maxg = g;
				}
				if (ming > g+ gvec[i][j])
				{
					ming = g;
				}


				if (maxb < b+ bvec[i][j])
				{
					maxb = b;
					//printf("maxb: %f", maxb);
				}
				if (minb > b+ bvec[i][j])
				{
					minb = b;

				}

				//loadedImage->SetRGB(i,j,r,g,b);
				rvecy[i][j] = r;
				gvecy[i][j] = g;
				bvecy[i][j] = b;
				
				rvec[i][j] += rvecy[i][j];
				gvec[i][j] += gvecy[i][j];
				bvec[i][j] += bvecy[i][j];
				
			}
	}

	for (int i = 1; i < imgWidth-1; i++)
	{
		for (int j = 1; j < imgHeight-1; j++)
		{
			//			if(i == imgWidth-2 && j==imgHeight-2)
			//			{
			//				printf("minr: %f ming: %f minb: %f maxr: %f maxg: %f maxb: %f", minr, ming, minb, maxr, maxg, maxb);
			//			}
			r = rvec[i][j] + abs(minr);
			g = gvec[i][j] + abs(ming);
			b = bvec[i][j] + abs(minb);
			//			if(i == imgWidth-2 && j==imgHeight-2)
			//			{
			//				printf("r: %f, g: %f, b: %f", r,g,b);
			//			}
			if (maxr > 255)
			{
				r = round(r * (255 / (maxr + abs(minr))));
				//printf("minr: %f ming: %f minb: %f maxr: %f maxg: %f maxb: %f", minr, ming, minb, maxr, maxg, maxb);
			}
			if (maxg > 255)
			{
				g = round(g * (255 / (maxg + abs(ming))));
				//printf("minr: %f ming: %f minb: %f maxr: %f maxg: %f maxb: %f", minr, ming, minb, maxr, maxg, maxb);
			}
			if (maxb > 255)
			{
				b = round(b * (255 / (maxb + abs(minb))));
				//printf("minr: %f ming: %f minb: %f maxr: %f maxg: %f maxb: %f", minr, ming, minb, maxr, maxg, maxb);
			}
			if(CheckIfInROI(i,j))
			{
				loadedImage.SetRGB(i, j, r, g, b);
			}
		}
	}

	
	printf(" Finished convolution function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}


//My Function ---> To be modified!
void MyFrame::OnMyFunctionImage(wxCommandEvent & WXUNUSED(event)) {

	printf("My function...");
	wxImage tmpImage = loadedImage.Copy();

	wxTextEntryDialog *openTextEntryDialog = new wxTextEntryDialog ( this, _T("shift value"), _T(""), _T(""),wxTextEntryDialogStyle , wxDefaultPosition);
	if(openTextEntryDialog->ShowModal() == wxID_OK){
		wxString value = openTextEntryDialog->GetValue();
		double shift;
		if(!value.ToDouble(&shift)){ printf("error!");}

		double r,g,b;

		for(int i=0; i< loadedImage.GetWidth(); i++) {
			for(int j=0;j< loadedImage.GetHeight(); j++){
				// GET THE RGB VALUES
				r = tmpImage.GetRed(i,j) + round(shift);   // red pixel value
				g = tmpImage.GetGreen(i,j) + round(shift); // green pixel value
				b = tmpImage.GetBlue(i,j) + round(shift); // blue pixel value

				//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

				r = r<255 ? r:255;
				g = g<255 ? g:255;
				b = b<255 ? b:255;
				r = r>0 ? r:0;
				g = g>0 ? g:0;
				b = b>0 ? b:0;



				// SAVE THE RGB VALUES
				if(CheckIfInROI(i,j))
				{
					loadedImage.SetRGB(i, j, r, g, b);
				}
			}
		}
	}

	printf(" Finished My function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}


//lab6 methods
void MyFrame::OnAddNoiseImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Adding Noise...\n");
	wxImage tmpImage = loadedImage.Copy();

	int locationx, locationy;

	for(int i = 0; i< 1000; i++)
	{
		locationx = rand() % loadedImage.GetWidth();
		locationy = rand() % loadedImage.GetHeight();
		if(CheckIfInROI(locationx,locationy))
		{
			if(i%2 == 0)
			{
				loadedImage.SetRGB(locationx,locationy, 0,0,0);
			}
			else
			{
				loadedImage.SetRGB(locationx,locationy,255,255,255);
			}
		}
	}

	//	unsigned char r,g,b;
	//
	//	for(int i=0; i< loadedImage.GetWidth(); i++) {
	//		for(int j=0;j< loadedImage.GetHeight(); j++){
	//			// GET THE RGB VALUES
	//			r = tmpImage.GetRed(i,j);   // red pixel value
	//			g = tmpImage.GetGreen(i,j); // green pixel value
	//			b = tmpImage.GetBlue(i,j); // blue pixel value
	//
	//			//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);
	//
	//			// SAVE THE RGB VALUES
	//			loadedImage.SetRGB(i, j, r, g, b);
	//		}
	//	}

	//printf(" Finished My function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMinFilterImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Min function...");
	wxImage tmpImage = loadedImage.Copy();

	//unsigned char r,g,b;

	for(int i=1; i< loadedImage.GetWidth()-1; i++) {
		for(int j=1;j< loadedImage.GetHeight()-1; j++){
			// GET THE RGB VALUES



			//			r = tmpImage.GetRed(i,j);   // red pixel value
			//			g = tmpImage.GetGreen(i,j); // green pixel value
			//			b = tmpImage.GetBlue(i,j); // blue pixel value

			int redVals[] = { tmpImage.GetRed(i-1,j-1), tmpImage.GetRed(i-1,j), tmpImage.GetRed(i-1,j+1), tmpImage.GetRed(i,j-1), tmpImage.GetRed(i,j), tmpImage.GetRed(i,j+1) ,tmpImage.GetRed(i+1,j-1) ,tmpImage.GetRed(i+1,j) ,tmpImage.GetRed(i+1,j+1)};
			int greenVals[] = { tmpImage.GetGreen(i-1,j-1), tmpImage.GetGreen(i-1,j), tmpImage.GetGreen(i-1,j+1), tmpImage.GetGreen(i,j-1), tmpImage.GetGreen(i,j), tmpImage.GetGreen(i,j+1) ,tmpImage.GetGreen(i+1,j-1) ,tmpImage.GetGreen(i+1,j) ,tmpImage.GetGreen(i+1,j+1)};
			int blueVals[] = { tmpImage.GetBlue(i-1,j-1), tmpImage.GetBlue(i-1,j), tmpImage.GetBlue(i-1,j+1), tmpImage.GetBlue(i,j-1), tmpImage.GetBlue(i,j), tmpImage.GetBlue(i,j+1) ,tmpImage.GetBlue(i+1,j-1) ,tmpImage.GetBlue(i+1,j) ,tmpImage.GetBlue(i+1,j+1)};






			//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

			// SAVE THE RGB VALUES
			if(CheckIfInROI(i,j))
			{
				loadedImage.SetRGB(i, j, *std::min_element(redVals, redVals+9), *std::min_element(greenVals, greenVals+9), *std::min_element(blueVals, blueVals+9));
			}
		}
	}

	//printf(" Finished My function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMaxFilterImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Max function...");
	wxImage tmpImage = loadedImage.Copy();

	//unsigned char r,g,b;

	for(int i=1; i< loadedImage.GetWidth()-1; i++) {
		for(int j=1;j< loadedImage.GetHeight()-1; j++){
			// GET THE RGB VALUES



			//			r = tmpImage.GetRed(i,j);   // red pixel value
			//			g = tmpImage.GetGreen(i,j); // green pixel value
			//			b = tmpImage.GetBlue(i,j); // blue pixel value

			int redVals[] = { tmpImage.GetRed(i-1,j-1), tmpImage.GetRed(i-1,j), tmpImage.GetRed(i-1,j+1), tmpImage.GetRed(i,j-1), tmpImage.GetRed(i,j), tmpImage.GetRed(i,j+1) ,tmpImage.GetRed(i+1,j-1) ,tmpImage.GetRed(i+1,j) ,tmpImage.GetRed(i+1,j+1)};
			int greenVals[] = { tmpImage.GetGreen(i-1,j-1), tmpImage.GetGreen(i-1,j), tmpImage.GetGreen(i-1,j+1), tmpImage.GetGreen(i,j-1), tmpImage.GetGreen(i,j), tmpImage.GetGreen(i,j+1) ,tmpImage.GetGreen(i+1,j-1) ,tmpImage.GetGreen(i+1,j) ,tmpImage.GetGreen(i+1,j+1)};
			int blueVals[] = { tmpImage.GetBlue(i-1,j-1), tmpImage.GetBlue(i-1,j), tmpImage.GetBlue(i-1,j+1), tmpImage.GetBlue(i,j-1), tmpImage.GetBlue(i,j), tmpImage.GetBlue(i,j+1) ,tmpImage.GetBlue(i+1,j-1) ,tmpImage.GetBlue(i+1,j) ,tmpImage.GetBlue(i+1,j+1)};






			//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

			// SAVE THE RGB VALUES
			if(CheckIfInROI(i,j))
			{
				loadedImage.SetRGB(i, j, *std::max_element(redVals, redVals+9), *std::max_element(greenVals, greenVals+9), *std::max_element(blueVals, blueVals+9));
			}
		}
	}

	//printf(" Finished My function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMidFilterImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Mid function...");
	wxImage tmpImage = loadedImage.Copy();

	//unsigned char r,g,b;

	for(int i=1; i< loadedImage.GetWidth()-1; i++) {
		for(int j=1;j< loadedImage.GetHeight()-1; j++){
			// GET THE RGB VALUES



			//			r = tmpImage.GetRed(i,j);   // red pixel value
			//			g = tmpImage.GetGreen(i,j); // green pixel value
			//			b = tmpImage.GetBlue(i,j); // blue pixel value

			int redVals[] = { tmpImage.GetRed(i-1,j-1), tmpImage.GetRed(i-1,j), tmpImage.GetRed(i-1,j+1), tmpImage.GetRed(i,j-1), tmpImage.GetRed(i,j), tmpImage.GetRed(i,j+1) ,tmpImage.GetRed(i+1,j-1) ,tmpImage.GetRed(i+1,j) ,tmpImage.GetRed(i+1,j+1)};
			int greenVals[] = { tmpImage.GetGreen(i-1,j-1), tmpImage.GetGreen(i-1,j), tmpImage.GetGreen(i-1,j+1), tmpImage.GetGreen(i,j-1), tmpImage.GetGreen(i,j), tmpImage.GetGreen(i,j+1) ,tmpImage.GetGreen(i+1,j-1) ,tmpImage.GetGreen(i+1,j) ,tmpImage.GetGreen(i+1,j+1)};
			int blueVals[] = { tmpImage.GetBlue(i-1,j-1), tmpImage.GetBlue(i-1,j), tmpImage.GetBlue(i-1,j+1), tmpImage.GetBlue(i,j-1), tmpImage.GetBlue(i,j), tmpImage.GetBlue(i,j+1) ,tmpImage.GetBlue(i+1,j-1) ,tmpImage.GetBlue(i+1,j) ,tmpImage.GetBlue(i+1,j+1)};






			//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

			// SAVE THE RGB VALUES
			if(CheckIfInROI(i,j))
			{
				loadedImage.SetRGB(i, j, round((*std::max_element(redVals, redVals+9)+*std::min_element(redVals, redVals+9))/2),
						round((*std::max_element(greenVals, greenVals+9)+*std::min_element(greenVals, greenVals+9))/2),
						round((*std::max_element(blueVals, blueVals+9)+*std::min_element(blueVals, blueVals+9))/2));
			}
		}
	}

	//printf(" Finished My function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}



//lab7

void MyFrame::OnNegLinearTransformImage(wxCommandEvent & WXUNUSED(event))
{
	printf("Negative Linear Transform function...");
	wxImage tmpImage = loadedImage.Copy();

	unsigned char r,g,b;

	for(int i=0; i< loadedImage.GetWidth(); i++) {
		for(int j=0;j< loadedImage.GetHeight(); j++){
			// GET THE RGB VALUES
			r = tmpImage.GetRed(i,j);   // red pixel value
			g = tmpImage.GetGreen(i,j); // green pixel value
			b = tmpImage.GetBlue(i,j); // blue pixel value

			//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

			// SAVE THE RGB VALUES
			if(CheckIfInROI(i,j))
			{
				loadedImage.SetRGB(i, j, 255-r, 255-g, 255-b);
			}
		}
	}

	printf(" Finished Linear Transform function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnLogTransformImage(wxCommandEvent & WXUNUSED(event))
{
	wxTextEntryDialog dlg(this, _T("Choose constant value"), _T(""), _T(""), wxTextEntryDialogStyle, wxDefaultPosition);
	//dlg.SetTextValidator(wxFILTER_ALPHA);
	if ( dlg.ShowModal() == wxID_OK )
	{
		// We can be certain that this string contains letters only.
		wxString value = dlg.GetValue();
		double valueDouble;
		if(!value.ToDouble(&valueDouble)){ printf("error!");}
		printf("valueDouble: %f \n", valueDouble);
		printf("Log Transform function...");
		wxImage tmpImage = loadedImage.Copy();

		unsigned char r,g,b;

		for(int i=0; i< loadedImage.GetWidth(); i++) {
			for(int j=0;j< loadedImage.GetHeight(); j++){
				// GET THE RGB VALUES
				r = tmpImage.GetRed(i,j);   // red pixel value
				g = tmpImage.GetGreen(i,j); // green pixel value
				b = tmpImage.GetBlue(i,j); // blue pixel value

				//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

				// SAVE THE RGB VALUES
				if(CheckIfInROI(i,j))
				{
					loadedImage.SetRGB(i, j, valueDouble*log(1+r), valueDouble*log(1+g), valueDouble*log(1+b));
				}
			}
		}

		printf(" Finished Log function.\n");
		Refresh();
		UndoImages.push_back(loadedImage);

	}

}

void MyFrame::OnPowerTransformImage(wxCommandEvent & WXUNUSED(event))
{
	wxTextEntryDialog dlg(this, _T("Choose Constant value"), _T(""), _T(""), wxTextEntryDialogStyle, wxDefaultPosition);
	wxTextEntryDialog dlg1(this, _T("Choose Power value"), _T(""), _T(""), wxTextEntryDialogStyle, wxDefaultPosition);

	if ( dlg.ShowModal() == wxID_OK )
	{
		if(dlg1.ShowModal() == wxID_OK)
		{
			// We can be certain that this string contains letters only.
			wxString value = dlg.GetValue();
			wxString power = dlg1.GetValue();
			double valueDouble, powerDouble;
			if(!value.ToDouble(&valueDouble)){ printf("error!");}
			if(!power.ToDouble(&powerDouble)){ printf("error!");}
			//printf("valueDouble: %f \n", valueDouble);
			printf("Log Transform function...");
			wxImage tmpImage = loadedImage.Copy();

			unsigned char r,g,b;

			for(int i=0; i< loadedImage.GetWidth(); i++) {
				for(int j=0;j< loadedImage.GetHeight(); j++){
					// GET THE RGB VALUES
					r = tmpImage.GetRed(i,j);   // red pixel value
					g = tmpImage.GetGreen(i,j); // green pixel value
					b = tmpImage.GetBlue(i,j); // blue pixel value

					//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

					// SAVE THE RGB VALUES
					if(CheckIfInROI(i,j))
					{
						loadedImage.SetRGB(i, j, valueDouble*pow(r,powerDouble), valueDouble*pow(g,powerDouble), valueDouble*pow(b,powerDouble));
					}

				}
			}

			printf(" Finished Log function.\n");
			Refresh();
			UndoImages.push_back(loadedImage);
		}

	}
}

void MyFrame::OnRandLookupTableTransformImage(wxCommandEvent & WXUNUSED(event))
{
	int r,g,b;
	int RandomTable [256];
	for(int i = 0; i< 256; i++)
	{
		RandomTable[i] = rand() % 255;
	}

				for(int i=0; i< loadedImage.GetWidth(); i++) {
					for(int j=0;j< loadedImage.GetHeight(); j++){
						// GET THE RGB VALUES
						r = loadedImage.GetRed(i,j);   // red pixel value
						g = loadedImage.GetGreen(i,j); // green pixel value
						b = loadedImage.GetBlue(i,j); // blue pixel value

						//printf("(%d,%d) [r = %x  | g = %x | b = %x] \n",i,j,r,g,b);

						// SAVE THE RGB VALUES
						if(CheckIfInROI(i,j))
						{
							loadedImage.SetRGB(i, j, RandomTable[r], RandomTable[g], RandomTable[b]);
						}

					}
				}

				printf(" Finished Log function.\n");
				Refresh();
				UndoImages.push_back(loadedImage);
	
}





//ROI selection code
void MyFrame::OnLeftDown( wxMouseEvent& event )
{
	selecting = true;
	wxClientDC dc_canvas(this);
	//wxDCOverlay( m_overlay, &dc_canvas ).Clear();
	m_overlay.Reset();

	m_currentpoint = wxPoint( event.GetX() , event.GetY() );

	// CancelSelections();
	this->CaptureMouse();
}

void MyFrame::OnMotion( wxMouseEvent& event )
{
	if(selecting)
	{
		int x = event.GetX();
		int y = event.GetY();


		//draw a selection rectangle on the overlay
		wxClientDC dc_canvas(this);

		wxDCOverlay( m_overlay, &dc_canvas ).Clear();
		dc_canvas.SetPen( wxPen( *wxLIGHT_GREY, 2 ) );
		dc_canvas.SetBrush( *wxTRANSPARENT_BRUSH );
		dc_canvas.DrawRectangle( wxRect( m_currentpoint , wxPoint(x,y) ) );
	}


}

void MyFrame::OnLeftUp( wxMouseEvent& event )
{


	//housekeeping
	// m_rubberBand = false;
	this->ReleaseMouse();
	selecting = false;
	//process the selection: run through the segments and redraw any that
	//lie within the selection rectangle
	//wxRect selection( m_currentpoint , wxPoint( event.GetX() , event.GetY() ) ) ;

	ROI[0] = m_currentpoint.x<event.GetX() ? m_currentpoint.x : event.GetX();
	ROI[1] = m_currentpoint.y<event.GetY() ? m_currentpoint.y : event.GetY();
	ROI[2] = m_currentpoint.x>event.GetX() ? m_currentpoint.x : event.GetX();
	ROI[3] = m_currentpoint.y>event.GetY() ? m_currentpoint.y : event.GetY();





}

bool MyFrame::CheckIfInROI(int x, int y)
{
	//printf("%d",(ROI[0] <= x && x <= ROI[2] && ROI[1] <= y && y <= ROI[3]));
	return (ROI[0] <= x && x <= ROI[2] && ROI[1] <= y && y <= ROI[3]);

}



//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE BELOW--------------------//
//-----------------------------------------------------------//
//###########################################################//

//RESET IMAGE
void MyFrame::OnResetImage(wxCommandEvent & WXUNUSED(event)) {

	printf("Reseting image...");
	if (loadedImage.Ok()) {
		loadedImage.Destroy();
	}

	if (origImage.Ok()) {
		loadedImage = origImage.Copy();
		Refresh();
	}

	printf("Finished Reseting.\n");

}

//IMAGE SAVING
void MyFrame::OnSaveImage(wxCommandEvent & WXUNUSED(event)) {

	printf("Saving image...");

	loadedImage.SaveFile(wxT("Saved_Image.bmp"), wxBITMAP_TYPE_BMP);

	printf("Finished Saving.\n");
}

void MyFrame::OnExit (wxCommandEvent & WXUNUSED(event)) {

	Close(TRUE);
}

void MyFrame::OnPaint(wxPaintEvent & WXUNUSED(event)) {

	wxAutoBufferedPaintDC dc(this);
	if(loadedImage.Ok()) {
		dc.DrawBitmap(wxBitmap(loadedImage), 0, 0, false);//given bitmap xcoord y coord and transparency
		ROI[0] = 0;
		ROI[1] = 0;
		ROI[2] = loadedImage.GetWidth();
		ROI[3] = loadedImage.GetHeight();
		
	}
	
}


void MyFrame::Undo(wxCommandEvent & WXUNUSED(event))
{
	if(UndoImages.size() >0)
	{
	printf("Undo image...");
		if (loadedImage.Ok()) {
			loadedImage.Destroy();
		}

		if (origImage.Ok()) {
			
			loadedImage = UndoImages.back().Copy();
			Refresh();
		}

		printf("Finished Undo.");
	
	
	
	
	
	printf(" size of undo stack: %lu \n", UndoImages.size());
	UndoImages.pop_back();
	}
	else
	{
		printf("can't undo\n");
	}
	
}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
EVT_MENU ( LOAD_FILE_ID,  MyFrame::OnOpenFile)
EVT_MENU ( EXIT_ID,  MyFrame::OnExit)
EVT_MENU ( UNDO_ID, MyFrame::Undo)

//###########################################################//
//----------------------START MY EVENTS ---------------------//
//###########################################################//

EVT_MENU ( RESET_IMAGE_ID,  MyFrame::OnResetImage)
EVT_MENU ( INVERT_IMAGE_ID,  MyFrame::OnInvertImage)
EVT_MENU ( SCALE_IMAGE_ID,  MyFrame::OnScaleImage)
EVT_MENU ( SAVE_IMAGE_ID,  MyFrame::OnSaveImage)
EVT_MENU ( CONVOLUTE_IMAGE_ID, MyFrame::OnConvoluteImage)
EVT_MENU ( MY_IMAGE_ID,  MyFrame::OnMyFunctionImage)//--->To be modified!

//lab6
EVT_MENU ( NOISE_IMAGE_ID, MyFrame::OnAddNoiseImage)
EVT_MENU ( MIN_FILTER_IMAGE_ID, MyFrame::OnMinFilterImage)
EVT_MENU ( MAX_FILTER_IMAGE_ID, MyFrame::OnMaxFilterImage)
EVT_MENU ( MID_FILTER_IMAGE_ID, MyFrame::OnMidFilterImage)

//lab7
EVT_MENU ( NEG_LINEAR_TRANSFORM_IMAGE_ID, MyFrame::OnNegLinearTransformImage)
EVT_MENU ( LOG_TRANSFORM_IMAGE_ID, MyFrame::OnLogTransformImage)
EVT_MENU ( POWER_TRANSFORM_IMAGE_ID, MyFrame::OnPowerTransformImage)
EVT_MENU ( RAND_LOOKUP_TABLE_TRANSFORM_ID, MyFrame::OnRandLookupTableTransformImage)

////ROI
//EVT_MENU ( LEFT_DOWN_ID, MyFrame::OnLeftDown)
//EVT_MENU ( LEFT_UP_ID, MyFrame::OnLeftUp)
//EVT_MENU ( MOTION_ID, MyFrame::OnMotion)

//###########################################################//
//----------------------END MY EVENTS -----------------------//
//###########################################################//

EVT_PAINT (MyFrame::OnPaint)
END_EVENT_TABLE()
