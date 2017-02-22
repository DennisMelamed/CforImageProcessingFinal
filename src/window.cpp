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

IMPLEMENT_APP (BasicApplication)

bool BasicApplication::OnInit() {
	wxInitAllImageHandlers();
	MyFrame *frame = new MyFrame(_("Basic Frame"), 50, 50, 800, 600);
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width,
		int height) :
		wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos),
				wxSize(width, height)) {

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
	lab7Menu->Append(RAND_LOOKUP_TABLE_TRANSFORM_ID, _T("&Random Lookup"));

	//MENU FOR LAB 8
	lab8Menu = new wxMenu;
	lab8Menu->Append(HISTOGRAM_EQUILIZATION_ID, _T("&Histogram Equilization"));

	//MENU FOR LAB 9
	lab9Menu = new wxMenu;
	lab9Menu->Append(SIMPLE_THRESHOLD_ID, _T("&Simple Threshold"));
	lab9Menu->Append(AUTOMATED_THRESHOLD_ID, _T("&Automated Threshold"));

	menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(lab6Menu, _T("&Order Statistic Filtering"));
	menuBar->Append(lab7Menu, _T("&Point Processing"));
	menuBar->Append(lab8Menu, _T("Histogram Functions"));
	menuBar->Append(lab9Menu, _T("Threshold Functions"));

	SetMenuBar (menuBar);
	CreateStatusBar(3);

	SetBackgroundStyle (wxBG_STYLE_PAINT);

	//this = new wxPanel( this );
	this->Bind(wxEVT_LEFT_DOWN, &MyFrame::OnLeftDown, this);
	this->Bind(wxEVT_LEFT_UP, &MyFrame::OnLeftUp, this);
	this->Bind(wxEVT_MOTION, &MyFrame::OnMotion, this);
	selecting = false;
	ROI[0] = 0;
	ROI[1] = 0;
	ROI[2] = 0;
	ROI[3] = 0;

}

MyFrame::~MyFrame() {

	/* release resources */
	if (loadedImage.Ok()) {
		loadedImage.Destroy();
	}

	if (origImage.Ok()) {
		origImage.Destroy();
	}

}

