/* header */
#include "engine.h"

#include "io/waffle_utils.h"

/* standard c-libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* SDL */
#include "SDL.h"
#include "SDL_main.h"

#ifndef PRINT_STATS
#define PRINT_STATS 1
#endif

#ifndef SOUND_DISABLED
#define SOUND_DISABLED 1
#endif

#if !SOUND_DISABLED
#include "SDL_mixer.h"
#endif

/* Chipmunk physics library */
#include "chipmunk.h"

#include "state/statesystem.h"

/* drawing code */
#include "graphics/draw.h"	//opengl included in draw.h
#include "graphics/particles.h"
#include "audio/sound.h"

#include "../space/game.h" //TODO remove dependency
#include "we_unitinfo.h"

#define GAME_VERSION "PRE-ALPHA 9.0" //TMP placement for this define
#if TARGET_OS_IPHONE
#define glOrtho glOrthof
#define glMatrixMode(x)

#include "hgversion.h"
#endif
#ifndef HGVERSION
#define HGVERSION
#endif

#define STRING_2(x) #x
#define STRING_1(x) STRING_2(x)
#define REVISION_ID  STRING_1(HGVERSION)

#define FULL_VERSION_STRING GAME_VERSION" "REVISION_ID

#define FPS_LIMIT 1

static Uint32 thisTime = 0;
static Uint32 lastTime;

static int paused = 0;

static float fps;
static float frames;
we_bool debug_logtime = 1;

#define FPS_SLEEP_TIME 16

/* definition of external variables */
char fps_buf[15];
int GAME_WIDTH;
float ASPECT_RATIO;
int WINDOW_WIDTH, WINDOW_HEIGHT;

SDL_Joystick *accelerometer;

float dt = 0;
int mdt = 0;
unsigned char *keys;

static SDL_GLContext glcontext;

SDL_Window *window;
static SDL_Rect fullscreen_dimensions;

#include "input/button.h"

static int main_running = 1;

configuration config = {
		.fullscreen = 1,
		.width = 1920,
		.height = 1200
};

static int main_destroy();

static void main_pause(void)
{
	statesystem_pause();
	paused = 1;
}

static void main_unpause(void)
{
	paused = 0;
}

static int HandleAppEvents(void *userdata, SDL_Event *event)
{
	switch (event->type)
	{
	case SDL_QUIT:
		main_stop();
		return 1;
#if TARGET_OS_IPHONE
	case SDL_APP_TERMINATING:
		/* Terminate the app.
           Shut everything down before returning from this function.
		 */
		SDL_Log("SDL_APP_TERMINATING");
		return 0;
	case SDL_APP_LOWMEMORY:
		/* You will get this when your app is paused and iOS wants more memory.
           Release as much memory as possible.
		 */
		SDL_Log("SDL_APP_LOWMEMORY");
		return 0;
	case SDL_APP_WILLENTERBACKGROUND:
		/* Prepare your app to go into the background.  Stop loops, etc.
           This gets called when the user hits the home button, or gets a call.
		 */
		main_pause();
		SDL_Log("SDL_APP_WILLENTERBACKGROUND");
		return 0;
	case SDL_APP_DIDENTERBACKGROUND:
		/* This will get called if the user accepted whatever sent your app to the background.
           If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
           When you get this, you have 5 seconds to save all your state or the app will be terminated.
           Your app is NOT active at this point.
		 */
		SDL_Log("SDL_APP_DIDENTERBACKGROUND");
		return 0;
	case SDL_APP_WILLENTERFOREGROUND:
		/* This call happens when your app is coming back to the foreground.
           Restore all your state here.
		 */
		main_unpause();
		SDL_Log("SDL_APP_WILLENTERFOREGROUND");
		return 0;
	case SDL_APP_DIDENTERFOREGROUND:
		/* Restart your loops here.
           Your app is interactive and getting CPU again.
		 */
		main_unpause();
		SDL_Log("SDL_APP_DIDENTERFOREGROUND");
		return 0;
#endif
	default:
		/* No special processing, add it to the event queue */
		return 1;
	}
}

