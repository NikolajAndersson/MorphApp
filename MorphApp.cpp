/*
	Programming III exam, Art & Technology, 3/10-14
	Program & Music created by Nikolaj Andersson  
	
*/
#include <iostream>
#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
using namespace al;

// Class 
class Shape {
private:
	// private member variables  
	float radius;
	Vec3f mPosition; // three-component vector (x,y,z)
	float mScale; 
	float b; 
 	int dots;

public: 
	
	Mesh mMesh; 

	// default constructor that generates a circle shape
	Shape(float m = 2, float n1 = 1, float n2 = 1, float n3 = 2, float a = 1) 
	: mPosition(0, 0, 0), mScale(1), b(1), dots(1024)  {
		
		// set up the primitive line for for mMesh
		mMesh.primitive(Graphics::LINE_LOOP);
		
		for(int i = 0; i < dots; i++){
		// calculate the angle, theta
		float theta = float(i) / dots * 2*M_PI;
		
		// Superformula takes six variables and theta 
		radius = pow(((pow((cos(m*theta/4)/a), (n2*2))) + pow((sin(m * theta/4)/b), n3) ), (-1/n2));
		
		// convert polar to cartesian coordinates
		float x = radius * cos(theta); 
		float y = radius * sin(theta); 
		
		// create the mesh, using the generated x and y values
		mMesh.vertex(x, y);
		// add hue to each dot, going from 0 to 1
		mMesh.color(HSV(float(i)/dots, 1, 1));		
		}
	}
	// draw method that sets the parameters: mPosition, stroke, scale, mMesh later gets called in onDraw 
	void draw(Graphics& g){
		g.pushMatrix(Graphics::MODELVIEW);
			g.translate(mPosition);
			g.stroke(3);
			g.scale(mScale);
			g.draw(mMesh);
		g.popMatrix();
	}	
};

// create MorphApp that inheriterce
class MyApp : public App{
public: 
	
	gam::SamplePlayer<> play, sineone, sinetwo, sinethree, sinefour, sinefive, sinesix;
	
	int sampleNumber; 		// Which sound sample to play (-1 means no playback)
	float transformSpeed;	// transform speed for morphing
	float volume; 			// volume to control backing track
	float frac;				// a variable that goes from 0 and count up. 
	
	// the shapes forms i want to use
	Shape circle;
	Shape sun;
	Shape square;
	Shape triangle;
	Shape star;
	Shape flower;
	
	// shapes used for morphing
	Shape shape[6];
	Shape morph;
		
	// MorphApps default constructor, assign parameters for each individual shape, 'star(5, 10, 5, 10)' draws a star-like shape 
	MyApp() : sun(20, 1, 1, 8), square(4, 1, 5, 6), triangle(3, 1, 5, 10), star(5, 10, 5, 10), flower(20, 1, 1, 8, 0.4) {
		
		// load samples from soundfiles directory
		play.load(RUN_MAIN_SOURCE_PATH "soundfiles/saw-wave-piece.wav");
		sineone.load(RUN_MAIN_SOURCE_PATH "soundfiles/sineone.wav");
		sinetwo.load(RUN_MAIN_SOURCE_PATH "soundfiles/sinetwo.wav");
		sinethree.load(RUN_MAIN_SOURCE_PATH "soundfiles/sinethree.wav");
		sinefour.load(RUN_MAIN_SOURCE_PATH "soundfiles/sinefour.wav");
		sinefive.load(RUN_MAIN_SOURCE_PATH "soundfiles/sinefive.wav");
		sinesix.load(RUN_MAIN_SOURCE_PATH "soundfiles/sinesix.wav");		

		// Initialize variables and startout shape
		sampleNumber = -1; // sampleNumber equals -1 since I don't want any morphing to happen before a key is pressed
		transformSpeed = 0.05;  // assign transformation speed, if you don't manually set it using the left click or mouse drag 
		frac = 0; 
		volume = 0.3; 		// assign volume, if you don't manually set it using the right click or mouse drag  
		morph = flower; 	// assign the flower shape to first appear on screen
		
		// initializing every array in shape to a form
		shape[0] = circle;
		shape[1] = triangle;
		shape[2] = square;
		shape[3] = star;		
		shape[4] = sun;
		shape[5] = flower;
		
		// set camera position, create window, toggle full screen on and remove default keyboard controls 
		nav().pos(0, 0, 7.5);
		initWindow();	
		window().fullScreen(true);
		window().remove(navControl()); 
		
		// initialize audio & sample rate, block size, output channels, input channel
		initAudio(44100, 128, 2,0);	
	}
	// callback function onAnimate and dt is time pr. second since last frame
	void onAnimate(double dt){
	
	// if sampleNumber not equal to one, change shape to the new desired shape with the index number of sampleNumber	
	if(sampleNumber != -1){
		
		morph.mMesh.reset(); 	// clears morphs mesh
		
		// frac is how far the morphing is, if 0 then nothing happens, if 1, then new desired shape is created
		// everything in between 0 and 1 is the actual morphing, the faster frac counts up, the faster the morphing
		frac += transformSpeed * dt; 

		// for loops through all the vertices in the shapes and set all the vertices 		 
		for(int i = 0; i < circle.mMesh.vertices().size(); i++){
			// create a 3-vector point v
			Vec3f v; 
		   
			// access the mesh through an array store every vertex in morph and the new shape
			Vec3f v1 = morph.mMesh.vertices()[i];
			Vec3f v2 = shape[sampleNumber].mMesh.vertices()[i];
		   
		   	// frac count up from 0 to 1
			// the closer the frac is to 1, the more of the new shape is visible
			v = (v1 * (1-frac)) + (v2 * frac);
   
			// set morphs mesh to the v value 
			morph.mMesh.vertex(v);
			// since the mesh has been reset, a color also needs to added to every mesh point
			morph.mMesh.color(HSV(float(i)/circle.mMesh.vertices().size(), 1, 1));					
		}	
		if(frac >= 1){ 		   // if frac is more or equal to 1, it means the morphing is done
			sampleNumber = -1; // set sampleNumber to -1, which will stop the morphing
		}									
	} 
}
	