void MyFrame::OnOpenFile(wxCommandEvent & WXUNUSED(event)) {

	wxFileDialog openFileDialog(this, _T("Open file"), _T(""), _T(""),
			FILETYPES, wxFD_OPEN, wxDefaultPosition);

	if (openFileDialog.ShowModal() == wxID_OK) {
		wxString filename = openFileDialog.GetFilename();
		wxString path = openFileDialog.GetPath();
		printf("Loading image from file...");

		if (loadedImage.Ok()) {
			loadedImage.Destroy();
		}

		if (origImage.Ok()) {
			origImage.Destroy();
		}
		UndoImages.clear();

		loadedImage = wxImage(path); //Image Loaded form file

		if (loadedImage.Ok()) {
			origImage = loadedImage.Copy();
			UndoImages.push_back(origImage);

			printf("Done! \n");
		} else {
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

	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			{
				loadedImage.SetRGB(i, j, 255 - tmpImage.GetRed(i, j),
						255 - tmpImage.GetGreen(i, j),
						255 - tmpImage.GetBlue(i, j));
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
	wxTextEntryDialog *openTextEntryDialog = new wxTextEntryDialog(this,
			_T("scale value"), _T(""), _T(""), wxTextEntryDialogStyle,
			wxDefaultPosition);
	if (openTextEntryDialog->ShowModal() == wxID_OK) {
		wxString value = openTextEntryDialog->GetValue();
		double factor;
		if (!value.ToDouble(&factor)) {
			printf("error!");
		}


		wxImage tmpImage = loadedImage.Copy();

		double r, g, b;


		std::vector < std::vector<double>
		> rvec(loadedImage.GetWidth(),
				std::vector<double>(loadedImage.GetHeight(), 0));
		std::vector < std::vector<double>
		> gvec(loadedImage.GetWidth(),
				std::vector<double>(loadedImage.GetHeight(), 0));
		std::vector < std::vector<double>
		> bvec(loadedImage.GetWidth(),
				std::vector<double>(loadedImage.GetHeight(), 0));


		double maxr = 255, maxg = 255, maxb = 255;
		double minr = 0, ming = 0, minb = 0;
		for (int i = 0; i < loadedImage.GetWidth(); i++)
			for (int j = 0; j < loadedImage.GetHeight(); j++) {
				r = tmpImage.GetRed(i, j);
				r = round(r * factor);
				if (maxr < r) {
					maxr = r;
				}
				if (minr > r) {
					minr = r;
				}

				g = tmpImage.GetGreen(i, j);
				g = round(g * factor);
				if (maxg < g) {
					maxg = g;
				}
				if (ming > g) {
					ming = g;
				}

				b = tmpImage.GetBlue(i, j);
				b = round(b * factor);
				if (maxb < b) {
					maxb = b;
				}
				if (minb > b) {
					minb = b;
				}

				//loadedImage->SetRGB(i,j,r,g,b);
				rvec[i][j] = r;
				gvec[i][j] = g;
				bvec[i][j] = b;
			}

		for (int i = 0; i < loadedImage.GetWidth(); i++) {
			for (int j = 0; j < loadedImage.GetHeight(); j++) {
				r = rvec[i][j];
				g = gvec[i][j];
				b = bvec[i][j];
				if (maxr > 255) {
					r = round(r * (255 / maxr));
				}
				if (maxg > 255) {
					g = round(g * (255 / maxg));
				}
				if (maxb > 255) {
					b = round(b * (255 / maxb));
				}
				loadedImage.SetRGB(i, j, r, g, b);
			}
		}

		printf(" Finished scaling.\n");
		Refresh();
		UndoImages.push_back(loadedImage);
	}
}

void MyFrame::OnConvoluteImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Convolute function...");

	wxImage tmpImage = loadedImage.Copy();



	double r, g, b;
	int decision = 0;
	int imgWidth = loadedImage.GetWidth();
	int imgHeight = loadedImage.GetHeight();

	wxArrayString choices;
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
	std::vector < std::vector<double> > mask(3, std::vector<double>(3, 0));

	wxSingleChoiceDialog openSingleChoiceDialog(this,
			_T("Select Mask for Convolution"), _T(""), choices, NULL,
			wxCHOICEDLG_STYLE, wxDefaultPosition);

	if (openSingleChoiceDialog.ShowModal() == wxID_OK) {
		decision = openSingleChoiceDialog.GetSelection();
		switch (decision) {
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

	std::vector < std::vector<double>
	> rvec(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double>
	> gvec(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double>
	> bvec(imgWidth, std::vector<double>(imgHeight, 0));

	//only for bi-directional edge detect
	std::vector < std::vector<double>
	> rvecy(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double>
	> gvecy(imgWidth, std::vector<double>(imgHeight, 0));
	std::vector < std::vector<double>
	> bvecy(imgWidth, std::vector<double>(imgHeight, 0));

	double maxr = 255, maxg = 255, maxb = 255;
	double minr = 0, ming = 0, minb = 0;
	for (int i = 1; i < imgWidth - 1; i++)
		for (int j = 1; j < imgHeight - 1; j++) {
			r = 0;
			b = 0;
			g = 0;


			for (int a = -1; a < 2; a++) {
				for (int c = -1; c < 2; c++) {

					r += ((loadedImage.GetRed(i - c, j - a))
							* mask[c + 1][a + 1]);
					
					g += ((loadedImage.GetGreen(i - c, j - a))
							* mask[c + 1][a + 1]);
					b += ((loadedImage.GetBlue(i - c, j - a))
							* mask[c + 1][a + 1]);
				}
			}

			if (maxr < r) {
				maxr = r;
			}
			if (minr > r) {
				minr = r;
			}

			if (maxg < g) {
				maxg = g;
			}
			if (ming > g) {
				ming = g;
			}

			if (maxb < b) {
				maxb = b;

			}
			if (minb > b) {
				minb = b;

			}

			//loadedImage->SetRGB(i,j,r,g,b);
			rvec[i][j] = r;
			gvec[i][j] = g;
			bvec[i][j] = b;
		}
	if (decision == 10) {
		mask[0][0] = -1;
		mask[0][1] = -2;
		mask[0][2] = -1;
		mask[1][0] = 0;
		mask[1][1] = 0;
		mask[1][2] = 0;
		mask[2][0] = 1;
		mask[2][1] = 2;
		mask[2][2] = 1;

		for (int i = 1; i < imgWidth - 1; i++)
			for (int j = 1; j < imgHeight - 1; j++) {
				r = 0;
				b = 0;
				g = 0;


				for (int a = -1; a < 2; a++) {
					for (int c = -1; c < 2; c++) {

						r += ((loadedImage.GetRed(i - c, j - a))
								* mask[c + 1][a + 1]);
						g += ((loadedImage.GetGreen(i - c, j - a))
								* mask[c + 1][a + 1]);
						b += ((loadedImage.GetBlue(i - c, j - a))
								* mask[c + 1][a + 1]);
					}
				}

				if (maxr < r + rvec[i][j]) {
					maxr = r;
				}
				if (minr > r + rvec[i][j]) {
					minr = r;
				}

				if (maxg < g + gvec[i][j]) {
					maxg = g;
				}
				if (ming > g + gvec[i][j]) {
					ming = g;
				}

				if (maxb < b + bvec[i][j]) {
					maxb = b;

				}
				if (minb > b + bvec[i][j]) {
					minb = b;

				}


				rvecy[i][j] = r;
				gvecy[i][j] = g;
				bvecy[i][j] = b;

				rvec[i][j] += rvecy[i][j];
				gvec[i][j] += gvecy[i][j];
				bvec[i][j] += bvecy[i][j];

			}
	}

	for (int i = 1; i < imgWidth - 1; i++) {
		for (int j = 1; j < imgHeight - 1; j++) {

			r = rvec[i][j] + abs(minr);
			g = gvec[i][j] + abs(ming);

			if (maxr > 255) {
				r = round(r * (255 / (maxr + abs(minr))));

			}
			if (maxg > 255) {
				g = round(g * (255 / (maxg + abs(ming))));

			}
			if (maxb > 255) {
				b = round(b * (255 / (maxb + abs(minb))));

			}
			if (CheckIfInROI(i, j)) {
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

	printf("Shifting Function");
	wxImage tmpImage = loadedImage.Copy();

	wxTextEntryDialog *openTextEntryDialog = new wxTextEntryDialog(this,
			_T("shift value"), _T(""), _T(""), wxTextEntryDialogStyle,
			wxDefaultPosition);
	if (openTextEntryDialog->ShowModal() == wxID_OK) {
		wxString value = openTextEntryDialog->GetValue();
		double shift;
		if (!value.ToDouble(&shift)) {
			printf("error!");
		}

		double r, g, b;

		for (int i = 0; i < loadedImage.GetWidth(); i++) {
			for (int j = 0; j < loadedImage.GetHeight(); j++) {
				// GET THE RGB VALUES
				r = tmpImage.GetRed(i, j) + round(shift); // red pixel value
				g = tmpImage.GetGreen(i, j) + round(shift); // green pixel value
				b = tmpImage.GetBlue(i, j) + round(shift); // blue pixel value



				r = r < 255 ? r : 255;
				g = g < 255 ? g : 255;
				b = b < 255 ? b : 255;
				r = r > 0 ? r : 0;
				g = g > 0 ? g : 0;
				b = b > 0 ? b : 0;

				// SAVE THE RGB VALUES
				if (CheckIfInROI(i, j)) {
					loadedImage.SetRGB(i, j, r, g, b);
				}
			}
		}
	}

	printf(" Finished shifting function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

//lab6 methods
void MyFrame::OnAddNoiseImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Adding Noise...\n");
	wxImage tmpImage = loadedImage.Copy();

	int locationx, locationy;

	for (int i = 0; i < 1000; i++) {
		locationx = rand() % loadedImage.GetWidth();
		locationy = rand() % loadedImage.GetHeight();
		if (CheckIfInROI(locationx, locationy)) {
			if (i % 2 == 0) {
				loadedImage.SetRGB(locationx, locationy, 0, 0, 0);
			} else {
				loadedImage.SetRGB(locationx, locationy, 255, 255, 255);
			}
		}
	}


	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMinFilterImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Min function...");
	wxImage tmpImage = loadedImage.Copy();



	for (int i = 1; i < loadedImage.GetWidth() - 1; i++) {
		for (int j = 1; j < loadedImage.GetHeight() - 1; j++) {


			int redVals[] = { tmpImage.GetRed(i - 1, j - 1), tmpImage.GetRed(
					i - 1, j), tmpImage.GetRed(i - 1, j + 1), tmpImage.GetRed(i,
							j - 1), tmpImage.GetRed(i, j), tmpImage.GetRed(i, j + 1),
							tmpImage.GetRed(i + 1, j - 1), tmpImage.GetRed(i + 1, j),
							tmpImage.GetRed(i + 1, j + 1) };
			int greenVals[] = { tmpImage.GetGreen(i - 1, j - 1),
					tmpImage.GetGreen(i - 1, j), tmpImage.GetGreen(i - 1,
							j + 1), tmpImage.GetGreen(i, j - 1),
							tmpImage.GetGreen(i, j), tmpImage.GetGreen(i, j + 1),
							tmpImage.GetGreen(i + 1, j - 1), tmpImage.GetGreen(i + 1,
									j), tmpImage.GetGreen(i + 1, j + 1) };
			int blueVals[] = { tmpImage.GetBlue(i - 1, j - 1), tmpImage.GetBlue(
					i - 1, j), tmpImage.GetBlue(i - 1, j + 1), tmpImage.GetBlue(
							i, j - 1), tmpImage.GetBlue(i, j), tmpImage.GetBlue(i,
									j + 1), tmpImage.GetBlue(i + 1, j - 1), tmpImage.GetBlue(
											i + 1, j), tmpImage.GetBlue(i + 1, j + 1) };



			// SAVE THE RGB VALUES
			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j,
						*std::min_element(redVals, redVals + 9),
						*std::min_element(greenVals, greenVals + 9),
						*std::min_element(blueVals, blueVals + 9));
			}
		}
	}


	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMaxFilterImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Max function...");
	wxImage tmpImage = loadedImage.Copy();



	for (int i = 1; i < loadedImage.GetWidth() - 1; i++) {
		for (int j = 1; j < loadedImage.GetHeight() - 1; j++) {


			int redVals[] = { tmpImage.GetRed(i - 1, j - 1), tmpImage.GetRed(
					i - 1, j), tmpImage.GetRed(i - 1, j + 1), tmpImage.GetRed(i,
							j - 1), tmpImage.GetRed(i, j), tmpImage.GetRed(i, j + 1),
							tmpImage.GetRed(i + 1, j - 1), tmpImage.GetRed(i + 1, j),
							tmpImage.GetRed(i + 1, j + 1) };
			int greenVals[] = { tmpImage.GetGreen(i - 1, j - 1),
					tmpImage.GetGreen(i - 1, j), tmpImage.GetGreen(i - 1,
							j + 1), tmpImage.GetGreen(i, j - 1),
							tmpImage.GetGreen(i, j), tmpImage.GetGreen(i, j + 1),
							tmpImage.GetGreen(i + 1, j - 1), tmpImage.GetGreen(i + 1,
									j), tmpImage.GetGreen(i + 1, j + 1) };
			int blueVals[] = { tmpImage.GetBlue(i - 1, j - 1), tmpImage.GetBlue(
					i - 1, j), tmpImage.GetBlue(i - 1, j + 1), tmpImage.GetBlue(
							i, j - 1), tmpImage.GetBlue(i, j), tmpImage.GetBlue(i,
									j + 1), tmpImage.GetBlue(i + 1, j - 1), tmpImage.GetBlue(
											i + 1, j), tmpImage.GetBlue(i + 1, j + 1) };


			// SAVE THE RGB VALUES
			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j,
						*std::max_element(redVals, redVals + 9),
						*std::max_element(greenVals, greenVals + 9),
						*std::max_element(blueVals, blueVals + 9));
			}
		}
	}


	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnMidFilterImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Mid function...");
	wxImage tmpImage = loadedImage.Copy();



	for (int i = 1; i < loadedImage.GetWidth() - 1; i++) {
		for (int j = 1; j < loadedImage.GetHeight() - 1; j++) {


			int redVals[] = { tmpImage.GetRed(i - 1, j - 1), tmpImage.GetRed(
					i - 1, j), tmpImage.GetRed(i - 1, j + 1), tmpImage.GetRed(i,
							j - 1), tmpImage.GetRed(i, j), tmpImage.GetRed(i, j + 1),
							tmpImage.GetRed(i + 1, j - 1), tmpImage.GetRed(i + 1, j),
							tmpImage.GetRed(i + 1, j + 1) };
			int greenVals[] = { tmpImage.GetGreen(i - 1, j - 1),
					tmpImage.GetGreen(i - 1, j), tmpImage.GetGreen(i - 1,
							j + 1), tmpImage.GetGreen(i, j - 1),
							tmpImage.GetGreen(i, j), tmpImage.GetGreen(i, j + 1),
							tmpImage.GetGreen(i + 1, j - 1), tmpImage.GetGreen(i + 1,
									j), tmpImage.GetGreen(i + 1, j + 1) };
			int blueVals[] = { tmpImage.GetBlue(i - 1, j - 1), tmpImage.GetBlue(
					i - 1, j), tmpImage.GetBlue(i - 1, j + 1), tmpImage.GetBlue(
							i, j - 1), tmpImage.GetBlue(i, j), tmpImage.GetBlue(i,
									j + 1), tmpImage.GetBlue(i + 1, j - 1), tmpImage.GetBlue(
											i + 1, j), tmpImage.GetBlue(i + 1, j + 1) };

			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j,
						round(
								(*std::max_element(redVals, redVals + 9)
				+ *std::min_element(redVals,
						redVals + 9)) / 2),
						round(
								(*std::max_element(greenVals, greenVals + 9)
				+ *std::min_element(greenVals,
						greenVals + 9)) / 2),
						round(
								(*std::max_element(blueVals, blueVals + 9)
				+ *std::min_element(blueVals,
						blueVals + 9)) / 2));
			}
		}
	}


	Refresh();
	UndoImages.push_back(loadedImage);
}

//lab7

void MyFrame::OnNegLinearTransformImage(wxCommandEvent & WXUNUSED(event)) {
	printf("Negative Linear Transform function...");
	wxImage tmpImage = loadedImage.Copy();

	unsigned char r, g, b;

	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			// GET THE RGB VALUES
			r = tmpImage.GetRed(i, j); // red pixel value
			g = tmpImage.GetGreen(i, j); // green pixel value
			b = tmpImage.GetBlue(i, j); // blue pixel value


			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j, 255 - r, 255 - g, 255 - b);
			}
		}
	}

	printf(" Finished Linear Transform function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnLogTransformImage(wxCommandEvent & WXUNUSED(event)) {
	wxTextEntryDialog dlg(this, _T("Choose constant value"), _T(""), _T(""),
			wxTextEntryDialogStyle, wxDefaultPosition);

	if (dlg.ShowModal() == wxID_OK) {

		wxString value = dlg.GetValue();
		double valueDouble;
		if (!value.ToDouble(&valueDouble)) {
			printf("error!");
		}

		printf("Log Transform function...");
		wxImage tmpImage = loadedImage.Copy();

		unsigned char r, g, b;

		for (int i = 0; i < loadedImage.GetWidth(); i++) {
			for (int j = 0; j < loadedImage.GetHeight(); j++) {
				// GET THE RGB VALUES
				r = tmpImage.GetRed(i, j); // red pixel value
				g = tmpImage.GetGreen(i, j); // green pixel value
				b = tmpImage.GetBlue(i, j); // blue pixel value

				if (CheckIfInROI(i, j)) {
					loadedImage.SetRGB(i, j, valueDouble * log(1 + r),
							valueDouble * log(1 + g), valueDouble * log(1 + b));
				}
			}
		}

		printf(" Finished Log function.\n");
		Refresh();
		UndoImages.push_back(loadedImage);

	}

}

void MyFrame::OnPowerTransformImage(wxCommandEvent & WXUNUSED(event)) {
	wxTextEntryDialog dlg(this, _T("Choose Constant value"), _T(""), _T(""),
			wxTextEntryDialogStyle, wxDefaultPosition);
	wxTextEntryDialog dlg1(this, _T("Choose Power value"), _T(""), _T(""),
			wxTextEntryDialogStyle, wxDefaultPosition);

	if (dlg.ShowModal() == wxID_OK) {
		if (dlg1.ShowModal() == wxID_OK) {
			// We can be certain that this string contains letters only.
			wxString value = dlg.GetValue();
			wxString power = dlg1.GetValue();
			double valueDouble, powerDouble;
			if (!value.ToDouble(&valueDouble)) {
				printf("error!");
			}
			if (!power.ToDouble(&powerDouble)) {
				printf("error!");
			}

			printf("Log Transform function...");
			wxImage tmpImage = loadedImage.Copy();

			unsigned char r, g, b;

			for (int i = 0; i < loadedImage.GetWidth(); i++) {
				for (int j = 0; j < loadedImage.GetHeight(); j++) {
					// GET THE RGB VALUES
					r = tmpImage.GetRed(i, j); // red pixel value
					g = tmpImage.GetGreen(i, j); // green pixel value
					b = tmpImage.GetBlue(i, j); // blue pixel value



					// SAVE THE RGB VALUES
					if (CheckIfInROI(i, j)) {
						loadedImage.SetRGB(i, j,
								valueDouble * pow(r, powerDouble),
								valueDouble * pow(g, powerDouble),
								valueDouble * pow(b, powerDouble));
					}

				}
			}

			printf(" Finished Log function.\n");
			Refresh();
			UndoImages.push_back(loadedImage);
		}

	}
}

void MyFrame::OnRandLookupTableTransformImage(
		wxCommandEvent & WXUNUSED(event)) {
	int r, g, b;
	int RandomTable[256];
	for (int i = 0; i < 256; i++) {
		RandomTable[i] = rand() % 255;
	}

	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			// GET THE RGB VALUES
			r = loadedImage.GetRed(i, j); // red pixel value
			g = loadedImage.GetGreen(i, j); // green pixel value
			b = loadedImage.GetBlue(i, j); // blue pixel value


			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j, RandomTable[r], RandomTable[g],
						RandomTable[b]);
			}

		}
	}

	printf(" Finished Log function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);

}