static void setAspectRatio(void) {
	//SDL_ShowCursor(!config.fullscreen);

	if (config.fullscreen) {
		WINDOW_WIDTH = fullscreen_dimensions.w;
		WINDOW_HEIGHT = fullscreen_dimensions.h;
		if (WINDOW_WIDTH < WINDOW_HEIGHT) {
			int t = WINDOW_HEIGHT;
			WINDOW_HEIGHT = WINDOW_WIDTH;
			WINDOW_WIDTH = t;
		}
	} else {
		WINDOW_WIDTH = config.width;
		WINDOW_HEIGHT = config.height;
	}

	ASPECT_RATIO = 1.0f * WINDOW_WIDTH / WINDOW_HEIGHT;
	GAME_WIDTH = GAME_HEIGHT * ASPECT_RATIO;
}

static void display_init(void)
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		SDL_Log("ERROR: SDL_INIT_VIDEO: %s", SDL_GetError());
		exit(-1);
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING,1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); //AA not supported on Android test device
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GLES2 ? 0 : 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	SDL_SetHint("SDL_IOS_ORIENTATIONS", "LandscapeLeft LandscapeRight");

	Uint32 flags = SDL_WINDOW_OPENGL |
			SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_BORDERLESS |
			(config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

	SDL_GetDisplayBounds(0, &fullscreen_dimensions);
	setAspectRatio();

	int bpp;
	SDL_DisplayMode mode;
	Uint32 Rmask, Gmask, Bmask, Amask;

	SDL_GetDesktopDisplayMode(0, &mode);
	SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);

#if !ARCADE_MODE
	SDL_Log("Current mode: %dx%d@%dHz, %d bits-per-pixel (%s)", mode.w, mode.h, mode.refresh_rate, bpp,
			SDL_GetPixelFormatName(mode.format));
#endif

	window = SDL_CreateWindow("SPACE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, flags);

	if (!window) {
		char *err =  SDL_GetError();
		SDL_Log("ERROR - could not create window!\n");
		SDL_Log("%s", err);
		//SDL_Quit();
		//exit(-1);
	}

	SDL_ShowWindow(window);
}

static void initGL(void)
{
	// Create an OpenGL context associated with the window.
	glcontext = SDL_GL_CreateContext(window);
	#if __WIN32__
		GLenum glewError = glewInit();
		if( glewError != GLEW_OK )
		{
			SDL_Log( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
			exit(-1);
		}
		if (glewGetExtension("GL_EXT_framebuffer_object"))
		{
			SDL_Log( "Have extension" );
		}else{
			SDL_Log( "Error no extension");
		}
	#endif
	if (!glcontext) {
		SDL_Log("SDL ERROR: %s", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(FPS_LIMIT); // 1 = v-sync on

	glEnable(GL_BLEND);
	draw_blend(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	/* set the clear color to dark blue */
	//	glClearColor(0, 0.08, 0.15, 1);

	/* print gl info */
#if !ARCADE_MODE
	SDL_Log("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	SDL_Log("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	SDL_Log("GL_VERSION: %s\n", glGetString(GL_VERSION));
	SDL_Log("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));
	int max_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
	SDL_Log("Max texture size = %d", max_size);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_size);
	SDL_Log("Max texture units = %d", max_size);
#endif

	glEnableVertexAttribArray(0); // vertex
	glEnableVertexAttribArray(1); // texture
	glEnableVertexAttribArray(2); // colors

	draw_load_identity();
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glClearColor(0,0,0, 1);
	glClearColor(0,0,0.05,1);
	//glClearColor(0.5,0.5,0.5,1);

	//glEnable(GL_MULTISAMPLE); //GLES1!
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0,0, WINDOW_WIDTH,WINDOW_HEIGHT); //scissor test

	//draw_enable_tex2d();
}

#if GLES2
#define GLSL_CODE(type, x) { \
	"#version 100\n" \
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n" \
    "precision highp float;           \n" \
    "#else                            \n" \
    "precision mediump float;         \n" \
    "#endif                           \n" \
	#x \
	}

#else
#define GLSL_CODE(type, x) { \
	"#version 120\n" \
    "#define lowp   \n" \
    "#define mediump\n" \
    "#define highp  \n" \
	#x \
	}

#endif


GLuint gl_program, gl_vertshader, gl_fragshader;

void we_ortho(float W_2, float H_2)
{
	float left=-W_2, right=W_2, bottom=-H_2, top=H_2;

    float a =  2.0f / (right - left);
    float b =  2.0f / (top   - bottom);

    float tx = -(right + left)   / (right - left);
    float ty = -(top   + bottom) / (top   - bottom);

    //TODO use matrix.c instead;
    float ortho[16] = {
        a, 0, 0, tx,
        0, b, 0, ty,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    GLint projectionUniform = glGetUniformLocation(gl_program, "uProjection");
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, &ortho[0]);
}

void printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program)) {
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char infoLog[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0) {
			//Print Log
			printf("%s\n", infoLog);
		}
	} else {
		printf("Name %d is not a program\n", program);
	}
}
void printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader)) {
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char infoLog[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0) {
			//Print Log
			printf("%s\n", infoLog);
		}

	} else {
		printf("Name %d is not a shader\n", shader);
	}
}

