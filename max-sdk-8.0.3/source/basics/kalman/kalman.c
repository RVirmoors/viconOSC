/**
	@file
	kalman - extended kalman filter

	@ingroup	examples
*/

// positioning interval
//static const double T = 1;

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "tinyekf_cfg.h"
#include "tiny_ekf.h"



static void blkfill(ekf_t * ekf, const double * a, int off)
{
	off *= 2;

	ekf->Q[off][off] = a[0];
	ekf->Q[off][off + 1] = a[1];
	ekf->Q[off + 1][off] = a[2];
	ekf->Q[off + 1][off + 1] = a[3];
}


static void init(ekf_t * ekf)
{

	// initial covariances of state noise, measurement noise, process noise
	double P0 = 1;
	double R0 = 1;
	double Q0 = 0.05;

	int i, j;

	for (i = 0; i < 3; ++i) {
		ekf->P[i][i] = P0;
		ekf->Q[i][i] = Q0;
	}

	for (i = 0; i < 9; ++i)
		ekf->R[i][i] = R0;

	// position
	ekf->x[0] = 0;
	ekf->x[1] = 0;
	ekf->x[2] = 0;
	// acc (body)
	ekf->xB[0] = 0;
	ekf->xB[1] = 0;
	ekf->xB[2] = 0;
}

static void model(ekf_t * ekf, double SV[Mobs], double SV_Meas[Nsta])
{
	int i, j;
	double a[3];

	// state model
//	for (i = 0; i < Nsta; i++) {
//		ekf->x[i] = ekf->x[i] + ekf->Q[i][i];
//	}

	// meas model
	for (i = 0; i < Nsta; i++) {
		SV_Meas[i] = SV[i];// +ekf->R[i][i];	// world meas
		ekf->xB[i] = SV[i + 3];// +ekf->R[i + 3][i + 3]; // body meas
		a[i] = SV[i + 6];// +ekf->R[i + 6][i + 6]; // angular rate meas
		ekf->o[i] += a[i] * T;			// body orientation
	}

	// body - world transf matrix
	ekf->Rbw[0][0] = cos(ekf->o[1])*cos(ekf->o[2]);
	ekf->Rbw[0][1] = cos(ekf->o[1])*sin(ekf->o[2]);
	ekf->Rbw[0][2] = -sin(ekf->o[1]);
	ekf->Rbw[1][0] = sin(ekf->o[0])*sin(ekf->o[1])*cos(ekf->o[2])-
					 sin(ekf->o[0])*sin(ekf->o[2]);
	ekf->Rbw[1][1] = sin(ekf->o[0])*sin(ekf->o[1])*sin(ekf->o[2]) -
					 cos(ekf->o[0])*cos(ekf->o[2]);
	ekf->Rbw[1][2] = sin(ekf->o[0])*cos(ekf->o[1]);
	ekf->Rbw[2][0] = cos(ekf->o[0])*sin(ekf->o[1])*cos(ekf->o[2]) +
					 sin(ekf->o[0])*sin(ekf->o[2]);
	ekf->Rbw[2][1] = cos(ekf->o[0])*sin(ekf->o[1])*sin(ekf->o[2]) -
					 sin(ekf->o[0])*cos(ekf->o[2]);
	ekf->Rbw[2][2] = cos(ekf->o[0])*cos(ekf->o[1]);
	
	for (j = 0; j < Nsta; j++) {
		ekf->fx[j] = ekf->x[j];
	}

	for (j = 0; j < Nsta; ++j)
		ekf->F[j][j] = 1;

	for (i = 0; i < Mobs; ++i) {
		ekf->hx[i] = 0;
	}

	for (i = 0; i < Nsta; ++i) {
		ekf->H[i][i] = 1;
	}
}

////////////////////////// object struct
typedef struct _kalman
{
	t_object					ob;			// the object itself (must be first)
	void *m_outlet1;
} t_kalman;

///////////////////////// function prototypes
//// standard set
void *kalman_new(t_symbol *s, long argc, t_atom *argv);
void kalman_free(t_kalman *x);
void kalman_assist(t_kalman *x, void *b, long m, long a, char *s);
void kalman_input(t_symbol *s, long argc, t_atom *argv);

//////////////////////// global class pointer variable
void *kalman_class;

ekf_t ekf;
// Make a place to store the data from the inlet and the output of the EKF
double SV_Pos[Mobs];
double SV_Meas[Mobs]; 

void ext_main(void *r)
{
	t_class *c;

	c = class_new("kalman", (method)kalman_new, (method)kalman_free, (long)sizeof(t_kalman),
				  0L /* leave NULL!! */, A_GIMME, 0);

	/* you CAN'T call this from the patcher */
	class_addmethod(c, (method)kalman_assist,			"assist",		A_CANT, 0);
	class_addmethod(c, (method)kalman_input,			"pos",		A_GIMME, 0);

	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	kalman_class = c;

	post("Initializing EKF object");

	// Do generic EKF initialization

	ekf_init(&ekf, Nsta, Mobs);

	// Do local initialization
	init(&ekf);
}

void kalman_assist(t_kalman *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "I am inlet %ld", a);
	}
	else {	// outlet
		sprintf(s, "I am outlet %ld", a);
	}
}

void kalman_free(t_kalman *x)
{
	;
}

void kalman_input(t_kalman *x, t_symbol *s, long argc, t_atom *argv) {
	if (argc != (Mobs)) {
		object_error((t_object *)x, "wrong number of params! Need %d observations.", Mobs);
	}
	else {
		// get position measurements
		for (int i = 0; i < Mobs; i++) {
			SV_Pos[i] = atom_getfloat(&argv[i]);
		}

		model(&ekf, SV_Pos, SV_Meas);  // sv_pos: vicon, acc, gyro

		post("input is %f %f", SV_Pos[0], SV_Meas[0]);
		ekf_step(&ekf, SV_Meas);  // sv_rho is the output of the meas. model

		t_atom result[3];
		// get result from EKF output
		for (int k = 0; k < 3; ++k)
			atom_setfloat(&result[k], ekf.x[k]);
		// print result to outlet
		outlet_list(x->m_outlet1, 0L, 3, result);
		post("error is %f %f", ekf.P[0][0], ekf.P[1][1]);
	}
}


void *kalman_new(t_symbol *s, long argc, t_atom *argv)
{
	t_kalman *x = NULL;
	long i;

	if ((x = (t_kalman *)object_alloc(kalman_class))) {
	//	object_post((t_object *)x, "a new %s object was instantiated: %p", s->s_name, x);
	//	object_post((t_object *)x, "it has %ld arguments", argc);

		for (i = 0; i < argc; i++) {
			if ((argv + i)->a_type == A_FLOAT) {
				object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
			} else {
				object_error((t_object *)x, "forbidden argument");
			}
		}
	}

	x->m_outlet1 = listout((t_object *)x);
	return (x);
}