void MyFrame::OnHistogramEquilization(wxCommandEvent & WXUNUSED(event)) {
	int r, g, b;
	double HistoTableRed[256];
	double HistoTableGreen[256];
	double HistoTableBlue[256];

	double NormalizedRedTable[256];
	double NormalizedGreenTable[256];
	double NormalizedBlueTable[256];

	for (int a = 0; a < 256; a++) {
		HistoTableRed[a] = 0;
		HistoTableGreen[a] = 0;
		HistoTableBlue[a] = 0;

	}

	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			HistoTableRed[loadedImage.GetRed(i, j)]++;
			HistoTableGreen[loadedImage.GetGreen(i, j)]++;
			HistoTableBlue[loadedImage.GetBlue(i, j)]++;

		}
	}

	for (int a = 0; a < 256; a++) {

		if (a != 0) {
			NormalizedRedTable[a] =
					floor(
							255
							* ((NormalizedRedTable[a - 1] / 255)
									+ (HistoTableRed[a]
									                 / (loadedImage.GetWidth()
									                		 * loadedImage.GetHeight()))));
			NormalizedGreenTable[a] =
					floor(
							255
							* ((NormalizedGreenTable[a - 1] / 255)
									+ (HistoTableGreen[a]
									                   / (loadedImage.GetWidth()
									                		   * loadedImage.GetHeight()))));
			NormalizedBlueTable[a] =
					floor(
							255
							* ((NormalizedBlueTable[a - 1] / 255)
									+ (HistoTableBlue[a]
									                  / (loadedImage.GetWidth()
									                		  * loadedImage.GetHeight()))));
		} else {
			NormalizedRedTable[a] = floor(
					255
					* (HistoTableRed[a]
					                 / (loadedImage.GetWidth()
					                		 * loadedImage.GetHeight())));
			NormalizedGreenTable[a] = floor(
					255
					* (HistoTableGreen[a]
					                   / (loadedImage.GetWidth()
					                		   * loadedImage.GetHeight())));
			NormalizedBlueTable[a] = floor(
					255
					* (HistoTableBlue[a]
					                  / (loadedImage.GetWidth()
					                		  * loadedImage.GetHeight())));
		}

	}

	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			// GET THE RGB VALUES
			r = loadedImage.GetRed(i, j); // red pixel value
			g = loadedImage.GetGreen(i, j); // green pixel value
			b = loadedImage.GetBlue(i, j); // blue pixel value



			// SAVE THE RGB VALUES
			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j, NormalizedRedTable[r],
						NormalizedGreenTable[g], NormalizedBlueTable[b]);
			}

		}
	}

	printf(" Finished histo function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

//lab 9
void MyFrame::OnSimpleThresholdImage(wxCommandEvent & WXUNUSED(event)) {
	int r, g, b;

	wxTextEntryDialog *openTextEntryDialog = new wxTextEntryDialog(this,
			_T("threshold value"), _T(""), _T(""), wxTextEntryDialogStyle,
			wxDefaultPosition);
	if (openTextEntryDialog->ShowModal() == wxID_OK) {
		wxString value = openTextEntryDialog->GetValue();
		double thresh;
		if (!value.ToDouble(&thresh)) {
			printf("error!");
		}
		for (int i = 0; i < loadedImage.GetWidth(); i++) {
			for (int j = 0; j < loadedImage.GetHeight(); j++) {
				// GET THE RGB VALUES
				r = loadedImage.GetRed(i, j); // red pixel value
				g = loadedImage.GetGreen(i, j); // green pixel value
				b = loadedImage.GetBlue(i, j); // blue pixel value



				// SAVE THE RGB VALUES
				if (CheckIfInROI(i, j)) {
					loadedImage.SetRGB(i, j, r < thresh ? 0 : 255,
							g < thresh ? 0 : 255, b < thresh ? 0 : 255);
				}

			}
		}
	}

	printf(" Finished Simple Threshold function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::OnAutomatedThresholdImage(wxCommandEvent & WXUNUSED(event)) {
	int r, g, b;

	double red_thresh, green_thresh, blue_thresh, old_red_thresh, old_green_thresh, old_blue_thresh;
	double too_small_difference = .1;

	double thresh [3];
	double thresh2 [3];


	FindInitialThreshold(loadedImage, thresh);
	FindInitialThreshold2(loadedImage, thresh2);


	red_thresh = (thresh[0] + thresh2[0])/2;
	green_thresh = (thresh[1] + thresh2[1])/2;
	blue_thresh = (thresh[2] + thresh2[2])/2;

	old_red_thresh = 0;
	old_green_thresh = 0;
	old_blue_thresh = 0;


	while(abs(red_thresh-old_red_thresh) > too_small_difference)
	{
		old_red_thresh = red_thresh;
		red_thresh = FindRedThreshold(loadedImage, red_thresh);
	}
	while(abs(green_thresh-old_green_thresh) > too_small_difference)
		{
			old_green_thresh = green_thresh;
			green_thresh = FindGreenThreshold(loadedImage, green_thresh);
		}
	while(abs(blue_thresh-old_blue_thresh) > too_small_difference)
		{
			old_blue_thresh = blue_thresh;
			blue_thresh = FindBlueThreshold(loadedImage, blue_thresh);
		}


	printf("Threshold values being used: %f, %f, %f\n", red_thresh, green_thresh, blue_thresh);
	for (int i = 0; i < loadedImage.GetWidth(); i++) {
		for (int j = 0; j < loadedImage.GetHeight(); j++) {
			// GET THE RGB VALUES
			r = loadedImage.GetRed(i, j); // red pixel value
			g = loadedImage.GetGreen(i, j); // green pixel value
			b = loadedImage.GetBlue(i, j); // blue pixel value



			// SAVE THE RGB VALUES
			if (CheckIfInROI(i, j)) {
				loadedImage.SetRGB(i, j, r < red_thresh ? 0 : 255, g < green_thresh ? 0 : 255, b < blue_thresh ? 0 : 255);
			}

		}
	}



	printf(" Finished Automated Threshold function.\n");
	Refresh();
	UndoImages.push_back(loadedImage);
}

void MyFrame::FindInitialThreshold(wxImage loadedImage, double *thresh)
{
	int red_sum = loadedImage.GetRed(0,0) + 
			loadedImage.GetRed(0,loadedImage.GetHeight()-1) + 
			loadedImage.GetRed(loadedImage.GetWidth()-1,0) + 
			loadedImage.GetRed(loadedImage.GetWidth()-1,loadedImage.GetHeight()-1);
	
	int green_sum = loadedImage.GetGreen(0,0) + 
					loadedImage.GetGreen(0,loadedImage.GetHeight()-1) + 
					loadedImage.GetGreen(loadedImage.GetWidth()-1,0) + 
					loadedImage.GetGreen(loadedImage.GetWidth()-1,loadedImage.GetHeight()-1);
	
	
	int blue_sum = loadedImage.GetBlue(0,0) + 
			loadedImage.GetBlue(0,loadedImage.GetHeight()-1) + 
			loadedImage.GetBlue(loadedImage.GetWidth()-1,0) + 
			loadedImage.GetBlue(loadedImage.GetWidth()-1,loadedImage.GetHeight()-1);

	thresh[0] = red_sum/4;
	thresh[1] = green_sum/4;
	thresh[2] = blue_sum/4;


}

void MyFrame::FindInitialThreshold2(wxImage loadedImage, double *thresh)
{

	long unsigned int red_sum =0;
	long unsigned int green_sum = 0;
	long unsigned int blue_sum =0;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(!((i==0&&j==0) || (i==loadedImage.GetWidth() && j==0) || (i==0&&j==loadedImage.GetHeight()) || (i==loadedImage.GetWidth() && j==loadedImage.GetHeight()) ))
			{
				red_sum   += loadedImage.GetRed(i,j);
				green_sum += loadedImage.GetGreen(i,j);
				blue_sum  += loadedImage.GetBlue(i,j);
			}
		}
	}


	thresh[0] = red_sum  /((loadedImage.GetHeight()*loadedImage.GetWidth())-4);
	thresh[1] = green_sum/((loadedImage.GetHeight()*loadedImage.GetWidth())-4);
	thresh[2] = blue_sum /((loadedImage.GetHeight()*loadedImage.GetWidth())-4);


}