static void initGP(void)
{
	//TODO write and compile shader program here

	/*reference:
	http://lazyfoo.net/tutorials/OpenGL/29_hello_glsl/index.php
	http://lazyfoo.net/tutorials/OpenGL/30_loading_text_file_shaders/index.php
	http://lazyfoo.net/tutorials/OpenGL/31_glsl_matrices_color_and_uniforms/index.php
	http://lazyfoo.net/tutorials/OpenGL/34_glsl_texturing/index.php
	http://www.khronos.org/registry/gles/specs/2.0/GLSL_ES_Specification_1.0.17.pdf
	*/

	GLint programSuccess = GL_FALSE;
	gl_program = glCreateProgram();

	/* Vertex Shader */
	gl_vertshader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *vertexShaderCode[] = GLSL_CODE(GL_VERTEX_SHADER,
		uniform mat4 uProjection;
		uniform float uTime;

		attribute vec2 aVertex;
		attribute vec2 aTexCoord;
		attribute vec4 aColor;

		varying vec2 verCoord;
		varying vec2 texCoord;
		varying vec4 col;

		const float PI = 3.141592;

		void main() {
			gl_Position = uProjection * vec4(aVertex.xy, 0, 1); //TODO create and send in projection matrix
			verCoord = gl_Position.xy;
			texCoord = aTexCoord;
			col = aColor;
		}
	);
	glShaderSource(gl_vertshader, 1, vertexShaderCode, NULL);
	glCompileShader(gl_vertshader);

	// Check for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(gl_vertshader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE) {
		printf("Unable to compile vertex shader %d!\n", gl_vertshader);
		printShaderLog(gl_vertshader);
		we_error("error - vertex shader");
	}
	glAttachShader(gl_program, gl_vertshader);


	/* Fragment Shader */
	gl_fragshader = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *fragmentShaderCode[] = GLSL_CODE(GL_FRAGMENT_SHADER,
		uniform sampler2D texUnit;
		uniform float uTime;

		varying vec2 verCoord;
		varying vec2 texCoord;
		varying vec4 col;

		vec3 rgb2hsv(vec3 c)
		{
		    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
		    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
		    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

		    float d = q.x - min(q.w, q.y);
		    float e = 1.0e-10;
		    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
		}

		vec3 hsv2rgb(vec3 c)
		{
		    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
		    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
		    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
		}

		void main() {
			float light = 1.0;
			vec4 tex = texture2D(texUnit, texCoord);
			//vec3 hsv = rgb2hsv(tex.xyz);
			//float hf = 0.5;
			//float sf = 0.1;
			//float h = uTime * hf - floor(uTime * hf);
			//float s = uTime * sf - floor(uTime * sf);
			//hsv.x += h;
			//hsv.y += s;
			//tex.xyz = hsv2rgb(hsv);

			gl_FragColor = tex * (col/255.0) * vec4(light,light,light,1.0);
			//gl_FragColor = texture2D(texUnit, texCoord) * (col/255.0) * vec4(light,light,light,1) - 0.3*abs(vec4(cos(verCoord.x/68-uTime*3.5)/2,sin(verCoord.y/50-uTime*1.5),cos(verCoord.y/130+uTime/2)*sin(verCoord.x/120+uTime*2),0));
		}
	);
	glShaderSource(gl_fragshader, 1, fragmentShaderCode, NULL);
	glCompileShader(gl_fragshader);

	// Check for errors
	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(gl_fragshader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE) {
		printf("Unable to compile fragment shader %d!\n", gl_vertshader);
		printShaderLog(gl_fragshader);
		we_error("error - fragment shader");
	}

	glAttachShader(gl_program, gl_fragshader);

	/* Link program */
	glLinkProgram(gl_program);
	glUseProgram(gl_program);

	// Check for errors
	glGetProgramiv(gl_program, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE) {
		printf("Error linking program %d!\n", gl_program);
		printShaderLog(gl_program);
		we_error("error - program shader");
	}

	we_ortho(GAME_WIDTH / 2, GAME_HEIGHT / 2); //GLES1!
}

