// equations from https://www.researchgate.net/profile/Zheyao_Wang/publication/254057855_Motion_Measurement_Using_Inertial_Sensors_Ultrasonic_Sensors_and_Magnetometers_With_Extended_Kalman_Filter_for_Data_Fusion/links/55dfa37008aede0b572b912e.pdf

inlets = 3 // acc, w, ypr
outlets = 3 // S, V, a

var g = 0.98
var deltaT = 0.1;
var acc = [0,0,0], w = [0,0,0], V = [0,0,0], ypr = [0,0,0], a = [0,0,0], S = [0,0,0];
var axHist = [];

if (jsarguments.length>1)
	g = jsarguments[1];
	
function bang()
{
	post("g is", g, "\n");
	acc = [0,0,0];
 	w = [0,0,0];
 	V = [0,0,0];
 	ypr = [0,0,0];
 	a = [0,0,0];
 	S = [0,0,0];
}

function list()
{
	var l = arrayfromargs(arguments);
	for (var i = 0; i < 3; i++) {
		if (inlet == 0)
			acc[i] = l[i];
		if (inlet == 1)
			w[i] = l[i];
		if (inlet == 2)
			ypr[i] = l[i];
	}
	ec14();
	ec13();
	outlet(0, S);
	outlet(1, V);
	outlet(2, a);
}

function ec14() {	// update V and S
	for (var i = 0; i < 3; i++) {
		V[i] += a[i] * deltaT;
		S[i] += V[i] * deltaT;
	}
}

function ec13() {
	a[0] = acc[0] - w[1]*V[2] + w[2]*V[1] + g*Math.sin(ypr[1]);

//	a[1] = acc[1] + w[0]*V[2] - w[2]*V[0] - g*Math.sin(ypr[2])*Math.cos(ypr[1]);
//	a[2] = acc[2] + w[0]*V[1] - w[1]*V[0] - g*Math.cos(ypr[2])*Math.cos(ypr[1]);
}