double MyFrame::FindRedThreshold(wxImage loadedImage, double current_thresh)
{
	int sum =0;
	int counter =0;
	double mu_o, mu_b;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetRed(i,j) > current_thresh)
			{
				sum += loadedImage.GetRed(i,j);
				counter++;

			}
		}
	}	
	mu_o = sum/counter;
	sum = 0;
	counter = 0;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetRed(i,j) < current_thresh)
			{
				sum += loadedImage.GetRed(i,j);
				counter++;

			}
		}
	}
	mu_b = sum/counter;
	return (mu_b+mu_o)/2;
}
double MyFrame::FindGreenThreshold(wxImage loadedImage, double current_thresh)
{
	int sum =0;
	int counter =0;
	double mu_o, mu_b;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetGreen(i,j) > current_thresh)
			{
				sum += loadedImage.GetGreen(i,j);
				counter++;

			}
		}
	}	
	mu_o = sum/counter;
	sum = 0;
	counter = 0;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetGreen(i,j) < current_thresh)
			{
				sum += loadedImage.GetGreen(i,j);
				counter++;

			}
		}
	}
	mu_b = sum/counter;
	return (mu_b+mu_o)/2;
}
double MyFrame::FindBlueThreshold(wxImage loadedImage, double current_thresh)
{
	int sum =0;
	int counter =0;
	double mu_o, mu_b;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetBlue(i,j) > current_thresh)
			{
				sum += loadedImage.GetBlue(i,j);
				counter++;

			}
		}
	}	
	mu_o = sum/counter;
	sum = 0;
	counter = 0;
	for (int i = 0; i < loadedImage.GetWidth(); i++) 
	{
		for (int j = 0; j < loadedImage.GetHeight(); j++) 
		{
			if(loadedImage.GetBlue(i,j) < current_thresh)
			{
				sum += loadedImage.GetBlue(i,j);
				counter++;

			}
		}
	}
	mu_b = sum/counter;
	return (mu_b+mu_o)/2;
}