static void net_init(void)
{
#ifdef GOT_SDL_NET
	if (SDLNet_Init() == -1) {
		fprintf(stderr, "ERROR: SDLNet_Init: %s\n", SDLNet_GetError());
		exit(-1);
	}
#endif
}

#define TIMEUSE_INIT Uint32 startTime = SDL_GetTicks()
#define TIMEUSE(x)  fprintf(stderr,"STATS time: %6.3f "x"\n", (SDL_GetTicks()-startTime) / 1000.0)
#define INITCALL(func) fprintf(stderr, "INIT: "#func"\n"), func(), TIMEUSE(#func" done")

static void main_init(void)
{
	we_info_init();
	TIMEUSE_INIT;
	srand(time(0)); /* init pseudo-random generator */
	//TODO Make sure faulty inits stops the program from proceeding
	INITCALL(waffle_init);      /* prepare game resources and general methods*/
	INITCALL(game_config);	/* load default and user-changed settings */
	INITCALL(display_init);
	INITCALL(initGL);           /* setup a gl context */
	INITCALL(initGP);          /* setup a gl context */
	INITCALL(finger_init);
	INITCALL(net_init);
    accelerometer = SDL_JoystickOpen(0);
#if !ARCADE_MODE
    if (accelerometer == NULL) {
        SDL_Log("Could not open joystick (accelerometer)");
    }
#endif

	//cpInitChipmunk);
    INITCALL(sound_init);
    INITCALL(texture_init);
    INITCALL(sprite_init);
    INITCALL(bmfont_init);
    INITCALL(draw_init);
    INITCALL(particles_init);
	//INITCALL(font_init);      /* currently not in use) */
    INITCALL(statesystem_init);
    INITCALL(layersystem_init);
    INITCALL(object_init);
    INITCALL(objectsystem_init);
	INITCALL(jparse_init);
    INITCALL(game_init);

	// Handle iOS app-events (pause, low-memory, terminating, etc...) and SDL_QUIT
	SDL_SetEventFilter(HandleAppEvents, NULL);

#if TARGET_OS_IPHONE
	// Initialize the Game Center for scoring and matchmaking
    //InitGameCenter(); //TODO support GameCenter
#endif

	TIMEUSE("ALL INIT DONE time");
	lastTime = SDL_GetTicks();

#if !ARCADE_MODE
	fprintf(stderr, "%s\t%5s ms\t%4s ms\t%4s ms\t%6s ms\t%6s ms\t%6s ms\n", "FPS", "AVG", "MIN", "MAX", "Update", "Draw", "GLSwap");
#endif
}