	// The audio callback function at rate of samplerate/blocksize
	void onSound(AudioIOData& io){
		gam::sampleRate(io.fps()); // set sample rate for sound objects to match frames pr second.
		
		while(io()){ 
			// assign sine-sample to sample[]
			float sample[6] = {sineone(), sinetwo(), sinethree(), sinefour(), sinefive(), sinesix()};
			float bTrack = 0;
			float melody = 0;
	
			// Play sample according to value of sampleNumber
			if(sampleNumber != -1){
				melody = sample[sampleNumber];													
			} 
			// assign bTrack to the sampleplayer object play()
			bTrack = play();
			// loop play() sample
			play.loop();
			
			// assign the outputs with the desired gain
			float out1 = bTrack*volume + melody*0.5 ;
			float out2 = out1;
			
			// assign outputs to speaker 0 and 1
			io.out(0) = out1;
			io.out(1) = out2;
		}
	}
	
	// a key was pressed 
	void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		// Change sample number according to key press
		switch(k.key()){	
		case 'm': 				// if m is pressed down	
			frac = 0; 			// set frac & sampleNumber to 0
			sampleNumber = 0; 	
			sineone.reset();	// reset pinhead of sineone sample, so it keeps playing from the start of the sample
				break;			// break out of switch
		case 'n': 
			frac = 0; 			// same happens here, except that the sampleNumber has changed because of the new key 
			sampleNumber = 1;	// and the sample that matches is reset 
			sinetwo.reset();
				break;
		case 'b': 
			frac = 0; 
			sampleNumber = 2;	
			sinethree.reset();	
				break;
		case 'v': 
			frac = 0; 
			sampleNumber = 3;
			sinefour.reset();
				break;	
		case 'c': 
			frac = 0; 
			sampleNumber = 4;
			sinefive.reset();
				break;	
		case 'x': 
			frac = 0; 
			sampleNumber = 5;
			sinesix.reset();
				break;	
		case ' ':				// if spacebar is pressed down 
			sampleNumber = -1;	// stop morphing and sound
			 	break;
		case Keyboard::RETURN:	// if enter/return  	
		case Keyboard::ENTER: 
			play.reset();	 	// reset the backing track, play it from the beginning
				break;	
		} 
	}
	// Mouse was dragged
	void onMouseDrag(const ViewpointWindow& w, const Mouse& m){
		// drag on the x-axis
		float mxOnScreen = float(m.x()) / w.width(); // map x coordinate to a value between 0 and 1
		// set the mapped x-coordinate of mouse to transformation speed
		transformSpeed = mxOnScreen;				
		
		// drag on the y-axis
		float myOnScreen = 1-(float(m.y()) / w.height()); // map y coordinate to a value between 0 and 1
		// assign normalized y-coordinate of mouse to 'play' volume 
		volume = myOnScreen; 		
	}
		// A button was pressed
	void onMouseDown(const ViewpointWindow& w, const Mouse& m){
		float mxOnScreen = float(m.x()) / w.width(); 		// map x coordinate to a value between 0 and 1
		float myOnScreen = 1-(float(m.y()) / w.height());	// map y coordinate to a value between 0 and 1

		switch(m.button()){		// which button was pressed?
		case 0: 				// case 0 = left click
		transformSpeed = mxOnScreen; // set mapped x coordinate to transform speed
			break;
			
		case 2:						// case 2 = right click
		volume = myOnScreen; 		// set mapped y coordinate to 'play' volume
			break;	
		}
	}
	
	// callback function
	void onDraw(Graphics& g, const Viewpoint& v){
		// call draw, a member function of the class Shape 
		morph.draw(g);	
	}
};

int main(){
	MyApp().start();
}