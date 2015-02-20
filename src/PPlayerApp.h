#pragma once
#include "ofMain.h"

//PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
//PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

class PPlayerApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:


	int h, width, height, length;
	float x, y, z;
	bool vsync;
	ofVec3f center;
	bool show_stats;

	vector<vector <ofImage> > all_panoramas;
	int texture_index;
	void scanTextureFolder();
	void cycleTextures();

	//vector< ofRectangle > viewports;
	ofImage mask_image;
	ofShader shader;
	ofRectangle black_fade_mask;
	bool initViewPorts();
	bool bViewportsInitialized;
	ofEasyCam camera;
	float fov;
	float fade_factor;

	float rotation;
	

	ofSerial serial;
	bool bSerialDeviceReady;
	bool bControllerConnected;
	bool establishControllerConnection(ofSerial & _serial);
	float last_conn_try;
	int last_send_q_time;
	bool b_responded;
	int response_time;
	float send_request_time;

	bool button_state_1, button_state_2;
	void button1pressed();
	void button2pressed();

	string trimStringRight(string str);
	string trimStringLeft(string str);
	string trimString(string str);
	string getSerialString(ofSerial &the_serial, char until);

	unsigned char buf[15];
	bool bArduinoSetup;
	void digitalPinChanged(const int & pinNum);
	void updateSerial();

	string buttonState;
	string potValue;

	//potentiometer smoothing
	static const int numReadings = 5;
	int readings[numReadings];
	int index, total, average;



	//////////////
	//enable vsync
	//////////////
	bool IsExtensionSupported( char* szTargetExtension )
	{
		const unsigned char *pszExtensions = NULL;
		const unsigned char *pszStart;
		unsigned char *pszWhere, *pszTerminator;

		// Extension names should not have spaces
		pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
		if( pszWhere || *szTargetExtension == '\0' )
			return false;

		// Get Extensions String
		pszExtensions = glGetString( GL_EXTENSIONS );

		// Search The Extensions String For An Exact Copy
		pszStart = pszExtensions;
		for(;;)
		{
			pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
			if( !pszWhere )
				break;
			pszTerminator = pszWhere + strlen( szTargetExtension );
			if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
				if( *pszTerminator == ' ' || *pszTerminator == '\0' )
					return true;
			pszStart = pszTerminator;
		}
		return false;
	}

	bool InitVSync()
	{
		if (IsExtensionSupported("WGL_EXT_swap_control"))
		{
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglGetSwapIntervalEXT");
			return true;
		}
		return false;// WGL_EXT_swap_control not supported
	}
	bool SetVSync(bool VSync)
	{
		if(!wglSwapIntervalEXT) return false;
		wglSwapIntervalEXT(VSync);
		return true;
	}
	bool GetVSync(bool* VSync)
	{
		if(!wglGetSwapIntervalEXT) return false;//VSynce value is not valid...
		*VSync = wglGetSwapIntervalEXT();
		return true;
	}

};