static void check_events(void)
{
	SDL_Event event;
	SDL_PumpEvents();
	keys = (unsigned char *)SDL_GetKeyboardState(NULL);

	while (SDL_PollEvent(&event)) {
		statesystem_push_event(&event);
#if !GOT_TOUCH
		SDL_Event sim_event;
#endif
		static int render_outside = 1; // for debug purposes
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_T:
				render_outside ^=0x5;
				GLfloat W_2 = GAME_WIDTH / 2 * render_outside;
				GLfloat H_2 = GAME_HEIGHT / 2 * render_outside;
				we_ortho(W_2, H_2);
				break;
			case SDL_SCANCODE_R:
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			default:
				break;
			}
			break;
		case SDL_FINGERUP:
			finger_release(event.tfinger.fingerId);
			break;
#if !GOT_TOUCH
		case SDL_MOUSEBUTTONDOWN:
			sim_event.type = SDL_FINGERDOWN;
			sim_event.tfinger.fingerId = event.button.button;
			float x = event.button.x;
			float y = event.button.y;

			sim_event.tfinger.x = x / WINDOW_WIDTH;
			sim_event.tfinger.y = y / WINDOW_HEIGHT;
			statesystem_push_event(&sim_event);
			static float last_x, last_y;
			static int pressed;
			pressed = 1;
			last_x = x;
			last_y = y;
			break;
		case SDL_MOUSEMOTION:
			if (pressed) {
			sim_event.type = SDL_FINGERMOTION;
			sim_event.tfinger.fingerId = event.button.button;
			float x = event.button.x;
			float y = event.button.y;

			sim_event.tfinger.x = x / WINDOW_WIDTH;
			sim_event.tfinger.y = y / WINDOW_HEIGHT;

			sim_event.tfinger.dx = (event.button.x - last_x) / WINDOW_WIDTH;
			sim_event.tfinger.dy = (event.button.y - last_y) / WINDOW_HEIGHT;
			last_x = x;
			last_y = y;

			statesystem_push_event(&sim_event);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			sim_event.type = SDL_FINGERUP;
			sim_event.tfinger.fingerId = event.button.button;
			sim_event.tfinger.x = (float) event.button.x / WINDOW_WIDTH;
			sim_event.tfinger.y = (float) event.button.y / WINDOW_HEIGHT;
			statesystem_push_event(&sim_event);
			pressed = 0;
			finger_release(sim_event.tfinger.fingerId);
			break;
#endif
		}
	}
}

static void main_sleep(void)
{
#if FPS_LIMIT
		//not use 100% of cpu
		if (keys[SDL_SCANCODE_Z])
			SDL_Delay(14);
		else if (keys[SDL_SCANCODE_C])
			SDL_Delay(50);

		static int FPS_EXTRA_SLEEP = 0;
		if (FPS_EXTRA_SLEEP >= 2)
			FPS_EXTRA_SLEEP = 0;
		else
			++FPS_EXTRA_SLEEP;

		Uint32 sleep_delta = SDL_GetTicks() - thisTime;
		if (sleep_delta < FPS_SLEEP_TIME) {
			SDL_Delay((FPS_SLEEP_TIME) - sleep_delta + (FPS_EXTRA_SLEEP != 0));
		}
#else
		SDL_Delay(1);
#endif
}

GLfloat engine_time = 0;

