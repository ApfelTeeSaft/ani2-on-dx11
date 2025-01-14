
#include <dsound.h>
#include "sos.h"
#include "macros.h"
#include "bootsnd.h"
#include "pitches.h"

#ifdef STARTUPANIMATION

#pragma data_seg("INIT_RW")
#pragma code_seg("INIT")
#pragma bss_seg("INIT_RW")
#pragma const_seg("INIT_RD")

// Tell linker to put bootsound code and data into INIT section
#pragma comment(linker, "/merge:DSOUND=INIT")

#pragma comment(linker, "/merge:INIT_RD=INIT")
#pragma comment(linker, "/merge:INIT_RW=INIT")

#endif //STARTUPANIMATION

DSENVELOPEDESC	Env1a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x5,						// hold
	0x20,						// decay
	0x0,						// release
	0x7f,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	Env1m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x0,						// hold
	0x10,						// decay
	0x0,						// release
	0x1f,						// sustain
	0x10,						// pitch scale
	0x7f,						// filter scale
};




DSENVELOPEDESC	Env2a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x0,						// hold
	0x40,						// decay
	0x0,						// release
	0x3f,						// sustain
	0x7f,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	Env2m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x100,						// attack
	0x0,						// hold
	0x10,						// decay
	0x0,						// release
	0x1f,						// sustain
	0x40,						// pitch scale
	0x4f,						// filter scale
};
	

DSENVELOPEDESC	Env3a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x3,						// hold
	0x10,						// decay
	0x20,						// release
	0x10,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	Env3m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x0,						// hold
	0x10,						// decay
	0x0,						// release
	0x1f,						// sustain
	0x10,						// pitch scale
	0x1f,						// filter scale
};



DSENVELOPEDESC	OpenEnva = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x0,						// attack
	0x0,						// hold
	0x0,						// decay
	0x0,						// release
	0xff,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	OpenEnvm = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0,						// attack
	0x0,						// hold
	0x00,						// decay
	0x00,						// release
	0xff,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

extern	unsigned short  Sin128[];
extern	unsigned short	Saw128[];
extern	unsigned short	Noise8192[];
extern	unsigned short	FM32768[];
extern	unsigned short	Glock[];

extern	unsigned short	Bubble[];
extern	unsigned short	ThunEl16[];
extern	unsigned short	ReverseThunEl16[];


//
// patch is table, loop, env1, env2
// so, patches are defined by a waveform, a length, a loop address
// and the pair of envelopes.
//


const struct DSPpatch PatchSin = {
	Sin128,			// start addr
	256,			// length
	0,				// loop start
	256,			// loop end
	1,				// loop sound
	&Env1a,			// ampitude envelope
	&Env1m			// multi purpose envelope
};

///////////////////////////////////////////////////////////////////////
DSENVELOPEDESC	SawEnv1a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x2,						// hold
	0x10,						// decay
	0x0,						// release
	0x9f,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	SawEnv1m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x10,						// attack
	0x100,						// hold
	0x100,						// decay
	0x80,						// release
	0xff,						// sustain
	0x00,						// pitch scale
	-80,						// filter scale
};


const struct DSPpatch PatchSaw1 = {
	Saw128,			// start addr
	256,				// length
	0,				// loop start
	256,			// loop end
	1,				// loop sound
	&SawEnv1a,			// ampitude envelope
	&SawEnv1m			// multi purpose envelope
};




///////////////////////////////////////////////////////////////////////

DSENVELOPEDESC	SawEnv2a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x0,						// hold
	0x40,						// decay
	0x0,						// release
	0x3f,						// sustain
	0x7f,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	SawEnv2m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x100,						// attack
	0x0,						// hold
	0x10,						// decay
	0x0,						// release
	0x1f,						// sustain
	0x00,						// pitch scale
	0x0,						// filter scale
};

const struct DSPpatch PatchSaw2 = {
	Saw128,			// start addr
	256,				// length
	0,				// loop start
	256,			// loop end
	1,				// loop sound
	&SawEnv2a,			// ampitude envelope
	&SawEnv2m			// multi purpose envelope
};



	

