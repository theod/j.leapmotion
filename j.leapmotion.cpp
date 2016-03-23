/** @file
 *
 * @ingroup implementationMaxExternals
 *
 * @brief j.leapmotion
 *
 * @details
 *
 * @author Théo de la Hogue (based on the Masayuki Akamatsu aka.leapmotion external source)
 *
 * @copyright © 2014 by Théo de la Hogue @n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

#include "Leap.h"

#include <iostream>

#define _USE_MATH_DEFINES // To get definition of M_PI
#include <math.h>

////////////////////////// object struct
typedef struct _leapmotion
{
	t_object            ob;
	int64_t             frame_id_save;
    t_symbol*           stateNames[4];
	void                *outlets[7];
	Leap::Controller    *leap;
} t_leapmotion;

#define end_frame_out 0
#define gesture_out 1
#define tool_out 2
#define finger_out 3
#define hand_out 4
#define frame_out 5
#define	start_frame_out 6

///////////////////////// function prototypes
//// standard set
void *leapmotion_new(t_symbol *s, long argc, t_atom *argv);
void leapmotion_free(t_leapmotion *x);
void leapmotion_assist(t_leapmotion *x, void *b, long m, long a, char *s);

void leapmotion_bang(t_leapmotion *x);

//////////////////////// global class pointer variable
void *leapmotion_class;

//////////////////////// Max functions
int C74_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("j.leapmotion", (method)leapmotion_new, (method)leapmotion_free, (long)sizeof(t_leapmotion),
				  0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)leapmotion_bang, "bang", 0);
    
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)leapmotion_assist, "assist", A_CANT, 0);
	
	class_register(CLASS_BOX, c);
	leapmotion_class = c;
    
	return 0;
}

void *leapmotion_new(t_symbol *s, long argc, t_atom *argv)
{
	t_leapmotion *x = NULL;
    
	if ((x = (t_leapmotion *)object_alloc((t_class *)leapmotion_class)))
	{
		object_post((t_object *)x, "j.leapmotion 0.1(32/64 bit) for The Leap 2.1.6");
        
        x->frame_id_save = 0;
        
        // prepare state symbols
        x->stateNames[0] = gensym("invalid");
        x->stateNames[1] = gensym("start");
        x->stateNames[2] = gensym("update");
        x->stateNames[3] = gensym("end");
        
        // make several outlets
        x->outlets[start_frame_out] = outlet_new(x, 0);  // start_frame bang outlet
        x->outlets[frame_out] = outlet_new(x, 0);        // frame_out anything outlet
        x->outlets[hand_out] = outlet_new(x, 0);         // hand_out anything outlet
        x->outlets[finger_out] = outlet_new(x, 0);       // finger_out anything outlet
        x->outlets[tool_out] = outlet_new(x, 0);         // tool_out anything outlet
        x->outlets[gesture_out] = outlet_new(x, 0);      // gesture_out anything outlet
        x->outlets[end_frame_out] = outlet_new(x, 0);    // end_frame bang outlet
        
        // create a controller
        x->leap = new Leap::Controller;
        
        // allow the external to receive data even if it is not the foreground application
        x->leap->setPolicy(Leap::Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES);
        
        // Allow gesture recognition
        x->leap->enableGesture(Leap::Gesture::TYPE_CIRCLE);
        x->leap->enableGesture(Leap::Gesture::TYPE_SWIPE);
        x->leap->enableGesture(Leap::Gesture::TYPE_KEY_TAP);
        x->leap->enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
    }
    
    return x;
}

void leapmotion_free(t_leapmotion *x)
{
	delete (Leap::Controller *)(x->leap);
}

void leapmotion_assist(t_leapmotion *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET)            // Inlet
        strcpy(dst, "input");
	else {								// Outlets
		switch(arg) {
			case 0:
            strcpy(dst, "end frame");
            break;
			case 1:
            strcpy(dst, "gestures info");
            break;
			case 2:
            strcpy(dst, "tools info");
            break;
			case 3:
            strcpy(dst, "fingers info");
            break;
            case 4:
            strcpy(dst, "hands info");
            break;
            case 5:
            strcpy(dst, "frames info");
            break;
            case 6:
            strcpy(dst, "start frame");
            break;
		}
 	}
}

void leapmotion_bang(t_leapmotion *x)
{
    // theo : create a j_sym_list symbol here because the _sym_list crashes
    t_symbol *j_sym_list = gensym("list");
    
	const Leap::Frame frame = x->leap->frame();
	const int64_t frame_id = frame.id();
	
	// ignore the same frame
	if (frame_id == x->frame_id_save) return;
	x->frame_id_save = frame_id;
	
    /// output start frame bang /////////////////////////////////////////////
    outlet_bang(x->outlets[start_frame_out]);
    
    
    /// output frame info ///////////////////////////////////////////////////
	const Leap::HandList hands = frame.hands();
	const size_t numHands = hands.count();
	const Leap::ToolList tools = frame.tools();
	const size_t numTools = tools.count();
    const Leap::GestureList gestures = frame.gestures();
	const size_t numGestures = gestures.count();
	
	t_atom frame_data[5];
	atom_setlong(frame_data, frame_id);
	atom_setlong(frame_data+1, frame.timestamp());
	atom_setlong(frame_data+2, numHands);
	atom_setlong(frame_data+3, numTools);
    atom_setlong(frame_data+4, numGestures);
	outlet_anything(x->outlets[frame_out], j_sym_list, 5, frame_data);
    
	
    /// output hand info ////////////////////////////////////////////////////
	for (size_t i = 0; i < numHands; i++)
	{
        const Leap::Hand &hand = hands[i];
        t_atom hand_data[20];
		
        // id
		const int32_t hand_id = hand.id();
        
        atom_setlong(hand_data+0, hand_id);
        
        // palm position
        const Leap::Vector position = hand.palmPosition();
        const Leap::Vector direction = hand.direction();
        
        atom_setfloat(hand_data+1, position.x);
        atom_setfloat(hand_data+2, position.y);
        atom_setfloat(hand_data+3, position.z);
        atom_setfloat(hand_data+4, direction.x);
        atom_setfloat(hand_data+5, direction.y);
        atom_setfloat(hand_data+6, direction.z);
        
        // palm velocity
        const Leap::Vector velocity = hand.palmVelocity();
        
        atom_setfloat(hand_data+7, velocity.x);
        atom_setfloat(hand_data+8, velocity.y);
        atom_setfloat(hand_data+9, velocity.z);
        
        // palm normal
        const Leap::Vector normal = hand.palmNormal();
        
        atom_setfloat(hand_data+10, normal.x);
        atom_setfloat(hand_data+11, normal.y);
        atom_setfloat(hand_data+12, normal.z);
        
        // ball
        const Leap::Vector sphereCenter = hand.sphereCenter();
        const double sphereRadius = hand.sphereRadius();
        
        atom_setfloat(hand_data+13, sphereCenter.x);
        atom_setfloat(hand_data+14, sphereCenter.y);
        atom_setfloat(hand_data+15, sphereCenter.z);
        atom_setfloat(hand_data+16, sphereRadius);
        
        // misc
        const double pinch = hand.pinchStrength();
		const double grab = hand.grabStrength();
		const bool isLeft = hand.isLeft();
        
		atom_setfloat(hand_data+17, pinch);
		atom_setfloat(hand_data+18, grab);
		atom_setlong(hand_data+19, isLeft);
		
        outlet_anything(x->outlets[hand_out], j_sym_list, 20, hand_data);
        
        
        /// output finger info //////////////////////////////////////////////
		const Leap::FingerList &fingers = hand.fingers();
		const size_t numFingers = fingers.count();
		
		for (size_t j = 0; j < numFingers; j++)
		{
            const Leap::Finger &finger = fingers[j];
            t_atom finger_data[15];
            
			// ids
			const int32_t finger_id = finger.id();
            
            atom_setlong(finger_data+0, finger_id);
			atom_setlong(finger_data+1, hand_id);
            
            // position
			const Leap::Vector position = finger.tipPosition();
			
			atom_setfloat(finger_data+2, position.x);
			atom_setfloat(finger_data+3, position.y);
			atom_setfloat(finger_data+4, position.z);
            
            // direction
            const Leap::Vector direction = finger.direction();
            
			atom_setfloat(finger_data+5, direction.x);
			atom_setfloat(finger_data+6, direction.y);
			atom_setfloat(finger_data+7, direction.z);
            
            // velocity
            const Leap::Vector velocity = finger.tipVelocity();
            
			atom_setfloat(finger_data+8, velocity.x);
			atom_setfloat(finger_data+9, velocity.y);
			atom_setfloat(finger_data+10, velocity.z);
            
            // width and length
            const double width = finger.width();
			const double lenght = finger.length();
            
			atom_setfloat(finger_data+11, width);
			atom_setfloat(finger_data+12, lenght);
            
            // misc
			const bool isExtended = finger.isExtended();
			const int32_t type = finger.type();
            
			atom_setlong(finger_data+13, isExtended);
			atom_setlong(finger_data+14, type);
			
			outlet_anything(x->outlets[finger_out], j_sym_list, 15, finger_data);
		}
	}
    
    /// output tool info ///////////////////////////////////////////////////
    for (size_t i = 0; i < numTools; i++)
	{
        const Leap::Tool &tool = tools[i];
        t_atom tool_data[13];
        
        // id
        const int32_t tool_id = tool.id();
        
        atom_setlong(tool_data+0, tool_id);
        
        // position
        const Leap::Vector position = tool.tipPosition();
        
        atom_setfloat(tool_data+1, position.x);
        atom_setfloat(tool_data+2, position.y);
        atom_setfloat(tool_data+3, position.z);
        
        // direction
        const Leap::Vector direction = tool.direction();
        
        atom_setfloat(tool_data+4, direction.x);
        atom_setfloat(tool_data+5, direction.y);
        atom_setfloat(tool_data+6, direction.z);
        
        // velocity
        const Leap::Vector velocity = tool.tipVelocity();
        
        atom_setfloat(tool_data+7, velocity.x);
        atom_setfloat(tool_data+8, velocity.y);
        atom_setfloat(tool_data+9, velocity.z);
        
        // width and length
        const double width = tool.width();
        const double lenght = tool.length();
        
        atom_setfloat(tool_data+10, width);
        atom_setfloat(tool_data+11, lenght);
        
        // misc
        const bool isExtended = tool.isExtended();
        
        atom_setlong(tool_data+12, isExtended);
        
        outlet_anything(x->outlets[tool_out], j_sym_list, 13, tool_data);
    }
    
    /// output gesture info ////////////////////////////////////////////////
    for (size_t i = 0; i < numGestures; i++)
	{
        const Leap::Gesture &gesture = gestures[i];
       
        // depending on the type of the gesture
        switch (gesture.type()) {
            
            /// output circle info ////////////////////////////////////////////////
            case Leap::Gesture::TYPE_CIRCLE:
            {
                Leap::CircleGesture circle = gesture;
                
                t_atom circle_data[8];
                
                // type (as first data for routing)
                atom_setsym(circle_data+0, gensym("circle"));
                
                // id
                const int32_t circle_id = gesture.id();
                
                atom_setlong(circle_data+1, circle_id);
                
                // state
                const int32_t circle_state = gesture.state();
                
                atom_setsym(circle_data+2, x->stateNames[circle_state]);
                
                // progress
                const double progress = circle.progress();
                
                atom_setfloat(circle_data+3, progress);
                
                // radius
                const double radius = circle.radius();
                
                atom_setfloat(circle_data+4, radius);
                
                // angle swept since last frame
                float sweptAngle = 0;
                if (circle.state() != Leap::Gesture::STATE_START)
                {
                    Leap::CircleGesture previousUpdate = Leap::CircleGesture(x->leap->frame(1).gesture(circle.id()));
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * M_PI;
                }
                atom_setfloat(circle_data+5, sweptAngle);
                
                // clockwiseness
                const bool clockwiseness = circle.pointable().direction().angleTo(circle.normal()) <= M_PI/2;
                
                atom_setlong(circle_data+6, clockwiseness ? 1 : 0);
                
                outlet_anything(x->outlets[gesture_out], j_sym_list, 7, circle_data);
                break;
            }
                
            /// output swipe info ////////////////////////////////////////////////
            case Leap::Gesture::TYPE_SWIPE:
            {
                Leap::SwipeGesture swipe = gesture;
                
                t_atom swipe_data[7];
                
                // type (as first data for routing)
                atom_setsym(swipe_data+0, gensym("swipe"));
                
                // id
                const int32_t swipe_id = gesture.id();
                
                atom_setlong(swipe_data+1, swipe_id);
                
                // state
                const int32_t swipe_state = gesture.state();
                
                atom_setsym(swipe_data+2, x->stateNames[swipe_state]);
                
                // direction
                const Leap::Vector direction = swipe.direction();
                
                atom_setfloat(swipe_data+3, direction.x);
                atom_setfloat(swipe_data+4, direction.y);
                atom_setfloat(swipe_data+5, direction.z);
                
                // speed
                const double speed = swipe.speed();
                
                atom_setfloat(swipe_data+6, speed);
                
                outlet_anything(x->outlets[gesture_out], j_sym_list, 7, swipe_data);
                break;
            }
                
            /// output key tap info ////////////////////////////////////////////////
            case Leap::Gesture::TYPE_KEY_TAP:
            {
                Leap::KeyTapGesture key_tap = gesture;
                
                t_atom key_tap_data[9];
                
                // type (as first data for routing)
                atom_setsym(key_tap_data+0, gensym("key_tap"));
                
                // id
                const int32_t key_tap_id = gesture.id();
                
                atom_setlong(key_tap_data+1, key_tap_id);
                
                // state
                const int32_t key_tap_state = gesture.state();
                
                atom_setsym(key_tap_data+2, x->stateNames[key_tap_state]);
                
                // position
                const Leap::Vector position = key_tap.position();
                
                atom_setfloat(key_tap_data+3, position.x);
                atom_setfloat(key_tap_data+4, position.y);
                atom_setfloat(key_tap_data+5, position.z);
                
                // direction
                const Leap::Vector direction = key_tap.direction();
                
                atom_setfloat(key_tap_data+6, direction.x);
                atom_setfloat(key_tap_data+7, direction.y);
                atom_setfloat(key_tap_data+8, direction.z);
                
                outlet_anything(x->outlets[gesture_out], j_sym_list, 9, key_tap_data);
                break;
            }
                
            /// output screen tap info ////////////////////////////////////////////////
            case Leap::Gesture::TYPE_SCREEN_TAP:
            {
                Leap::ScreenTapGesture screen_tap = gesture;
                
                t_atom screen_tap_data[9];
                
                // type (as first data for routing)
                atom_setsym(screen_tap_data+0, gensym("screen_tap"));
                
                // id
                const int32_t screen_tap_id = gesture.id();
                
                atom_setlong(screen_tap_data+1, screen_tap_id);
                
                // state
                const int32_t screen_tap_state = gesture.state();
                
                atom_setsym(screen_tap_data+2, x->stateNames[screen_tap_state]);
                
                // position
                const Leap::Vector position = screen_tap.position();
                
                atom_setfloat(screen_tap_data+3, position.x);
                atom_setfloat(screen_tap_data+4, position.y);
                atom_setfloat(screen_tap_data+5, position.z);
                
                // direction
                const Leap::Vector direction = screen_tap.direction();
                
                atom_setfloat(screen_tap_data+6, direction.x);
                atom_setfloat(screen_tap_data+7, direction.y);
                atom_setfloat(screen_tap_data+8, direction.z);
                
                outlet_anything(x->outlets[gesture_out], j_sym_list, 9, screen_tap_data);
                
                break;
            }
            default:
                object_error((t_object*)x, "unknown gesture type");
                break;
        }
    }
	
     /// output end frame bang /////////////////////////////////////////////
	outlet_bang(x->outlets[end_frame_out]);
}