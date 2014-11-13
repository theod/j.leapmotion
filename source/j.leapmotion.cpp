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
	t_object	ob;
	int64_t		frame_id_save;
	void		*start_frame_outlet;
    void		*frame_outlet;
    void		*hand_outlet;
    void		*finger_outlet;
    void		*tool_outlet;
    void		*gesture_outlet;
    void		*end_frame_outlet;
	Leap::Controller	*leap;
} t_leapmotion;

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
        
        // Make several outlets
        x->start_frame_outlet = outlet_new(x, NULL);        // start_frame bang outlet
        x->frame_outlet = outlet_new(x, NULL);              // frame_out anything outlet
        x->hand_outlet = outlet_new(x, NULL);               // hand_out anything outlet
        x->finger_outlet = outlet_new(x, NULL);             // finger_out anything outlet
        x->tool_outlet = outlet_new(x, NULL);               // tool_out anything outlet
        x->gesture_outlet = outlet_new(x, NULL);            // gesture_out anything outlet
        x->end_frame_outlet = outlet_new(x, NULL);          // end_frame bang outlet
        
        // Create a controller
        x->leap = new Leap::Controller;
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
	const Leap::Frame frame = x->leap->frame();
	const int64_t frame_id = frame.id();
	
	// ignore the same frame
	if (frame_id == x->frame_id_save) return;
	x->frame_id_save = frame_id;
	
    /// output start frame bang /////////////////////////////////////////////
    outlet_bang(x->start_frame_outlet);
    
    
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
	outlet_anything(x->frame_outlet, _sym_list, 5, frame_data);
    
	
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
		
        outlet_anything(x->hand_outlet, _sym_list, 20, hand_data);
        
        
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
			
			outlet_anything(x->finger_outlet, _sym_list, 15, finger_data);
		}
	}
    
    /// output tool info ///////////////////////////////////////////////////
    for (size_t i = 0; i < numTools; i++)
	{
        const Leap::Tool &tool = tools[i];
        t_atom tool_data[20];
        
        outlet_anything(x->tool_outlet, _sym_list, 20, tool_data);
    }
    
    /// output gesture info ////////////////////////////////////////////////
    for (size_t i = 0; i < numGestures; i++)
	{
        const Leap::Gesture &gesture = gestures[i];
        t_atom gesture_data[20];
        
        outlet_anything(x->gesture_outlet, _sym_list, 20, gesture_data);
    }
	
     /// output end frame bang /////////////////////////////////////////////
	outlet_anything(x->end_frame_outlet, _sym_bang, 0, NULL);
}