///////////////////////////////////////////////////////////////////////
const struct DSPpatch PatchSaw3 = {
	Saw128,			// start addr
	256,			// length
	0,				// loop start
	256,			// loop end
	1,				// loop sound
	&Env3a,			// ampitude envelope
	&Env3m			// multi purpose envelope
};



const struct DSPpatch PatchSquare = {
	Saw128,			// start addr
	256,			// length
	0,				// loop start
	256,			// loop end
	1,				// loop sound
	&Env3a,			// ampitude envelope
	&Env3m			// multi purpose envelope
};


/////////////////////////////////////////////////////////////////////
DSENVELOPEDESC	NoiseEnv1a = {
	DSEG_AMPLITUDE,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x1,						// attack
	0x3,						// hold
	0x10,						// decay
	0x20,						// release
	0xff,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

DSENVELOPEDESC	NoiseEnv1m = {
	DSEG_MULTI,				// EG
	DSEG_MODE_DELAY,			// mode
	0,							// delay
	0x100,						// attack
	0x0,						// hold
	0x30,						// decay
	0xc0,						// release
	0xff,						// sustain
	0x0,						// pitch scale
	0x0,						// filter scale
};

/////////////////////////////////////////////////////////////////////
const struct DSPpatch PatchEnvNoise1 = {
	Noise8192,			// start addr
	8192*2,			// length
	0,				// loop start
	8192*2,			// loop end
	1,				// loop sound
	&NoiseEnv1a,			// ampitude envelope
	&NoiseEnv1m			// multi purpose envelope
};


//////////////////////////////
const struct DSPpatch PatchFM = {
	FM32768,			// start addr
	32768,			// length
	0,				// loop start
	32768,			// loop end
	0,				// loop sound
	&OpenEnva,			// ampitude envelope
	&OpenEnvm			// multi purpose envelope
};
///////////////////////////////////////////////////////////////
const struct DSPpatch PatchGlock = {
	Glock,			// start addr
	3768*2,			// length...from glock.equ
	0,				// loop start
	3768*2,			// loop end
	0,				// Don't loop this sound
	&OpenEnva,			// ampitude envelope
	&OpenEnvm			// multi purpose envelope
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
const struct DSPpatch PatchThunEl16 = {
	ThunEl16,			// start addr
	0x5540*2,			// length...from glock.equ
	0,				// loop start
	0x5540*2,			// loop end
	0,				// Don't loop this sound
	&OpenEnva,			// ampitude envelope
	&OpenEnvm			// multi purpose envelope
};
///////////////////////////////////////////////////////////////
const struct DSPpatch PatchRevThun = {
	ReverseThunEl16,			// start addr
	0x5540*2,			// length...from glock.equ
	0,				// loop start
	0x5540*2,			// loop end
	0,				// Don't loop this sound
	&OpenEnva,			// ampitude envelope
	&OpenEnvm			// multi purpose envelope
};
///////////////////////////////////////////////////////////////

const struct DSPpatch PatchBubble = {
	Bubble,			// start addr
	6719*2,			// length...from glock.equ
	0,				// loop start
	6719*2,			// loop end
	1,				//  loop this sound
	&OpenEnva,			// ampitude envelope
	&OpenEnvm			// multi purpose envelope
};


#define	PSIN1		0
#define	PSAW1		1
#define	PSQUARE		2
#define	PSAW2		3
#define	PSAW3		4
#define	PNOISE1		5
#define	PGLOCK		6
#define	PBUBBLE		7
#define	PFM		8
#define	PTHUNEL16 	9
#define	PREVTHUN	10

struct DSPpatch	const *Patches[] = {
	&PatchSin,			// patch 0
	&PatchSaw1,			// patch 1
	&PatchSquare,		// patch 2
	&PatchSaw2,			// patch 3
	&PatchSaw3,			// patch 4
	&PatchEnvNoise1,	// patch 5
	&PatchGlock,		// patch 6
	&PatchBubble,		// 7
	&PatchFM,		// 8
	&PatchThunEl16,		//9
	&PatchRevThun		// 10
};



// throbbing bass
//
const unsigned short Boot0[] = {
	fset(29000,26000),
	patch(PSAW1),
	volume(10),
	rest(194-50),
	note(dd1,130),
	rest(2),
	note(dd1,125),
	rest(3),
	note(dd1,70),
	rest(2),
	note(dd1,77),
	rest(3),
	note(dd1,97),
	rest(3),
	note(dd1,91),
	rest(3),
	note(dd1,47),
	rest(3),
	note(dd1,51),
	rest(3),
	note(dd1,57),
	rest(3),
	note(dd1,132),
	rest(0),





//	rest(194),
//	fset(29000,26000),
//	patch(PSAW1),
//	volume(10),
//	loop(13),
//
//		note(dd1,57),
//		rest(3),
//	endloop,
//	note(dd1,28),
//	rest(2),
//	loop(6),
//		volume(15),
//		note(dd1,27),
//		rest(3),
//	endloop,
//
	loop(255),
	rest(20000),
	endloop
};






//noise whoosh for opening
const unsigned short Boot1[] = {
	fset(1000,26000),
	patch(5),
	volume(43),
	note(cc2,1),
	loop(10),
		finc(3000,26000),
		slur(cc2,13),
	endloop,
	ring(4),
	volume(20),
	loop(60),
		volume(1),
		finc(-1000,26000),
		slur(cc2,15),
	endloop,

	loop(255),
	rest(2000),
	endloop
};

// bubbling sound
const unsigned short Boot2[] = {
	patch(PBUBBLE),
	volume(12),
	fset(6000,26000),
	rest(134),
	note(dd2,412+60),
	loop(20),
		ring(20),
		finc(1000,26000),
	endloop,
	loop(30),
		ring(10),
		volume(4),
	endloop,
	loop(255),
		rest(20000),
	endloop
};


// first and last flashes
const unsigned short Boot3[] = {
	patch(PTHUNEL16),
	volume(40),
	fset(32767,26000),
	rest(134),
	note(dd3,820),
	volume(-25),
	note(dd3,200),
	loop(20),
		ring(20),
		volume(2),
		finc(-1000,26000),
	endloop,
	ring(10000),
	loop(255),
		rest(2000),
	endloop
};

// fast noisy stuff...
//
const unsigned short Boot4[] = {
	rest(194),
	fset(32000,26000),
	patch(PNOISE1),
	volume(160),
	loop(8),
		volume(-3),
		note(cc4,15),
		rest(5),
		note(ff4,15),
		volume(-3),
		rest(5),
		note(gg5,15),
		rest(5),
		volume(-3),
		note(ff4,15),
		rest(5),
	endloop,
	loop(3),
		xpose(0x100),
		volume(-3),
		note(cc4,10),
		rest(5),
		note(ff4,10),
		volume(-3),
		rest(5),
		note(gg5,10),
		rest(5),
		volume(-3),
		note(ff4,10),
		rest(5),
	endloop,


	note(ff4,10),

	loop(255),
	rest(20000),
	endloop
};

// glocks....
const unsigned short Boot5[] = {
	patch(PGLOCK),
	volume(55),
	rest(1114),
	fset(32000,26000),
	
	note(as2,20),
	note(ff2,20),
	note(as1,20),

	volume(10),	
	loop(6),

		note(as2,18),
		rest(2),

		volume(30),
		note(as2,18),
		rest(2),

		volume(20),

		note(as2,18),
		rest(2),
		
		volume(-35),

		finc(-2500,26000),
	endloop,

	loop(255),
	rest(20000),
	endloop
};

// glocks....
const unsigned short Boot6[] = {




//	patch(PGLOCK),
//	volume(10),
//	loop(100),
//		note(cc3,55),
//		rest(5),
//	endloop,
//	rest(1194),
//	fset(32000,26000),

//	note(cc3,40),
//	rest(1),
//	note(ff3,40),
//	rest(1),
//	note(cc3,40),
//	rest(1),
//	note(gg3,400),
//	rest(1),

//	loop(40),
//		note(cc3,18),
//		rest(2),
//		volume(2),
//	endloop,
//	note(cc3,500),
	loop(255),
	rest(20000),
	endloop
};

/////
// glocks....
const unsigned short Boot8[] = {
	patch(PGLOCK),
	volume(100),
	rest(1194),
	fset(32000,26000),
//	note(cc2,40),
//	rest(1),
//	note(ff2,40),
//	rest(1),
//	note(cc2,40),
//	rest(1),
//	note(gg2,400),
//	rest(1),



//	loop(40),
//		note(gg2,18),
//		rest(2),
//		volume(2),
//	endloop,
//	note(cc3,500),

	loop(255),
	rest(20000),
	endloop
};


// bass beg....
const unsigned short Boot7[] = {
	patch(PSAW2),
	xpose(0x60),
	volume(18),
	fset(1000,26000),
	note(dd2,1),
	loop(19),
		slur(dd2,10),
		finc(1500,2600),
	endloop,
	rest(800),
	note(dd2,1),
	loop(30),
		slur(dd2,10),
		finc(-750,2600),
	endloop,
	loop(30),
		slur(dd2,5),
		volume(3),
	endloop,


	loop(255),
	rest(2000),
	endloop
};

////////////////////////////////////////////////////////
// Boot track 9:  Pulses in sync with blob

const unsigned short Boot9[] = {
	patch(PTHUNEL16),
	volume(50),
	rest(326-50),
	note(aa2,280),
	rest(0),
	volume(-15),
	note(aa2,244),
	rest(0),
	volume(-15),
	note(aa2,500),

	loop(255),
	rest(2000),
	endloop
};

////////////////////////////////////////////////////////
// Boot track A:  Pulses in sync with blob

const unsigned short BootA[] = {
	patch(PTHUNEL16),
	volume(50),
	rest(454-50),
	note(aa2,252),
	rest(0),
	note(aa2,194),
	rest(0),
	volume(-20),
	note(aa2,200),


	loop(255),
	rest(2000),
	endloop
};

////////////////////////////////////////////////////////
// Boot track B:  Pulses in sync with blob

const unsigned short BootB[] = {
	patch(PTHUNEL16),
	volume(50),
	rest(526-50),
	note(aa2,274),
	rest(0),
	note(aa2,154),
	rest(0),
	volume(-20),
	note(aa2,200),

	loop(255),
	rest(2000),
	endloop
};



const unsigned short *Boot[] = {
	Boot0,Boot1,Boot2,Boot3,Boot4,Boot5,Boot6,Boot7,Boot8,Boot9,BootA,BootB
};
/////////////////////////////////////////////////////////////////////////

const unsigned short Tune0[] = {

	patch(PREVTHUN),
	note(cc3,800),
	patch(PTHUNEL16),
	note(cc3,800),

	rest(200),
	patch(PSIN1),

	loop(5),
	loop(120),
		note(cc2,50),
		xpose(200),
		rest(0),
	endloop,
	loop(120),
		note(cc0,50),
		xpose(-200),
		rest(0),
	endloop,
	endloop,
	note(5,6),
	rest(0x10),
	
//	0,1,2,3,4
};

const unsigned short Tune1[] = {

//	fset(6000,26000),
	patch(PSIN1),

	note(cc3,200),

};

const unsigned short *Tune[] = {
	Tune0
};

const struct sound sound_calls[] = {
	0,0,0,NULL,				//0 
	1,0,0xfff,Boot,			//1
	1,0,0x001,Tune,			//2
	1,0,0x001,Tune,			//2
};


struct sound sound_calls2[2];

int	max_sound_call = 0x2;
int	default_clock_value = 80;