static void main_tick(void *data)
{
	thisTime = SDL_GetTicks();
	dt = (thisTime - lastTime) / 1000.0f;
	lastTime = thisTime;

	frames += dt;
	fps++;

	dt = dt > 0.1 ? 0.1 : dt;
	mdt = dt * 1000;


    engine_time +=dt;
    GLint uTime = glGetUniformLocation(gl_program, "uTime");
    glUniform1f(uTime, engine_time);

	//TODO detect matrix stack unbalance
	draw_matrix_clear();

	//clear all
	draw_color4f(0,0,0,1);
	glDisable(GL_SCISSOR_TEST);
	glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT);
	//draw_box(cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);
	glEnable(GL_SCISSOR_TEST);

	if (!paused) {
		check_events();

		static float update_time = 0, draw_time = 0, swap_time = 0;
		int timer = SDL_GetTicks();
		statesystem_update();
		update_time += SDL_GetTicks() - timer;
		timer = SDL_GetTicks();
		statesystem_draw();
		draw_time += SDL_GetTicks() - timer;
		timer = SDL_GetTicks();
		SDL_GL_SwapWindow(window);
		swap_time += SDL_GetTicks() - timer;

		int gl_error = glGetError();
		if (gl_error) SDL_Log("main.c: %d  GL_ERROR: %d\n",__LINE__,gl_error);

		static int min_frame = 10000, max_frame = 0;
		int frame_time = (SDL_GetTicks() - thisTime);
		if (frame_time < min_frame) min_frame = frame_time;
		if (frame_time > max_frame) max_frame = frame_time;

		if (frames >= 1) {
			float avg_frame = 1000.0/fps;
			update_time /=fps;
			draw_time /=fps;
			swap_time /=fps;
			sprintf(&fps_buf[0], "%.2f FPS ", fps);
#if !ARCADE_MODE && PRINT_STATS
			if (debug_logtime) {
				fprintf(stderr, "%5.2f\t%5.1f\t%4d\t%4d\t%6.2f\t%6.2f\t%6.2f\n", fps, avg_frame, min_frame, max_frame, update_time, draw_time, swap_time);
			}
#endif
			frames = 0;
			fps = 0;
			min_frame = 10000;
			max_frame = 0;
			update_time = 0;
			draw_time = 0;
			swap_time = 0;
		}

		/*// rendering is not supported outside of a drawing call
		draw_load_identity();
		setTextAlign(TEXT_RIGHT);
		setTextSize(15);
		float box_width = strlen(FULL_VERSION_STRING)*15*1.5+10;
		draw_color4f(0,0,0,1);
		draw_box(cpv(GAME_WIDTH/2-box_width,-GAME_HEIGHT/2),cpv(box_width,40),0,0);
		draw_color4f(0.7,0,0,1);
		font_drawText(GAME_WIDTH/2-5,-GAME_HEIGHT/2+20,FULL_VERSION_STRING);
		*/

	}

	//input handler
	if (keys[SDL_SCANCODE_F12]) {
		main_stop();
	}
	if (keys[SDL_SCANCODE_R]) {
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (keys[SDL_SCANCODE_F] && keys[SDL_SCANCODE_LCTRL]) {
		config.fullscreen ^= 1; // Toggle fullscreen
		if (SDL_SetWindowFullscreen(window, config.fullscreen)) {
			SDL_Log("%s",SDL_GetError());
			config.fullscreen ^= 1; // Re-toggle
		} else {
			setAspectRatio();
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			we_ortho(GAME_WIDTH / 2, GAME_HEIGHT / 2);

			SDL_Log("DEBUG: WINDOW FULLSCREEN CHANGED -> %d", config.fullscreen);
		}
		keys[SDL_SCANCODE_F] = 0;
	}

	if (keys[SDL_SCANCODE_Z]) {
		SDL_Delay(40);
	}

	if (!main_running) {
		main_destroy();
	}
}

static int main_run(void)
{
#if __IPHONEOS__
    // Initialize the Game Center for scoring and matchmaking
    //InitGameCenter(); //TODO support GameCenter

    // Set up the game to run in the window animation callback on iOS
    // so that Game Center and so forth works correctly.
	SDL_iPhoneSetAnimationCallback(window, 1, main_tick, NULL);
#else
	//START GAME
#if ARCADE_MODE
		printf("start %s\n", "999");
#endif

	while (main_running) {
		main_tick(NULL);
		main_sleep();
	}
#endif
	return 0;
}

static int main_destroy(void) {
#if !ARCADE_MODE
	SDL_Log("DEBUG - SDL_destroy\n");
#endif
	//cpSpaceFreeChildren(space);

	game_destroy();

	/* destroy states */
	statesystem_destroy();
	objectsystem_destroy();
	object_destroy();
	layersystem_destroy();

	particles_destroy();

	sound_destroy();
	draw_destroy();
	font_destroy();

	waffle_destroy();

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	//SDL_GL_MakeCurrent(NULL, NULL);

	glDeleteShader(gl_fragshader);
	glDeleteShader(gl_vertshader);
	glDeleteProgram(gl_program);
	SDL_GL_DeleteContext(glcontext);

	extern int stat_hm_adds, stat_hm_rehash;
	fprintf(stderr, "hashmap stats: %d adds (%.2f %% rehash)\n", stat_hm_adds, 100.0 * stat_hm_rehash / stat_hm_adds);
#ifdef GOT_SDL_NET
	SDLNet_Quit();
#endif
#if !TARGET_OS_IPHONE
	SDL_DestroyWindow(window);
	SDL_Quit();
	SDL_Log("exit 0");
	exit(0);
#endif

	return 0;
}

int main(int argc, char *args[]) {
	main_init();
	main_run();
	return 0;
}

void main_stop(void) {
	main_running = 0;
}

/*
 * Converts normalized (e.g. touch input) coordinates to game window coordinates
 */
void normalized2game(float *x, float *y)
{
	*x = (*x - 0.5f) * GAME_WIDTH;
	*y = (0.5f - *y) * GAME_HEIGHT;
}