//ROI selection code
void MyFrame::OnLeftDown(wxMouseEvent& event) {
	selecting = true;
	wxClientDC dc_canvas(this);
	//wxDCOverlay( m_overlay, &dc_canvas ).Clear();
	m_overlay.Reset();

	m_currentpoint = wxPoint(event.GetX(), event.GetY());

	// CancelSelections();
	this->CaptureMouse();
}

void MyFrame::OnMotion(wxMouseEvent& event) {
	if (selecting) {
		int x = event.GetX();
		int y = event.GetY();

		//draw a selection rectangle on the overlay
		wxClientDC dc_canvas(this);

		wxDCOverlay(m_overlay, &dc_canvas).Clear();
		dc_canvas.SetPen(wxPen(*wxLIGHT_GREY, 2));
		dc_canvas.SetBrush(*wxTRANSPARENT_BRUSH);
		dc_canvas.DrawRectangle(wxRect(m_currentpoint, wxPoint(x, y)));
	}

}

void MyFrame::OnLeftUp(wxMouseEvent& event) {


	this->ReleaseMouse();
	selecting = false;


	ROI[0] = m_currentpoint.x < event.GetX() ? m_currentpoint.x : event.GetX();
	ROI[1] = m_currentpoint.y < event.GetY() ? m_currentpoint.y : event.GetY();
	ROI[2] = m_currentpoint.x > event.GetX() ? m_currentpoint.x : event.GetX();
	ROI[3] = m_currentpoint.y > event.GetY() ? m_currentpoint.y : event.GetY();

}

