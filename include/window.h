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

class BasicApplication : public wxApp {
    public:
        virtual bool OnInit();
};

class MyFrame : public wxFrame {
    protected:
        wxMenuBar  *menuBar;//main menu bar
        wxMenu     *fileMenu;//file menu
        wxMenu	   *lab6Menu;
        wxMenu	   *lab7Menu;
        wxMenu	   *lab8Menu;
        wxMenu     *lab9Menu;
        wxToolBar  *toolbar;//tollbar not necessary to use

        //ROI declarations
        wxPanel* m_canvas;
        wxPoint m_currentpoint;
        wxOverlay m_overlay;
        bool selecting;

        int ROI [4];



        std::vector<wxImage> UndoImages;

        int oldWidth, oldHeight; // save old dimensions

        wxImage loadedImage, origImage, activeImage; // image loaded from file and original placeholder

        /* declear message handler */
        void OnResetImage(wxCommandEvent & event);
        void OnInvertImage(wxCommandEvent & event);
        void OnScaleImage(wxCommandEvent & event);
        void OnSaveImage(wxCommandEvent & event);
        void OnConvoluteImage(wxCommandEvent & event);
        void OnMyFunctionImage(wxCommandEvent & event); //---> To be modified!

        //lab6
        void OnAddNoiseImage(wxCommandEvent & event);
        void OnMinFilterImage(wxCommandEvent & event);
        void OnMaxFilterImage(wxCommandEvent & event);
        void OnMidFilterImage(wxCommandEvent & event);

        //lab7
        void OnNegLinearTransformImage(wxCommandEvent & event);
        void OnLogTransformImage(wxCommandEvent & event);
        void OnPowerTransformImage(wxCommandEvent & event);
        void OnRandLookupTableTransformImage(wxCommandEvent & event);


        //lab 8
        void OnHistogramEquilization(wxCommandEvent & event);

        //lab 9
        void OnSimpleThresholdImage(wxCommandEvent & event);
        void OnAutomatedThresholdImage(wxCommandEvent & event);
        void FindInitialThreshold(wxImage loadedImage, double* thresh);
        void FindInitialThreshold2(wxImage loadedImage, double* thresh);
        double FindRedThreshold(wxImage loadedImage, double current_thresh);
        double FindGreenThreshold(wxImage loadedImage, double current_thresh);
        double FindBlueThreshold(wxImage loadedImage, double current_thresh);

        //ROI methods
        void OnLeftDown( wxMouseEvent& event );
        void OnLeftUp( wxMouseEvent& event );
        void OnMotion( wxMouseEvent& event );
        bool CheckIfInROI(int x, int y);

        void Undo(wxCommandEvent & event);

    public:
        MyFrame(const wxString title, int xpos, int ypos, int width, int height);
        virtual ~MyFrame();

        void OnExit(wxCommandEvent & event);
        void OnOpenFile(wxCommandEvent & event);
        void OnPaint(wxPaintEvent & event);

        DECLARE_EVENT_TABLE()
};

enum {
    EXIT_ID = wxID_HIGHEST + 1,
    LOAD_FILE_ID,
    RESET_IMAGE_ID,
    INVERT_IMAGE_ID,
    SCALE_IMAGE_ID,
    SAVE_IMAGE_ID,
    CONVOLUTE_IMAGE_ID,
    //lab6
    NOISE_IMAGE_ID,
    MIN_FILTER_IMAGE_ID,
    MAX_FILTER_IMAGE_ID,
    MID_FILTER_IMAGE_ID,
    //lab7
    NEG_LINEAR_TRANSFORM_IMAGE_ID,
    LOG_TRANSFORM_IMAGE_ID,
    POWER_TRANSFORM_IMAGE_ID,
    RAND_LOOKUP_TABLE_TRANSFORM_ID,
    //lab8
    HISTOGRAM_EQUILIZATION_ID,
    //lab9
    SIMPLE_THRESHOLD_ID,
    AUTOMATED_THRESHOLD_ID,
    //
    MY_IMAGE_ID, //--->To be modified!
    //ROI
    LEFT_DOWN_ID,
    LEFT_UP_ID,
    MOTION_ID,

    UNDO_ID

};
