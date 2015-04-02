#pragma once
#include "ofMain.h"
#include "PanoramaCube.h"
#include "Binocular.h"
#include "SerialControl.h"
#include "SimpleAnimatable.h"

//PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
//PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

class PPlayerApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	void exit();

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
	int width, height, length;  //dimensions of panorama cube
	float x, y, z;				//position of panorama cube
	ofVec3f center;
	PanoramaCube pCube;

    bool use_shutdown_timer;
    int shutdown_time;
    int currentTime;

	bool show_stats;
	bool fullscreen;
	bool loadSettings(string filename);
	void loadDefaultSettings();

	vector<vector <ofImage> > all_panoramas;
	unsigned char panorama_index;
	unsigned char new_panorama_index;
	bool scanTextureFolder();
	bool all_textures_loaded;
	void cycleTextures_up();
	void cycleTextures_down();

	Binocular binocular;
	ofEasyCam camera;
	int fov;
	float rotation_scale;
	float rotation_offset;

	//animations betwen different panoramas
	SimpleAnimatable animatable;
	float blending_speed;

	//movement within the panorama, the viewing direction
	float viewing_direction;

	//serial threaded
	SerialControl serial_control;

	void button1pressed(bool &state);
	void button2pressed(bool &state);
	void sensor_value_changed(int &value);
	void fade_to(bool &is_black);
	int last_sensor_value;
	bool last_recieved_fading_state;
	bool current_fading_state;

	string buttonState;
	string potValue;

	/////////////////////////////
	//HELPERS
	/////////////////////////////
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