bool MyFrame::CheckIfInROI(int x, int y) {

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
		UndoImages.clear();
		UndoImages.push_back(loadedImage);
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

void MyFrame::OnExit(wxCommandEvent & WXUNUSED(event)) {

	Close (TRUE);
}

void MyFrame::OnPaint(wxPaintEvent & WXUNUSED(event)) {

	wxAutoBufferedPaintDC dc(this);
	if (loadedImage.Ok()) {
		dc.DrawBitmap(wxBitmap(UndoImages.back().Copy()), 0, 0, false); //given bitmap xcoord y coord and transparency
		ROI[0] = 0;
		ROI[1] = 0;
		ROI[2] = loadedImage.GetWidth();
		ROI[3] = loadedImage.GetHeight();

	}

}

void MyFrame::Undo(wxCommandEvent & WXUNUSED(event)) {
	wxCommandEvent foo;
	if(UndoImages.size() == 2)
	{
		MyFrame::OnResetImage(foo);
	}
	else if (UndoImages.size() > 1) {
		printf("Undo image...");
		if (loadedImage.Ok()) {
			loadedImage.Destroy();
		}

		if (origImage.Ok()) {

			loadedImage = UndoImages.back().Copy();
			Refresh();
		}

		printf("Finished Undo.");

		UndoImages.pop_back();
		printf(" size of undo stack: %lu \n", UndoImages.size());
		
	} else {
		printf("can't undo\n");
	}

}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
EVT_MENU ( LOAD_FILE_ID, MyFrame::OnOpenFile)
EVT_MENU ( EXIT_ID, MyFrame::OnExit)
EVT_MENU ( UNDO_ID, MyFrame::Undo)

//###########################################################//
//----------------------START MY EVENTS ---------------------//
//###########################################################//

EVT_MENU ( RESET_IMAGE_ID, MyFrame::OnResetImage)
EVT_MENU ( INVERT_IMAGE_ID, MyFrame::OnInvertImage)
EVT_MENU ( SCALE_IMAGE_ID, MyFrame::OnScaleImage)
EVT_MENU ( SAVE_IMAGE_ID, MyFrame::OnSaveImage)
EVT_MENU ( CONVOLUTE_IMAGE_ID, MyFrame::OnConvoluteImage)
EVT_MENU ( MY_IMAGE_ID, MyFrame::OnMyFunctionImage)//--->To be modified!

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

//lab8
EVT_MENU ( HISTOGRAM_EQUILIZATION_ID, MyFrame::OnHistogramEquilization)

//lab9
EVT_MENU ( SIMPLE_THRESHOLD_ID, MyFrame::OnSimpleThresholdImage)
EVT_MENU ( AUTOMATED_THRESHOLD_ID, MyFrame::OnAutomatedThresholdImage)

////ROI
//EVT_MENU ( LEFT_DOWN_ID, MyFrame::OnLeftDown)
//EVT_MENU ( LEFT_UP_ID, MyFrame::OnLeftUp)
//EVT_MENU ( MOTION_ID, MyFrame::OnMotion)

//###########################################################//
//----------------------END MY EVENTS -----------------------//
//###########################################################//

EVT_PAINT (MyFrame::OnPaint)
END_EVENT_TABLE()
